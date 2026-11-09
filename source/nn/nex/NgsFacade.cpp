#include "nn/nex/NgsFacade.hpp"

#include <nn/act/client_cpp.h>
#include <nn/fp/fp_cpp.h>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/ErrorCodeConverter.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/JobCafeLogin.hpp"
#include "nn/nex/JobNNIDLogin.hpp"
#include "nn/nex/JobTerminate.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/NgsBridgeImp.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/Reconstruction.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/UserContext.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum ResultCode
{
    SUCCESS_RESULT = 0x10001,
    ERROR_INVALID_ARGUMENT = static_cast<int>(0x8001000Au),
    ERROR_BUSY = static_cast<int>(0x80010004u),
    ERROR_CALL_NOT_INITIATED = static_cast<int>(0x8001000Du),
    ERROR_ACCOUNT_FAILED = static_cast<int>(0x80680001u),
    ERROR_ACCOUNT_UNAVAILABLE = static_cast<int>(0x80650000u),
    ERROR_PRINCIPAL_ID_MISMATCH = static_cast<int>(0x80650009u)
};

const unsigned int ACT_ACCOUNT_ID_UNAVAILABLE_DESCRIPTION = 0xFA80;

enum
{
    HTTP_STATUS_OK = 200
};

}

int g_nasErrorCodeBase = 20000;
int g_httpErrorCodeBase = 23000;

NgsFacade::NgsFacade()
    : RendezVous(),
      m_reserved0(0),
      m_pLoginJob(0),
      m_pTerminateJob(0),
      m_iNasErrorCode(0),
      m_iHttpStatusCode(HTTP_STATUS_OK),
      m_loginResult(SUCCESS_RESULT),
      m_uiLoginErrorCode(0),
      m_reserved1(false),
      m_reserved2(false),
      m_reserved3(false),
      m_authenticationInfo()
{
    RootTransport::GetInstance()->EnableUPnPSupport(false);
    RegisterPreTerminateCallback(OnTerminate, UserContext(this));
    m_authenticationInfo = CustomDataHolder<AuthenticationInfo>(new AuthenticationInfo());
    m_pNgsBridge = new NgsBridgeImp(this);
}

NgsFacade::~NgsFacade()
{
    if (m_pCredentials != 0)
    {
        CallContext callContext;
        Terminate(&callContext);
        callContext.Wait(0xFFFFFFFF);
    }

    delete m_authenticationInfo.Release();

    if (m_pLoginJob != 0)
    {
        if (m_pLoginJob->GetState() != Job::Complete)
            m_pLoginJob->SetToComplete();

        m_pLoginJob->ReleaseRef();
        m_pLoginJob = 0;
    }

    if (m_pTerminateJob != 0)
    {
        if (m_pTerminateJob->GetState() != Job::Complete)
            m_pTerminateJob->SetToComplete();

        m_pTerminateJob->ReleaseRef();
        m_pTerminateJob = 0;
    }

    delete m_pNgsBridge;
}

bool NgsFacade::TerminateJobIsInProgress() const
{
    return m_pTerminateJob != 0 && m_pTerminateJob->GetState() != Job::Complete;
}

void NgsFacade::TerminateImpl(CallContext* pCallContext)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!pCallContext->InitiateCall())
        return;

    if (TerminateJobIsInProgress())
        return;

    if (m_pTerminateJob != 0)
    {
        m_pTerminateJob->ReleaseRef();
        m_pTerminateJob = 0;
    }

    m_pTerminateJob = new JobTerminate(pCallContext->m_uiCallId, this);
    m_pTerminateJob->AddRef();
    Scheduler::GetInstance()->Queue(m_pTerminateJob, false);
}

void NgsFacade::OnTerminate(CallContext* pCallContext, const UserContext* pUserContext)
{
    static_cast<NgsFacade*>(pUserContext->GetPointer())->TerminateImpl(pCallContext);
}

