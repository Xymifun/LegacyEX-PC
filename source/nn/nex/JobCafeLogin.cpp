#include "nn/nex/JobCafeLogin.hpp"

#include <string.h>
#include <coreinit/debug.h>
#include <coreinit/event.h>
#include <nn/act/client_cpp.h>
#include <nn/fp/fp_cpp.h>

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/GlobalVariables.hpp"
#include "nn/nex/NgsFacade.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace act
{

nn::Result AcquireNexServiceToken(ACTNexAuthenticationResult* pResult, unsigned int uiGameId) asm(
    "AcquireNexServiceToken__Q2_2nn3actFP26ACTNexAuthenticationResultUi");

}
}

namespace nn
{
namespace nex
{

namespace
{

const int WAIT_CONTEXT_LOGIN = 250;
const int WAIT_FOREVER = -1;
const unsigned int AUTHENTICATION_THREAD_STACK_SIZE = 0;
const unsigned int AUTHENTICATION_THREAD_WAIT_FOREVER = 0xFFFFFFFF;
const unsigned int AUTHENTICATION_VALUE_UNLIMITED = 0xFFFFFFFF;

enum LoginResult
{
    LOGIN_CALL_FAILED_RESULT = static_cast<int>(0x8001000Du),
    LOGIN_TIMED_OUT_RESULT = static_cast<int>(0x8001000Bu),
    LOGIN_THREAD_FAILED_RESULT = static_cast<int>(0x8001000Cu),
    LOGIN_NOT_FOUND_RESULT = static_cast<int>(0x80010001u),
    LOGIN_ACCOUNT_FAILED_RESULT = static_cast<int>(0x80680001u),
    LOGIN_TOKEN_EXPIRED_RESULT = static_cast<int>(0x80680006u)
};

const NNResult INVALID_RESULT = {-1};

OSEvent s_authenticationEvent;
nn::Result s_authenticationResult(INVALID_RESULT);
ACTNexAuthenticationResult s_nexAuthentication;
nn::Result s_registerAccountResult(INVALID_RESULT);

void RegisterAccountAsyncCallback(nn::Result result, void*)
{
    s_registerAccountResult = result;
    OSSignalEvent(&s_authenticationEvent);
}

}

JobCafeLogin::JobCafeLogin(unsigned int uiPrincipalId)
    : JobLogin(),
      m_pAuthenticationThread(0),
      m_uiPrincipalId(uiPrincipalId)
{
    if (uiPrincipalId == 0)
        m_uiPrincipalId = nn::act::GetPrincipalId();
}

JobCafeLogin::~JobCafeLogin()
{
    if (m_pAuthenticationThread != 0)
    {
        if (m_pAuthenticationThread->IsLaunched() && !m_pAuthenticationThread->IsFinished())
            m_pAuthenticationThread->Wait(AUTHENTICATION_THREAD_WAIT_FOREVER);

        if (m_pAuthenticationThread != 0)
        {
            delete m_pAuthenticationThread;
            m_pAuthenticationThread = 0;
        }
    }
}

void JobCafeLogin::StepFirst()
{
    SetStep(Step(static_cast<Step::Function>(&JobCafeLogin::StepRegisterAccount), L""));
}

void JobCafeLogin::StepRegisterAccount()
{
    if (nn::act::HasNfsAccount())
    {
        SetStep(Step(static_cast<Step::Function>(&JobCafeLogin::StepGetGameAuthentication), L""));
        return;
    }

    OSInitEvent(&s_authenticationEvent, 0, OS_EVENT_MODE_MANUAL);
    nn::Result result = nn::fp::RegisterAccountAsync(RegisterAccountAsyncCallback, 0);
    if (result.IsSuccess())
    {
        SetStep(Step(static_cast<Step::Function>(&JobCafeLogin::StepWaitingForRegisterAccount), L""));
        return;
    }

    m_pNgsFacade->m_uiLoginErrorCode = nn::fp::ResultToErrorCode(result);
    Complete(qResult(LOGIN_ACCOUNT_FAILED_RESULT));
}

void JobCafeLogin::StepWaitingForRegisterAccount()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetToComplete();
        return;
    }

