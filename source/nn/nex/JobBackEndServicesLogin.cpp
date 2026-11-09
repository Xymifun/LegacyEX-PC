#include "nn/nex/JobBackEndServicesLogin.hpp"

#include <string.h>

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/AuthenticationClient.hpp"
#include "nn/nex/BackEndServices.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/ClientStreamManager.hpp"
#include "nn/nex/ConnectionManager.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/JobConnectEndPoint.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PRUDPStream.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/SecureConnectionClient.hpp"
#include "nn/nex/SecureStream.hpp"
#include "nn/nex/SpecialArray.hpp"
#include "nn/nex/StreamManager.hpp"
#include "nn/nex/StringStream.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint64_t TRACE_LOGIN = 4;
const unsigned int RESERVED_PRINCIPAL_ID = 2;
const unsigned int STATION_URL_PUBLIC_TYPE_BIT = 1;
const int AUTHENTICATION_STREAM_ID = 1;
const int AUTHENTICATION_URL_TYPE = 2;
const int SECURE_URL_PRINCIPAL_ID = 2;
const int SECURE_URL_TYPE = 2;
const unsigned int ENDPOINT_FAULT_CANCELLED = 6;

enum LoginResult
{
    LOGIN_CANCELLED_RESULT = static_cast<int>(0x80010004u),
    LOGIN_INVALID_ARGUMENT_RESULT = static_cast<int>(0x8001000Au),
    LOGIN_CALL_FAILED_RESULT = static_cast<int>(0x8001000Du),
    LOGIN_ALREADY_LOGGED_IN_RESULT = static_cast<int>(0x80030001u),
    LOGIN_SUCCEEDED_RESULT = 0x10001
};

}

AnyObjectHolder<Data, String> g_registerURLsExtraData;

JobBackEndServicesLogin::JobBackEndServicesLogin(unsigned int uiCallId, BackEndServices* pBackEndServices,
                                                 qResult* pResult, const String& rUserName, const char* pPassword,
                                                 const wchar_t* pHost, unsigned short uiPort,
                                                 Credentials** ppCredentials, String* pErrorMessage,
                                                 AnyObjectHolder<Data, String>* pExtraData, int iTimeout,
                                                 StreamManagerFactory pStreamManagerFactory)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(uiCallId),
      m_userName(rUserName),
      m_host(pHost),
      m_uiPort(uiPort),
      m_ppCredentials(ppCredentials),
      m_connectContext(),
      m_protocolContext(0),
      m_authResult(),
      m_jobResult(),
      m_buffer(),
      m_pBackEndServices(pBackEndServices),
      m_publicURL(),
      m_connectionData(),
      m_pResult(pResult),
      m_pErrorMessage(pErrorMessage),
      m_pExtraData(pExtraData),
      m_pConnectJob(0),
      m_pAuthenticationEndPoint(0),
      m_pSecureEndPoint(0),
      m_deadline(Time::ConvertTimeoutToDeadline(iTimeout)),
      m_bCancelled(false),
      m_pStreamManagerFactory(pStreamManagerFactory)
{
    m_traceData = TRACE_LOGIN;
    m_pPassword = 0;
    if (pPassword != 0)
    {
        m_pPassword = SpecialNewArray<char>(strlen(pPassword) + 1, L"", 0);
        strcpy(m_pPassword, pPassword);
    }

    *m_ppCredentials = 0;
    SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogin::ValidateArguments), L""));
}

JobBackEndServicesLogin::~JobBackEndServicesLogin()
{
    if (m_pConnectJob != 0)
    {
        m_pConnectJob->ReleaseRef();
        m_pConnectJob = 0;
    }

    if (m_pPassword != 0)
        SpecialDeleteArray(m_pPassword);
}

void JobBackEndServicesLogin::CancelJob()
{
    if (m_bCancelled)
        return;

    m_bCancelled = true;

    if (m_pConnectJob != 0)
    {
        m_pConnectJob->CancelJob();
        m_pConnectJob->ReleaseRef();
        m_pConnectJob = 0;
    }

    if (m_pAuthenticationEndPoint != 0)
        m_pAuthenticationEndPoint->SignalFaultEvent(ENDPOINT_FAULT_CANCELLED, true);

    if (m_pSecureEndPoint != 0)
        m_pSecureEndPoint->SignalFaultEvent(ENDPOINT_FAULT_CANCELLED, true);
}

