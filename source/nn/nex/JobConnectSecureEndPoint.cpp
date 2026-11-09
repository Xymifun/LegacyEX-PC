#include "nn/nex/JobConnectSecureEndPoint.hpp"

#include "nn/nex/AuthenticationClient.hpp"
#include "nn/nex/BitStream.hpp"
#include "nn/nex/ConnectionManager.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/JobConnectEndPoint.hpp"
#include "nn/nex/KerberosAuthentication.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SecureConnectionClient.hpp"
#include "nn/nex/SecureEndPoint.hpp"
#include "nn/nex/SecureStream.hpp"
#include "nn/nex/StreamManager.hpp"
#include "nn/nex/Ticket.hpp"
#include "nn/nex/TicketManager.hpp"

void ClientRequestCallback(nn::nex::CallContext* pCallContext, const nn::nex::UserContext* pUserContext)
{
    nn::nex::Job* pJob = static_cast<nn::nex::Job*>(pUserContext->GetPointer());
    nn::nex::ScopedCS lock(&nn::nex::Scheduler::GetInstance()->GetCriticalSection());
    if (pJob->GetState() == nn::nex::Job::Suspended)
        pJob->Resume();
}

namespace nn
{
namespace nex
{

namespace
{

enum ConnectResult
{
    CONNECT_SUCCEEDED_RESULT = 0x10001,
    CONNECT_CALL_FAILED_RESULT = static_cast<int>(0x8001000Du),
    CONNECT_FAILED_RESULT = static_cast<int>(0x80050001u),
    CONNECT_INVALID_URL_RESULT = static_cast<int>(0x80050003u),
    CONNECT_INVALID_RESPONSE_RESULT = static_cast<int>(0x8005000Au)
};

}

JobConnectSecureEndPoint::JobConnectSecureEndPoint(SecureEndPoint* pSecureEndPoint, const StationURL* pStationURL,
                                                   Buffer* pRequestData, Buffer* pResponseData,
                                                   EndPoint::ConnectionCallback pCallback,
                                                   const UserContext& rUserContext, unsigned int uiTimeout)
    : Job(JOB_TYPE_2, DebugString(L"")),
      m_pSecureEndPoint(pSecureEndPoint),
      m_stationURL(*pStationURL),
      m_pResponseData(pResponseData),
      m_pCallback(pCallback),
      m_userContext(rUserContext),
      m_pConnectJob(0)
{
    m_uiStep = STEP_PARSE_URL;
    m_deadline = Time::ConvertTimeoutToDeadline(uiTimeout);
    m_result = qResult(static_cast<int>(CONNECT_SUCCEEDED_RESULT));
    m_pEndPoint = 0;
    m_uiPrincipalID = 0;
    m_uiConnectionID = 0;
    m_pTicket = 0;
    m_pResponseStream = 0;
    m_pRequestStream = 0;

    if (m_userContext.GetPointer() != 0)
        static_cast<JobConnectEndPoint*>(m_userContext.GetPointer())->m_pCancelTarget = this;
}

JobConnectSecureEndPoint::~JobConnectSecureEndPoint()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    if (m_pConnectJob != 0)
    {
        m_pConnectJob->ReleaseRef();
        m_pConnectJob = 0;
    }

    if (m_pTicket != 0)
    {
        SecureStream* pStream = static_cast<SecureStream*>(m_pSecureEndPoint->GetStream());
        pStream->GetAuthenticationClient()->m_pTicketManager->ReleaseTicket(m_pTicket);
        m_pTicket = 0;
    }

    if (m_pRequestStream != 0)
    {
        delete m_pRequestStream;
        m_pRequestStream = 0;
    }

