#include "nn/nex/AccountManagementProtocolClient.hpp"

#include "nn/nex/AccountData.hpp"
#include "nn/nex/BasicAccountInfo.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/DateTime.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/ResultRange.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/_DDL_AccountData.hpp"
#include "nn/nex/_DDL_BasicAccountInfo.hpp"
#include "nn/nex/_DDL_ResultRange.hpp"
#include "nn/nex/_Type_qresult.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);
typedef _Type_any<DDLClassMetaType<Data>, _Type_string> DataAny;
typedef AccountManagementProtocolClient::DataHolder DataHolder;
typedef AccountManagementProtocolClient::DataHolderList DataHolderList;

const unsigned int METHOD_ID_MASK = 0x7FFF;
enum
{
    RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u)
};
const unsigned int METHOD_ID_COUNT = 31;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;
enum MethodId
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
ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

AccountManagementProtocolClient::AccountManagementProtocolClient(unsigned short uiProtocolID) : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(uiProtocolID,
                                                                         new AccountManagementProtocolClient());
}

void AccountManagementProtocolClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
{
    const unsigned int uiMethodID = Protocol::ExtractMethodID(pMessage) & METHOD_ID_MASK;
    if (uiMethodID - 1 >= METHOD_ID_COUNT - 1)
    {
        ClientProtocol::SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
    if (s_ProtoReturn[uiMethodID] != 0)
        s_ProtoReturn[uiMethodID](pMessage, pCallContext);
}

Protocol* AccountManagementProtocolClient::CreateResponder() const
{
    return new AccountManagementProtocolClient();
}

void AccountManagementProtocolClient::ProtoReturn_CreateAccount(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult == 0)
    {
        qResult discardedResult[1];
        pResult = &discardedResult[0];
    }
    _Type_qresult::Extract(pMessage, pResult);
}

bool AccountManagementProtocolClient::CallCreateAccount(ProtocolCallContext* pCallContext, qResult* pResult,
                                                        const String& rName, const String& rKey,
                                                        const unsigned int& rGroups, const String& rEmail)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CREATE_ACCOUNT);
    _Type_string::Add(&message, rName);
    _Type_string::Add(&message, rKey);
    _Type_uint32::Add(&message, rGroups);
    _Type_string::Add(&message, rEmail);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
    }
    s_ProtoReturn[METHOD_CREATE_ACCOUNT] = ProtoReturn_CreateAccount;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::CreateAccount_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   qResult* pResult, const String& rName,
                                                                   const String& rKey, const unsigned int& rGroups,
                                                                   const String& rEmail)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_CREATE_ACCOUNT);
    _Type_string::Add(pMessage, rName);
    _Type_string::Add(pMessage, rKey);
    _Type_uint32::Add(pMessage, rGroups);
    _Type_string::Add(pMessage, rEmail);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
    }
    s_ProtoReturn[METHOD_CREATE_ACCOUNT] = ProtoReturn_CreateAccount;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_DeleteAccount(Message*, ProtocolCallContext*)
{
}

bool AccountManagementProtocolClient::CallDeleteAccount(ProtocolCallContext* pCallContext,
                                                        const unsigned int& rPrincipalID)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_DELETE_ACCOUNT);
    _Type_uint32::Add(&message, rPrincipalID);
    s_ProtoReturn[METHOD_DELETE_ACCOUNT] = ProtoReturn_DeleteAccount;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::DeleteAccount_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   const unsigned int& rPrincipalID)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_DELETE_ACCOUNT);
    _Type_uint32::Add(pMessage, rPrincipalID);
    s_ProtoReturn[METHOD_DELETE_ACCOUNT] = ProtoReturn_DeleteAccount;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_DisableAccount(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult == 0)
    {
        qResult discardedResult[1];
        pResult = &discardedResult[0];
    }
    _Type_qresult::Extract(pMessage, pResult);
}

