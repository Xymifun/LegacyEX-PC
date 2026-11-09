#include "nn/nex/JobManageAccount.hpp"

#include "nn/nex/AccountManagementCommand.hpp"
#include "nn/nex/AccountManagementProtocolClient.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/KerberosEncryption.hpp"

namespace nn
{
namespace nex
{

namespace
{

const int MANAGE_ACCOUNT_CALL_FAILED_RESULT = static_cast<int>(0x8001000Du);

}

JobManageAccount::JobManageAccount(unsigned int uiCallId, AccountManagementProtocolClient* pProtocolClient,
                                   const String& rName, const String& rPassword, unsigned int uiGroups,
                                   const String& rEmail, qResult* pResult, FuncType funcType,
                                   AnyObjectHolder<Data, String>* pPublicData,
                                   AnyObjectHolder<Data, String>* pPrivateData,
                                   AnyObjectHolder<Data, String>* pCustomData, unsigned int* pPrincipalID,
                                   String* pPidHmac)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(uiCallId),
      m_pProtocolClient(pProtocolClient),
      m_name(rName),
      m_uiGroups(uiGroups),
      m_email(rEmail),
      m_funcType(funcType),
      m_publicData(),
      m_privateData(),
      m_customData(),
      m_pPrincipalID(pPrincipalID),
      m_pPidHmac(pPidHmac),
      m_key(),
      m_pResult(pResult),
      m_pSuccess(0),
      m_keyContext(),
      m_callContext(0),
      m_pCommand(0)
{
    rPassword.CreateCopy(&m_pPassword);
    if (pPublicData != 0 && pPublicData->GetObject() != 0)
        m_publicData.SetObject(pPublicData->GetObject()->Clone());
    if (pPrivateData != 0 && pPrivateData->GetObject() != 0)
        m_privateData.SetObject(pPrivateData->GetObject()->Clone());
    if (pCustomData != 0 && pCustomData->GetObject() != 0)
        m_customData.SetObject(pCustomData->GetObject()->Clone());
    else
        m_customData.SetObject(new Data());
    SetStep(Step(static_cast<Step::Function>(&JobManageAccount::PasswordInitialHash), L""));
}

JobManageAccount::JobManageAccount(unsigned int uiCallId, AccountManagementProtocolClient* pProtocolClient,
                                   AccountManagementCommand* pCommand)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(uiCallId),
      m_pProtocolClient(pProtocolClient),
      m_name(),
      m_email(),
      m_publicData(),
      m_privateData(),
      m_customData(),
      m_key(),
      m_keyContext(),
      m_callContext(0),
      m_pCommand(pCommand)
{
    const String& rPassword = pCommand->m_strPassword;
    rPassword.CreateCopy(&m_pPassword);
    SetStep(Step(static_cast<Step::Function>(&JobManageAccount::PasswordInitialHash), L""));
}

JobManageAccount::JobManageAccount(unsigned int uiCallId, AccountManagementProtocolClient* pProtocolClient,
                                   const String& rPassword, bool* pSuccess, FuncType funcType)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(uiCallId),
      m_pProtocolClient(pProtocolClient),
      m_name(),
      m_uiGroups(0),
      m_email(),
      m_funcType(funcType),
      m_publicData(),
      m_privateData(),
      m_customData(),
      m_key(),
      m_pResult(0),
      m_pSuccess(pSuccess),
      m_keyContext(),
      m_callContext(0),
      m_pCommand(0)
{
    rPassword.CreateCopy(&m_pPassword);
    SetStep(Step(static_cast<Step::Function>(&JobManageAccount::PasswordInitialHash), L""));
}

void JobManageAccount::Cleanup()
{
    if (m_publicData.GetObject() != 0)
        delete m_publicData.Release();
    if (m_privateData.GetObject() != 0)
        delete m_privateData.Release();
    m_keyContext.Cancel(CallContext::CallCanceled);
    if (m_pCommand != 0)
    {
        delete m_pCommand;
        m_pCommand = 0;
    }
}

JobManageAccount::~JobManageAccount()
{
    String::ReleaseCopy(m_pPassword);
    Cleanup();
}

void JobManageAccount::Cancel()
{
    Cleanup();
    SetToComplete();
}

void JobManageAccount::CompleteJobImpl(CallContext::State state, qResult result)
{
    SetToComplete();
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
        pCallContext->SetStateImpl(state, result, true);
}

void JobManageAccount::PasswordInitialHash()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        Cancel();
        return;
    }
    ResumeOnCallCompletion(
        &m_keyContext, new Step(static_cast<Step::Function>(&JobManageAccount::PasswordInitialHashCompletion), L""));
    if (KerberosEncryption::InitializeKey(&m_keyContext, m_pPassword, &m_key))
    {
        if (m_keyContext.GetState() == CallContext::CallInProgress)
            SetToSuspended();
        else
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    }
    else
    {
        CompleteJobImpl(CallContext::CallFailed, qResult(MANAGE_ACCOUNT_CALL_FAILED_RESULT));
    }
}

