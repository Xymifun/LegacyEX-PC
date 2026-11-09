#include "nn/nex/AccountManagementCommand.hpp"

#include "nn/nex/AccountManagementClient.hpp"
#include "nn/nex/AccountManagementProtocolClient.hpp"

namespace nn
{
namespace nex
{

AccountManagementCommand::AccountManagementCommand(const DebugString& rDescription, const String& strPassword)
    : m_description(rDescription),
      m_strPassword(strPassword)
{
}

AccountManagementCommand::~AccountManagementCommand()
{
}

bool AccountManagementCommand::Invoke(ProtocolCallContext* pCallContext)
{
    return InvokeImpl(pCallContext);
}

LookupOrCreateAccountCommand::LookupOrCreateAccountCommand(const DebugString& rDescription,
                                                           AccountManagementProtocolClient* pProtocolClient,
                                                           const String& strPrincipalName, const String& strPassword,
                                                           const String& strEmail, unsigned int uiGroups,
                                                           AnyObjectHolder<Data, String>* pCustomData,
                                                           unsigned int* pPrincipalID)
    : AccountManagementCommand(rDescription, strPassword),
      m_pProtocolClient(pProtocolClient),
      m_strPrincipalName(strPrincipalName),
      m_strEmail(strEmail),
      m_uiGroups(uiGroups),
      m_pPrincipalID(pPrincipalID)
{
    m_oCustomData.SetObject(pCustomData->GetObject()->Clone());
}

LookupOrCreateAccountCommand::~LookupOrCreateAccountCommand()
{
}

bool LookupOrCreateAccountCommand::InvokeImpl(ProtocolCallContext* pCallContext)
{
    return m_pProtocolClient->CallLookupOrCreateAccount(pCallContext, m_strPrincipalName, m_strKey, m_uiGroups,
                                                        m_strEmail, m_oCustomData, m_pPrincipalID);
}


GuestCreateAccountCommand::~GuestCreateAccountCommand()
{
}

bool GuestCreateAccountCommand::InvokeImpl(ProtocolCallContext* pCallContext)
{
    char* szAccessKey = 0;
    m_oSandboxConnectionInfo.GetCredentialsProd().m_password.CreateCopy(&szAccessKey);
    char* szPassword = 0;
    m_strPassword.CreateCopy(&szPassword);
    const SandboxConnectionInfo::HostCredentials& oCredentials = m_oSandboxConnectionInfo.GetCredentialsProd();
    bool bResult = AccountManagementClient::CreateAccount(pCallContext, szAccessKey, oCredentials.m_host,
                                                          oCredentials.m_uiPort, m_strPrincipalName, szPassword,
                                                          m_uiGroups, m_strEmail);
    String::ReleaseCopy(szAccessKey);
    String::ReleaseCopy(szPassword);
    return bResult;
}

GuestCustomCreateAccountCommand::~GuestCustomCreateAccountCommand()
{
}

bool GuestCustomCreateAccountCommand::InvokeImpl(ProtocolCallContext* pCallContext)
{
    char* szAccessKey = 0;
    m_oSandboxConnectionInfo.GetCredentialsProd().m_password.CreateCopy(&szAccessKey);
    char* szPassword = 0;
    m_strPassword.CreateCopy(&szPassword);
    const SandboxConnectionInfo::HostCredentials& oCredentials = m_oSandboxConnectionInfo.GetCredentialsProd();
    bool bResult = AccountManagementClient::CustomCreateAccount(pCallContext, szAccessKey, oCredentials.m_host,
                                                                oCredentials.m_uiPort, m_strPrincipalName, szPassword,
                                                                m_uiGroups, m_strEmail, *m_pCustomData, m_pPrincipalID);
    String::ReleaseCopy(szAccessKey);
    String::ReleaseCopy(szPassword);
    return bResult;
}

}
}
