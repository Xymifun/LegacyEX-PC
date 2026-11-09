#include "nn/nex/TicketGrantingProtocolClient.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/RVConnectionData.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/_Type_buffer.hpp"
#include "nn/nex/_Type_qresult.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);
typedef _Type_any<DDLClassMetaType<Data>, _Type_string> CustomDataAny;

const unsigned int METHOD_ID_MASK = 0x7FFF;
const unsigned int METHOD_ID_COUNT = 7;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;
enum MethodId
{
    METHOD_LOGIN = 1,
    METHOD_LOGIN_EX = 2,
    METHOD_REQUEST_TICKET = 3,
    METHOD_GET_PID = 4,
    METHOD_GET_NAME = 5,
    METHOD_LOGIN_WITH_CONTEXT = 6
};
ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

void TicketGrantingProtocolClient::ProtoReturn_Login(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult != 0)
    {
        _Type_qresult::Extract(pMessage, pResult);
        if (pMessage->HasError())
            return;
    }
    else
    {
        qResult discardedResult[1];
        _Type_qresult::Extract(pMessage, &discardedResult[0]);
        if (pMessage->HasError())
            return;
    }
    unsigned int uiDiscardedPrincipalID;
    unsigned int* pPrincipalID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(1));
    unsigned int* pDestination = &uiDiscardedPrincipalID;
    if (pPrincipalID != 0)
        pDestination = pPrincipalID;
    pMessage->ExtractPrimitive(pDestination);
    if (pMessage->HasError())
        return;
    Buffer* pTicket = static_cast<Buffer*>(pCallContext->GetReturnValuePtr(2));
    if (pTicket != 0)
    {
        _Type_buffer::Extract(pMessage, pTicket);
        if (pMessage->HasError())
            return;
    }
    else
    {
        Buffer discardedTicket[1];
        _Type_buffer::Extract(pMessage, &discardedTicket[0]);
        if (pMessage->HasError())
            return;
    }
    RVConnectionData* pConnectionData = static_cast<RVConnectionData*>(pCallContext->GetReturnValuePtr(3));
    if (pConnectionData != 0)
    {
        _DDL_RVConnectionData::Extract(pMessage, pConnectionData);
        if (pMessage->HasError())
            return;
    }
    else
    {
        RVConnectionData discardedConnectionData[1];
        _DDL_RVConnectionData::Extract(pMessage, &discardedConnectionData[0]);
        if (pMessage->HasError())
            return;
    }
    String* pReturnMessage = static_cast<String*>(pCallContext->GetReturnValuePtr(4));
    if (pReturnMessage != 0)
    {
        _Type_string::Extract(pMessage, pReturnMessage);
        return;
    }
    String discardedReturnMessage[1];
    _Type_string::Extract(pMessage, &discardedReturnMessage[0]);
}

void TicketGrantingProtocolClient::ProtoReturn_LoginEx(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult != 0)
    {
        _Type_qresult::Extract(pMessage, pResult);
        if (pMessage->HasError())
            return;
    }
    else
    {
        qResult discardedResult[1];
        _Type_qresult::Extract(pMessage, &discardedResult[0]);
        if (pMessage->HasError())
            return;
    }
    unsigned int uiDiscardedPrincipalID;
    unsigned int* pPrincipalID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(1));
    unsigned int* pDestination = &uiDiscardedPrincipalID;
    if (pPrincipalID != 0)
        pDestination = pPrincipalID;
    pMessage->ExtractPrimitive(pDestination);
    if (pMessage->HasError())
        return;
    Buffer* pTicket = static_cast<Buffer*>(pCallContext->GetReturnValuePtr(2));
    if (pTicket != 0)
    {
        _Type_buffer::Extract(pMessage, pTicket);
        if (pMessage->HasError())
            return;
    }
    else
    {
        Buffer discardedTicket[1];
        _Type_buffer::Extract(pMessage, &discardedTicket[0]);
        if (pMessage->HasError())
            return;
    }
    RVConnectionData* pConnectionData = static_cast<RVConnectionData*>(pCallContext->GetReturnValuePtr(3));
    if (pConnectionData != 0)
    {
        _DDL_RVConnectionData::Extract(pMessage, pConnectionData);
        if (pMessage->HasError())
            return;
    }
    else
    {
        RVConnectionData discardedConnectionData[1];
        _DDL_RVConnectionData::Extract(pMessage, &discardedConnectionData[0]);
        if (pMessage->HasError())
            return;
    }
    String* pReturnMessage = static_cast<String*>(pCallContext->GetReturnValuePtr(4));
    if (pReturnMessage != 0)
    {
        _Type_string::Extract(pMessage, pReturnMessage);
        return;
    }
    String discardedReturnMessage[1];
    _Type_string::Extract(pMessage, &discardedReturnMessage[0]);
}