void JobManageAccount::PasswordInitialHashCompletion()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        Cancel();
        return;
    }
    if (m_keyContext.GetState() == CallContext::CallSucceeded)
    {
        if (m_pCommand != 0)
        {
            m_pCommand->m_strKey = m_key.ToString();
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CallAccountManagementCommand), L""));
            return;
        }
        switch (m_funcType)
        {
        case CreateAccountFunc:
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CallCreateAccount), L""));
            break;
        case CreateAccountWithCustomDataFunc:
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CallCreateAccountWithCustomData), L""));
            break;
        case ChangePasswordFunc:
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CallChangePassword), L""));
            break;
        case UpdateAccountFunc:
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CallUpdateAccount), L""));
            break;
        case ChangePasswordByGuestFunc:
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CallChangePasswordByGuest), L""));
            break;
        case CustomCreateAccountFunc:
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CallCustomCreateAccount), L""));
            break;
        case NintendoCreateAccountFunc:
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CallNintendoCreateAccount), L""));
            break;
        }
    }
    else
    {
        SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    }
}

void JobManageAccount::CallCreateAccount()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        Cancel();
        return;
    }
    m_callContext.Reset();
    ResumeOnCallCompletion(&m_callContext, new Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    m_callContext.m_completionResult = m_pResult;
    if (m_pProtocolClient->CallCreateAccount(&m_callContext, m_pResult, m_name, m_key.ToString(), m_uiGroups, m_email))
    {
        if (m_callContext.GetState() == CallContext::CallInProgress)
            SetToSuspended();
        else
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    }
    else
    {
        CompleteJobImpl(CallContext::CallFailed, qResult(MANAGE_ACCOUNT_CALL_FAILED_RESULT));
    }
}

void JobManageAccount::CallCreateAccountWithCustomData()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        Cancel();
        return;
    }
    m_callContext.Reset();
    ResumeOnCallCompletion(&m_callContext, new Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    if (m_pProtocolClient->CallCreateAccountWithCustomData(&m_callContext, m_name, m_key.ToString(), m_uiGroups,
                                                           m_email, m_publicData, m_privateData))
    {
        if (m_callContext.GetState() == CallContext::CallInProgress)
            SetToSuspended();
        else
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    }
    else
    {
        CompleteJobImpl(CallContext::CallFailed, qResult(MANAGE_ACCOUNT_CALL_FAILED_RESULT));
    }
}

