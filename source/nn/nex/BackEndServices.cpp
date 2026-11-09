#include "nn/nex/BackEndServices.hpp"

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/AuthenticationClient.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/ClientStreamManager.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/JobBackEndServicesLogin.hpp"
#include "nn/nex/JobBackEndServicesLoginWithData.hpp"
#include "nn/nex/JobBackEndServicesLogout.hpp"
#include "nn/nex/JobBackEndServicesTerminate.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/NintendoNotificationEventManager.hpp"
#include "nn/nex/NotificationEventManager.hpp"
#include "nn/nex/PRUDPStream.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/RendezVousLoginOperation.hpp"
#include "nn/nex/RendezVousLogoutOperation.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SecureConnectionClient.hpp"
#include "nn/nex/SecureStream.hpp"
#include "nn/nex/StreamManager.hpp"

namespace
{

void CallOperationOnCompletion(nn::nex::CallContext*, const nn::nex::UserContext*)
{
    nn::nex::ScopedCS lock(&nn::nex::Scheduler::GetInstance()->GetCriticalSection());
}

}

namespace nn
{
namespace nex
{

namespace
{

enum ResultCode
{
    SUCCESS_RESULT = 0x10001,
    NOT_CONNECTED_RESULT = static_cast<int>(0x800300D2u)
};

enum
{
    NOTIFICATION_EVENT_PROTOCOL_ID = 14,
    NINTENDO_NOTIFICATION_EVENT_PROTOCOL_ID = 100,
    NOTIFICATION_PROTOCOL_FLAG = 8,
    DEFAULT_LOGIN_TIMEOUT = 30000
};

}

void BackEndServices::InternalData::Set(const CallContext::CompletionCallback& rCallback,
                                        const UserContext& rUserContext)
{
    m_cbCallback = rCallback;
    m_userContext = rUserContext;
    m_bIsSet = true;
}

BackEndServices::EffectiveLoginTimeout::EffectiveLoginTimeout() : m_iTimeout(0)
{
}

BackEndServices::BackEndServices()
    : m_preTerminate(),
      m_terminateCallContext(),
      m_iDefaultLoginTimeout(DEFAULT_LOGIN_TIMEOUT),
      m_effectiveLoginTimeout(),
      m_pLoginOperation(0),
      m_pLogoutOperation(0),
      m_reserved1(true),
      m_stationURL()
{
    m_pCredentials = 0;
    Core::AcquireInstance();
    Network::AcquireInstance();
    RVClientCore::AcquireInstance();
    m_uiPort = RootTransport::GetWellKnownPortNumber();
    m_uiStreamFlags = 0;
    m_uiStreamParameter = 1;
    m_reserved0 = false;
    m_pNotificationEventManager = 0;
    m_pNintendoNotificationEventManager = 0;
    m_pStreamManagerFactory = 0;
    m_pLoginJob = 0;
    m_pLogoutJob = 0;
    m_pTerminateJob = 0;
    m_pTerminateContext = 0;
    m_pNotificationEventManager = new NotificationEventManager(0);
    m_pNotificationEventManager->SetProtocolID(NOTIFICATION_EVENT_PROTOCOL_ID);
    m_pNotificationEventManager->SetFlag(NOTIFICATION_PROTOCOL_FLAG);
    m_pNintendoNotificationEventManager = new NintendoNotificationEventManager(0);
    m_pNintendoNotificationEventManager->SetProtocolID(NINTENDO_NOTIFICATION_EVENT_PROTOCOL_ID);
    m_pNintendoNotificationEventManager->SetFlag(NOTIFICATION_PROTOCOL_FLAG);
}

BackEndServices::~BackEndServices()
{
    if (m_pCredentials != 0)
    {
        CallContext callContext;
        Terminate(&callContext);
        callContext.Wait(0xFFFFFFFF);
    }

    if (m_pTerminateJob != 0)
        m_pTerminateJob->ReleaseRef();

    if (m_pLoginJob != 0)
        m_pLoginJob->ReleaseRef();

    if (m_pLogoutJob != 0)
    {
        m_pLogoutJob->ReleaseRef();
        m_pLogoutJob = 0;
    }

    delete m_pNotificationEventManager;
    m_pNotificationEventManager = 0;
    delete m_pNintendoNotificationEventManager;
    m_pNintendoNotificationEventManager = 0;
    m_pTerminateContext = 0;

    if (m_pLoginOperation != 0)
    {
        delete m_pLoginOperation;
        m_pLoginOperation = 0;
    }

    if (m_pLogoutOperation != 0)
    {
        delete m_pLogoutOperation;
        m_pLogoutOperation = 0;
    }

    RVClientCore::ReleaseInstance();
    Network::ReleaseInstance();
    Core::ReleaseInstance();
}

bool BackEndServices::TerminateJobIsInProgress() const
{
    return m_pTerminateJob != 0 && m_pTerminateJob->GetState() != Job::Complete;
}

bool BackEndServices::TerminateImpl()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (TerminateJobIsInProgress())
        return false;

