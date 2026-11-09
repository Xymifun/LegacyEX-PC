#include "nn/nex/JobCreateAccount.hpp"

#include <string.h>

#include "nn/nex/AccountManagementClient.hpp"
#include "nn/nex/AccountManagementServiceDDL.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/RendezVous.hpp"
#include "nn/nex/SpecialArray.hpp"

namespace nn
{
namespace nex
{

namespace
{

const int CREATE_ACCOUNT_SUCCEEDED_RESULT = 0x10001;
const int CREATE_ACCOUNT_CALL_FAILED_RESULT = static_cast<int>(0x8001000Du);

}

JobCreateAccount::JobCreateAccount(unsigned int uiCallId, const char* pGuestPassword, const String& rHost,
                                   unsigned short uiPort, const String& rName, const char* pPassword,
                                   unsigned int uiGroups, const String& rEmail, FuncType funcType,
                                   AnyObjectHolder<Data, String>* pPublicData,
                                   AnyObjectHolder<Data, String>* pPrivateData,
                                   AnyObjectHolder<Data, String>* pCustomData, unsigned int* pPrincipalID,
                                   String* pPidHmac)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(uiCallId),
      m_pGuestCredentials(0),
      m_host(rHost),
      m_uiPort(uiPort),
      m_name(rName),
      m_uiGroups(uiGroups),
      m_email(rEmail),
      m_funcType(funcType),
      m_publicData(),
      m_privateData(),
      m_customData(),
      m_pPrincipalID(pPrincipalID),
      m_pPidHmac(pPidHmac),
      m_createResult(),
      m_jobResult(),
      m_pRendezVous(new RendezVous()),
      m_pAccountManagementClient(new AccountManagementClient()),
      m_loginContext(0),
      m_createContext(0),
      m_logoutContext(0),
      m_reservedContext(0)
{
    m_jobResult = qResult(CREATE_ACCOUNT_SUCCEEDED_RESULT);
    SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::LoginGuest), L""));
    m_pPassword = 0;
    if (pPassword != 0)
    {
        m_pPassword = SpecialNewArray<char>(strlen(pPassword) + 1, L"", 0);
        if (strlen(pPassword) != 0)
            strcpy(m_pPassword, pPassword);
        else
            m_pPassword[0] = '\0';
    }
    m_pGuestPassword = 0;
    if (pGuestPassword != 0)
    {
        m_pGuestPassword = SpecialNewArray<char>(strlen(pGuestPassword) + 1, L"", 0);
        if (strlen(pGuestPassword) != 0)
            strcpy(m_pGuestPassword, pGuestPassword);
        else
            m_pGuestPassword[0] = '\0';
    }
    if (pPublicData == 0 || pPublicData->GetObject() == 0)
        m_publicData.SetObject(new PublicData());
    else
        m_publicData.SetObject(pPublicData->GetObject()->Clone());
    if (pPrivateData == 0 || pPrivateData->GetObject() == 0)
        m_privateData.SetObject(new PrivateData());
    else
        m_privateData.SetObject(pPrivateData->GetObject()->Clone());
    if (pCustomData != 0 && pCustomData->GetObject() != 0)
        m_customData.SetObject(pCustomData->GetObject()->Clone());
}

JobCreateAccount::~JobCreateAccount()
{
    if (m_pPassword != 0)
        SpecialDeleteArray(m_pPassword);
    if (m_pGuestPassword != 0)
        SpecialDeleteArray(m_pGuestPassword);
    delete m_pAccountManagementClient;
    delete m_pRendezVous;
}

void JobCreateAccount::CompleteJob(qResult result)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        if (result && m_jobResult)
            pCallContext->SignalSuccess(result);
        else if (result)
            pCallContext->SignalFailure(m_jobResult);
        else
            pCallContext->SignalFailure(result);
    }
    SetToComplete();
}

void JobCreateAccount::LoginGuest()
{
    if (!m_pRendezVous->SilentLogin(&m_loginContext, L"guest", m_pGuestPassword, m_host, m_uiPort,
                                    &m_pGuestCredentials))
    {
        const qResult& result = m_loginContext.GetResult();
        CompleteJob(result);
        return;
    }
    SetToSuspended();
    ResumeOnCallCompletion(&m_loginContext,
                           new Step(static_cast<Step::Function>(&JobCreateAccount::ProcessLoginGuestResult), L""));
}

void JobCreateAccount::ProcessLoginGuestResult()
{
    if (m_loginContext.GetState() == CallContext::CallSucceeded)
    {
        switch (m_funcType)
        {
        case CreateAccountFunc:
            SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::CreateAccount), L""));
            break;
        case CreateAccountWithCustomDataFunc:
            SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::CreateAccountWithCustomData), L""));
            break;
        case ChangePasswordByGuestFunc:
            SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::ChangePasswordByGuest), L""));
            break;
        case CustomCreateAccountFunc:
            SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::CustomCreateAccount), L""));
            break;
        case NintendoCreateAccountFunc:
            SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::NintendoCreateAccount), L""));
            break;
        }
    }
    else
    {
        CompleteJob(m_loginContext.GetResult());
    }
}

