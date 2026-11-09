#include "nn/nex/JobLoginOrCreateAccount.hpp"

#include "nn/nex/AccountManagementCommand.hpp"
#include "nn/nex/BackEndServices.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum LoginOrCreateAccountResult
{
    LOGIN_TIMED_OUT_RESULT = static_cast<int>(0x8001000Bu),
    LOGIN_CALL_FAILED_RESULT = static_cast<int>(0x8001000Du),
    LOGIN_ACCOUNT_NOT_FOUND_RESULT = static_cast<int>(0x80030064u)
};

}

JobLoginOrCreateAccount::~JobLoginOrCreateAccount()
{
    delete m_pCreateAccountCommand;
}

void JobLoginOrCreateAccount::RVLogin()
{
    if (!m_pBackEndServices->EffectiveTimeoutIsSet())
    {
        int iTimeout = static_cast<int>(m_deadline - Time::GetTime());
        if (iTimeout < 0)
        {
            CompleteJob(qResult(LOGIN_TIMED_OUT_RESULT));
            return;
        }
        m_pBackEndServices->SetEffectiveLoginTimeout(iTimeout);
    }
    char* pPassword = 0;
    m_password.CreateCopy(&pPassword);
    const SandboxConnectionInfo::HostCredentials& rCredentials = m_sandboxConnectionInfo.GetCredentialsProd();
    if (!m_pBackEndServices->LoginImpl(&m_loginContext, 0, m_name, pPassword, rCredentials.m_host,
                                       rCredentials.m_uiPort, m_ppCredentials, m_pErrorMessage, m_pExtraData))
    {
        CompleteJob(qResult(LOGIN_CALL_FAILED_RESULT));
    }
    else
    {
        SetToSuspended();
        ResumeOnCallCompletion(
            &m_loginContext,
            new Step(static_cast<Step::Function>(&JobLoginOrCreateAccount::ProcessRVLoginResult), L""));
    }
    String::ReleaseCopy(pPassword);
}

void JobLoginOrCreateAccount::ProcessRVLoginResult()
{
    CallContext::State state = m_loginContext.GetState();
    if (state == CallContext::CallSucceeded)
    {
        CompleteJob(m_loginContext.GetResult());
        return;
    }
    if (m_loginContext.GetResult() == qResult(LOGIN_ACCOUNT_NOT_FOUND_RESULT))
    {
        if (!m_bCreateAccountAttempted)
        {
            m_bCreateAccountAttempted = true;
            SetStep(Step(static_cast<Step::Function>(&JobLoginOrCreateAccount::CreateAccount), L""));
        }
        else
        {
            CompleteJob(m_loginContext.GetResult());
        }
    }
    else
    {
        CompleteJob(m_loginContext.GetResult());
    }
}

void JobLoginOrCreateAccount::CreateAccount()
{
    AccountManagementCommand* pCommand = m_pCreateAccountCommand;
    if (!pCommand->Invoke(&m_createContext))
    {
        CompleteJob(qResult(LOGIN_CALL_FAILED_RESULT));
        return;
    }
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_createContext,
        new Step(static_cast<Step::Function>(&JobLoginOrCreateAccount::ProcessCreateAccountResult), L""));
}

void JobLoginOrCreateAccount::ProcessCreateAccountResult()
{
    if (m_createContext.GetState() == CallContext::CallSucceeded)
    {
        m_loginContext.Reset();
        SetStep(Step(static_cast<Step::Function>(&JobLoginOrCreateAccount::RVLogin), L""));
        return;
    }
    CompleteJob(m_createContext.GetResult());
}

void JobLoginOrCreateAccount::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        if (rResult == true)
            pCallContext->SignalSuccess(rResult);
        else
            pCallContext->SignalFailure(rResult);
    }
    SetToComplete();
}

}
}