    if (m_pTerminateJob != 0)
    {
        m_pTerminateJob->ReleaseRef();
        m_pTerminateJob = 0;
    }

    m_pTerminateJob = new JobBackEndServicesTerminate(m_pTerminateContext->m_uiCallId, this);
    m_pTerminateJob->AddRef();
    Scheduler::GetInstance()->Queue(m_pTerminateJob, false);
    return true;
}

void BackEndServices::TerminateCB(CallContext*, const UserContext* pUserContext)
{
    static_cast<BackEndServices*>(pUserContext->GetPointer())->TerminateImpl();
}

bool BackEndServices::Terminate(CallContext* pCallContext)
{
    if (!pCallContext->InitiateCall())
        return false;

    m_pTerminateContext = pCallContext;
    if (m_preTerminate.m_bIsSet)
    {
        m_terminateCallContext.Reset();
        m_terminateCallContext.RegisterCompletionCallback(TerminateCB, UserContext(this), true);
        m_preTerminate.m_cbCallback(&m_terminateCallContext, &m_preTerminate.m_userContext);
        return true;
    }

    return TerminateImpl();
}

StreamManager* BackEndServices::GetStreamManager() const
{
    if (m_pCredentials == 0)
        return 0;

    return m_pCredentials->m_streamManager;
}

SecureStream* BackEndServices::GetSecureStream() const
{
    StreamManager* pStreamManager = GetStreamManager();
    if (pStreamManager == 0)
        return 0;

    return static_cast<SecureStream*>(pStreamManager->GetSecureStream());
}

SecureConnectionClient* BackEndServices::GetSecureConnectionClient() const
{
    SecureStream* pSecureStream = GetSecureStream();
    if (pSecureStream == 0)
        return 0;

    SecureConnectionClient* pClient = pSecureStream->GetSecureConnectionClient();
    if (pClient != 0)
        pClient->SetDefaultCredentials(m_pCredentials);

    return pClient;
}

qResult BackEndServices::IsConnected() const
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pLogoutJob != 0 && m_pLogoutJob->GetState() != Job::Complete)
        return qResult(NOT_CONNECTED_RESULT);

    if (m_pLoginJob != 0 && m_pLoginJob->GetState() == Job::Complete)
    {
        SecureConnectionClient* pClient = GetSecureConnectionClient();
        if (pClient != 0 && pClient->IsConnected() && !pClient->IsFaulty() && pClient->m_pCredentials->m_value != 0
            && !RootTransport::GetInstance()->m_reserved4)
        {
            return qResult(SUCCESS_RESULT);
        }

        return qResult(NOT_CONNECTED_RESULT);
    }

    return qResult(NOT_CONNECTED_RESULT);
}

bool BackEndServices::Disconnect(CallContext* pCallContext)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    Platform::WarnObsoleteMethod(L"BackEndServices::Disconnect(CallContext*)",
                                 L"BackEndServices::Logout(CallContext*, Credentials*)");
    if (!pCallContext->InitiateCall())
        return false;

    pCallContext->SignalSuccess(qResult(SUCCESS_RESULT));
    return true;
}

void BackEndServices::RegisterOperationOnCallContext(CallContext& rCallContext, RendezVousOperation& rOperation)
{
    rCallContext.RegisterCompletionCallback(CallOperationOnCompletion, UserContext(&rOperation), false);
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
}

void BackEndServices::OnLogoutStart(CallContext& rCallContext)
{
    if (m_pLogoutOperation == 0)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        if (m_pLogoutOperation == 0)
            m_pLogoutOperation = new RendezVousLogoutOperation(*this);
    }

    RegisterOperationOnCallContext(rCallContext, *m_pLogoutOperation);
}