void JobBackEndServicesLogin::CompleteJob(qResult result)
{
    m_jobResult = result;

    if (!m_jobResult && m_pBackEndServices->GetCredentials() != 0)
    {
        m_connectContext.Reset();

        if (m_pBackEndServices->LogoutImpl(&m_connectContext, m_pBackEndServices->GetCredentials()))
        {
            SetToSuspended();
            ResumeOnCallCompletion(
                &m_connectContext,
                new Step(static_cast<Step::Function>(&JobBackEndServicesLogin::CompleteLogout), L""));
            return;
        }
    }

    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        if (m_pResult != 0)
            *m_pResult = m_jobResult;

        if (m_jobResult)
        {
            *m_ppCredentials = m_pBackEndServices->GetCredentials();
            pCallContext->SignalSuccess(m_jobResult);
            SetToComplete();
            return;
        }

        pCallContext->SignalFailure(m_jobResult);
    }

    SetToComplete();
}

void JobBackEndServicesLogin::ValidateArguments()
{
    if (m_userName.GetLength() == 0)
    {
        CompleteJob(qResult(LOGIN_INVALID_ARGUMENT_RESULT));
        return;
    }

    if (m_pPassword == 0)
    {
        CompleteJob(qResult(LOGIN_INVALID_ARGUMENT_RESULT));
        return;
    }

    SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogin::ConnectToAuthenticationService), L""));
}

StreamManager* JobBackEndServicesLogin::CreateStreamManager() const
{
    StreamManager* pStreamManager;
    bool bInitialized;

    if (m_pStreamManagerFactory == 0)
    {
        pStreamManager = new ClientStreamManager();
        bInitialized = pStreamManager->Initialize(m_pBackEndServices->m_uiPort, m_pBackEndServices->m_uiStreamFlags,
                                                  m_pBackEndServices->m_uiStreamParameter);
    }
    else
    {
        pStreamManager = m_pStreamManagerFactory();
        bInitialized = pStreamManager->Initialize(m_pBackEndServices->m_uiPort, m_pBackEndServices->m_uiStreamFlags,
                                                  m_pBackEndServices->m_uiStreamParameter);
    }

    if (bInitialized)
    {
        SecureStream* pSecureStream = new SecureStream();
        pSecureStream->SetAssociatedStream(pStreamManager->GetStream());
        pStreamManager->AssociateSecureStream(pSecureStream, SecureStreamConnectionFilter);
    }
    else if (m_pStreamManagerFactory == 0)
    {
        pStreamManager->Terminate();
        delete pStreamManager;
        pStreamManager = 0;
    }

    return pStreamManager;
}

void JobBackEndServicesLogin::ConnectToAuthenticationService()
{
    StringStream urlStream;
    ((((urlStream << L"prudp:/address=" << m_host << L";port=").StreamNumber(m_uiPort) << L";stream=")
          .StreamNumber(Stream::RendezVousSecure)
      << L";sid=")
         .StreamNumber(AUTHENTICATION_STREAM_ID)
     << L";type=")
        .StreamNumber(AUTHENTICATION_URL_TYPE);
    StationURL authenticationURL(urlStream.GetBuffer());

    StreamManager* pStreamManager = CreateStreamManager();
    if (pStreamManager == 0)
    {
        CompleteJob(qResult(LOGIN_CALL_FAILED_RESULT));
        return;
    }

    m_pBackEndServices->GetCredentials()->SetStreamManager(pStreamManager);
    m_pBackEndServices->GetCredentials()->m_streamManager->SetCredentials(m_pBackEndServices->GetCredentials());
    m_pBackEndServices->GetCredentials()->m_description = String(m_pPassword);
    m_pBackEndServices->GetCredentials()->m_name = m_userName;
    m_connectContext.Reset();

    ConnectionManager* pConnectionManager = m_pBackEndServices->GetStreamManager()->GetConnectionManager();
    unsigned int uiTimeout = Time::ConvertDeadlineToTimeout(m_deadline);
    bool bConnecting;
    {
        qList<StationURL> urls;
        urls.push_back(authenticationURL);
        bConnecting = pConnectionManager->ConnectImpl(&m_connectContext, 0, 0, urls, &m_pAuthenticationEndPoint,
                                                      uiTimeout, false, &m_pConnectJob);
    }

    if (!bConnecting)
    {
        CompleteJob(qResult(LOGIN_CALL_FAILED_RESULT));
        return;
    }

    m_pConnectJob->AddRef();
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_connectContext,
        new Step(static_cast<Step::Function>(&JobBackEndServicesLogin::ProcessAuthConnectionResult), L""));
}