void JobManageAccount::CallChangePassword()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        Cancel();
        return;
    }
    m_callContext.Reset();
    ResumeOnCallCompletion(&m_callContext, new Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    m_callContext.m_completionFlag = m_pSuccess;
    if (m_pProtocolClient->CallChangePassword(&m_callContext, m_pSuccess, m_key.ToString()))
    {
        if (m_callContext.GetState() == CallContext::CallInProgress)
            SetToSuspended();
        else
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    }
    else
    {
        CompleteJobImpl(CallContext::CallFailed, qResult(MANAGE_ACCOUNT_CALL_FAILED_RESULT));
    }
}

void JobManageAccount::CallUpdateAccount()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        Cancel();
        return;
    }
    m_callContext.Reset();
    ResumeOnCallCompletion(&m_callContext, new Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    if (m_pProtocolClient->CallUpdateAccount(&m_callContext, m_key.ToString(), m_email, m_publicData, m_privateData))
    {
        if (m_callContext.GetState() == CallContext::CallInProgress)
            SetToSuspended();
        else
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    }
    else
    {
        CompleteJobImpl(CallContext::CallFailed, qResult(MANAGE_ACCOUNT_CALL_FAILED_RESULT));
    }
}

void JobManageAccount::CallChangePasswordByGuest()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        Cancel();
        return;
    }
    m_callContext.Reset();
    ResumeOnCallCompletion(&m_callContext, new Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    if (m_pProtocolClient->CallChangePasswordByGuest(&m_callContext, m_name, m_email, m_key.ToString()))
    {
        if (m_callContext.GetState() == CallContext::CallInProgress)
            SetToSuspended();
        else
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    }
    else
    {
        CompleteJobImpl(CallContext::CallFailed, qResult(MANAGE_ACCOUNT_CALL_FAILED_RESULT));
    }
}

void JobManageAccount::CallCustomCreateAccount()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        Cancel();
        return;
    }
    m_callContext.Reset();
    ResumeOnCallCompletion(&m_callContext, new Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    if (m_pProtocolClient->CallCustomCreateAccount(&m_callContext, m_name, m_key.ToString(), m_uiGroups, m_email,
                                                   m_customData, m_pPrincipalID))
    {
        if (m_callContext.GetState() == CallContext::CallInProgress)
            SetToSuspended();
        else
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    }
    else
    {
        CompleteJobImpl(CallContext::CallFailed, qResult(MANAGE_ACCOUNT_CALL_FAILED_RESULT));
    }
}

void JobManageAccount::CallNintendoCreateAccount()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        Cancel();
        return;
    }
    m_callContext.Reset();
    ResumeOnCallCompletion(&m_callContext, new Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    if (m_pProtocolClient->CallNintendoCreateAccount(&m_callContext, m_name, m_key.ToString(), m_uiGroups, m_email,
                                                     m_customData, m_pPrincipalID, m_pPidHmac))
    {
        if (m_callContext.GetState() == CallContext::CallInProgress)
            SetToSuspended();
        else
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    }
    else
    {
        CompleteJobImpl(CallContext::CallFailed, qResult(MANAGE_ACCOUNT_CALL_FAILED_RESULT));
    }
}

void JobManageAccount::CallAccountManagementCommand()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        Cancel();
        return;
    }
    m_callContext.Reset();
    ResumeOnCallCompletion(&m_callContext, new Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    if (m_pCommand->Invoke(&m_callContext))
    {
        if (m_callContext.GetState() == CallContext::CallInProgress)
            SetToSuspended();
        else
            SetStep(Step(static_cast<Step::Function>(&JobManageAccount::CompleteJob), L""));
    }
    else
    {
        CompleteJobImpl(CallContext::CallFailed, qResult(MANAGE_ACCOUNT_CALL_FAILED_RESULT));
    }
}

void JobManageAccount::CompleteJob()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        Cancel();
        return;
    }
    if (m_keyContext.GetState() != CallContext::CallSucceeded)
    {
        CompleteJobImpl(m_keyContext.GetState(), m_keyContext.GetResult());
        return;
    }
    CompleteJobImpl(m_callContext.GetState(), m_callContext.GetResult());
}

}
}