bool AccountManagementProtocolClient::CallDisableAccount(ProtocolCallContext* pCallContext, qResult* pResult,
                                                         const unsigned int& rPrincipalID, const DateTime& rDate,
                                                         const String& rMessage)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_DISABLE_ACCOUNT);
    _Type_uint32::Add(&message, rPrincipalID);
    const unsigned long long uiDate = rDate;
    message.Append(reinterpret_cast<const unsigned char*>(&uiDate), 8, 1);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
    }
    s_ProtoReturn[METHOD_DISABLE_ACCOUNT] = ProtoReturn_DisableAccount;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::DisableAccount_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                    Message* pMessage, qResult* pResult,
                                                                    const unsigned int& rPrincipalID,
                                                                    const DateTime& rDate, const String& rMessage)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_DISABLE_ACCOUNT);
    _Type_uint32::Add(pMessage, rPrincipalID);
    const unsigned long long uiDate = rDate;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiDate), 8, 1);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
    }
    s_ProtoReturn[METHOD_DISABLE_ACCOUNT] = ProtoReturn_DisableAccount;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_ChangePassword(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

bool AccountManagementProtocolClient::CallChangePassword(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                         const String& rNewKey)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CHANGE_PASSWORD);
    _Type_string::Add(&message, rNewKey);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_CHANGE_PASSWORD] = ProtoReturn_ChangePassword;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::ChangePassword_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                    Message* pMessage, bool* pSuccess,
                                                                    const String& rNewKey)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_CHANGE_PASSWORD);
    _Type_string::Add(pMessage, rNewKey);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_CHANGE_PASSWORD] = ProtoReturn_ChangePassword;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_TestCapability(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

bool AccountManagementProtocolClient::CallTestCapability(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                         const unsigned int& rCapability)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_TEST_CAPABILITY);
    _Type_uint32::Add(&message, rCapability);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_TEST_CAPABILITY] = ProtoReturn_TestCapability;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::TestCapability_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                    Message* pMessage, bool* pSuccess,
                                                                    const unsigned int& rCapability)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_TEST_CAPABILITY);
    _Type_uint32::Add(pMessage, rCapability);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_TEST_CAPABILITY] = ProtoReturn_TestCapability;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_GetName(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pName = pCallContext->GetReturnValuePtr(0);
    if (pName != 0)
    {
        _Type_string::Extract(pMessage, static_cast<String*>(pName));
        return;
    }
    String discardedString[1];
    _Type_string::Extract(pMessage, &discardedString[0]);
}

bool AccountManagementProtocolClient::CallGetName(ProtocolCallContext* pCallContext, const unsigned int& rPrincipalID,
                                                  String* pName)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_NAME);
    _Type_uint32::Add(&message, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pName);
    }
    s_ProtoReturn[METHOD_GET_NAME] = ProtoReturn_GetName;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::GetName_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                             const unsigned int& rPrincipalID, String* pName)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_NAME);
    _Type_uint32::Add(pMessage, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pName);
    }
    s_ProtoReturn[METHOD_GET_NAME] = ProtoReturn_GetName;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_GetAccountData(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult == 0)
    {
        qResult discardedResult[1];
        pResult = &discardedResult[0];
    }
    _Type_qresult::Extract(pMessage, pResult);
    if (pMessage->HasError())
        return;
    AccountData* pAccountData = static_cast<AccountData*>(pCallContext->GetReturnValuePtr(1));
    if (pAccountData != 0)
    {
        _DDL_AccountData::Extract(pMessage, pAccountData);
        return;
    }
    AccountData discardedAccountData[1];
    _DDL_AccountData::Extract(pMessage, &discardedAccountData[0]);
}

bool AccountManagementProtocolClient::CallGetAccountData(ProtocolCallContext* pCallContext, qResult* pResult,
                                                         AccountData* pAccountData)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_ACCOUNT_DATA);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pAccountData);
    }
    s_ProtoReturn[METHOD_GET_ACCOUNT_DATA] = ProtoReturn_GetAccountData;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::GetAccountData_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                    Message* pMessage, qResult* pResult,
                                                                    AccountData* pAccountData)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_ACCOUNT_DATA);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pAccountData);
    }
    s_ProtoReturn[METHOD_GET_ACCOUNT_DATA] = ProtoReturn_GetAccountData;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_GetPrivateData(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
    if (pMessage->HasError())
        return;
    DataHolder* pData = static_cast<DataHolder*>(pCallContext->GetReturnValuePtr(1));
    if (pData != 0)
    {
        DataAny::Extract(pMessage, pData);
    }
    else
    {
        DataHolder discardedData[1];
        DataAny::Extract(pMessage, &discardedData[0]);
    }
}

bool AccountManagementProtocolClient::CallGetPrivateData(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                         DataHolder* pData)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PRIVATE_DATA);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pData);
    }
    s_ProtoReturn[METHOD_GET_PRIVATE_DATA] = ProtoReturn_GetPrivateData;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::GetPrivateData_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                    Message* pMessage, bool* pSuccess,
                                                                    DataHolder* pData)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_PRIVATE_DATA);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pData);
    }
    s_ProtoReturn[METHOD_GET_PRIVATE_DATA] = ProtoReturn_GetPrivateData;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_GetPublicData(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
    if (pMessage->HasError())
        return;
    DataHolder* pData = static_cast<DataHolder*>(pCallContext->GetReturnValuePtr(1));
    if (pData != 0)
    {
        DataAny::Extract(pMessage, pData);
    }
    else
    {
        DataHolder discardedData[1];
        DataAny::Extract(pMessage, &discardedData[0]);
    }
}

