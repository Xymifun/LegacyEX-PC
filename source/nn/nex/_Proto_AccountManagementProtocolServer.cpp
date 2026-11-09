#include "nn/nex/_Proto_AccountManagementProtocolServer.hpp"

#include <stdint.h>

#include "nn/nex/AccountData.hpp"
#include "nn/nex/BasicAccountInfo.hpp"
#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/DateTime.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ResultRange.hpp"
#include "nn/nex/_DDL_AccountData.hpp"
#include "nn/nex/_DDL_BasicAccountInfo.hpp"
#include "nn/nex/_Type_qlist.hpp"
#include "nn/nex/_Type_qresult.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef _Type_any<DDLClassMetaType<Data>, _Type_string> DataAny;

}

void _Proto_AccountManagementProtocolServer::ProtoStub_RetrieveAccount(
    Message*, Message* pResponse, _Proto_AccountManagementProtocolServer* pProtocol)
{
    AccountData accountData;
    DataHolder publicData;
    DataHolder privateData;
    pProtocol->RetrieveAccount(&accountData, &publicData, &privateData);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_RETRIEVE_ACCOUNT);
    _DDL_AccountData::Add(pResponse, accountData);
    DataAny::Add(pResponse, publicData);
    DataAny::Add(pResponse, privateData);
}

void _Proto_AccountManagementProtocolServer::ProtoStub_UpdateAccount(Message* pRequest, Message* pResponse,
                                                                     _Proto_AccountManagementProtocolServer* pProtocol)
{
    String key;
    _Type_string::Extract(pRequest, &key);
    if (pRequest->HasError())
        return;
    String email;
    _Type_string::Extract(pRequest, &email);
    if (pRequest->HasError())
        return;
    DataHolder publicData;
    DataAny::Extract(pRequest, &publicData);
    if (pRequest->HasError())
        return;
    DataHolder privateData;
    DataAny::Extract(pRequest, &privateData);
    if (pRequest->HasError())
        return;
    pProtocol->UpdateAccount(key, email, publicData, privateData);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_ACCOUNT);
}

void _Proto_AccountManagementProtocolServer::ProtoStub_ChangePasswordByGuest(
    Message* pRequest, Message* pResponse, _Proto_AccountManagementProtocolServer* pProtocol)
{
    String name;
    _Type_string::Extract(pRequest, &name);
    if (pRequest->HasError())
        return;
    String string1;
    _Type_string::Extract(pRequest, &string1);
    if (pRequest->HasError())
        return;
    String string2;
    _Type_string::Extract(pRequest, &string2);
    if (pRequest->HasError())
        return;
    pProtocol->ChangePasswordByGuest(name, string1, string2);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CHANGE_PASSWORD_BY_GUEST);
}

void _Proto_AccountManagementProtocolServer::ProtoStub_FindByNameLike(Message* pRequest, Message* pResponse,
                                                                      _Proto_AccountManagementProtocolServer* pProtocol)
{
    unsigned int uiGroups;
    _Type_uint32::Extract(pRequest, &uiGroups);
    if (pRequest->HasError())
        return;
    String pattern;
    _Type_string::Extract(pRequest, &pattern);
    if (pRequest->HasError())
        return;
    ResultRange range;
    DDLClassMetaType<ResultRange>::Extract(pRequest, &range);
    if (pRequest->HasError())
        return;
    qList<BasicAccountInfo> accountInfos;
    pProtocol->FindByNameLike(uiGroups, pattern, range, &accountInfos);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_BY_NAME_LIKE);
    _Type_std_list<DDLClassMetaType<BasicAccountInfo> >::Add(pResponse, accountInfos);
}