    if (m_pResponseStream != 0)
    {
        delete m_pResponseStream;
        m_pResponseStream = 0;
    }
}

void JobConnectSecureEndPoint::ParseURL()
{
    m_uiConnectionID = m_stationURL.GetConnectionID();
    m_uiPrincipalID = m_stationURL.GetPrincipalID();

    bool bHasAddress = false;
    if (m_stationURL.GetAddress().GetLength() > 0 && m_stationURL.GetPortNumber() > 0 && m_stationURL.GetStreamID() > 0)
    {
        bHasAddress = true;
    }

    if (bHasAddress)
    {
        StationURL url;
        url.SetInetAddress(m_stationURL.GetInetAddress());
        url.SetStreamID(m_stationURL.GetStreamID());

        ConnectionData connectionData;
        connectionData.m_StationURL = url;
        connectionData.m_uiValue = m_uiConnectionID;
        m_connectionData.push_back(connectionData);
    }

    if (m_uiPrincipalID == 0 && m_uiConnectionID == 0)
    {
        m_result = qResult(static_cast<int>(CONNECT_INVALID_URL_RESULT));
        m_uiStep = STEP_COMPLETE;
        SetToComplete();
    }
}

bool JobConnectSecureEndPoint::ReadyToPerformConnect()
{
    if (m_uiPrincipalID == 0 || m_connectionData.empty() || m_pTicket == 0)
        return false;

    return true;
}

void JobConnectSecureEndPoint::CheckConnectionData()
{
    unsigned int uiStep = STEP_REQUEST_CONNECTION_DATA;
    if (ReadyToPerformConnect())
        uiStep = STEP_PERFORM_CONNECT;
    m_uiStep = uiStep;
}

void JobConnectSecureEndPoint::RequestConnectionData()
{
    SecureStream* pStream = static_cast<SecureStream*>(m_pSecureEndPoint->GetStream());
    unsigned int uiSourcePrincipalID = pStream->m_pCredentials->GetPrincipalID();

    if (m_uiPrincipalID != 0 && m_pTicket == 0 && m_ticketContext.GetState() == CallContext::Ready)
    {
        m_ticketContext.RegisterCompletionCallback(ClientRequestCallback, UserContext(this), true);
        if (!pStream->GetAuthenticationClient()->m_pTicketManager->AcquireTicket(&m_ticketContext, uiSourcePrincipalID,
                                                                                 m_uiPrincipalID, &m_pTicket))
        {
            m_result = qResult(static_cast<int>(CONNECT_CALL_FAILED_RESULT));
            m_uiStep = STEP_COMPLETE;
            SetToComplete();
            return;
        }
    }

    if (m_connectionData.empty() && m_connectionDataContext.GetState() == CallContext::Ready)
    {
        m_connectionDataContext.RegisterCompletionCallback(ClientRequestCallback, UserContext(this), true);
        if (!pStream->GetSecureConnectionClient()->RequestConnectionData(&m_connectionDataContext, m_uiConnectionID,
                                                                         m_uiPrincipalID, &m_connectionData))
        {
            m_result = qResult(static_cast<int>(CONNECT_CALL_FAILED_RESULT));
            m_uiStep = STEP_COMPLETE;
            SetToComplete();
            return;
        }
    }

    if (m_ticketContext.GetState() == CallContext::CallInProgress
        || m_connectionDataContext.GetState() == CallContext::CallInProgress)
    {
        m_uiStep = STEP_CHECK_CONNECTION_DATA;
        SetToSuspended();
    }
    else
    {
        m_result = qResult(static_cast<int>(CONNECT_CALL_FAILED_RESULT));
        m_uiStep = STEP_COMPLETE;
        SetToComplete();
    }
}

void JobConnectSecureEndPoint::InitializeBufferRequest()
{
    SecureStream* pStream = static_cast<SecureStream*>(m_pSecureEndPoint->GetStream());
    delete m_pRequestStream;
    m_pRequestStream = new BitStream();
    KerberosAuthentication::PrepareConnectionRequest(m_pRequestStream, pStream->GetAuthenticationClient(), m_pTicket,
                                                     &m_uiNonce);
}

void JobConnectSecureEndPoint::BuildListURLs(qList<StationURL>* pURLs)
{
    qList<ConnectionData>::const_iterator it = m_connectionData.begin();
    while (it != m_connectionData.end())
    {
        StationURL url(it->m_StationURL);
        url.SetConnectionID(it->m_uiValue);
        pURLs->push_back(url);
        it++;
    }
}

void JobConnectSecureEndPoint::PerformConnect()
{
    if (m_connectionData.empty())
    {
        m_result = qResult(static_cast<int>(CONNECT_INVALID_URL_RESULT));
        m_uiStep = STEP_COMPLETE;
        SetToComplete();
        return;
    }

    SecureStream* pStream = static_cast<SecureStream*>(m_pSecureEndPoint->GetStream());
    ConnectionManager* pConnectionManager = pStream->m_pCredentials->m_streamManager->GetConnectionManager();
    InitializeBufferRequest();
    delete m_pResponseStream;
    m_pResponseStream = new BitStream;

    qList<StationURL> urls;
    BuildListURLs(&urls);
    m_connectContext.RegisterCompletionCallback(ConnectCompletionCallback, UserContext(this), false);
    if (!pConnectionManager->ConnectImpl(&m_connectContext, m_pRequestStream->GetBuffer(),
                                         m_pResponseStream->GetBuffer(), urls, &m_pEndPoint,
                                         Time::ConvertDeadlineToTimeout(m_deadline), false, &m_pConnectJob))
    {
        m_result = qResult(static_cast<int>(CONNECT_CALL_FAILED_RESULT));
        m_uiStep = STEP_PROCESS_CONNECT_RESULT;
        SetToRunning();
        return;
    }

    m_pConnectJob->AddRef();
    m_uiStep = STEP_PROCESS_CONNECT_RESULT;
    SetToSuspended();
}

void JobConnectSecureEndPoint::ProcessConnectResult()
{
    if (m_pConnectJob != 0)
    {
        m_pConnectJob->ReleaseRef();
        m_pConnectJob = 0;
    }

    if (m_result == true)
    {
        m_pResponseStream->AdjustLength();
        if (!KerberosAuthentication::ValidateConnectionResponse(m_pResponseStream, m_uiNonce))
            m_result = qResult(static_cast<int>(CONNECT_INVALID_RESPONSE_RESULT));

        if (m_pSecureEndPoint != 0)
            m_pSecureEndPoint->SetEncryptionKey(m_pTicket->GetKey());
    }

    m_uiStep = STEP_COMPLETE;
    SetToComplete();
}

void JobConnectSecureEndPoint::ConnectCompletion(CallContext* pCallContext)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();

