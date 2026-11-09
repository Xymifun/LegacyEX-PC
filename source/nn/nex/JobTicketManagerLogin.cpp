#include "nn/nex/JobTicketManagerLogin.hpp"

#include <string.h>

#include "nn/nex/AuthenticationClient.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/KerberosEncryption.hpp"
#include "nn/nex/SpecialArray.hpp"
#include "nn/nex/TicketGrantingProtocolClient.hpp"
#include "nn/nex/TicketManager.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum LoginResult
{
    CALL_INITIATION_FAILED_RESULT = static_cast<int>(0x8001000Du),
    LOGIN_RESULT_PENDING = static_cast<int>(0x8001000Bu),
    KEY_DERIVATION_FAILED_RESULT = static_cast<int>(0x80010001u),
    TICKET_DECRYPTION_FAILED_RESULT = static_cast<int>(0x80030065u),
    TICKET_VALIDATION_FAILED_RESULT = static_cast<int>(0x8003006Au),
    LOGIN_SUCCEEDED_RESULT = 0x10001
};

}

JobTicketManagerLogin::JobTicketManagerLogin(unsigned int uiCallId, TicketManager* pTicketManager, qResult* pResult,
                                             const String& rUserName, const char* pPassword, unsigned int* pPrincipalID,
                                             RVConnectionData* pConnectionData, String* pReturnMessage,
                                             AnyObjectHolder<Data, String>* pCustomData)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_keyContext(),
      m_key(),
      m_userName(),
      m_loginContext(0),
      m_ticketData()
{
    m_uiCallId = uiCallId;
    m_pTicketManager = pTicketManager;
    m_userName = rUserName;
    m_pPassword = SpecialNewArray<char>(strlen(pPassword) + 1, L"", 0);
    strcpy(m_pPassword, pPassword);
    m_pReturnMessage = pReturnMessage;
    m_pPrincipalID = pPrincipalID;
    m_pResult = pResult;
    m_pCustomData = pCustomData;
    m_pConnectionData = pConnectionData;
    SetStep(Step(static_cast<Step::Function>(&JobTicketManagerLogin::InitiateLogin), L""));
}

JobTicketManagerLogin::~JobTicketManagerLogin()
{
    if (m_pPassword != 0)
        SpecialDeleteArray(m_pPassword);
}

void JobTicketManagerLogin::CompleteJob(CallContext::State state, qResult result)
{
    SetToComplete();
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
        pCallContext->SetStateImpl(state, result, true);
}

void JobTicketManagerLogin::InitiateLogin()
{
    AuthenticationClient* pAuthenticationClient = m_pTicketManager->m_pAuthenticationClient;
    m_loginContext.m_expirationTime = pAuthenticationClient->m_loginDeadline;
    if (m_pCustomData == 0)
    {
        if (!pAuthenticationClient->m_pTicketGrantingProtocolClient->CallLogin(&m_loginContext, m_pResult, m_userName,
                                                                               m_pPrincipalID, &m_ticketData,
                                                                               m_pConnectionData, m_pReturnMessage))
        {
            CompleteJob(CallContext::CallFailed, qResult(CALL_INITIATION_FAILED_RESULT));
            return;
        }
    }
    else
    {
        if (!m_pTicketManager->m_pAuthenticationClient->m_pTicketGrantingProtocolClient->CallLoginEx(
                &m_loginContext, m_pResult, m_userName, m_pPrincipalID, &m_ticketData, m_pConnectionData,
                m_pReturnMessage, *m_pCustomData))
        {
            CompleteJob(CallContext::CallFailed, qResult(CALL_INITIATION_FAILED_RESULT));
            return;
        }
    }
    if (m_loginContext.GetState() == CallContext::CallInProgress)
    {
        SetToSuspended();
        ResumeOnCallCompletion(&m_loginContext,
                               new Step(static_cast<Step::Function>(&JobTicketManagerLogin::ProcessLoginResult), L""));
        return;
    }
    SetStep(Step(static_cast<Step::Function>(&JobTicketManagerLogin::ProcessLoginResult), L""));
}

void JobTicketManagerLogin::ProcessLoginResult()
{
    if (m_loginContext.GetState() == CallContext::CallSucceeded)
    {
        if (*m_pResult == true)
        {
            *m_pResult = qResult(LOGIN_RESULT_PENDING);
            SetStep(Step(static_cast<Step::Function>(&JobTicketManagerLogin::DeriveKey), L""));
        }
        else
        {
            CompleteJob(m_loginContext.GetState(), m_loginContext.GetResult());
        }
    }
    else
    {
        CompleteJob(CallContext::CallFailed, m_loginContext.GetResult());
    }
}

void JobTicketManagerLogin::DeriveKey()
{
    if (g_dispatcherThreadEnabled)
    {
        m_key = KerberosEncryption::CreateKey(*m_pPrincipalID, m_pPassword);
    }
    else
    {
        m_keyContext.m_expirationTime = m_pTicketManager->m_pAuthenticationClient->m_loginDeadline;
        if (!KerberosEncryption::CreateKey(&m_keyContext, *m_pPrincipalID, m_pPassword, &m_key))
        {
            CompleteJob(CallContext::CallFailed, qResult(KEY_DERIVATION_FAILED_RESULT));
            return;
        }
    }
    if (m_keyContext.GetState() == CallContext::CallInProgress)
    {
        SetToSuspended();
        ResumeOnCallCompletion(&m_keyContext,
                               new Step(static_cast<Step::Function>(&JobTicketManagerLogin::ValidateKey), L""));
    }
    else
    {
        SetStep(Step(static_cast<Step::Function>(&JobTicketManagerLogin::ValidateKey), L""));
    }
}

void JobTicketManagerLogin::ValidateKey()
{
    if (m_keyContext.GetState() == CallContext::CallSucceeded || g_dispatcherThreadEnabled)
    {
        m_pTicketManager->m_pAuthenticationClient->m_pKerberosEncryption->SetKey(m_key);
        Buffer decryptedTicket;
        if (!m_pTicketManager->m_pAuthenticationClient->m_pKerberosEncryption->Decrypt(m_ticketData, &decryptedTicket))
        {
            *m_pResult = qResult(TICKET_DECRYPTION_FAILED_RESULT);
            CompleteJob(CallContext::CallSucceeded, qResult(TICKET_VALIDATION_FAILED_RESULT));
            return;
        }
        *m_pResult = qResult(LOGIN_SUCCEEDED_RESULT);
        CompleteJob(m_loginContext.GetState(), qResult(LOGIN_SUCCEEDED_RESULT));
    }
    else
    {
        CompleteJob(m_keyContext.GetState(), m_keyContext.GetResult());
    }
}

}
}
