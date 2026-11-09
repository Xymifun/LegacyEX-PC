#include "nn/nex/HealthProtocolClient.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);
const unsigned int METHOD_ID_MASK = 0x7FFF;
enum
{
    RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u)
};
const unsigned int METHOD_ID_COUNT = 5;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;
enum MethodId
{
    METHOD_PING_DAEMON = 1,
    METHOD_PING_DATABASE = 2,
    METHOD_RUN_SANITY_CHECK = 3,
    METHOD_FIX_SANITY_ERRORS = 4
};
ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

HealthProtocolClient::HealthProtocolClient(unsigned short uiProtocolID) : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(uiProtocolID, new HealthProtocolClient());
}

void HealthProtocolClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
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

bool HealthProtocolClient::CallPingDaemon(ProtocolCallContext* pCallContext, bool* pReturnValue)
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
    AddMethodID(&message, METHOD_PING_DAEMON);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReturnValue);
    }
    s_ProtoReturn[METHOD_PING_DAEMON] = ProtoReturn_PingDaemon;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool HealthProtocolClient::PingDaemon_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     bool* pReturnValue)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_PING_DAEMON);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReturnValue);
    }
    s_ProtoReturn[METHOD_PING_DAEMON] = ProtoReturn_PingDaemon;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void HealthProtocolClient::ProtoReturn_PingDaemon(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedReturnValue;
    bool* pReturnValue = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedReturnValue;
    if (pReturnValue != 0)
        pDestination = pReturnValue;
    *pMessage >> *pDestination;
}

bool HealthProtocolClient::CallPingDatabase(ProtocolCallContext* pCallContext, bool* pReturnValue)
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
    AddMethodID(&message, METHOD_PING_DATABASE);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReturnValue);
    }
    s_ProtoReturn[METHOD_PING_DATABASE] = ProtoReturn_PingDatabase;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool HealthProtocolClient::PingDatabase_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                       bool* pReturnValue)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_PING_DATABASE);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReturnValue);
    }
    s_ProtoReturn[METHOD_PING_DATABASE] = ProtoReturn_PingDatabase;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void HealthProtocolClient::ProtoReturn_PingDatabase(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedReturnValue;
    bool* pReturnValue = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedReturnValue;
    if (pReturnValue != 0)
        pDestination = pReturnValue;
    *pMessage >> *pDestination;
}

bool HealthProtocolClient::CallRunSanityCheck(ProtocolCallContext* pCallContext, bool* pReturnValue)
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
    AddMethodID(&message, METHOD_RUN_SANITY_CHECK);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReturnValue);
    }
    s_ProtoReturn[METHOD_RUN_SANITY_CHECK] = ProtoReturn_RunSanityCheck;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool HealthProtocolClient::RunSanityCheck_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                         bool* pReturnValue)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_RUN_SANITY_CHECK);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReturnValue);
    }
    s_ProtoReturn[METHOD_RUN_SANITY_CHECK] = ProtoReturn_RunSanityCheck;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void HealthProtocolClient::ProtoReturn_RunSanityCheck(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedReturnValue;
    bool* pReturnValue = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedReturnValue;
    if (pReturnValue != 0)
        pDestination = pReturnValue;
    *pMessage >> *pDestination;
}

bool HealthProtocolClient::CallFixSanityErrors(ProtocolCallContext* pCallContext, bool* pReturnValue)
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
    AddMethodID(&message, METHOD_FIX_SANITY_ERRORS);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReturnValue);
    }
    s_ProtoReturn[METHOD_FIX_SANITY_ERRORS] = ProtoReturn_FixSanityErrors;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool HealthProtocolClient::FixSanityErrors_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          bool* pReturnValue)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_FIX_SANITY_ERRORS);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReturnValue);
    }
    s_ProtoReturn[METHOD_FIX_SANITY_ERRORS] = ProtoReturn_FixSanityErrors;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void HealthProtocolClient::ProtoReturn_FixSanityErrors(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedReturnValue;
    bool* pReturnValue = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedReturnValue;
    if (pReturnValue != 0)
        pDestination = pReturnValue;
    *pMessage >> *pDestination;
}

}
}
