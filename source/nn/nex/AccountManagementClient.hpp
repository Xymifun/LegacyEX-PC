#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ACCOUNTMANAGEMENTCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ACCOUNTMANAGEMENTCLIENT_HPP

#include "nn/nex/AccountManagementProtocolClient.hpp"
#include "nn/nex/AccountManagementServiceDDL.hpp"
#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/CustomDataHolder.hpp"
#include "nn/nex/ServiceClient.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class AccountData;
class BasicAccountInfo;
class Credentials;
class Data;
class DateTime;
class ProtocolCallContext;
class ResultRange;
class qResult;

class AccountManagementClient : public ServiceClient
{
public:
    AccountManagementClient();
    virtual ~AccountManagementClient();

    virtual void UpdateProtocolsDefaultCredentials(Credentials* pCredentials);

    static bool CreateAccount(ProtocolCallContext* pCallContext, const char* szAccessKey, const String& strHostName,
                              unsigned short uiPort, const String& strPrincipalName, const char* szPassword,
                              unsigned int uiGroups, const String& strEmail);
    bool CreateAccount(ProtocolCallContext* pCallContext, const String& strPrincipalName, const String& strKey,
                       unsigned int uiGroups, const String& strEmail);
    bool CreateAccount(ProtocolCallContext* pCallContext, qResult* pResult, const String& strPrincipalName,
                       const String& strKey, unsigned int uiGroups, const String& strEmail);
    static bool CreateAccount(ProtocolCallContext* pCallContext, const char* szAccessKey, const String& strHostName,
                              unsigned short uiPort, const String& strPrincipalName, const char* szPassword,
                              unsigned int uiGroups, const String& strEmail,
                              const CustomDataHolder<PublicData>& oPublicData,
                              const CustomDataHolder<PrivateData>& oPrivateData);
    bool CreateAccount(ProtocolCallContext* pCallContext, const String& strPrincipalName, const String& strKey,
                       unsigned int uiGroups, const String& strEmail, const AnyObjectHolder<Data, String>& oPublicData,
                       const AnyObjectHolder<Data, String>& oPrivateData);
    static bool CustomCreateAccount(ProtocolCallContext* pCallContext, const char* szAccessKey,
                                    const String& strHostName, unsigned short uiPort, const String& strPrincipalName,
                                    const char* szPassword, unsigned int uiGroups, const String& strEmail,
                                    const AnyObjectHolder<Data, String>& oAuthenticationData,
                                    unsigned int* pPrincipalID);
    bool CustomCreateAccount(ProtocolCallContext* pCallContext, const String& strPrincipalName, const String& strKey,
                             unsigned int uiGroups, const String& strEmail,
                             const AnyObjectHolder<Data, String>& oAuthenticationData, unsigned int* pPrincipalID);
    static bool NintendoCreateAccount(ProtocolCallContext* pCallContext, const char* szAccessKey,
                                      const String& strHostName, unsigned short uiPort, const String& strPrincipalName,
                                      const char* szPassword, unsigned int uiGroups, const String& strEmail,
                                      const AnyObjectHolder<Data, String>& oAuthenticationData,
                                      unsigned int* pPrincipalID, String* pStrPrincipalName);
    bool NintendoCreateAccount(ProtocolCallContext* pCallContext, const String& strPrincipalName, const String& strKey,
                               unsigned int uiGroups, const String& strEmail,
                               const AnyObjectHolder<Data, String>& oAuthenticationData, unsigned int* pPrincipalID,
                               String* pStrPrincipalName);
    bool LookupOrCreateAccount(ProtocolCallContext* pCallContext, const String& strPrincipalName,
                               const String& strPassword, unsigned int uiGroups, const String& strEmail,
                               const AnyObjectHolder<Data, String>& oAuthenticationData, unsigned int* pPrincipalID);
    bool ChangePassword(ProtocolCallContext* pCallContext, const String& strNewKey);
    bool ChangePassword(ProtocolCallContext* pCallContext, bool* pSuccess, const String& strNewKey);
    bool ChangePassword(ProtocolCallContext* pCallContext, const String& strPrincipalName, const String& strEmail,
                        const String& strNewKey);
    static bool ChangePassword(ProtocolCallContext* pCallContext, const char* szAccessKey, const String& strHostName,
                               unsigned short uiPort, const String& strPrincipalName, const String& strEmail,
                               const char* szNewPassword);
    bool UpdateAccount(ProtocolCallContext* pCallContext, const String& strKey, const String& strEmail,
                       const CustomDataHolder<PublicData>& oPublicData,
                       const CustomDataHolder<PrivateData>& oPrivateData);

