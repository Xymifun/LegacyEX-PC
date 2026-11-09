#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_ACCOUNTMANAGEMENTPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_ACCOUNTMANAGEMENTPROTOCOLSERVER_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class AccountData;
class BasicAccountInfo;
class Data;
class DateTime;
class ResultRange;

class _Proto_AccountManagementProtocolServer : public ServerProtocol
{
public:
    typedef AnyObjectHolder<Data, String> DataHolder;
    typedef qList<DataHolder> DataHolderList;

    enum _MethodID
    {
        METHOD_CREATE_ACCOUNT = 1,
        METHOD_DELETE_ACCOUNT = 2,
        METHOD_DISABLE_ACCOUNT = 3,
        METHOD_CHANGE_PASSWORD = 4,
        METHOD_TEST_CAPABILITY = 5,
        METHOD_GET_NAME = 6,
        METHOD_GET_ACCOUNT_DATA = 7,
        METHOD_GET_PRIVATE_DATA = 8,
        METHOD_GET_PUBLIC_DATA = 9,
        METHOD_GET_MULTIPLE_PUBLIC_DATA = 10,
        METHOD_UPDATE_ACCOUNT_NAME = 11,
        METHOD_UPDATE_ACCOUNT_EMAIL = 12,
        METHOD_UPDATE_CUSTOM_DATA = 13,
        METHOD_FIND_BY_NAME_REGEX = 14,
        METHOD_UPDATE_ACCOUNT_EXPIRY_DATE = 15,
        METHOD_UPDATE_ACCOUNT_EFFECTIVE_DATE = 16,
        METHOD_UPDATE_STATUS = 17,
        METHOD_GET_STATUS = 18,
        METHOD_GET_LAST_CONNECTION_STATS = 19,
        METHOD_RESET_PASSWORD = 20,
        METHOD_CREATE_ACCOUNT_WITH_CUSTOM_DATA = 21,
        METHOD_RETRIEVE_ACCOUNT = 22,
        METHOD_UPDATE_ACCOUNT = 23,
        METHOD_CHANGE_PASSWORD_BY_GUEST = 24,
        METHOD_FIND_BY_NAME_LIKE = 25,
        METHOD_CUSTOM_CREATE_ACCOUNT = 26,
        METHOD_NINTENDO_CREATE_ACCOUNT = 27,
        METHOD_LOOKUP_OR_CREATE_ACCOUNT = 28,
        METHOD_DISCONNECT_PRINCIPAL = 29,
        METHOD_DISCONNECT_ALL_PRINCIPALS = 30
    };