void JobBackEndServicesLogin::ProcessAuthConnectionResult()
{
    if (m_pConnectJob != 0)
    {
        m_pConnectJob->ReleaseRef();
        m_pConnectJob = 0;
    }

    if (m_connectContext.GetState() == CallContext::CallSucceeded)
    {
        m_pBackEndServices->GetCredentials()->SetAuthenticationConnection(m_pAuthenticationEndPoint);
        m_pBackEndServices->GetAuthenticationClient()->SetDefaultCredentials(m_pBackEndServices->GetCredentials());
        SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogin::Authenticate), L""));
        return;
    }

    if (m_pAuthenticationEndPoint != 0)
    {
        m_pAuthenticationEndPoint->Close();
        m_pAuthenticationEndPoint = 0;
    }

    CompleteJob(m_connectContext.GetResult());
}

void JobBackEndServicesLogin::Authenticate()
{
    BackEndServices* pBackEndServices = m_pBackEndServices;
    if (pBackEndServices->GetSecureStream()->m_pCredentials != 0)
    {
        CompleteJob(qResult(LOGIN_ALREADY_LOGGED_IN_RESULT));
        return;
    }

    AuthenticationClient* pAuthenticationClient = pBackEndServices->GetAuthenticationClient();
    m_protocolContext.m_expirationTime = m_deadline;
    if (!pAuthenticationClient->Login(&m_protocolContext, &m_authResult, m_userName, m_pPassword, &m_connectionData,
                                      m_pErrorMessage, m_pExtraData))
    {
        CompleteJob(qResult(LOGIN_CALL_FAILED_RESULT));
        return;
    }

    SetToSuspended();
    ResumeOnCallCompletion(
        &m_protocolContext,
        new Step(static_cast<Step::Function>(&JobBackEndServicesLogin::ProcessAuthenticationResult), L""));
}

void JobBackEndServicesLogin::ProcessAuthenticationResult()
{
    if (m_bCancelled)
    {
        CompleteJob(qResult(LOGIN_CANCELLED_RESULT));
        return;
    }

    if (m_protocolContext.GetState() != CallContext::CallSucceeded || !(m_authResult == true))
    {
        CompleteJob(m_authResult);
        return;
    }

    AuthenticationClient* pAuthenticationClient = m_pBackEndServices->GetAuthenticationClient();
    BackEndServices* pBackEndServices = m_pBackEndServices;
    pBackEndServices->GetCredentials()->m_value = pAuthenticationClient->m_uiPrincipalID;
    pBackEndServices->GetSecureStream()->SetCredentials(pBackEndServices->GetCredentials());
    RVClientCore::GetInstance()->SetGameServerTime(m_connectionData.m_Time);
    SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogin::ConnectToSecureConnectionService), L""));
}

void JobBackEndServicesLogin::ConnectToSecureConnectionService()
{
    if (m_pBackEndServices->GetAuthenticationClient()->m_uiPrincipalID == RESERVED_PRINCIPAL_ID)
    {
        m_pBackEndServices->GetCredentials()->m_state = 1;
        SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogin::CompleteLogin), L""));
        return;
    }

    String secureAddress;
    StationURL secureURL;
    if (m_connectionData.m_RegularProtocolsURL != StationURL(L""))
    {
        secureURL = m_connectionData.m_RegularProtocolsURL;
    }
    else
    {
        secureAddress.Format(L"prudps:/address=%ls;port=%d;stream=%d;sid=%d;PID=%d;CID=1;type=%d",
                             static_cast<const wchar_t*>(m_host), m_uiPort + 1, Stream::RendezVousSecure,
                             AUTHENTICATION_STREAM_ID, SECURE_URL_PRINCIPAL_ID, SECURE_URL_TYPE);
        secureURL = secureAddress;
    }

    m_connectContext.Reset();

    ConnectionManager* pConnectionManager = m_pBackEndServices->GetStreamManager()->GetConnectionManager();
    unsigned int uiTimeout = Time::ConvertDeadlineToTimeout(m_deadline);
    bool bConnecting;
    {
        qList<StationURL> urls;
        urls.push_back(secureURL);
        bConnecting = pConnectionManager->ConnectImpl(&m_connectContext, 0, 0, urls, &m_pSecureEndPoint, uiTimeout,
                                                      false, &m_pConnectJob);
    }

    if (!bConnecting)
    {
        CompleteJob(qResult(LOGIN_CALL_FAILED_RESULT));
        return;
    }

    m_pConnectJob->AddRef();
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_connectContext,
        new Step(static_cast<Step::Function>(&JobBackEndServicesLogin::ProcessSecConnConnectionResult), L""));
}

