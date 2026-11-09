#include "nn/nex/JobTicketManagerLoginWithData.hpp"

#include <string.h>

#include "nn/nex/AuthenticationClient.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CustomDataHolder.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/KerberosEncryption.hpp"
#include "nn/nex/LoginData.hpp"
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
    KEY_DERIVATION_FAILED_RESULT = static_cast<int>(0x80010001u),
    TICKET_DECRYPTION_FAILED_RESULT = static_cast<int>(0x80030065u),
    TICKET_VALIDATION_FAILED_RESULT = static_cast<int>(0x8003006Au),
    LOGIN_SUCCEEDED_RESULT = 0x10001
};

}

JobTicketManagerLoginWithData::JobTicketManagerLoginWithData(unsigned int uiCallId, TicketManager* pTicketManager,
                                                             qResult* pResult, const char* pPassword,
                                                             unsigned int* pPrincipalID,
                                                             RVConnectionData* pConnectionData,
                                                             CustomDataHolder<LoginData>* pLoginData)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_keyContext(),
      m_key(),
      m_pLoginData(pLoginData),
      m_loginContext(0),
      m_ticketData()
{
    m_pPrincipalID = pPrincipalID;
    m_uiCallId = uiCallId;
    m_pTicketManager = pTicketManager;
    m_pConnectionData = pConnectionData;
    m_pResult = pResult;
    m_pPassword = SpecialNewArray<char>(strlen(pPassword) + 1, L"", 0);
    strcpy(m_pPassword, pPassword);
    SetStep(Step(static_cast<Step::Function>(&JobTicketManagerLoginWithData::InitiateLogin), L""));
}

JobTicketManagerLoginWithData::~JobTicketManagerLoginWithData()
{
    if (m_pPassword != 0)
        SpecialDeleteArray(m_pPassword);
}

void JobTicketManagerLoginWithData::CompleteJob(CallContext::State state, qResult result)
{
    SetToComplete();
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
        pCallContext->SetStateImpl(state, result, true);
}

void JobTicketManagerLoginWithData::InitiateLogin()
{
    AuthenticationClient* pAuthenticationClient = m_pTicketManager->m_pAuthenticationClient;
    m_loginContext.m_expirationTime = pAuthenticationClient->m_loginDeadline;
    if (!m_pTicketManager->m_pAuthenticationClient->m_pTicketGrantingProtocolClient->CallLoginWithContext(
            &m_loginContext, m_pResult, *m_pLoginData, m_pPrincipalID, &m_ticketData, m_pConnectionData))
    {
        CompleteJob(CallContext::CallFailed, qResult(CALL_INITIATION_FAILED_RESULT));
        return;
    }
    if (m_loginContext.GetState() == CallContext::CallInProgress)
    {
        SetToSuspended();
        ResumeOnCallCompletion(
            &m_loginContext,
            new Step(static_cast<Step::Function>(&JobTicketManagerLoginWithData::ProcessLoginResult), L""));
        return;
    }
    SetStep(Step(static_cast<Step::Function>(&JobTicketManagerLoginWithData::ProcessLoginResult), L""));
}

void JobTicketManagerLoginWithData::ProcessLoginResult()
{
    if (m_loginContext.GetState() == CallContext::CallSucceeded)
    {
        if (*m_pResult == true)
        {
            SetStep(Step(static_cast<Step::Function>(&JobTicketManagerLoginWithData::DeriveKey), L""));
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

void JobTicketManagerLoginWithData::DeriveKey()
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
                               new Step(static_cast<Step::Function>(&JobTicketManagerLoginWithData::ValidateKey), L""));
    }
    else
    {
        SetStep(Step(static_cast<Step::Function>(&JobTicketManagerLoginWithData::ValidateKey), L""));
    }
}

void JobTicketManagerLoginWithData::ValidateKey()
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
        CompleteJob(m_loginContext.GetState(), qResult(LOGIN_SUCCEEDED_RESULT));
    }
    else
    {
        CompleteJob(m_keyContext.GetState(), m_keyContext.GetResult());
    }
}

}
}