void _Proto_AccountManagementProtocolServer::ProtoStub_CustomCreateAccount(
    Message* pRequest, Message* pResponse, _Proto_AccountManagementProtocolServer* pProtocol)
{
    String name;
    _Type_string::Extract(pRequest, &name);
    if (pRequest->HasError())
        return;
    String key;
    _Type_string::Extract(pRequest, &key);
    if (pRequest->HasError())
        return;
    unsigned int uiGroups;
    _Type_uint32::Extract(pRequest, &uiGroups);
    if (pRequest->HasError())
        return;
    String email;
    _Type_string::Extract(pRequest, &email);
    if (pRequest->HasError())
        return;
    DataHolder data;
    DataAny::Extract(pRequest, &data);
    if (pRequest->HasError())
        return;
    unsigned int uiPrincipalID;
    pProtocol->CustomCreateAccount(name, key, uiGroups, email, data, &uiPrincipalID);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CUSTOM_CREATE_ACCOUNT);
    _Type_uint32::Add(pResponse, uiPrincipalID);
}

void _Proto_AccountManagementProtocolServer::ProtoStub_NintendoCreateAccount(
    Message* pRequest, Message* pResponse, _Proto_AccountManagementProtocolServer* pProtocol)
{
    String name;
    _Type_string::Extract(pRequest, &name);
    if (pRequest->HasError())
        return;
    String key;
    _Type_string::Extract(pRequest, &key);
    if (pRequest->HasError())
        return;
    unsigned int uiGroups;
    _Type_uint32::Extract(pRequest, &uiGroups);
    if (pRequest->HasError())
        return;
    String email;
    _Type_string::Extract(pRequest, &email);
    if (pRequest->HasError())
        return;
    DataHolder data;
    DataAny::Extract(pRequest, &data);
    if (pRequest->HasError())
        return;
    unsigned int uiPrincipalID;
    String string;
    pProtocol->NintendoCreateAccount(name, key, uiGroups, email, data, &uiPrincipalID, &string);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_NINTENDO_CREATE_ACCOUNT);
    _Type_uint32::Add(pResponse, uiPrincipalID);
    _Type_string::Add(pResponse, string);
}

void _Proto_AccountManagementProtocolServer::ProtoStub_LookupOrCreateAccount(
    Message* pRequest, Message* pResponse, _Proto_AccountManagementProtocolServer* pProtocol)
{
    String name;
    _Type_string::Extract(pRequest, &name);
    if (pRequest->HasError())
        return;
    String key;
    _Type_string::Extract(pRequest, &key);
    if (pRequest->HasError())
        return;
    unsigned int uiGroups;
    _Type_uint32::Extract(pRequest, &uiGroups);
    if (pRequest->HasError())
        return;
    String email;
    _Type_string::Extract(pRequest, &email);
    if (pRequest->HasError())
        return;
    DataHolder data;
    DataAny::Extract(pRequest, &data);
    if (pRequest->HasError())
        return;
    unsigned int uiPrincipalID;
    pProtocol->LookupOrCreateAccount(name, key, uiGroups, email, data, &uiPrincipalID);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_LOOKUP_OR_CREATE_ACCOUNT);
    _Type_uint32::Add(pResponse, uiPrincipalID);
}

void _Proto_AccountManagementProtocolServer::ProtoStub_DisconnectPrincipal(
    Message* pRequest, Message* pResponse, _Proto_AccountManagementProtocolServer* pProtocol)
{
    unsigned int uiPrincipalID;
    _Type_uint32::Extract(pRequest, &uiPrincipalID);
    if (pRequest->HasError())
        return;
    const bool bResult = pProtocol->DisconnectPrincipal(uiPrincipalID);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_DISCONNECT_PRINCIPAL);
    *pResponse << bResult;
}

void _Proto_AccountManagementProtocolServer::ProtoStub_DisconnectAllPrincipals(
    Message*, Message* pResponse, _Proto_AccountManagementProtocolServer* pProtocol)
{
    const bool bResult = pProtocol->DisconnectAllPrincipals();
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_DISCONNECT_ALL_PRINCIPALS);
    *pResponse << bResult;
}