void TicketGrantingProtocolClient::ProtoReturn_RequestTicket(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult == 0)
    {
        qResult discardedResult[1];
        pResult = &discardedResult[0];
    }
    _Type_qresult::Extract(pMessage, pResult);
    if (!pMessage->HasError())
    {
        Buffer* pTicket = static_cast<Buffer*>(pCallContext->GetReturnValuePtr(1));
        if (pTicket != 0)
        {
            _Type_buffer::Extract(pMessage, pTicket);
            return;
        }
        Buffer discardedTicket[1];
        _Type_buffer::Extract(pMessage, &discardedTicket[0]);
    }
}

void TicketGrantingProtocolClient::ProtoReturn_GetPID(Message* pMessage, ProtocolCallContext* pCallContext)
{
    unsigned int uiDiscardedPrincipalID;
    unsigned int* pPrincipalID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(0));
    unsigned int* pDestination = &uiDiscardedPrincipalID;
    if (pPrincipalID != 0)
        pDestination = pPrincipalID;
    pMessage->ExtractPrimitive(pDestination);
}

void TicketGrantingProtocolClient::ProtoReturn_GetName(Message* pMessage, ProtocolCallContext* pCallContext)
{
    String* pUserName = static_cast<String*>(pCallContext->GetReturnValuePtr(0));
    if (pUserName != 0)
    {
        _Type_string::Extract(pMessage, pUserName);
        return;
    }
    String discardedUserName[1];
    _Type_string::Extract(pMessage, &discardedUserName[0]);
}

void TicketGrantingProtocolClient::ProtoReturn_LoginWithContext(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qResult* pResult = static_cast<qResult*>(pCallContext->GetReturnValuePtr(0));
    if (pResult != 0)
    {
        _Type_qresult::Extract(pMessage, pResult);
        if (pMessage->HasError())
            return;
    }
    else
    {
        qResult discardedResult[1];
        _Type_qresult::Extract(pMessage, &discardedResult[0]);
        if (pMessage->HasError())
            return;
    }
    unsigned int uiDiscardedPrincipalID;
    unsigned int* pPrincipalID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(1));
    unsigned int* pDestination = &uiDiscardedPrincipalID;
    if (pPrincipalID != 0)
        pDestination = pPrincipalID;
    pMessage->ExtractPrimitive(pDestination);
    if (pMessage->HasError())
        return;
    Buffer* pTicket = static_cast<Buffer*>(pCallContext->GetReturnValuePtr(2));
    if (pTicket != 0)
    {
        _Type_buffer::Extract(pMessage, pTicket);
        if (pMessage->HasError())
            return;
    }
    else
    {
        Buffer discardedTicket[1];
        _Type_buffer::Extract(pMessage, &discardedTicket[0]);
        if (pMessage->HasError())
            return;
    }
    RVConnectionData* pConnectionData = static_cast<RVConnectionData*>(pCallContext->GetReturnValuePtr(3));
    if (pConnectionData != 0)
    {
        _DDL_RVConnectionData::Extract(pMessage, pConnectionData);
        return;
    }
    RVConnectionData discardedConnectionData[1];
    _DDL_RVConnectionData::Extract(pMessage, &discardedConnectionData[0]);
}