bool AccountManagementProtocolClient::CallGetPublicData(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                        const unsigned int& rPrincipalID, DataHolder* pData)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PUBLIC_DATA);
    _Type_uint32::Add(&message, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pData);
    }
    s_ProtoReturn[METHOD_GET_PUBLIC_DATA] = ProtoReturn_GetPublicData;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::GetPublicData_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   bool* pSuccess, const unsigned int& rPrincipalID,
                                                                   DataHolder* pData)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_PUBLIC_DATA);
    _Type_uint32::Add(pMessage, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pData);
    }
    s_ProtoReturn[METHOD_GET_PUBLIC_DATA] = ProtoReturn_GetPublicData;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_GetMultiplePublicData(Message* pMessage,
                                                                        ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
    if (pMessage->HasError())
        return;
    DataHolderList* pDataList = static_cast<DataHolderList*>(pCallContext->GetReturnValuePtr(1));
    if (pDataList != 0)
    {
        _Type_std_list<DataAny>::Extract(pMessage, pDataList);
        return;
    }
    DataHolderList discardedDataList[1];
    _Type_std_list<DataAny>::Extract(pMessage, &discardedDataList[0]);
}

bool AccountManagementProtocolClient::CallGetMultiplePublicData(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                                const qList<unsigned int>& rPrincipalIDs,
                                                                DataHolderList* pDataList)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_MULTIPLE_PUBLIC_DATA);
    _Type_std_list<_Type_uint32>::Add(&message, rPrincipalIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pDataList);
    }
    s_ProtoReturn[METHOD_GET_MULTIPLE_PUBLIC_DATA] = ProtoReturn_GetMultiplePublicData;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::GetMultiplePublicData_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                           Message* pMessage, bool* pSuccess,
                                                                           const qList<unsigned int>& rPrincipalIDs,
                                                                           DataHolderList* pDataList)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_MULTIPLE_PUBLIC_DATA);
    _Type_std_list<_Type_uint32>::Add(pMessage, rPrincipalIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pDataList);
    }
    s_ProtoReturn[METHOD_GET_MULTIPLE_PUBLIC_DATA] = ProtoReturn_GetMultiplePublicData;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_UpdateAccountName(Message* pMessage,
                                                                    ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult == 0)
    {
        qResult discardedResult[1];
        pResult = &discardedResult[0];
    }
    _Type_qresult::Extract(pMessage, pResult);
}

bool AccountManagementProtocolClient::CallUpdateAccountName(ProtocolCallContext* pCallContext, qResult* pResult,
                                                            const String& rName)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_ACCOUNT_NAME);
    _Type_string::Add(&message, rName);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
    }
    s_ProtoReturn[METHOD_UPDATE_ACCOUNT_NAME] = ProtoReturn_UpdateAccountName;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::UpdateAccountName_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                       Message* pMessage, qResult* pResult,
                                                                       const String& rName)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_UPDATE_ACCOUNT_NAME);
    _Type_string::Add(pMessage, rName);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
    }
    s_ProtoReturn[METHOD_UPDATE_ACCOUNT_NAME] = ProtoReturn_UpdateAccountName;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_UpdateAccountEmail(Message* pMessage,
                                                                     ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult == 0)
    {
        qResult discardedResult[1];
        pResult = &discardedResult[0];
    }
    _Type_qresult::Extract(pMessage, pResult);
}

bool AccountManagementProtocolClient::CallUpdateAccountEmail(ProtocolCallContext* pCallContext, qResult* pResult,
                                                             const String& rEmail)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_ACCOUNT_EMAIL);
    _Type_string::Add(&message, rEmail);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
    }
    s_ProtoReturn[METHOD_UPDATE_ACCOUNT_EMAIL] = ProtoReturn_UpdateAccountEmail;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::UpdateAccountEmail_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                        Message* pMessage, qResult* pResult,
                                                                        const String& rEmail)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_UPDATE_ACCOUNT_EMAIL);
    _Type_string::Add(pMessage, rEmail);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
    }
    s_ProtoReturn[METHOD_UPDATE_ACCOUNT_EMAIL] = ProtoReturn_UpdateAccountEmail;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_UpdateCustomData(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult == 0)
    {
        qResult discardedResult[1];
        pResult = &discardedResult[0];
    }
    _Type_qresult::Extract(pMessage, pResult);
}

bool AccountManagementProtocolClient::CallUpdateCustomData(ProtocolCallContext* pCallContext, qResult* pResult,
                                                           const DataHolder& rPublicData,
                                                           const DataHolder& rPrivateData)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_CUSTOM_DATA);
    DataAny::Add(&message, rPublicData);
    DataAny::Add(&message, rPrivateData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
    }
    s_ProtoReturn[METHOD_UPDATE_CUSTOM_DATA] = ProtoReturn_UpdateCustomData;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::UpdateCustomData_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                      Message* pMessage, qResult* pResult,
                                                                      const DataHolder& rPublicData,
                                                                      const DataHolder& rPrivateData)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_UPDATE_CUSTOM_DATA);
    DataAny::Add(pMessage, rPublicData);
    DataAny::Add(pMessage, rPrivateData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
    }
    s_ProtoReturn[METHOD_UPDATE_CUSTOM_DATA] = ProtoReturn_UpdateCustomData;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_FindByNameRegex(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pAccountInfos = pCallContext->GetReturnValuePtr(0);
    if (pAccountInfos != 0)
    {
        _Type_std_list<DDLClassMetaType<BasicAccountInfo> >::Extract(
            pMessage, static_cast<qList<BasicAccountInfo>*>(pAccountInfos));
        return;
    }
    qList<BasicAccountInfo> discardedAccountInfos[1];
    _Type_std_list<DDLClassMetaType<BasicAccountInfo> >::Extract(pMessage, &discardedAccountInfos[0]);
}

