#include "nn/nex/AccountManagementClient.hpp"

#include "nn/nex/AccountManagementCommand.hpp"
#include "nn/nex/AnyDataAdapter.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/InstanceControl.hpp"
#include "nn/nex/JobCreateAccount.hpp"
#include "nn/nex/JobManageAccount.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int ACCOUNT_MANAGEMENT_SERVICE_ID = 14;
const unsigned short ACCOUNT_MANAGEMENT_PROTOCOL_ID = 25;

}

AccountManagementClient::AccountManagementClient() : ServiceClient(ACCOUNT_MANAGEMENT_SERVICE_ID), m_protocolClient(0)
{
    AnyDataAdapter::Register();
    m_protocolClient.SetProtocolID(ACCOUNT_MANAGEMENT_PROTOCOL_ID);
    AccountManagementProtocolClient* pProtocolClient = new AccountManagementProtocolClient();
    GetProtocolRegistry()->RegisterProtocol(ACCOUNT_MANAGEMENT_PROTOCOL_ID, pProtocolClient);
}

AccountManagementClient::~AccountManagementClient()
{
    AnyDataAdapter::Unregister();
}

void AccountManagementClient::UpdateProtocolsDefaultCredentials(Credentials* pCredentials)
{
    m_protocolClient.SetDefaultCredentials(pCredentials);
}

bool AccountManagementClient::CreateAccount(ProtocolCallContext* pCallContext, const char* szAccessKey,
                                            const String& strHostName, unsigned short uiPort,
                                            const String& strPrincipalName, const char* szPassword,
                                            unsigned int uiGroups, const String& strEmail)
{
    if (Scheduler::GetInstance() == 0)
        return false;

    ScopedCS instanceLock(&g_instanceTableCriticalSection);
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!pCallContext->InitiateCall())
        return false;

    JobCreateAccount* pJob = new JobCreateAccount(pCallContext->GetCallID(), szAccessKey, strHostName, uiPort,
                                                  strPrincipalName, szPassword, uiGroups, strEmail,
                                                  JobCreateAccount::CreateAccountFunc, 0, 0, 0, 0, 0);
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

bool AccountManagementClient::CreateAccount(ProtocolCallContext* pCallContext, const String& strPrincipalName,
                                            const String& strKey, unsigned int uiGroups, const String& strEmail)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    JobManageAccount* pJob = new JobManageAccount(pCallContext->GetCallID(), &m_protocolClient, strPrincipalName,
                                                  strKey, uiGroups, strEmail, 0, JobManageAccount::CreateAccountFunc, 0,
                                                  0, 0, 0, 0);
    Scheduler::GetInstance()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

bool AccountManagementClient::CreateAccount(ProtocolCallContext* pCallContext, qResult* pResult,
                                            const String& strPrincipalName, const String& strKey, unsigned int uiGroups,
                                            const String& strEmail)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    JobManageAccount* pJob = new JobManageAccount(pCallContext->GetCallID(), &m_protocolClient, strPrincipalName,
                                                  strKey, uiGroups, strEmail, pResult,
                                                  JobManageAccount::CreateAccountFunc, 0, 0, 0, 0, 0);
    Scheduler::GetInstance()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

bool AccountManagementClient::CreateAccount(ProtocolCallContext* pCallContext, const char* szAccessKey,
                                            const String& strHostName, unsigned short uiPort,
                                            const String& strPrincipalName, const char* szPassword,
                                            unsigned int uiGroups, const String& strEmail,
                                            const CustomDataHolder<PublicData>& oPublicData,
                                            const CustomDataHolder<PrivateData>& oPrivateData)
{
    if (Scheduler::GetInstance() == 0)
        return false;

    ScopedCS instanceLock(&g_instanceTableCriticalSection);
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!pCallContext->InitiateCall())
        return false;

    JobCreateAccount* pJob = new JobCreateAccount(
        pCallContext->GetCallID(), szAccessKey, strHostName, uiPort, strPrincipalName, szPassword, uiGroups, strEmail,
        JobCreateAccount::CreateAccountWithCustomDataFunc, const_cast<CustomDataHolder<PublicData>*>(&oPublicData),
        const_cast<CustomDataHolder<PrivateData>*>(&oPrivateData), 0, 0, 0);
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

bool AccountManagementClient::CreateAccount(ProtocolCallContext* pCallContext, const String& strPrincipalName,
                                            const String& strKey, unsigned int uiGroups, const String& strEmail,
                                            const AnyObjectHolder<Data, String>& oPublicData,
                                            const AnyObjectHolder<Data, String>& oPrivateData)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    JobManageAccount* pJob = new JobManageAccount(
        pCallContext->GetCallID(), &m_protocolClient, strPrincipalName, strKey, uiGroups, strEmail, 0,
        JobManageAccount::CreateAccountWithCustomDataFunc, const_cast<AnyObjectHolder<Data, String>*>(&oPublicData),
        const_cast<AnyObjectHolder<Data, String>*>(&oPrivateData), 0, 0, 0);
    Scheduler::GetInstance()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

bool AccountManagementClient::CustomCreateAccount(ProtocolCallContext* pCallContext, const char* szAccessKey,
                                                  const String& strHostName, unsigned short uiPort,
                                                  const String& strPrincipalName, const char* szPassword,
                                                  unsigned int uiGroups, const String& strEmail,
                                                  const AnyObjectHolder<Data, String>& oAuthenticationData,
                                                  unsigned int* pPrincipalID)
{
    if (Scheduler::GetInstance() == 0)
        return false;

    ScopedCS instanceLock(&g_instanceTableCriticalSection);
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!pCallContext->InitiateCall())
        return false;

    JobCreateAccount* pJob = new JobCreateAccount(
        pCallContext->GetCallID(), szAccessKey, strHostName, uiPort, strPrincipalName, szPassword, uiGroups, strEmail,
        JobCreateAccount::CustomCreateAccountFunc, 0, 0,
        const_cast<AnyObjectHolder<Data, String>*>(&oAuthenticationData), pPrincipalID, 0);
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