    explicit _Proto_AccountManagementProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual qResult CreateAccount(const String& rName, const String& rKey, const unsigned int& rGroups,
                                  const String& rEmail) = 0;
    virtual void DeleteAccount(const unsigned int& rPrincipalID) = 0;
    virtual qResult DisableAccount(const unsigned int& rPrincipalID, const DateTime& rDate, const String& rMessage) = 0;
    virtual bool ChangePassword(const String& rNewKey) = 0;
    virtual bool TestCapability(const unsigned int& rCapability) = 0;
    virtual void GetName(const unsigned int& rPrincipalID, String* pName) = 0;
    virtual qResult GetAccountData(AccountData* pAccountData) = 0;
    virtual bool GetPrivateData(DataHolder* pData) = 0;
    virtual bool GetPublicData(const unsigned int& rPrincipalID, DataHolder* pData) = 0;
    virtual bool GetMultiplePublicData(const qList<unsigned int>& rPrincipalIDs, DataHolderList* pDataList) = 0;
    virtual qResult UpdateAccountName(const String& rName) = 0;
    virtual qResult UpdateAccountEmail(const String& rEmail) = 0;
    virtual qResult UpdateCustomData(const DataHolder& rPublicData, const DataHolder& rPrivateData) = 0;
    virtual void FindByNameRegex(const unsigned int& rGroups, const String& rRegex, const ResultRange& rRange,
                                 qList<BasicAccountInfo>* pAccountInfos) = 0;
    virtual void UpdateAccountExpiryDate(const unsigned int& rPrincipalID, const DateTime& rDate,
                                         const String& rMessage) = 0;
    virtual void UpdateAccountEffectiveDate(const unsigned int& rPrincipalID, const DateTime& rDate,
                                            const String& rMessage) = 0;
    virtual void UpdateStatus(const String& rStatus) = 0;
    virtual void GetStatus(const unsigned int& rPrincipalID, String* pStatus) = 0;
    virtual void GetLastConnectionStats(const unsigned int& rPrincipalID, DateTime* pDate1, DateTime* pDate2,
                                        DateTime* pDate3) = 0;
    virtual bool ResetPassword() = 0;
    virtual void CreateAccountWithCustomData(const String& rName, const String& rKey, const unsigned int& rGroups,
                                             const String& rEmail, const DataHolder& rPublicData,
                                             const DataHolder& rPrivateData) = 0;
    virtual void RetrieveAccount(AccountData* pAccountData, DataHolder* pPublicData, DataHolder* pPrivateData) = 0;
    virtual void UpdateAccount(const String& rKey, const String& rEmail, const DataHolder& rPublicData,
                               const DataHolder& rPrivateData) = 0;
    virtual void ChangePasswordByGuest(const String& rName, const String& rString1, const String& rString2) = 0;
    virtual void FindByNameLike(const unsigned int& rGroups, const String& rPattern, const ResultRange& rRange,
                                qList<BasicAccountInfo>* pAccountInfos) = 0;
    virtual void CustomCreateAccount(const String& rName, const String& rKey, const unsigned int& rGroups,
                                     const String& rEmail, const DataHolder& rData, unsigned int* pPrincipalID) = 0;
    virtual void NintendoCreateAccount(const String& rName, const String& rKey, const unsigned int& rGroups,
                                       const String& rEmail, const DataHolder& rData, unsigned int* pPrincipalID,
                                       String* pString) = 0;
    virtual void LookupOrCreateAccount(const String& rName, const String& rKey, const unsigned int& rGroups,
                                       const String& rEmail, const DataHolder& rData, unsigned int* pPrincipalID) = 0;
    virtual bool DisconnectPrincipal(const unsigned int& rPrincipalID) = 0;
    virtual bool DisconnectAllPrincipals() = 0;

private:
    enum
    {
        RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u),
        RESPONSE_METHOD_FLAG = 0x8000
    };

    static void ProtoStub_RetrieveAccount(Message* pRequest, Message* pResponse,
                                          _Proto_AccountManagementProtocolServer* pProtocol);
    static void ProtoStub_UpdateAccount(Message* pRequest, Message* pResponse,
                                        _Proto_AccountManagementProtocolServer* pProtocol);
    static void ProtoStub_ChangePasswordByGuest(Message* pRequest, Message* pResponse,
                                                _Proto_AccountManagementProtocolServer* pProtocol);
    static void ProtoStub_FindByNameLike(Message* pRequest, Message* pResponse,
                                         _Proto_AccountManagementProtocolServer* pProtocol);
    static void ProtoStub_CustomCreateAccount(Message* pRequest, Message* pResponse,
                                              _Proto_AccountManagementProtocolServer* pProtocol);
    static void ProtoStub_NintendoCreateAccount(Message* pRequest, Message* pResponse,
                                                _Proto_AccountManagementProtocolServer* pProtocol);
    static void ProtoStub_LookupOrCreateAccount(Message* pRequest, Message* pResponse,
                                                _Proto_AccountManagementProtocolServer* pProtocol);
    static void ProtoStub_DisconnectPrincipal(Message* pRequest, Message* pResponse,
                                              _Proto_AccountManagementProtocolServer* pProtocol);
    static void ProtoStub_DisconnectAllPrincipals(Message* pRequest, Message* pResponse,
                                                  _Proto_AccountManagementProtocolServer* pProtocol);
};

}
}

#endif