bool AccountManagementProtocolClient::CallFindByNameRegex(ProtocolCallContext* pCallContext,
                                                          const unsigned int& rGroups, const String& rRegex,
                                                          const ResultRange& rRange,
                                                          qList<BasicAccountInfo>* pAccountInfos)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_FIND_BY_NAME_REGEX);
    _Type_uint32::Add(&message, rGroups);
    _Type_string::Add(&message, rRegex);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pAccountInfos);
    }
    s_ProtoReturn[METHOD_FIND_BY_NAME_REGEX] = ProtoReturn_FindByNameRegex;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::FindByNameRegex_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                     Message* pMessage, const unsigned int& rGroups,
                                                                     const String& rRegex, const ResultRange& rRange,
                                                                     qList<BasicAccountInfo>* pAccountInfos)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_FIND_BY_NAME_REGEX);
    _Type_uint32::Add(pMessage, rGroups);
    _Type_string::Add(pMessage, rRegex);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pAccountInfos);
    }
    s_ProtoReturn[METHOD_FIND_BY_NAME_REGEX] = ProtoReturn_FindByNameRegex;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_UpdateAccountExpiryDate(Message*, ProtocolCallContext*)
{
}

bool AccountManagementProtocolClient::CallUpdateAccountExpiryDate(ProtocolCallContext* pCallContext,
                                                                  const unsigned int& rPrincipalID,
                                                                  const DateTime& rDate, const String& rMessage)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_ACCOUNT_EXPIRY_DATE);
    _Type_uint32::Add(&message, rPrincipalID);
    const unsigned long long uiDate = rDate;
    message.Append(reinterpret_cast<const unsigned char*>(&uiDate), 8, 1);
    _Type_string::Add(&message, rMessage);
    s_ProtoReturn[METHOD_UPDATE_ACCOUNT_EXPIRY_DATE] = ProtoReturn_UpdateAccountExpiryDate;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::UpdateAccountExpiryDate_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                             Message* pMessage,
                                                                             const unsigned int& rPrincipalID,
                                                                             const DateTime& rDate,
                                                                             const String& rMessage)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_UPDATE_ACCOUNT_EXPIRY_DATE);
    _Type_uint32::Add(pMessage, rPrincipalID);
    const unsigned long long uiDate = rDate;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiDate), 8, 1);
    _Type_string::Add(pMessage, rMessage);
    s_ProtoReturn[METHOD_UPDATE_ACCOUNT_EXPIRY_DATE] = ProtoReturn_UpdateAccountExpiryDate;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_UpdateAccountEffectiveDate(Message*, ProtocolCallContext*)
{
}

bool AccountManagementProtocolClient::CallUpdateAccountEffectiveDate(ProtocolCallContext* pCallContext,
                                                                     const unsigned int& rPrincipalID,
                                                                     const DateTime& rDate, const String& rMessage)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_ACCOUNT_EFFECTIVE_DATE);
    _Type_uint32::Add(&message, rPrincipalID);
    const unsigned long long uiDate = rDate;
    message.Append(reinterpret_cast<const unsigned char*>(&uiDate), 8, 1);
    _Type_string::Add(&message, rMessage);
    s_ProtoReturn[METHOD_UPDATE_ACCOUNT_EFFECTIVE_DATE] = ProtoReturn_UpdateAccountEffectiveDate;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::UpdateAccountEffectiveDate_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                                Message* pMessage,
                                                                                const unsigned int& rPrincipalID,
                                                                                const DateTime& rDate,
                                                                                const String& rMessage)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_UPDATE_ACCOUNT_EFFECTIVE_DATE);
    _Type_uint32::Add(pMessage, rPrincipalID);
    const unsigned long long uiDate = rDate;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiDate), 8, 1);
    _Type_string::Add(pMessage, rMessage);
    s_ProtoReturn[METHOD_UPDATE_ACCOUNT_EFFECTIVE_DATE] = ProtoReturn_UpdateAccountEffectiveDate;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_UpdateStatus(Message*, ProtocolCallContext*)
{
}

bool AccountManagementProtocolClient::CallUpdateStatus(ProtocolCallContext* pCallContext, const String& rStatus)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_STATUS);
    _Type_string::Add(&message, rStatus);
    s_ProtoReturn[METHOD_UPDATE_STATUS] = ProtoReturn_UpdateStatus;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::UpdateStatus_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                  const String& rStatus)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_UPDATE_STATUS);
    _Type_string::Add(pMessage, rStatus);
    s_ProtoReturn[METHOD_UPDATE_STATUS] = ProtoReturn_UpdateStatus;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_GetStatus(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pStatus = pCallContext->GetReturnValuePtr(0);
    if (pStatus != 0)
    {
        _Type_string::Extract(pMessage, static_cast<String*>(pStatus));
        return;
    }
    String discardedString[1];
    _Type_string::Extract(pMessage, &discardedString[0]);
}