void JobCreateAccount::CreateAccount()
{
    m_pAccountManagementClient->Bind(m_pGuestCredentials);
    if (!m_pAccountManagementClient->CreateAccount(&m_createContext, &m_createResult, m_name, String(m_pPassword),
                                                   m_uiGroups, m_email))
    {
        m_jobResult = m_createContext.GetResult();
        SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::LogoutGuest), L""));
        return;
    }
    SetToSuspended();
    ResumeOnCallCompletion(&m_createContext,
                           new Step(static_cast<Step::Function>(&JobCreateAccount::ProcessCreateAccountResult), L""));
}

void JobCreateAccount::CustomCreateAccount()
{
    m_pAccountManagementClient->Bind(m_pGuestCredentials);
    if (!m_pAccountManagementClient->CustomCreateAccount(&m_createContext, m_name, String(m_pPassword), m_uiGroups,
                                                         m_email, m_customData, m_pPrincipalID))
    {
        m_jobResult = m_createContext.GetResult();
        SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::LogoutGuest), L""));
        return;
    }
    SetToSuspended();
    ResumeOnCallCompletion(&m_createContext,
                           new Step(static_cast<Step::Function>(&JobCreateAccount::ProcessCreateAccountResult), L""));
}

void JobCreateAccount::CreateAccountWithCustomData()
{
    m_pAccountManagementClient->Bind(m_pGuestCredentials);
    if (!m_pAccountManagementClient->CreateAccount(&m_createContext, m_name, String(m_pPassword), m_uiGroups, m_email,
                                                   m_publicData, m_privateData))
    {
        m_jobResult = qResult(static_cast<int>(CREATE_ACCOUNT_CALL_FAILED_RESULT));
        SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::LogoutGuest), L""));
        return;
    }
    SetToSuspended();
    ResumeOnCallCompletion(&m_createContext,
                           new Step(static_cast<Step::Function>(&JobCreateAccount::ProcessCreateAccountResult), L""));
}

void JobCreateAccount::NintendoCreateAccount()
{
    m_pAccountManagementClient->Bind(m_pGuestCredentials);
    if (!m_pAccountManagementClient->NintendoCreateAccount(&m_createContext, m_name, String(m_pPassword), m_uiGroups,
                                                           m_email, m_customData, m_pPrincipalID, m_pPidHmac))
    {
        m_jobResult = m_createContext.GetResult();
        SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::LogoutGuest), L""));
        return;
    }
    SetToSuspended();
    ResumeOnCallCompletion(&m_createContext,
                           new Step(static_cast<Step::Function>(&JobCreateAccount::ProcessCreateAccountResult), L""));
}

void JobCreateAccount::ChangePasswordByGuest()
{
    m_pAccountManagementClient->Bind(m_pGuestCredentials);
    if (!m_pAccountManagementClient->ChangePassword(&m_createContext, m_name, m_email, String(m_pPassword)))
    {
        m_jobResult = qResult(static_cast<int>(CREATE_ACCOUNT_CALL_FAILED_RESULT));
        SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::LogoutGuest), L""));
        return;
    }
    SetToSuspended();
    ResumeOnCallCompletion(&m_createContext,
                           new Step(static_cast<Step::Function>(&JobCreateAccount::ProcessCreateAccountResult), L""));
}

void JobCreateAccount::ProcessCreateAccountResult()
{
    if (m_createContext.GetState() == CallContext::CallSucceeded)
    {
        if (m_funcType == CreateAccountFunc)
        {
            if (!m_createResult)
                m_jobResult = m_createResult;
            else
                m_jobResult = qResult(static_cast<int>(CREATE_ACCOUNT_SUCCEEDED_RESULT));
        }
    }
    else
    {
        m_jobResult = m_createContext.GetResult();
    }
    delete m_pAccountManagementClient;
    m_pAccountManagementClient = 0;
    SetStep(Step(static_cast<Step::Function>(&JobCreateAccount::LogoutGuest), L""));
}

void JobCreateAccount::LogoutGuest()
{
    if (!m_pRendezVous->SilentLogout(&m_logoutContext, m_pGuestCredentials))
    {
        const qResult& result = m_logoutContext.GetResult();
        CompleteJob(result);
        return;
    }
    SetToSuspended();
    ResumeOnCallCompletion(&m_logoutContext,
                           new Step(static_cast<Step::Function>(&JobCreateAccount::ProcessLogoutGuestResult), L""));
}

void JobCreateAccount::ProcessLogoutGuestResult()
{
    m_pGuestCredentials = 0;
    CompleteJob(m_logoutContext.GetResult());
}

}
}