bool BackEndServices::LogoutImpl(CallContext* pCallContext, Credentials* pCredentials)
{
    if (m_pLogoutJob != 0)
    {
        if (m_pLogoutJob->GetState() != Job::Complete)
            return false;

        m_pLogoutJob->ReleaseRef();
        m_pLogoutJob = 0;
    }

    if (!pCallContext->InitiateCall())
        return false;

    if (m_pCredentials == 0)
    {
        pCallContext->SignalSuccess(qResult(SUCCESS_RESULT));
        return true;
    }

    m_pLogoutJob = new JobBackEndServicesLogout(pCallContext->m_uiCallId, this, pCredentials);
    m_pLogoutJob->AddRef();
    Scheduler::GetInstance()->Queue(m_pLogoutJob, false);
    return true;
}

bool BackEndServices::Logout_(CallContext* pCallContext, Credentials* pCredentials)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pTerminateJob != 0)
    {
        if (m_pTerminateJob->GetState() != Job::Complete)
            return false;

        m_pTerminateJob->ReleaseRef();
        m_pTerminateJob = 0;
    }

    if (m_pLoginJob != 0)
    {
        if (m_pLoginJob->GetState() != Job::Complete)
            return false;

        m_pLoginJob->ReleaseRef();
        m_pLoginJob = 0;
    }

    return LogoutImpl(pCallContext, pCredentials);
}

void BackEndServices::RegisterPreTerminateCallback(CallContext::CompletionCallback pCallback,
                                                   const UserContext& rUserContext)
{
    m_preTerminate.Set(pCallback, rUserContext);
}

void BackEndServices::ReleaseCredentials()
{
    if (m_pCredentials != 0)
        m_pCredentials->ReleaseRef();

    m_pCredentials = 0;
}

void BackEndServices::PostLogoutCleanup()
{
    ReleaseCredentials();
}

void BackEndServices::RegisterServerProtocols()
{
    m_pNotificationEventManager->m_uiPrincipalID = m_pCredentials->m_value;
    m_pNotificationEventManager->m_uiConnectionID = m_pCredentials->m_state;
    RVClientCore::GetInstance()->RegisterGlobalNotificationEventManager(m_pNotificationEventManager);
    m_pCredentials->m_streamManager->GetProtocolRequestBroker()->RegisterServerProtocol(
        NINTENDO_NOTIFICATION_EVENT_PROTOCOL_ID, m_pNintendoNotificationEventManager);
}

void BackEndServices::UnregisterServerProtocols()
{
    RVClientCore::GetInstance()->UnregisterGlobalNotificationEventManager(m_pNotificationEventManager);
    m_pCredentials->m_streamManager->GetProtocolRequestBroker()->UnregisterServerProtocol(
        NINTENDO_NOTIFICATION_EVENT_PROTOCOL_ID);
}

AuthenticationClient* BackEndServices::GetAuthenticationClient() const
{
    SecureStream* pSecureStream = GetSecureStream();
    if (pSecureStream == 0)
        return 0;

    AuthenticationClient* pClient = pSecureStream->GetAuthenticationClient();
    if (pClient != 0)
        pClient->SetDefaultCredentials(m_pCredentials);

    return pClient;
}

bool BackEndServices::RegisterNotificationEventHandler(NotificationEventHandler* pHandler)
{
    if (m_pNotificationEventManager == 0)
        return false;

    return m_pNotificationEventManager->RegisterNotificationEventHandler(pHandler);
}

bool BackEndServices::UnregisterNotificationEventHandler(NotificationEventHandler* pHandler)
{
    if (m_pNotificationEventManager == 0)
        return false;

    return m_pNotificationEventManager->UnregisterNotificationEventHandler(pHandler);
}

StreamManager* BackEndServices::CreateStreamManager() const
{
    StreamManager* pStreamManager;
    if (m_pStreamManagerFactory == 0)
        pStreamManager = new ClientStreamManager();
    else
        pStreamManager = m_pStreamManagerFactory();

    pStreamManager->Initialize(m_uiPort, m_uiStreamFlags, m_uiStreamParameter);
    SecureStream* pSecureStream = new SecureStream();
    pSecureStream->SetAssociatedStream(pStreamManager->GetStream());
    pStreamManager->AssociateSecureStream(pSecureStream, SecureStreamConnectionFilter);
    return pStreamManager;
}

void BackEndServices::SetSandboxAccessKey(const String& rKey)
{
    StreamManager::SetSandboxAccessKey(rKey);
}

bool BackEndServices::LoginJobIsInProgress() const
{
    return m_pLoginJob != 0 && m_pLoginJob->GetState() != Job::Complete;
}

bool BackEndServices::LogoutJobIsInProgress() const
{
    return m_pLogoutJob != 0 && m_pLogoutJob->GetState() != Job::Complete;
}