void _Proto_AccountManagementProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                                     bool* pResponseRequired, EndPoint*)
{
    switch (static_cast<uint16_t>(ExtractMethodID(pRequest)))
    {
    case METHOD_CREATE_ACCOUNT:
    {
        String name;
        _Type_string::Extract(pRequest, &name);
        if (pRequest->HasError())
            break;
        String key;
        _Type_string::Extract(pRequest, &key);
        if (pRequest->HasError())
            break;
        unsigned int uiGroups;
        _Type_uint32::Extract(pRequest, &uiGroups);
        if (pRequest->HasError())
            break;
        String email;
        _Type_string::Extract(pRequest, &email);
        if (pRequest->HasError())
            break;
        const qResult result = CreateAccount(name, key, uiGroups, email);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CREATE_ACCOUNT);
        _Type_qresult::Add(pResponse, result);
        break;
    }
    case METHOD_DELETE_ACCOUNT:
    {
        unsigned int uiPrincipalID;
        _Type_uint32::Extract(pRequest, &uiPrincipalID);
        if (pRequest->HasError())
            break;
        DeleteAccount(uiPrincipalID);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_DELETE_ACCOUNT);
        break;
    }
    case METHOD_DISABLE_ACCOUNT:
    {
        unsigned int uiPrincipalID;
        _Type_uint32::Extract(pRequest, &uiPrincipalID);
        if (pRequest->HasError())
            break;
        DateTime date;
        unsigned long long uiDate;
        pRequest->ExtractPrimitive(&uiDate);
        date = DateTime(uiDate);
        if (pRequest->HasError())
            break;
        String message;
        _Type_string::Extract(pRequest, &message);
        if (pRequest->HasError())
            break;
        const qResult result = DisableAccount(uiPrincipalID, date, message);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_DISABLE_ACCOUNT);
        _Type_qresult::Add(pResponse, result);
        break;
    }
    case METHOD_CHANGE_PASSWORD:
    {
        String newKey;
        _Type_string::Extract(pRequest, &newKey);
        if (pRequest->HasError())
            break;
        const bool bResult = ChangePassword(newKey);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CHANGE_PASSWORD);
        *pResponse << bResult;
        break;
    }
    case METHOD_TEST_CAPABILITY:
    {
        unsigned int uiCapability;
        _Type_uint32::Extract(pRequest, &uiCapability);
        if (pRequest->HasError())
            break;
        const bool bResult = TestCapability(uiCapability);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_TEST_CAPABILITY);
        *pResponse << bResult;
        break;
    }
    case METHOD_GET_NAME:
    {
        unsigned int uiPrincipalID;
        _Type_uint32::Extract(pRequest, &uiPrincipalID);
        if (pRequest->HasError())
            break;
        String name;
        GetName(uiPrincipalID, &name);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_NAME);
        _Type_string::Add(pResponse, name);
        break;
    }
    case METHOD_GET_ACCOUNT_DATA:
    {
        AccountData accountData;
        const qResult result = GetAccountData(&accountData);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_ACCOUNT_DATA);
        _Type_qresult::Add(pResponse, result);
        _DDL_AccountData::Add(pResponse, accountData);
        break;
    }
    case METHOD_GET_PRIVATE_DATA:
    {
        DataHolder data;
        const bool bResult = GetPrivateData(&data);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_PRIVATE_DATA);
        *pResponse << bResult;
        DataAny::Add(pResponse, data);
        break;
    }
    case METHOD_GET_PUBLIC_DATA:
    {
        unsigned int uiPrincipalID;
        _Type_uint32::Extract(pRequest, &uiPrincipalID);
        if (pRequest->HasError())
            break;
        DataHolder data;
        const bool bResult = GetPublicData(uiPrincipalID, &data);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_PUBLIC_DATA);
        *pResponse << bResult;
        DataAny::Add(pResponse, data);
        break;
    }
    case METHOD_GET_MULTIPLE_PUBLIC_DATA:
    {
        qList<unsigned int> principalIDs;
        _Type_std_list<_Type_uint32>::Extract(pRequest, &principalIDs);
        if (pRequest->HasError())
            break;
        DataHolderList dataList;
        const bool bResult = GetMultiplePublicData(principalIDs, &dataList);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_MULTIPLE_PUBLIC_DATA);
        *pResponse << bResult;
        _Type_std_list<DataAny>::Add(pResponse, dataList);
        break;
    }
    case METHOD_UPDATE_ACCOUNT_NAME:
    {
        String name;
        _Type_string::Extract(pRequest, &name);
        if (pRequest->HasError())
            break;
        const qResult result = UpdateAccountName(name);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_ACCOUNT_NAME);
        _Type_qresult::Add(pResponse, result);
        break;
    }
    case METHOD_UPDATE_ACCOUNT_EMAIL:
    {
        String email;
        _Type_string::Extract(pRequest, &email);
        if (pRequest->HasError())
            break;
        const qResult result = UpdateAccountEmail(email);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_ACCOUNT_EMAIL);
        _Type_qresult::Add(pResponse, result);
        break;
    }
    case METHOD_UPDATE_CUSTOM_DATA:
    {
        DataHolder publicData;
        DataAny::Extract(pRequest, &publicData);
        if (pRequest->HasError())
            break;
        DataHolder privateData;
        DataAny::Extract(pRequest, &privateData);
        if (pRequest->HasError())
            break;
        const qResult result = UpdateCustomData(publicData, privateData);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_CUSTOM_DATA);
        _Type_qresult::Add(pResponse, result);
        break;
    }
    case METHOD_FIND_BY_NAME_REGEX:
    {
        unsigned int uiGroups;
        _Type_uint32::Extract(pRequest, &uiGroups);
        if (pRequest->HasError())
            break;
        String regex;
        _Type_string::Extract(pRequest, &regex);
        if (pRequest->HasError())
            break;
        ResultRange range;
        DDLClassMetaType<ResultRange>::Extract(pRequest, &range);
        if (pRequest->HasError())
            break;
        qList<BasicAccountInfo> accountInfos;
        FindByNameRegex(uiGroups, regex, range, &accountInfos);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_BY_NAME_REGEX);
        _Type_std_list<DDLClassMetaType<BasicAccountInfo> >::Add(pResponse, accountInfos);
        break;
    }
    case METHOD_UPDATE_ACCOUNT_EXPIRY_DATE:
    {
        unsigned int uiPrincipalID;
        _Type_uint32::Extract(pRequest, &uiPrincipalID);
        if (pRequest->HasError())
            break;
        DateTime date;
        unsigned long long uiDate;
        pRequest->ExtractPrimitive(&uiDate);
        date = DateTime(uiDate);
        if (pRequest->HasError())
            break;
        String message;
        _Type_string::Extract(pRequest, &message);
        if (pRequest->HasError())
            break;
        UpdateAccountExpiryDate(uiPrincipalID, date, message);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_ACCOUNT_EXPIRY_DATE);
        break;
    }
    case METHOD_UPDATE_ACCOUNT_EFFECTIVE_DATE:
    {
        unsigned int uiPrincipalID;
        _Type_uint32::Extract(pRequest, &uiPrincipalID);
        if (pRequest->HasError())
            break;
        DateTime date;
        unsigned long long uiDate;
        pRequest->ExtractPrimitive(&uiDate);
        date = DateTime(uiDate);
        if (pRequest->HasError())
            break;
        String message;
        _Type_string::Extract(pRequest, &message);
        if (pRequest->HasError())
            break;
        UpdateAccountEffectiveDate(uiPrincipalID, date, message);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_ACCOUNT_EFFECTIVE_DATE);
        break;
    }
    case METHOD_UPDATE_STATUS:
    {
        String status;
        _Type_string::Extract(pRequest, &status);
        if (pRequest->HasError())
            break;
        UpdateStatus(status);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_STATUS);
        break;
    }
    case METHOD_GET_STATUS:
    {
        unsigned int uiPrincipalID;
        _Type_uint32::Extract(pRequest, &uiPrincipalID);
        if (pRequest->HasError())
            break;
        String status;
        GetStatus(uiPrincipalID, &status);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_STATUS);
        _Type_string::Add(pResponse, status);
        break;
    }
    case METHOD_GET_LAST_CONNECTION_STATS:
    {
        unsigned int uiPrincipalID;
        _Type_uint32::Extract(pRequest, &uiPrincipalID);
        if (pRequest->HasError())
            break;
        DateTime date1;
        DateTime date2;
        DateTime date3;
        GetLastConnectionStats(uiPrincipalID, &date1, &date2, &date3);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_LAST_CONNECTION_STATS);
        const unsigned long long uiDate1 = static_cast<const DateTime&>(date1);
        pResponse->Append(reinterpret_cast<const unsigned char*>(&uiDate1), 8, 1);
        const unsigned long long uiDate2 = static_cast<const DateTime&>(date2);
        pResponse->Append(reinterpret_cast<const unsigned char*>(&uiDate2), 8, 1);
        const unsigned long long uiDate3 = static_cast<const DateTime&>(date3);
        pResponse->Append(reinterpret_cast<const unsigned char*>(&uiDate3), 8, 1);
        break;
    }
    case METHOD_RESET_PASSWORD:
    {
        const bool bResult = ResetPassword();
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_RESET_PASSWORD);
        *pResponse << bResult;
        break;
    }
    case METHOD_CREATE_ACCOUNT_WITH_CUSTOM_DATA:
    {
        String name;
        _Type_string::Extract(pRequest, &name);
        if (pRequest->HasError())
            break;
        String key;
        _Type_string::Extract(pRequest, &key);
        if (pRequest->HasError())
            break;
        unsigned int uiGroups;
        _Type_uint32::Extract(pRequest, &uiGroups);
        if (pRequest->HasError())
            break;
        String email;
        _Type_string::Extract(pRequest, &email);
        if (pRequest->HasError())
            break;
        DataHolder publicData;
        DataAny::Extract(pRequest, &publicData);
        if (pRequest->HasError())
            break;
        DataHolder privateData;
        DataAny::Extract(pRequest, &privateData);
        if (pRequest->HasError())
            break;
        CreateAccountWithCustomData(name, key, uiGroups, email, publicData, privateData);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CREATE_ACCOUNT_WITH_CUSTOM_DATA);
        break;
    }
    case METHOD_RETRIEVE_ACCOUNT:
        ProtoStub_RetrieveAccount(pRequest, pResponse, this);
        break;
    case METHOD_UPDATE_ACCOUNT:
        ProtoStub_UpdateAccount(pRequest, pResponse, this);
        break;
    case METHOD_CHANGE_PASSWORD_BY_GUEST:
        ProtoStub_ChangePasswordByGuest(pRequest, pResponse, this);
        break;
    case METHOD_FIND_BY_NAME_LIKE:
        ProtoStub_FindByNameLike(pRequest, pResponse, this);
        break;
    case METHOD_CUSTOM_CREATE_ACCOUNT:
        ProtoStub_CustomCreateAccount(pRequest, pResponse, this);
        break;
    case METHOD_NINTENDO_CREATE_ACCOUNT:
        ProtoStub_NintendoCreateAccount(pRequest, pResponse, this);
        break;
    case METHOD_LOOKUP_OR_CREATE_ACCOUNT:
        ProtoStub_LookupOrCreateAccount(pRequest, pResponse, this);
        break;
    case METHOD_DISCONNECT_PRINCIPAL:
        ProtoStub_DisconnectPrincipal(pRequest, pResponse, this);
        break;
    case METHOD_DISCONNECT_ALL_PRINCIPALS:
        ProtoStub_DisconnectAllPrincipals(pRequest, pResponse, this);
        break;
    case 0:
    default:
        SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
    *pResponseRequired = true;
}

}
}
