#include "nn/nex/JobBackEndServicesLogout.hpp"

#include "nn/nex/AuthenticationClient.hpp"
#include "nn/nex/BackEndServices.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/ConnectionManager.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/SecureConnectionClient.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/StreamManager.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint64_t TRACE_BACK_END_SERVICES = 0x1000;
const unsigned short SECURE_CONNECTION_TYPE = 11;
const unsigned int STATION_URL_TYPE_BIT = 1;
const int WAIT_CONTEXT_STREAM_MANAGER_TERMINATION = 50;

enum LogoutResult
{
    LOGOUT_FAILED_RESULT = static_cast<int>(0x80010001u),
    LOGOUT_SUCCEEDED_RESULT = 0x10001
};

}

JobBackEndServicesLogout::JobBackEndServicesLogout(unsigned int uiCallId, BackEndServices* pBackEndServices,
                                                   Credentials* pCredentials)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_callContext()
{
    m_pBackEndServices = pBackEndServices;
    m_pCredentials = pCredentials;
    m_uiCallId = uiCallId;
    m_traceData = TRACE_BACK_END_SERVICES;
    m_pStreamManager = 0;

    if (pCredentials != 0)
    {
        EndPoint* pEndPoint = pCredentials->GetConnection(SECURE_CONNECTION_TYPE);
        if (pEndPoint != 0)
            Core::GetInstance()->GetCallContextRegister()->CancelCallContexts(pEndPoint, 0);
    }

    SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogout::DisconnectFromSecureConnectionService), L""));
}

JobBackEndServicesLogout::~JobBackEndServicesLogout()
{
}

void JobBackEndServicesLogout::DisconnectFromAuthenticationService()
{
    if (m_pBackEndServices->GetAuthenticationClient() == 0
        || !m_pBackEndServices->GetAuthenticationClient()->IsConnected())
    {
        SetStep(
            Step(static_cast<Step::Function>(&JobBackEndServicesLogout::DisconnectFromSecureConnectionService), L""));
    }
    else
    {
        BackEndServices* pBackEndServices = m_pBackEndServices;
        ConnectionManager* pConnectionManager = pBackEndServices->GetStreamManager()->GetConnectionManager();
        if (!pConnectionManager->Disconnect(&m_callContext,
                                            pBackEndServices->m_pCredentials->m_authenticationConnection))
        {
            SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogout::CompleteLogoutFailure), L""));
        }
        else
        {
            SetToSuspended();
            ResumeOnCallCompletion(
                &m_callContext,
                new Step(static_cast<Step::Function>(&JobBackEndServicesLogout::ProcessAuthDisconnectionResult), L""));
        }
    }
}

void JobBackEndServicesLogout::ProcessAuthDisconnectionResult()
{
    if (m_callContext.GetState() == CallContext::CallSucceeded)
    {
        m_pBackEndServices->GetAuthenticationClient()->Logout();
        m_pBackEndServices->GetAuthenticationClient()->Unbind();
        m_pBackEndServices->GetCredentials()->SetAuthenticationConnection(0);
        SetStep(
            Step(static_cast<Step::Function>(&JobBackEndServicesLogout::DisconnectFromSecureConnectionService), L""));
        m_callContext.Reset();
    }
    else
    {
        SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogout::CompleteLogoutFailure), L""));
        m_callContext.Reset();
    }
}

void JobBackEndServicesLogout::DisconnectFromSecureConnectionService()
{
    BackEndServices* pBackEndServices = m_pBackEndServices;
    if (pBackEndServices->m_pCredentials->m_secureConnection == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogout::MarkCredentialsInvalid), L""));
        return;
    }

    ConnectionManager* pConnectionManager = pBackEndServices->GetStreamManager()->GetConnectionManager();
    if (!pConnectionManager->Disconnect(&m_callContext, pBackEndServices->m_pCredentials->m_secureConnection))
    {
        SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogout::ProcessSecConnDisconnectionResult), L""));
        return;
    }

    SetToSuspended();
    ResumeOnCallCompletion(
        &m_callContext,
        new Step(static_cast<Step::Function>(&JobBackEndServicesLogout::ProcessSecConnDisconnectionResult), L""));
}

void JobBackEndServicesLogout::ProcessSecConnDisconnectionResult()
{
    m_pBackEndServices->GetSecureConnectionClient()->Unbind();
    m_pBackEndServices->GetCredentials()->SetSecureConnection(0);
    m_pBackEndServices->UnregisterServerProtocols();
    StationURL emptyURL;
    m_pBackEndServices->m_stationURL = emptyURL;
    SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogout::MarkCredentialsInvalid), L""));
}

void JobBackEndServicesLogout::MarkCredentialsInvalid()
{
    unsigned int uiConnectionID = m_pCredentials->m_state;
    if (uiConnectionID != 0 && Network::GetInstance() != 0)
    {
        qProtectedList<StationURL>& rLocalURLs = Network::GetInstance()->GetLocalURLs();
        ScopedCS lock(&rLocalURLs.GetCriticalSection());
        qList<StationURL>& rURLs = rLocalURLs.GetList();
        qList<StationURL>::iterator it = rURLs.begin();
        while (it != rURLs.end())
        {
            if (it->GetRVConnectionID() == uiConnectionID)
            {
                if (((it->GetType() >> STATION_URL_TYPE_BIT) & 1) != 0)
                {
                    it = rURLs.erase(it);
                    continue;
                }

                it->SetRVConnectionID(0);
            }

            ++it;
        }
    }

    if (m_pCredentials->m_streamManager != 0)
    {
        m_pStreamManager = m_pCredentials->MarkInvalid();
        SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogout::TerminateStreamManager), L""));
        return;
    }

    SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogout::CompleteLogout), L""));
}

void JobBackEndServicesLogout::TerminateStreamManager()
{
    m_pStreamManager->Terminate();
    SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogout::WaitForStreamManagerTermination), L""));
}

void JobBackEndServicesLogout::WaitForStreamManagerTermination()
{
    if (!m_pStreamManager->IsTerminated())
    {
        SetToWaiting(WAIT_CONTEXT_STREAM_MANAGER_TERMINATION);
        return;
    }

    delete m_pStreamManager;
    m_pStreamManager = 0;
    SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesLogout::CompleteLogout), L""));
}

void JobBackEndServicesLogout::CompleteLogout()
{
    m_pBackEndServices->PostLogoutCleanup();
    SetToComplete();
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
        pCallContext->SetStateImpl(CallContext::CallSucceeded, qResult(LOGOUT_SUCCEEDED_RESULT), true);
}

void JobBackEndServicesLogout::CompleteLogoutFailure()
{
    m_pBackEndServices->PostLogoutCleanup();
    SetToComplete();
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
        pCallContext->SetStateImpl(CallContext::CallFailed, qResult(LOGOUT_FAILED_RESULT), false);
}

}
}