bool AccountManagementProtocolClient::CallGetStatus(ProtocolCallContext* pCallContext, const unsigned int& rPrincipalID,
                                                    String* pStatus)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_STATUS);
    _Type_uint32::Add(&message, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pStatus);
    }
    s_ProtoReturn[METHOD_GET_STATUS] = ProtoReturn_GetStatus;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::GetStatus_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               const unsigned int& rPrincipalID, String* pStatus)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_STATUS);
    _Type_uint32::Add(pMessage, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pStatus);
    }
    s_ProtoReturn[METHOD_GET_STATUS] = ProtoReturn_GetStatus;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_GetLastConnectionStats(Message* pMessage,
                                                                         ProtocolCallContext* pCallContext)
{
    DateTime* pDate1 = static_cast<DateTime*>(pCallContext->GetReturnValuePtr(0));
    if (pDate1 != 0)
    {
        unsigned long long uiDate;
        pMessage->ExtractPrimitive(&uiDate);
        *pDate1 = DateTime(uiDate);
    }
    else
    {
        DateTime discardedDate[1];
        unsigned long long uiDiscardedDate;
        pMessage->ExtractPrimitive(&uiDiscardedDate);
        discardedDate[0] = DateTime(uiDiscardedDate);
    }
    if (pMessage->HasError())
        return;
    DateTime* pDate2 = static_cast<DateTime*>(pCallContext->GetReturnValuePtr(1));
    if (pDate2 != 0)
    {
        unsigned long long uiDate;
        pMessage->ExtractPrimitive(&uiDate);
        *pDate2 = DateTime(uiDate);
    }
    else
    {
        DateTime discardedDate[1];
        unsigned long long uiDiscardedDate;
        pMessage->ExtractPrimitive(&uiDiscardedDate);
        discardedDate[0] = DateTime(uiDiscardedDate);
    }
    if (pMessage->HasError())
        return;
    DateTime* pDate3 = static_cast<DateTime*>(pCallContext->GetReturnValuePtr(2));
    if (pDate3 != 0)
    {
        unsigned long long uiDate;
        pMessage->ExtractPrimitive(&uiDate);
        *pDate3 = DateTime(uiDate);
    }
    else
    {
        DateTime discardedDate[1];
        unsigned long long uiDiscardedDate;
        pMessage->ExtractPrimitive(&uiDiscardedDate);
        discardedDate[0] = DateTime(uiDiscardedDate);
    }
}

bool AccountManagementProtocolClient::CallGetLastConnectionStats(ProtocolCallContext* pCallContext,
                                                                 const unsigned int& rPrincipalID, DateTime* pDate1,
                                                                 DateTime* pDate2, DateTime* pDate3)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_LAST_CONNECTION_STATS);
    _Type_uint32::Add(&message, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pDate1);
        pCallContext->AddReturnValuePtr(pDate2);
        pCallContext->AddReturnValuePtr(pDate3);
    }
    s_ProtoReturn[METHOD_GET_LAST_CONNECTION_STATS] = ProtoReturn_GetLastConnectionStats;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::GetLastConnectionStats_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                            Message* pMessage,
                                                                            const unsigned int& rPrincipalID,
                                                                            DateTime* pDate1, DateTime* pDate2,
                                                                            DateTime* pDate3)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_LAST_CONNECTION_STATS);
    _Type_uint32::Add(pMessage, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pDate1);
        pCallContext->AddReturnValuePtr(pDate2);
        pCallContext->AddReturnValuePtr(pDate3);
    }
    s_ProtoReturn[METHOD_GET_LAST_CONNECTION_STATS] = ProtoReturn_GetLastConnectionStats;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_ResetPassword(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

bool AccountManagementProtocolClient::CallResetPassword(ProtocolCallContext* pCallContext, bool* pSuccess)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_RESET_PASSWORD);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_RESET_PASSWORD] = ProtoReturn_ResetPassword;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::ResetPassword_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   bool* pSuccess)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_RESET_PASSWORD);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_RESET_PASSWORD] = ProtoReturn_ResetPassword;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_CreateAccountWithCustomData(Message*, ProtocolCallContext*)
{
}