void BackEndServices::SetEffectiveLoginTimeout(int iTimeout)
{
    m_effectiveLoginTimeout = iTimeout;
}

bool BackEndServices::EffectiveTimeoutIsSet() const
{
    return m_effectiveLoginTimeout != 0;
}

ProtocolRegistry* BackEndServices::GetProtocolRegistry() const
{
    return RVClientCore::GetInstance()->GetProtocolRegistry();
}

bool BackEndServices::LoginImpl(CallContext* pCallContext, qResult* pResult, const wchar_t* pUserName,
                                const char* pPassword, const wchar_t* pHostName, unsigned short uiPort,
                                Credentials** ppCredentials, String* pErrorMessage,
                                AnyObjectHolder<Data, String>* pExtraData)
{
    Core::GetInstance()->SetTerminateImmediately(false);
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pTerminateJob != 0)
    {
        if (m_pTerminateJob->GetState() != Job::Complete)
            return false;

        m_pTerminateJob->ReleaseRef();
        m_pTerminateJob = 0;
    }

    if (m_pLoginJob != 0)
    {
        if (m_pLoginJob->GetState() != Job::Complete)
            return false;

        m_pLoginJob->ReleaseRef();
        m_pLoginJob = 0;
    }

    if (m_pLogoutJob != 0)
    {
        if (m_pLogoutJob->GetState() != Job::Complete)
            return false;

        m_pLogoutJob->ReleaseRef();
        m_pLogoutJob = 0;
    }

    if (!pCallContext->InitiateCall())
        return false;

    if (m_pCredentials == 0)
        m_pCredentials = new Credentials();

    if (m_effectiveLoginTimeout == 0)
        m_effectiveLoginTimeout = m_iDefaultLoginTimeout;

    m_pLoginJob = new JobBackEndServicesLogin(pCallContext->m_uiCallId, this, pResult, String(pUserName), pPassword,
                                              pHostName, uiPort, ppCredentials, pErrorMessage, pExtraData,
                                              m_effectiveLoginTimeout, m_pStreamManagerFactory);
    m_pLoginJob->AddRef();
    Scheduler::GetInstance()->Queue(m_pLoginJob, false);
    return true;
}

bool BackEndServices::LoginImplWithData(CallContext* pCallContext, qResult* pResult, const char* pName,
                                        const wchar_t* pHostName, unsigned short uiPort,
                                        const CustomDataHolder<LoginData>& rLoginData,
                                        const AnyObjectHolder<Data, String>& rExtraData, Credentials** ppCredentials)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pTerminateJob != 0)
    {
        if (m_pTerminateJob->GetState() != Job::Complete)
            return false;

        m_pTerminateJob->ReleaseRef();
        m_pTerminateJob = 0;
    }

    if (m_pLoginJob != 0)
    {
        if (m_pLoginJob->GetState() != Job::Complete)
            return false;

        m_pLoginJob->ReleaseRef();
        m_pLoginJob = 0;
    }

    if (m_pLogoutJob != 0)
    {
        if (m_pLogoutJob->GetState() != Job::Complete)
            return false;

        m_pLogoutJob->ReleaseRef();
        m_pLogoutJob = 0;
    }

    if (!pCallContext->InitiateCall())
        return false;

    if (m_pCredentials == 0)
    {
        m_pCredentials = new Credentials(0, CreateStreamManager());
        m_pCredentials->m_streamManager->SetCredentials(m_pCredentials);
    }

    if (m_effectiveLoginTimeout == 0)
        m_effectiveLoginTimeout = m_iDefaultLoginTimeout;

    m_pCredentials->m_description = String(pName);
    m_pCredentials->m_name = rLoginData.GetObject()->m_Name;
    m_pLoginJob = new JobBackEndServicesLoginWithData(pCallContext->m_uiCallId, this, pResult, pName, pHostName, uiPort,
                                                      rLoginData, rExtraData, ppCredentials, m_effectiveLoginTimeout);
    m_pLoginJob->AddRef();
    Scheduler::GetInstance()->Queue(m_pLoginJob, false);
    return true;
}

void BackEndServices::OnLoginStart(CallContext& rCallContext)
{
    if (m_pLoginOperation == 0)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        if (m_pLoginOperation == 0)
            m_pLoginOperation = new RendezVousLoginOperation(*this);
    }

    RegisterOperationOnCallContext(rCallContext, *m_pLoginOperation);
}

void BackEndServices::CancelLoginJob()
{
    if (m_pLoginJob == 0)
        return;

    m_pLoginJob->CancelJob();
}

}
}