bool AccountManagementClient::CustomCreateAccount(ProtocolCallContext* pCallContext, const String& strPrincipalName,
                                                  const String& strKey, unsigned int uiGroups, const String& strEmail,
                                                  const AnyObjectHolder<Data, String>& oAuthenticationData,
                                                  unsigned int* pPrincipalID)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    JobManageAccount* pJob = new JobManageAccount(
        pCallContext->GetCallID(), &m_protocolClient, strPrincipalName, strKey, uiGroups, strEmail, 0,
        JobManageAccount::CustomCreateAccountFunc, 0, 0,
        const_cast<AnyObjectHolder<Data, String>*>(&oAuthenticationData), pPrincipalID, 0);
    Scheduler::GetInstance()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

bool AccountManagementClient::NintendoCreateAccount(ProtocolCallContext* pCallContext, const char* szAccessKey,
                                                    const String& strHostName, unsigned short uiPort,
                                                    const String& strPrincipalName, const char* szPassword,
                                                    unsigned int uiGroups, const String& strEmail,
                                                    const AnyObjectHolder<Data, String>& oAuthenticationData,
                                                    unsigned int* pPrincipalID, String* pStrPrincipalName)
{
    if (Scheduler::GetInstance() == 0)
        return false;

    ScopedCS instanceLock(&g_instanceTableCriticalSection);
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!pCallContext->InitiateCall())
        return false;

    JobCreateAccount* pJob = new JobCreateAccount(
        pCallContext->GetCallID(), szAccessKey, strHostName, uiPort, strPrincipalName, szPassword, uiGroups, strEmail,
        JobCreateAccount::NintendoCreateAccountFunc, 0, 0,
        const_cast<AnyObjectHolder<Data, String>*>(&oAuthenticationData), pPrincipalID, pStrPrincipalName);
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

bool AccountManagementClient::NintendoCreateAccount(ProtocolCallContext* pCallContext, const String& strPrincipalName,
                                                    const String& strKey, unsigned int uiGroups, const String& strEmail,
                                                    const AnyObjectHolder<Data, String>& oAuthenticationData,
                                                    unsigned int* pPrincipalID, String* pStrPrincipalName)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    JobManageAccount* pJob = new JobManageAccount(
        pCallContext->GetCallID(), &m_protocolClient, strPrincipalName, strKey, uiGroups, strEmail, 0,
        JobManageAccount::NintendoCreateAccountFunc, 0, 0,
        const_cast<AnyObjectHolder<Data, String>*>(&oAuthenticationData), pPrincipalID, pStrPrincipalName);
    Scheduler::GetInstance()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

bool AccountManagementClient::LookupOrCreateAccount(ProtocolCallContext* pCallContext, const String& strPrincipalName,
                                                    const String& strPassword, unsigned int uiGroups,
                                                    const String& strEmail,
                                                    const AnyObjectHolder<Data, String>& oAuthenticationData,
                                                    unsigned int* pPrincipalID)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    LookupOrCreateAccountCommand* pCommand = new LookupOrCreateAccountCommand(
        DebugString(L""), &m_protocolClient, strPrincipalName, strPassword, strEmail, uiGroups,
        const_cast<AnyObjectHolder<Data, String>*>(&oAuthenticationData), pPrincipalID);
    JobManageAccount* pJob = new JobManageAccount(pCallContext->GetCallID(), &m_protocolClient, pCommand);
    Scheduler::GetInstance()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

bool AccountManagementClient::ChangePassword(ProtocolCallContext* pCallContext, const String& strNewKey)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    JobManageAccount* pJob = new JobManageAccount(pCallContext->GetCallID(), &m_protocolClient, strNewKey, 0,
                                                  JobManageAccount::ChangePasswordFunc);
    Scheduler::GetInstance()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

bool AccountManagementClient::ChangePassword(ProtocolCallContext* pCallContext, bool* pSuccess, const String& strNewKey)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    pCallContext->m_completionFlag = pSuccess;
    JobManageAccount* pJob = new JobManageAccount(pCallContext->GetCallID(), &m_protocolClient, strNewKey, pSuccess,
                                                  JobManageAccount::ChangePasswordFunc);
    Scheduler::GetInstance()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

bool AccountManagementClient::ChangePassword(ProtocolCallContext* pCallContext, const String& strPrincipalName,
                                             const String& strEmail, const String& strNewKey)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    JobManageAccount* pJob = new JobManageAccount(pCallContext->GetCallID(), &m_protocolClient, strPrincipalName,
                                                  strNewKey, 0, strEmail, 0,
                                                  JobManageAccount::ChangePasswordByGuestFunc, 0, 0, 0, 0, 0);
    Scheduler::GetInstance()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

bool AccountManagementClient::ChangePassword(ProtocolCallContext* pCallContext, const char* szAccessKey,
                                             const String& strHostName, unsigned short uiPort,
                                             const String& strPrincipalName, const String& strEmail,
                                             const char* szNewPassword)
{
    if (Scheduler::GetInstance() == 0)
        return false;

    ScopedCS instanceLock(&g_instanceTableCriticalSection);
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!pCallContext->InitiateCall())
        return false;

    JobCreateAccount* pJob = new JobCreateAccount(pCallContext->GetCallID(), szAccessKey, strHostName, uiPort,
                                                  strPrincipalName, szNewPassword, 0, strEmail,
                                                  JobCreateAccount::ChangePasswordByGuestFunc, 0, 0, 0, 0, 0);
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

}
}
