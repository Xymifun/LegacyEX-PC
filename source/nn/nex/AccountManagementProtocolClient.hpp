#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ACCOUNTMANAGEMENTPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ACCOUNTMANAGEMENTPROTOCOLCLIENT_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class AccountData;
class BasicAccountInfo;
class Data;
class DateTime;
class Message;
class ProtocolCallContext;
class ResultRange;
class qResult;

class AccountManagementProtocolClient : public ClientProtocol
{
public:
    typedef AnyObjectHolder<Data, String> DataHolder;
    typedef qList<DataHolder> DataHolderList;

    explicit AccountManagementProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;

    static void ProtoReturn_CreateAccount(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallCreateAccount(ProtocolCallContext* pCallContext, qResult* pResult, const String& rName, const String& rKey,
                           const unsigned int& rGroups, const String& rEmail);
    static void ProtoReturn_DeleteAccount(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallDeleteAccount(ProtocolCallContext* pCallContext, const unsigned int& rPrincipalID);
    static void ProtoReturn_DisableAccount(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallDisableAccount(ProtocolCallContext* pCallContext, qResult* pResult, const unsigned int& rPrincipalID,
                            const DateTime& rDate, const String& rMessage);
    static void ProtoReturn_ChangePassword(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallChangePassword(ProtocolCallContext* pCallContext, bool* pSuccess, const String& rNewKey);
    static void ProtoReturn_TestCapability(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallTestCapability(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rCapability);
    static void ProtoReturn_GetName(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetName(ProtocolCallContext* pCallContext, const unsigned int& rPrincipalID, String* pName);
    static void ProtoReturn_GetAccountData(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetAccountData(ProtocolCallContext* pCallContext, qResult* pResult, AccountData* pAccountData);
    static void ProtoReturn_GetPrivateData(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetPrivateData(ProtocolCallContext* pCallContext, bool* pSuccess, DataHolder* pData);
    static void ProtoReturn_GetPublicData(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetPublicData(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rPrincipalID,
                           DataHolder* pData);
    static void ProtoReturn_GetMultiplePublicData(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetMultiplePublicData(ProtocolCallContext* pCallContext, bool* pSuccess,
                                   const qList<unsigned int>& rPrincipalIDs, DataHolderList* pDataList);
    static void ProtoReturn_UpdateAccountName(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallUpdateAccountName(ProtocolCallContext* pCallContext, qResult* pResult, const String& rName);
    static void ProtoReturn_UpdateAccountEmail(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallUpdateAccountEmail(ProtocolCallContext* pCallContext, qResult* pResult, const String& rEmail);
    static void ProtoReturn_UpdateCustomData(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallUpdateCustomData(ProtocolCallContext* pCallContext, qResult* pResult, const DataHolder& rPublicData,
                              const DataHolder& rPrivateData);
    static void ProtoReturn_FindByNameRegex(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallFindByNameRegex(ProtocolCallContext* pCallContext, const unsigned int& rGroups, const String& rRegex,
                             const ResultRange& rRange, qList<BasicAccountInfo>* pAccountInfos);
    static void ProtoReturn_UpdateAccountExpiryDate(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallUpdateAccountExpiryDate(ProtocolCallContext* pCallContext, const unsigned int& rPrincipalID,
                                     const DateTime& rDate, const String& rMessage);
    static void ProtoReturn_UpdateAccountEffectiveDate(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallUpdateAccountEffectiveDate(ProtocolCallContext* pCallContext, const unsigned int& rPrincipalID,
                                        const DateTime& rDate, const String& rMessage);
    static void ProtoReturn_UpdateStatus(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallUpdateStatus(ProtocolCallContext* pCallContext, const String& rStatus);
    static void ProtoReturn_GetStatus(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetStatus(ProtocolCallContext* pCallContext, const unsigned int& rPrincipalID, String* pStatus);
    static void ProtoReturn_GetLastConnectionStats(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetLastConnectionStats(ProtocolCallContext* pCallContext, const unsigned int& rPrincipalID,
                                    DateTime* pDate1, DateTime* pDate2, DateTime* pDate3);
    static void ProtoReturn_ResetPassword(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallResetPassword(ProtocolCallContext* pCallContext, bool* pSuccess);
    static void ProtoReturn_CreateAccountWithCustomData(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallCreateAccountWithCustomData(ProtocolCallContext* pCallContext, const String& rName, const String& rKey,
                                         const unsigned int& rGroups, const String& rEmail,
                                         const DataHolder& rPublicData, const DataHolder& rPrivateData);
    static void ProtoReturn_RetrieveAccount(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallRetrieveAccount(ProtocolCallContext* pCallContext, AccountData* pAccountData, DataHolder* pPublicData,
                             DataHolder* pPrivateData);
    static void ProtoReturn_UpdateAccount(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallUpdateAccount(ProtocolCallContext* pCallContext, const String& rKey, const String& rEmail,
                           const DataHolder& rPublicData, const DataHolder& rPrivateData);
    static void ProtoReturn_ChangePasswordByGuest(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallChangePasswordByGuest(ProtocolCallContext* pCallContext, const String& rName, const String& rString1,
                                   const String& rString2);
    static void ProtoReturn_FindByNameLike(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallFindByNameLike(ProtocolCallContext* pCallContext, const unsigned int& rGroups, const String& rPattern,
                            const ResultRange& rRange, qList<BasicAccountInfo>* pAccountInfos);
    static void ProtoReturn_CustomCreateAccount(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallCustomCreateAccount(ProtocolCallContext* pCallContext, const String& rName, const String& rKey,
                                 const unsigned int& rGroups, const String& rEmail, const DataHolder& rData,
                                 unsigned int* pPrincipalID);
    static void ProtoReturn_NintendoCreateAccount(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallNintendoCreateAccount(ProtocolCallContext* pCallContext, const String& rName, const String& rKey,
                                   const unsigned int& rGroups, const String& rEmail, const DataHolder& rData,
                                   unsigned int* pPrincipalID, String* pString);
    static void ProtoReturn_LookupOrCreateAccount(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallLookupOrCreateAccount(ProtocolCallContext* pCallContext, const String& rName, const String& rKey,
                                   const unsigned int& rGroups, const String& rEmail, const DataHolder& rData,
                                   unsigned int* pPrincipalID);
    static void ProtoReturn_DisconnectPrincipal(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallDisconnectPrincipal(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rPrincipalID);
    static void ProtoReturn_DisconnectAllPrincipals(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallDisconnectAllPrincipals(ProtocolCallContext* pCallContext, bool* pSuccess);

    static bool CreateAccount_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, qResult* pResult,
                                             const String& rName, const String& rKey, const unsigned int& rGroups,
                                             const String& rEmail);
    static bool DeleteAccount_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                             const unsigned int& rPrincipalID);
    static bool DisableAccount_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, qResult* pResult,
                                              const unsigned int& rPrincipalID, const DateTime& rDate,
                                              const String& rMessage);
    static bool ChangePassword_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                              const String& rNewKey);
    static bool TestCapability_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                              const unsigned int& rCapability);
    static bool GetName_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                       const unsigned int& rPrincipalID, String* pName);
    static bool GetAccountData_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, qResult* pResult,
                                              AccountData* pAccountData);
    static bool GetPrivateData_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                              DataHolder* pData);
    static bool GetPublicData_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                             const unsigned int& rPrincipalID, DataHolder* pData);
    static bool GetMultiplePublicData_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     bool* pSuccess, const qList<unsigned int>& rPrincipalIDs,
                                                     DataHolderList* pDataList);
    static bool UpdateAccountName_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, qResult* pResult,
                                                 const String& rName);
    static bool UpdateAccountEmail_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                  qResult* pResult, const String& rEmail);
    static bool UpdateCustomData_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, qResult* pResult,
                                                const DataHolder& rPublicData, const DataHolder& rPrivateData);
    static bool FindByNameRegex_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               const unsigned int& rGroups, const String& rRegex,
                                               const ResultRange& rRange, qList<BasicAccountInfo>* pAccountInfos);
    static bool UpdateAccountExpiryDate_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                       const unsigned int& rPrincipalID, const DateTime& rDate,
                                                       const String& rMessage);
    static bool UpdateAccountEffectiveDate_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          const unsigned int& rPrincipalID, const DateTime& rDate,
                                                          const String& rMessage);
    static bool UpdateStatus_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                            const String& rStatus);
    static bool GetStatus_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                         const unsigned int& rPrincipalID, String* pStatus);
    static bool GetLastConnectionStats_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      const unsigned int& rPrincipalID, DateTime* pDate1,
                                                      DateTime* pDate2, DateTime* pDate3);
    static bool ResetPassword_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess);
    static bool CreateAccountWithCustomData_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                           const String& rName, const String& rKey,
                                                           const unsigned int& rGroups, const String& rEmail,
                                                           const DataHolder& rPublicData,
                                                           const DataHolder& rPrivateData);
    static bool RetrieveAccount_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               AccountData* pAccountData, DataHolder* pPublicData,
                                               DataHolder* pPrivateData);
    static bool UpdateAccount_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, const String& rKey,
                                             const String& rEmail, const DataHolder& rPublicData,
                                             const DataHolder& rPrivateData);
    static bool ChangePasswordByGuest_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     const String& rName, const String& rString1,
                                                     const String& rString2);
    static bool FindByNameLike_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                              const unsigned int& rGroups, const String& rPattern,
                                              const ResultRange& rRange, qList<BasicAccountInfo>* pAccountInfos);
    static bool CustomCreateAccount_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const String& rName, const String& rKey, const unsigned int& rGroups,
                                                   const String& rEmail, const DataHolder& rData,
                                                   unsigned int* pPrincipalID);
    static bool NintendoCreateAccount_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     const String& rName, const String& rKey,
                                                     const unsigned int& rGroups, const String& rEmail,
                                                     const DataHolder& rData, unsigned int* pPrincipalID,
                                                     String* pString);
    static bool LookupOrCreateAccount_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     const String& rName, const String& rKey,
                                                     const unsigned int& rGroups, const String& rEmail,
                                                     const DataHolder& rData, unsigned int* pPrincipalID);
    static bool DisconnectPrincipal_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                                   const unsigned int& rPrincipalID);
    static bool DisconnectAllPrincipals_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                       bool* pSuccess);
};

}
}

#endif