bool AccountManagementProtocolClient::CallCreateAccountWithCustomData(ProtocolCallContext* pCallContext,
                                                                      const String& rName, const String& rKey,
                                                                      const unsigned int& rGroups, const String& rEmail,
                                                                      const DataHolder& rPublicData,
                                                                      const DataHolder& rPrivateData)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CREATE_ACCOUNT_WITH_CUSTOM_DATA);
    _Type_string::Add(&message, rName);
    _Type_string::Add(&message, rKey);
    _Type_uint32::Add(&message, rGroups);
    _Type_string::Add(&message, rEmail);
    DataAny::Add(&message, rPublicData);
    DataAny::Add(&message, rPrivateData);
    s_ProtoReturn[METHOD_CREATE_ACCOUNT_WITH_CUSTOM_DATA] = ProtoReturn_CreateAccountWithCustomData;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::CreateAccountWithCustomData_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const String& rName, const String& rKey,
    const unsigned int& rGroups, const String& rEmail, const DataHolder& rPublicData, const DataHolder& rPrivateData)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_CREATE_ACCOUNT_WITH_CUSTOM_DATA);
    _Type_string::Add(pMessage, rName);
    _Type_string::Add(pMessage, rKey);
    _Type_uint32::Add(pMessage, rGroups);
    _Type_string::Add(pMessage, rEmail);
    DataAny::Add(pMessage, rPublicData);
    DataAny::Add(pMessage, rPrivateData);
    s_ProtoReturn[METHOD_CREATE_ACCOUNT_WITH_CUSTOM_DATA] = ProtoReturn_CreateAccountWithCustomData;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_RetrieveAccount(Message* pMessage, ProtocolCallContext* pCallContext)
{
    AccountData* pAccountData = static_cast<AccountData*>(pCallContext->GetReturnValuePtr(0));
    if (pAccountData != 0)
    {
        _DDL_AccountData::Extract(pMessage, pAccountData);
        if (pMessage->HasError())
            return;
    }
    else
    {
        AccountData discardedAccountData[1];
        _DDL_AccountData::Extract(pMessage, &discardedAccountData[0]);
        if (pMessage->HasError())
            return;
    }
    DataHolder* pPublicData = static_cast<DataHolder*>(pCallContext->GetReturnValuePtr(1));
    if (pPublicData != 0)
    {
        DataAny::Extract(pMessage, pPublicData);
        if (pMessage->HasError())
            return;
    }
    else
    {
        DataHolder discardedData[1];
        DataAny::Extract(pMessage, &discardedData[0]);
        if (pMessage->HasError())
            return;
    }
    DataHolder* pPrivateData = static_cast<DataHolder*>(pCallContext->GetReturnValuePtr(2));
    if (pPrivateData != 0)
    {
        DataAny::Extract(pMessage, pPrivateData);
    }
    else
    {
        DataHolder discardedData[1];
        DataAny::Extract(pMessage, &discardedData[0]);
    }
}

bool AccountManagementProtocolClient::CallRetrieveAccount(ProtocolCallContext* pCallContext, AccountData* pAccountData,
                                                          DataHolder* pPublicData, DataHolder* pPrivateData)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_RETRIEVE_ACCOUNT);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pAccountData);
        pCallContext->AddReturnValuePtr(pPublicData);
        pCallContext->AddReturnValuePtr(pPrivateData);
    }
    s_ProtoReturn[METHOD_RETRIEVE_ACCOUNT] = ProtoReturn_RetrieveAccount;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::RetrieveAccount_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                     Message* pMessage, AccountData* pAccountData,
                                                                     DataHolder* pPublicData, DataHolder* pPrivateData)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_RETRIEVE_ACCOUNT);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pAccountData);
        pCallContext->AddReturnValuePtr(pPublicData);
        pCallContext->AddReturnValuePtr(pPrivateData);
    }
    s_ProtoReturn[METHOD_RETRIEVE_ACCOUNT] = ProtoReturn_RetrieveAccount;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_UpdateAccount(Message*, ProtocolCallContext*)
{
}

bool AccountManagementProtocolClient::CallUpdateAccount(ProtocolCallContext* pCallContext, const String& rKey,
                                                        const String& rEmail, const DataHolder& rPublicData,
                                                        const DataHolder& rPrivateData)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_ACCOUNT);
    _Type_string::Add(&message, rKey);
    _Type_string::Add(&message, rEmail);
    DataAny::Add(&message, rPublicData);
    DataAny::Add(&message, rPrivateData);
    s_ProtoReturn[METHOD_UPDATE_ACCOUNT] = ProtoReturn_UpdateAccount;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::UpdateAccount_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   const String& rKey, const String& rEmail,
                                                                   const DataHolder& rPublicData,
                                                                   const DataHolder& rPrivateData)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_UPDATE_ACCOUNT);
    _Type_string::Add(pMessage, rKey);
    _Type_string::Add(pMessage, rEmail);
    DataAny::Add(pMessage, rPublicData);
    DataAny::Add(pMessage, rPrivateData);
    s_ProtoReturn[METHOD_UPDATE_ACCOUNT] = ProtoReturn_UpdateAccount;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_ChangePasswordByGuest(Message*, ProtocolCallContext*)
{
}