qResult NgsFacade::LoginAndRequestAuthenticationTokenImpl(ProtocolCallContext* pCallContext, unsigned int uiLoginParam,
                                                          const wchar_t* pSandboxAccessKey, const char* pFirstString,
                                                          char* pSecondString, const void* pToken, int iFirstValue,
                                                          unsigned int uiSecondValue)
{
    m_iNasErrorCode = 0;
    m_uiLoginErrorCode = 0;
    m_iHttpStatusCode = HTTP_STATUS_OK;
    m_loginResult = qResult(SUCCESS_RESULT);

    if (pCallContext == 0 || pSandboxAccessKey == 0 || (pFirstString != 0 && pSecondString == 0))
        return qResult(ERROR_INVALID_ARGUMENT);

    if (!nn::fp::IsInitialized())
        return qResult(ERROR_ACCOUNT_UNAVAILABLE);

    char accountId[nn::act::AccountIdSize];
    if (nn::act::GetAccountId(accountId)
        == nn::Result(nn::Result::LEVEL_USAGE, nn::Result::RESULT_MODULE_NN_ACT,
                      ACT_ACCOUNT_ID_UNAVAILABLE_DESCRIPTION))
        return qResult(ERROR_ACCOUNT_UNAVAILABLE);

    if (uiSecondValue != 0 && uiSecondValue != nn::act::GetPrincipalId())
        return qResult(ERROR_PRINCIPAL_ID_MISMATCH);

    if (m_pLoginJob != 0)
    {
        if (m_pLoginJob->GetState() != Job::Complete)
            return qResult(ERROR_BUSY);

        m_pLoginJob->ReleaseRef();
        m_pLoginJob = 0;
    }

    if (BackEndServices::LoginJobIsInProgress())
        return qResult(ERROR_BUSY);

    if (m_pCredentials != 0)
        return qResult(ERROR_BUSY);

    if (TerminateJobIsInProgress() || BackEndServices::TerminateJobIsInProgress())
        return qResult(ERROR_BUSY);

    unsigned short uiPort = RootTransport::GetNextPortNumber();
    RootTransport::GetInstance()->SetWellKnownPortNumber(uiPort);
    m_uiPort = uiPort;
    NATProperties natProperties;
    RootTransport::GetInstance()->SetNATProperties(natProperties);
    SetSandboxAccessKey(String(pSandboxAccessKey));

    if (!pCallContext->InitiateCall())
        return qResult(ERROR_CALL_NOT_INITIATED);

    if (pToken == 0)
    {
        JobCafeLogin* pCafeLoginJob = new JobCafeLogin();
        m_pLoginJob = pCafeLoginJob;
        pCafeLoginJob->Initialize(pCallContext->m_uiCallId, uiLoginParam, this, pFirstString, pSecondString,
                                  iFirstValue);
    }
    else
    {
        JobNNIDLogin* pNNIDLoginJob = new JobNNIDLogin();
        pNNIDLoginJob->Initialize(pCallContext->m_uiCallId, uiLoginParam, this, pFirstString, pSecondString, pToken,
                                  iFirstValue, uiSecondValue);
        m_pLoginJob = pNNIDLoginJob;
    }

    m_pLoginJob->AddRef();
    Scheduler::GetInstance()->Queue(m_pLoginJob, false);
    return qResult(SUCCESS_RESULT);
}

qResult NgsFacade::LoginAndRequestAuthenticationToken(ProtocolCallContext* pCallContext, unsigned int uiLoginParam,
                                                      const wchar_t* pSandboxAccessKey, const char* pFirstString,
                                                      char* pSecondString, int iFirstValue, unsigned int uiSecondValue)
{
    return LoginAndRequestAuthenticationTokenImpl(pCallContext, uiLoginParam, pSandboxAccessKey, pFirstString,
                                                  pSecondString, 0, iFirstValue, uiSecondValue);
}

qResult NgsFacade::Login(ProtocolCallContext* pCallContext, unsigned int uiLoginParam, const wchar_t* pSandboxAccessKey,
                         int iFirstValue, unsigned int uiSecondValue)
{
    return LoginAndRequestAuthenticationToken(pCallContext, uiLoginParam, pSandboxAccessKey, 0, 0, iFirstValue,
                                              uiSecondValue);
}

qResult NgsFacade::Login(ProtocolCallContext* pCallContext, const void* pToken, const wchar_t* pSandboxAccessKey,
                         int iFirstValue, unsigned int uiSecondValue)
{
    return LoginAndRequestAuthenticationTokenImpl(pCallContext, 0, pSandboxAccessKey, 0, 0, pToken, iFirstValue,
                                                  uiSecondValue);
}

qResult NgsFacade::Logout(ProtocolCallContext* pCallContext)
{
    if (pCallContext == 0)
        return qResult(ERROR_INVALID_ARGUMENT);

    if (m_pLoginJob != 0 && m_pLoginJob->GetState() != Job::Complete)
        return qResult(ERROR_BUSY);

    if (!RendezVous::Logout(pCallContext, m_pCredentials))
        return qResult(ERROR_CALL_NOT_INITIATED);

    return qResult(SUCCESS_RESULT);
}

bool NgsFacade::LoginJobIsInProgress() const
{
    return m_pLoginJob != 0 && m_pLoginJob->GetState() != Job::Complete;
}

int NgsFacade::GetLastLoginErrorCode() const
{
    if (m_loginResult == qResult(ERROR_ACCOUNT_FAILED))
    {
        if (m_uiLoginErrorCode != 0)
            return m_uiLoginErrorCode;

        if (m_iHttpStatusCode != HTTP_STATUS_OK)
            return g_httpErrorCodeBase + m_iHttpStatusCode;

        return g_nasErrorCodeBase + m_iNasErrorCode;
    }

    return ErrorCodeConverter::ConvertToNetworkErrorCode(m_loginResult);
}

}
}