    if (pCallContext->GetState() != CallContext::CallSucceeded)
    {
        m_result = qResult(static_cast<int>(CONNECT_FAILED_RESULT));
        Resume();
        csScheduler.Leave();
        return;
    }

    m_pSecureEndPoint->SetAssociatedEndPoint(m_pEndPoint);
    m_pEndPoint->SetPrincipalID(m_uiPrincipalID);
    m_pSecureEndPoint->SetPrincipalID(m_uiPrincipalID);
    m_pSecureEndPoint->SetConnectionID(m_pEndPoint->GetConnectionID());
    Resume();
    csScheduler.Leave();
}

void JobConnectSecureEndPoint::ConnectCompletionCallback(CallContext* pCallContext, const UserContext* pUserContext)
{
    static_cast<JobConnectSecureEndPoint*>(pUserContext->GetPointer())->ConnectCompletion(pCallContext);
}

void JobConnectSecureEndPoint::ExecuteStep()
{
    switch (m_uiStep)
    {
    case STEP_PARSE_URL:
        ParseURL();
        m_uiStep = STEP_CHECK_CONNECTION_DATA;
        break;
    case STEP_CHECK_CONNECTION_DATA:
        CheckConnectionData();
        break;
    case STEP_REQUEST_CONNECTION_DATA:
        RequestConnectionData();
        break;
    case STEP_PERFORM_CONNECT:
        PerformConnect();
        break;
    case STEP_PROCESS_CONNECT_RESULT:
        ProcessConnectResult();
        break;
    }
}

void JobConnectSecureEndPoint::Execute()
{
    do
    {
        ExecuteStep();
    } while (GetState() == Running);

    if (GetState() == Complete && m_pCallback != 0)
        m_pCallback(m_pSecureEndPoint, m_result, &m_userContext);
}

void JobConnectSecureEndPoint::CancelJob()
{
    if (m_pConnectJob != 0)
        m_pConnectJob->CancelJob();
}

}
}
