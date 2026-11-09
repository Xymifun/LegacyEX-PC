#include "nn/nex/JobNNIDLogin.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/NgsFacade.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

namespace
{

const int WAIT_CONTEXT_LOGIN = 250;
const unsigned int AUTHENTICATION_VALUE_UNLIMITED = 0xFFFFFFFF;

enum LoginResult
{
    LOGIN_CALL_FAILED_RESULT = static_cast<int>(0x8001000Du)
};

}

JobNNIDLogin::JobNNIDLogin() : JobLogin(), m_pToken(0), m_uiPrincipalId(0)
{
}

JobNNIDLogin::~JobNNIDLogin()
{
}

void JobNNIDLogin::Initialize(unsigned int uiCallId, unsigned int uiLoginParam, NgsFacade* pNgsFacade,
                              const char* pFirstString, char* pSecondString, const void* pToken, int iTimeout,
                              unsigned int uiPrincipalId)
{
    JobLogin::Initialize(uiCallId, uiLoginParam, pNgsFacade, pFirstString, pSecondString, iTimeout);
    m_pToken = static_cast<const ACTNexAuthenticationResult*>(pToken);
    m_uiPrincipalId = uiPrincipalId;
}

void JobNNIDLogin::StepFirst()
{
    SetStep(Step(static_cast<Step::Function>(&JobNNIDLogin::StepGameLogin), L""));
}

void JobNNIDLogin::StepGameLogin()
{
    const ACTNexAuthenticationResult* pToken = m_pToken;
    m_protocolContext.Reset();
    m_pNgsFacade->m_iDefaultLoginTimeout = m_iTimeout;
    m_authenticationInfo = CustomDataHolder<AuthenticationInfo>(new AuthenticationInfo);
    AuthenticationInfo* pInfo = m_authenticationInfo.GetObject();
    pInfo->m_Name = String(pToken->token);
    {
        StringStream userName;
        userName.StreamNumber(m_uiPrincipalId);
        String host(pToken->host);
        if (m_pNgsFacade->m_reserved1)
            m_authenticationInfo.GetObject()->m_uiValue3 = AUTHENTICATION_VALUE_UNLIMITED;

        unsigned short uiPort = pToken->port;
        m_authenticationInfo.GetObject()->m_uiValue2 = true;
        if (!m_pNgsFacade->RendezVous::Login(&m_protocolContext, userName.GetBuffer(), pToken->password, host, uiPort,
                                             m_authenticationInfo, &m_pCredentials, 0))
        {
            Complete(qResult(LOGIN_CALL_FAILED_RESULT));
            return;
        }
    }

    SetStep(Step(static_cast<Step::Function>(&JobNNIDLogin::StepWaitingForGameLogin), L""));
}

void JobNNIDLogin::StepWaitingForGameLogin()
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
    Complete(result);
}

}
}