    if (OSWaitEventWithTimeout(&s_authenticationEvent, 0))
    {
        if (s_registerAccountResult.IsSuccess())
        {
            SetStep(Step(static_cast<Step::Function>(&JobCafeLogin::StepGetGameAuthentication), L""));
            return;
        }

        m_pNgsFacade->m_uiLoginErrorCode = nn::fp::ResultToErrorCode(s_registerAccountResult);
        Complete(qResult(LOGIN_ACCOUNT_FAILED_RESULT));
        return;
    }

    if (!Wait())
        Complete(qResult(LOGIN_TIMED_OUT_RESULT));
}

void JobCafeLogin::StepGetGameAuthentication()
{
    GlobalVariables* pGlobals = GlobalVariables::GetInstancePtr();
    if (pGlobals->m_nexTokenMap.find(m_uiPrincipalId) != pGlobals->m_nexTokenMap.end()
        && pGlobals->m_nexPasswordMap.find(m_uiPrincipalId) != pGlobals->m_nexPasswordMap.end()
        && pGlobals->m_nexHostMap.find(m_uiPrincipalId) != pGlobals->m_nexHostMap.end()
        && pGlobals->m_nexPortMap.find(m_uiPrincipalId) != pGlobals->m_nexPortMap.end())
    {
        SetStep(Step(static_cast<Step::Function>(&JobCafeLogin::StepGameLogin), L""));
        return;
    }

    OSInitEvent(&s_authenticationEvent, 0, OS_EVENT_MODE_MANUAL);
    if (m_pAuthenticationThread != 0)
    {
        if (m_pAuthenticationThread->IsLaunched() && !m_pAuthenticationThread->IsFinished())
            m_pAuthenticationThread->Wait(AUTHENTICATION_THREAD_WAIT_FOREVER);

        delete m_pAuthenticationThread;
        m_pAuthenticationThread = 0;
    }

    m_pAuthenticationThread = new AuthenticationThread(String(L"Authentication Thread"));
    m_pAuthenticationThread->SetObjectMethod(&JobCafeLogin::NexAuthenticationFunc, this, 0);
    if (m_pAuthenticationThread->Launch(AUTHENTICATION_THREAD_STACK_SIZE))
    {
        SetStep(Step(static_cast<Step::Function>(&JobCafeLogin::StepWaitingForGetGameAuthentication), L""));
        return;
    }

    Complete(qResult(LOGIN_THREAD_FAILED_RESULT));
}

void JobCafeLogin::StepWaitingForGetGameAuthentication()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetToComplete();
        return;
    }

    if (OSWaitEventWithTimeout(&s_authenticationEvent, 0))
    {
        if (s_authenticationResult.IsSuccess())
        {
            GlobalVariables* pGlobals = GlobalVariables::GetInstancePtr();
            pGlobals->m_nexTokenMap[m_uiPrincipalId] = s_nexAuthentication.token;
            pGlobals->m_nexPasswordMap[m_uiPrincipalId] = s_nexAuthentication.password;
            pGlobals->m_nexHostMap[m_uiPrincipalId] = s_nexAuthentication.host;
            pGlobals->m_nexPortMap[m_uiPrincipalId] = s_nexAuthentication.port;
            SetStep(Step(static_cast<Step::Function>(&JobCafeLogin::StepGameLogin), L""));
            return;
        }

        m_pNgsFacade->m_uiLoginErrorCode = nn::fp::ResultToErrorCode(s_authenticationResult);
        Complete(qResult(LOGIN_ACCOUNT_FAILED_RESULT));
        return;
    }

    if (!Wait())
        Complete(qResult(LOGIN_TIMED_OUT_RESULT));
}