    bool DisableAccount(ProtocolCallContext* pCallContext, unsigned int uiPrincipalID, const DateTime& dtUntil,
                        const String& strMessage);
    bool DisableAccount(ProtocolCallContext* pCallContext, qResult* pResult, unsigned int uiPrincipalID,
                        const DateTime& dtUntil, const String& strMessage);
    bool DeleteAccount(ProtocolCallContext* pCallContext, unsigned int uiPrincipalID);
    bool DisconnectPrincipal(ProtocolCallContext* pCallContext, unsigned int uiPrincipalID);
    bool DisconnectAllPrincipals(ProtocolCallContext* pCallContext);
    bool TestCapability(ProtocolCallContext* pCallContext, unsigned int uiCapability);
    bool TestCapability(ProtocolCallContext* pCallContext, bool* pSuccess, unsigned int uiCapability);
    bool UpdateAccountName(ProtocolCallContext* pCallContext, const String& strName);
    bool UpdateAccountName(ProtocolCallContext* pCallContext, qResult* pResult, const String& strName);
    bool UpdateAccountEmail(ProtocolCallContext* pCallContext, const String& strEmail);
    bool UpdateAccountEmail(ProtocolCallContext* pCallContext, qResult* pResult, const String& strEmail);
    bool UpdateCustomData(ProtocolCallContext* pCallContext, const CustomDataHolder<PublicData>& oPublicData,
                          const CustomDataHolder<PrivateData>& oPrivateData);
    bool UpdateCustomData(ProtocolCallContext* pCallContext, qResult* pResult,
                          const AnyObjectHolder<Data, String>& oPublicData,
                          const AnyObjectHolder<Data, String>& oPrivateData);
    bool GetName(ProtocolCallContext* pCallContext, unsigned int uiPrincipalID, String* pStrName);
    bool GetAccountData(ProtocolCallContext* pCallContext, AccountData* pAccountData);
    bool GetAccountData(ProtocolCallContext* pCallContext, qResult* pResult, AccountData* pAccountData);
    bool GetPublicData(ProtocolCallContext* pCallContext, unsigned int uiPrincipalID,
                       CustomDataHolder<PublicData>* pPublicData);
    bool GetPublicData(ProtocolCallContext* pCallContext, qList<unsigned int> lstPrincipalID,
                       qList<AnyObjectHolder<Data, String> >* pLstPublicData);
    bool GetPublicData(ProtocolCallContext* pCallContext, bool* pSuccess, unsigned int uiPrincipalID,
                       AnyObjectHolder<Data, String>* pPublicData);
    bool GetPublicData(ProtocolCallContext* pCallContext, bool* pSuccess, qList<unsigned int> lstPrincipalID,
                       qList<AnyObjectHolder<Data, String> >* pLstPublicData);
    bool GetPrivateData(ProtocolCallContext* pCallContext, CustomDataHolder<PrivateData>* pPrivateData);
    bool GetPrivateData(ProtocolCallContext* pCallContext, bool* pSuccess, AnyObjectHolder<Data, String>* pPrivateData);
    bool RetrieveAccount(ProtocolCallContext* pCallContext, AccountData* pAccountData,
                         AnyObjectHolder<Data, String>* pPublicData, AnyObjectHolder<Data, String>* pPrivateData);
    bool FindByNameRegex(ProtocolCallContext* pCallContext, unsigned int uiGroups, const String& strRegex,
                         const ResultRange& oRange, qList<BasicAccountInfo>* pLstAccounts);
    bool FindByNameLike(ProtocolCallContext* pCallContext, unsigned int uiGroups, const String& strLike,
                        const ResultRange& oRange, qList<BasicAccountInfo>* pLstAccounts);
    bool UpdateAccountExpiryDate(ProtocolCallContext* pCallContext, unsigned int uiPrincipalID,
                                 const DateTime& dtExpiry, const String& strExpiredMessage);
    bool UpdateAccountEffectiveDate(ProtocolCallContext* pCallContext, unsigned int uiPrincipalID,
                                    const DateTime& dtEffectiveFrom, const String& strNotEffectiveMessage);
    bool UpdateStatus(ProtocolCallContext* pCallContext, const String& strStatus);
    bool GetStatus(ProtocolCallContext* pCallContext, unsigned int uiPrincipalID, String* pStrStatus);
    bool GetLastConnectionStats(ProtocolCallContext* pCallContext, unsigned int uiPrincipalID,
                                DateTime* pLastSessionLogin, DateTime* pLastSessionLogout,
                                DateTime* pCurrentSessionLogin);
    bool ResetPassword(ProtocolCallContext* pCallContext);

private:
    AccountManagementProtocolClient m_protocolClient;
};

}
}

#endif