bool AccountManagementProtocolClient::CallChangePasswordByGuest(ProtocolCallContext* pCallContext, const String& rName,
                                                                const String& rString1, const String& rString2)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CHANGE_PASSWORD_BY_GUEST);
    _Type_string::Add(&message, rName);
    _Type_string::Add(&message, rString1);
    _Type_string::Add(&message, rString2);
    s_ProtoReturn[METHOD_CHANGE_PASSWORD_BY_GUEST] = ProtoReturn_ChangePasswordByGuest;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::ChangePasswordByGuest_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                           Message* pMessage, const String& rName,
                                                                           const String& rString1,
                                                                           const String& rString2)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_CHANGE_PASSWORD_BY_GUEST);
    _Type_string::Add(pMessage, rName);
    _Type_string::Add(pMessage, rString1);
    _Type_string::Add(pMessage, rString2);
    s_ProtoReturn[METHOD_CHANGE_PASSWORD_BY_GUEST] = ProtoReturn_ChangePasswordByGuest;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_FindByNameLike(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pAccountInfos = pCallContext->GetReturnValuePtr(0);
    if (pAccountInfos != 0)
    {
        _Type_std_list<DDLClassMetaType<BasicAccountInfo> >::Extract(
            pMessage, static_cast<qList<BasicAccountInfo>*>(pAccountInfos));
        return;
    }
    qList<BasicAccountInfo> discardedAccountInfos[1];
    _Type_std_list<DDLClassMetaType<BasicAccountInfo> >::Extract(pMessage, &discardedAccountInfos[0]);
}

bool AccountManagementProtocolClient::CallFindByNameLike(ProtocolCallContext* pCallContext, const unsigned int& rGroups,
                                                         const String& rPattern, const ResultRange& rRange,
                                                         qList<BasicAccountInfo>* pAccountInfos)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_FIND_BY_NAME_LIKE);
    _Type_uint32::Add(&message, rGroups);
    _Type_string::Add(&message, rPattern);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pAccountInfos);
    }
    s_ProtoReturn[METHOD_FIND_BY_NAME_LIKE] = ProtoReturn_FindByNameLike;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::FindByNameLike_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                    Message* pMessage, const unsigned int& rGroups,
                                                                    const String& rPattern, const ResultRange& rRange,
                                                                    qList<BasicAccountInfo>* pAccountInfos)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_FIND_BY_NAME_LIKE);
    _Type_uint32::Add(pMessage, rGroups);
    _Type_string::Add(pMessage, rPattern);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pAccountInfos);
    }
    s_ProtoReturn[METHOD_FIND_BY_NAME_LIKE] = ProtoReturn_FindByNameLike;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_CustomCreateAccount(Message* pMessage,
                                                                      ProtocolCallContext* pCallContext)
{
    unsigned int uiDiscardedPrincipalID;
    unsigned int* pPrincipalID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(0));
    unsigned int* pDestination = &uiDiscardedPrincipalID;
    if (pPrincipalID != 0)
        pDestination = pPrincipalID;
    pMessage->ExtractPrimitive(pDestination);
}

bool AccountManagementProtocolClient::CallCustomCreateAccount(ProtocolCallContext* pCallContext, const String& rName,
                                                              const String& rKey, const unsigned int& rGroups,
                                                              const String& rEmail, const DataHolder& rData,
                                                              unsigned int* pPrincipalID)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CUSTOM_CREATE_ACCOUNT);
    _Type_string::Add(&message, rName);
    _Type_string::Add(&message, rKey);
    _Type_uint32::Add(&message, rGroups);
    _Type_string::Add(&message, rEmail);
    DataAny::Add(&message, rData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPrincipalID);
    }
    s_ProtoReturn[METHOD_CUSTOM_CREATE_ACCOUNT] = ProtoReturn_CustomCreateAccount;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::CustomCreateAccount_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const String& rName, const String& rKey,
    const unsigned int& rGroups, const String& rEmail, const DataHolder& rData, unsigned int* pPrincipalID)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_CUSTOM_CREATE_ACCOUNT);
    _Type_string::Add(pMessage, rName);
    _Type_string::Add(pMessage, rKey);
    _Type_uint32::Add(pMessage, rGroups);
    _Type_string::Add(pMessage, rEmail);
    DataAny::Add(pMessage, rData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPrincipalID);
    }
    s_ProtoReturn[METHOD_CUSTOM_CREATE_ACCOUNT] = ProtoReturn_CustomCreateAccount;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_NintendoCreateAccount(Message* pMessage,
                                                                        ProtocolCallContext* pCallContext)
{
    unsigned int uiDiscardedPrincipalID;
    unsigned int* pPrincipalID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(0));
    unsigned int* pDestination = &uiDiscardedPrincipalID;
    if (pPrincipalID != 0)
        pDestination = pPrincipalID;
    pMessage->ExtractPrimitive(pDestination);
    if (pMessage->HasError())
        return;
    String* pString = static_cast<String*>(pCallContext->GetReturnValuePtr(1));
    if (pString != 0)
    {
        _Type_string::Extract(pMessage, pString);
        return;
    }
    String discardedString[1];
    _Type_string::Extract(pMessage, &discardedString[0]);
}