void JobCafeLogin::StepGameLogin()
{
    GlobalVariables* pGlobals = GlobalVariables::GetInstancePtr();
    if (pGlobals->m_nexTokenMap.find(m_uiPrincipalId) == pGlobals->m_nexTokenMap.end()
        || pGlobals->m_nexPasswordMap.find(m_uiPrincipalId) == pGlobals->m_nexPasswordMap.end()
        || pGlobals->m_nexHostMap.find(m_uiPrincipalId) == pGlobals->m_nexHostMap.end()
        || pGlobals->m_nexPortMap.find(m_uiPrincipalId) == pGlobals->m_nexPortMap.end())
    {
        Complete(qResult(LOGIN_NOT_FOUND_RESULT));
        return;
    }

    if (m_iTimeout == WAIT_FOREVER)
    {
        m_pNgsFacade->m_iDefaultLoginTimeout = WAIT_FOREVER;
    }
    else
    {
        int iElapsed = static_cast<int>(Time::GetTime() - m_startTime);
        if (m_iTimeout <= iElapsed)
        {
            Complete(qResult(LOGIN_TIMED_OUT_RESULT));
            return;
        }

        m_pNgsFacade->m_iDefaultLoginTimeout = m_iTimeout - iElapsed;
    }

    m_protocolContext.Reset();
    m_authenticationInfo = CustomDataHolder<AuthenticationInfo>(new AuthenticationInfo);
    if (m_pNgsFacade->m_reserved1)
        m_authenticationInfo.GetObject()->m_uiValue3 = AUTHENTICATION_VALUE_UNLIMITED;

    m_authenticationInfo.GetObject()->m_Name = pGlobals->m_nexTokenMap[m_uiPrincipalId];
    StringStream userName;
    userName.StreamNumber(m_uiPrincipalId);
    String host(pGlobals->m_nexHostMap[m_uiPrincipalId]);
    unsigned short uiPort = pGlobals->m_nexPortMap[m_uiPrincipalId];
    char* pPassword;
    pGlobals->m_nexPasswordMap[m_uiPrincipalId].CreateCopy(&pPassword);
    bool bLoggingIn = m_pNgsFacade->RendezVous::Login(&m_protocolContext, userName.GetBuffer(),
                                                      s_nexAuthentication.password, host, uiPort, m_authenticationInfo,
                                                      &m_pCredentials, &m_errorMessage);
    String::ReleaseCopy(pPassword);
    if (!bLoggingIn)
    {
        Complete(qResult(LOGIN_CALL_FAILED_RESULT));
        return;
    }

    SetStep(Step(static_cast<Step::Function>(&JobCafeLogin::StepWaitingForGameLogin), L""));
}

void JobCafeLogin::StepWaitingForGameLogin()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetToComplete();
        return;
    }

    if (m_protocolContext.GetState() != CallContext::CallSucceeded
        && m_protocolContext.GetState() != CallContext::CallFailed
        && m_protocolContext.GetState() != CallContext::CallCanceled)
    {
        SetToWaiting(WAIT_CONTEXT_LOGIN);
        return;
    }

    qResult result = m_protocolContext.GetResult();
    if (!(result == qResult(LOGIN_TOKEN_EXPIRED_RESULT)))
    {
        Complete(result);
        return;
    }

    GlobalVariables* pGlobals = GlobalVariables::GetInstancePtr();
    pGlobals->m_nexTokenMap.erase(m_uiPrincipalId);
    pGlobals->m_nexPasswordMap.erase(m_uiPrincipalId);
    pGlobals->m_nexHostMap.erase(m_uiPrincipalId);
    pGlobals->m_nexPortMap.erase(m_uiPrincipalId);
    SetStep(Step(static_cast<Step::Function>(&JobCafeLogin::StepGetGameAuthentication), L""));
}

void JobCafeLogin::NexAuthenticationFunc(void*)
{
    memset(&s_nexAuthentication, 0, sizeof(s_nexAuthentication));
    s_authenticationResult = nn::act::AcquireNexServiceToken(&s_nexAuthentication, m_uiLoginParam);
    OSReport("GameID:%08x Success:%d AcquireNexServiceTokenResult:%08x\n", m_uiLoginParam,
             s_authenticationResult.IsSuccess(), NNResult(s_authenticationResult).value);
    OSSignalEvent(&s_authenticationEvent);
}

}
}