void JobBackEndServicesLogin::ProcessSecConnConnectionResult()
{
    if (m_pConnectJob != 0)
    {
        m_pConnectJob->ReleaseRef();
        m_pConnectJob = 0;
    }

    if (m_connectContext.GetState() == CallContext::CallSucceeded)
    {
        m_pBackEndServices->GetCredentials()->SetSecureConnection(m_pSecureEndPoint);
        m_pBackEndServices->GetSecureConnectionClient()->Bind(m_pBackEndServices->GetCredentials());
        SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogin::RegisterURLs), L""));
        return;
    }

    if (m_pSecureEndPoint != 0)
    {
        m_pSecureEndPoint->Close();
        m_pSecureEndPoint = 0;
    }

    CompleteJob(m_connectContext.GetResult());
}

void JobBackEndServicesLogin::RegisterURLs()
{
    m_protocolContext.Reset();
    m_protocolContext.m_expirationTime = m_deadline;
    if (!m_pBackEndServices->GetSecureConnectionClient()->RegisterURLsEx(&m_protocolContext, &m_authResult,
                                                                         &m_publicURL, g_registerURLsExtraData))
    {
        CompleteJob(qResult(LOGIN_CALL_FAILED_RESULT));
        return;
    }

    SetToSuspended();
    ResumeOnCallCompletion(&m_protocolContext,
                           new Step(static_cast<Step::Function>(&JobBackEndServicesLogin::CompleteLogin), L""));
}

void JobBackEndServicesLogin::CompleteLogin()
{
    if (m_pBackEndServices->GetAuthenticationClient()->m_uiPrincipalID != RESERVED_PRINCIPAL_ID)
    {
        if (m_protocolContext.GetState() != CallContext::CallSucceeded)
        {
            CompleteJob(m_protocolContext.GetResult());
            return;
        }

        if (m_authResult == false)
        {
            CompleteJob(m_authResult);
            return;
        }
    }

    m_publicURL.SetRVConnectionID(m_pBackEndServices->GetCredentials()->m_state);
    m_publicURL.SetPlatformType();

    if (m_pBackEndServices->m_reserved1)
    {
        qProtectedList<StationURL>& rLocalURLs = Network::GetInstance()->GetLocalURLs();
        ScopedCS lock(&rLocalURLs.GetCriticalSection());
        qList<StationURL>& rURLs = rLocalURLs.GetList();
        for (qList<StationURL>::iterator it = rURLs.begin(); it != rURLs.end(); ++it)
        {
            if (((it->GetType() >> STATION_URL_PUBLIC_TYPE_BIT) & 1) == 0)
                it->SetRVConnectionID(m_publicURL.GetRVConnectionID());

            if (String::IsEqual(it->GetAddress(), m_publicURL.GetAddress()))
            {
                it->SetType(m_publicURL.GetType());
                it->SetRVConnectionID(m_publicURL.GetRVConnectionID());
                m_publicURL.SetPortNumber(it->GetPortNumber());
            }
        }

        Network::GetInstance()->AddLocalURL(m_publicURL);
    }

    m_pBackEndServices->m_stationURL = m_publicURL;
    m_pBackEndServices->RegisterServerProtocols();
    SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogin::DisconnectFromAuthenticationService), L""));
}

void JobBackEndServicesLogin::CompleteLogout()
{
    CompleteJob(m_jobResult);
}

void JobBackEndServicesLogin::DisconnectFromAuthenticationService()
{
    EndPoint* pEndPoint = m_pBackEndServices->GetCredentials()->m_authenticationConnection;
    m_pBackEndServices->GetCredentials()->SetAuthenticationConnection(m_pSecureEndPoint);
    m_pBackEndServices->GetAuthenticationClient()->Rebind(m_pBackEndServices->GetCredentials());
    m_pAuthenticationEndPoint = 0;
    m_connectContext.Reset();
    m_connectContext.m_expirationTime = m_deadline;

    if (!m_pBackEndServices->GetStreamManager()->GetConnectionManager()->Disconnect(&m_connectContext, pEndPoint))
    {
        CompleteJob(qResult(LOGIN_SUCCEEDED_RESULT));
        return;
    }

    SetToSuspended();
    ResumeOnCallCompletion(
        &m_connectContext,
        new Step(static_cast<Step::Function>(&JobBackEndServicesLogin::ProcessAuthDisconnectionResult), L""));
}

void JobBackEndServicesLogin::ProcessAuthDisconnectionResult()
{
    CompleteJob(qResult(LOGIN_SUCCEEDED_RESULT));
}

}
}