bool AccountManagementProtocolClient::CallNintendoCreateAccount(ProtocolCallContext* pCallContext, const String& rName,
                                                                const String& rKey, const unsigned int& rGroups,
                                                                const String& rEmail, const DataHolder& rData,
                                                                unsigned int* pPrincipalID, String* pString)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_NINTENDO_CREATE_ACCOUNT);
    _Type_string::Add(&message, rName);
    _Type_string::Add(&message, rKey);
    _Type_uint32::Add(&message, rGroups);
    _Type_string::Add(&message, rEmail);
    DataAny::Add(&message, rData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPrincipalID);
        pCallContext->AddReturnValuePtr(pString);
    }
    s_ProtoReturn[METHOD_NINTENDO_CREATE_ACCOUNT] = ProtoReturn_NintendoCreateAccount;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::NintendoCreateAccount_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const String& rName, const String& rKey,
    const unsigned int& rGroups, const String& rEmail, const DataHolder& rData, unsigned int* pPrincipalID,
    String* pString)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_NINTENDO_CREATE_ACCOUNT);
    _Type_string::Add(pMessage, rName);
    _Type_string::Add(pMessage, rKey);
    _Type_uint32::Add(pMessage, rGroups);
    _Type_string::Add(pMessage, rEmail);
    DataAny::Add(pMessage, rData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPrincipalID);
        pCallContext->AddReturnValuePtr(pString);
    }
    s_ProtoReturn[METHOD_NINTENDO_CREATE_ACCOUNT] = ProtoReturn_NintendoCreateAccount;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_LookupOrCreateAccount(Message* pMessage,
                                                                        ProtocolCallContext* pCallContext)
{
    unsigned int uiDiscardedPrincipalID;
    unsigned int* pPrincipalID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(0));
    unsigned int* pDestination = &uiDiscardedPrincipalID;
    if (pPrincipalID != 0)
        pDestination = pPrincipalID;
    pMessage->ExtractPrimitive(pDestination);
}

bool AccountManagementProtocolClient::CallLookupOrCreateAccount(ProtocolCallContext* pCallContext, const String& rName,
                                                                const String& rKey, const unsigned int& rGroups,
                                                                const String& rEmail, const DataHolder& rData,
                                                                unsigned int* pPrincipalID)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_LOOKUP_OR_CREATE_ACCOUNT);
    _Type_string::Add(&message, rName);
    _Type_string::Add(&message, rKey);
    _Type_uint32::Add(&message, rGroups);
    _Type_string::Add(&message, rEmail);
    DataAny::Add(&message, rData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPrincipalID);
    }
    s_ProtoReturn[METHOD_LOOKUP_OR_CREATE_ACCOUNT] = ProtoReturn_LookupOrCreateAccount;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::LookupOrCreateAccount_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const String& rName, const String& rKey,
    const unsigned int& rGroups, const String& rEmail, const DataHolder& rData, unsigned int* pPrincipalID)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_LOOKUP_OR_CREATE_ACCOUNT);
    _Type_string::Add(pMessage, rName);
    _Type_string::Add(pMessage, rKey);
    _Type_uint32::Add(pMessage, rGroups);
    _Type_string::Add(pMessage, rEmail);
    DataAny::Add(pMessage, rData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPrincipalID);
    }
    s_ProtoReturn[METHOD_LOOKUP_OR_CREATE_ACCOUNT] = ProtoReturn_LookupOrCreateAccount;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_DisconnectPrincipal(Message* pMessage,
                                                                      ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

bool AccountManagementProtocolClient::CallDisconnectPrincipal(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                              const unsigned int& rPrincipalID)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_DISCONNECT_PRINCIPAL);
    _Type_uint32::Add(&message, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_DISCONNECT_PRINCIPAL] = ProtoReturn_DisconnectPrincipal;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::DisconnectPrincipal_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                         Message* pMessage, bool* pSuccess,
                                                                         const unsigned int& rPrincipalID)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_DISCONNECT_PRINCIPAL);
    _Type_uint32::Add(pMessage, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_DISCONNECT_PRINCIPAL] = ProtoReturn_DisconnectPrincipal;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void AccountManagementProtocolClient::ProtoReturn_DisconnectAllPrincipals(Message* pMessage,
                                                                          ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

bool AccountManagementProtocolClient::CallDisconnectAllPrincipals(ProtocolCallContext* pCallContext, bool* pSuccess)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_DISCONNECT_ALL_PRINCIPALS);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_DISCONNECT_ALL_PRINCIPALS] = ProtoReturn_DisconnectAllPrincipals;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool AccountManagementProtocolClient::DisconnectAllPrincipals_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                             Message* pMessage, bool* pSuccess)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_DISCONNECT_ALL_PRINCIPALS);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_DISCONNECT_ALL_PRINCIPALS] = ProtoReturn_DisconnectAllPrincipals;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

}
}