TicketGrantingProtocolClient::TicketGrantingProtocolClient(unsigned short uiProtocolID) : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(uiProtocolID,
                                                                         new TicketGrantingProtocolClient());
}

void TicketGrantingProtocolClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
{
    const unsigned int uiMethodID = Protocol::ExtractMethodID(pMessage) & METHOD_ID_MASK;
    if (uiMethodID - 1 >= METHOD_ID_COUNT - 1)
    {
        ClientProtocol::SetCallError(qResult(static_cast<int>(0x80010002u)));
        return;
    }
    if (s_ProtoReturn[uiMethodID] != 0)
        s_ProtoReturn[uiMethodID](pMessage, pCallContext);
}

Protocol* TicketGrantingProtocolClient::CreateResponder() const
{
    return new TicketGrantingProtocolClient();
}

bool TicketGrantingProtocolClient::CallLogin(ProtocolCallContext* pCallContext, qResult* pResult,
                                             const String& rUserName, unsigned int* pPrincipalID, Buffer* pTicket,
                                             RVConnectionData* pConnectionData, String* pReturnMessage)
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
    AddMethodID(&message, METHOD_LOGIN);
    _Type_string::Add(&message, rUserName);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pPrincipalID);
        pCallContext->AddReturnValuePtr(pTicket);
        pCallContext->AddReturnValuePtr(pConnectionData);
        pCallContext->AddReturnValuePtr(pReturnMessage);
    }
    s_ProtoReturn[METHOD_LOGIN] = ProtoReturn_Login;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool TicketGrantingProtocolClient::Login_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        qResult* pResult, const String& rUserName,
                                                        unsigned int* pPrincipalID, Buffer* pTicket,
                                                        RVConnectionData* pConnectionData, String* pReturnMessage)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_LOGIN);
    _Type_string::Add(pMessage, rUserName);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pPrincipalID);
        pCallContext->AddReturnValuePtr(pTicket);
        pCallContext->AddReturnValuePtr(pConnectionData);
        pCallContext->AddReturnValuePtr(pReturnMessage);
    }
    s_ProtoReturn[METHOD_LOGIN] = ProtoReturn_Login;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool TicketGrantingProtocolClient::CallLoginEx(ProtocolCallContext* pCallContext, qResult* pResult,
                                               const String& rUserName, unsigned int* pPrincipalID, Buffer* pTicket,
                                               RVConnectionData* pConnectionData, String* pReturnMessage,
                                               const AnyObjectHolder<Data, String>& rCustomData)
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
    AddMethodID(&message, METHOD_LOGIN_EX);
    _Type_string::Add(&message, rUserName);
    CustomDataAny::Add(&message, rCustomData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pPrincipalID);
        pCallContext->AddReturnValuePtr(pTicket);
        pCallContext->AddReturnValuePtr(pConnectionData);
        pCallContext->AddReturnValuePtr(pReturnMessage);
    }
    s_ProtoReturn[METHOD_LOGIN_EX] = ProtoReturn_LoginEx;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool TicketGrantingProtocolClient::LoginEx_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          qResult* pResult, const String& rUserName,
                                                          unsigned int* pPrincipalID, Buffer* pTicket,
                                                          RVConnectionData* pConnectionData, String* pReturnMessage,
                                                          const AnyObjectHolder<Data, String>& rCustomData)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_LOGIN_EX);
    _Type_string::Add(pMessage, rUserName);
    CustomDataAny::Add(pMessage, rCustomData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pPrincipalID);
        pCallContext->AddReturnValuePtr(pTicket);
        pCallContext->AddReturnValuePtr(pConnectionData);
        pCallContext->AddReturnValuePtr(pReturnMessage);
    }
    s_ProtoReturn[METHOD_LOGIN_EX] = ProtoReturn_LoginEx;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool TicketGrantingProtocolClient::CallRequestTicket(ProtocolCallContext* pCallContext, qResult* pResult,
                                                     const unsigned int& rSourcePrincipalID,
                                                     const unsigned int& rTargetPrincipalID, Buffer* pTicket)
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
    AddMethodID(&message, METHOD_REQUEST_TICKET);
    _Type_uint32::Add(&message, rSourcePrincipalID);
    _Type_uint32::Add(&message, rTargetPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pTicket);
    }
    s_ProtoReturn[METHOD_REQUEST_TICKET] = ProtoReturn_RequestTicket;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool TicketGrantingProtocolClient::RequestTicket_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                qResult* pResult,
                                                                const unsigned int& rSourcePrincipalID,
                                                                const unsigned int& rTargetPrincipalID, Buffer* pTicket)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_REQUEST_TICKET);
    _Type_uint32::Add(pMessage, rSourcePrincipalID);
    _Type_uint32::Add(pMessage, rTargetPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pTicket);
    }
    s_ProtoReturn[METHOD_REQUEST_TICKET] = ProtoReturn_RequestTicket;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool TicketGrantingProtocolClient::CallGetPID(ProtocolCallContext* pCallContext, unsigned int* pPrincipalID,
                                              const String& rUserName)
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
    AddMethodID(&message, METHOD_GET_PID);
    _Type_string::Add(&message, rUserName);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPrincipalID);
    }
    s_ProtoReturn[METHOD_GET_PID] = ProtoReturn_GetPID;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool TicketGrantingProtocolClient::GetPID_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                         unsigned int* pPrincipalID, const String& rUserName)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_PID);
    _Type_string::Add(pMessage, rUserName);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPrincipalID);
    }
    s_ProtoReturn[METHOD_GET_PID] = ProtoReturn_GetPID;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool TicketGrantingProtocolClient::CallGetName(ProtocolCallContext* pCallContext, String* pUserName,
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
    AddMethodID(&message, METHOD_GET_NAME);
    _Type_uint32::Add(&message, rPrincipalID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pUserName);
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

bool TicketGrantingProtocolClient::GetName_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          String* pUserName, const unsigned int& rPrincipalID)
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
        pCallContext->AddReturnValuePtr(pUserName);
    }
    s_ProtoReturn[METHOD_GET_NAME] = ProtoReturn_GetName;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool TicketGrantingProtocolClient::CallLoginWithContext(ProtocolCallContext* pCallContext, qResult* pResult,
                                                        const AnyObjectHolder<Data, String>& rLoginData,
                                                        unsigned int* pPrincipalID, Buffer* pTicket,
                                                        RVConnectionData* pConnectionData)
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
    AddMethodID(&message, METHOD_LOGIN_WITH_CONTEXT);
    CustomDataAny::Add(&message, rLoginData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pPrincipalID);
        pCallContext->AddReturnValuePtr(pTicket);
        pCallContext->AddReturnValuePtr(pConnectionData);
    }
    s_ProtoReturn[METHOD_LOGIN_WITH_CONTEXT] = ProtoReturn_LoginWithContext;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool TicketGrantingProtocolClient::LoginWithContext_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   qResult* pResult,
                                                                   const AnyObjectHolder<Data, String>& rLoginData,
                                                                   unsigned int* pPrincipalID, Buffer* pTicket,
                                                                   RVConnectionData* pConnectionData)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_LOGIN_WITH_CONTEXT);
    CustomDataAny::Add(pMessage, rLoginData);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResult);
        pCallContext->AddReturnValuePtr(pPrincipalID);
        pCallContext->AddReturnValuePtr(pTicket);
        pCallContext->AddReturnValuePtr(pConnectionData);
    }
    s_ProtoReturn[METHOD_LOGIN_WITH_CONTEXT] = ProtoReturn_LoginWithContext;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

}
}
