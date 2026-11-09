#include "nn/nex/MonitoringProtocolClient.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/DDLSerialization.hpp"
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
const unsigned int METHOD_ID_COUNT = 3;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;
enum MethodId
{
    METHOD_PING_DAEMON = 1,
    METHOD_GET_CLUSTER_MEMBERS = 2
};
ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

MonitoringProtocolClient::MonitoringProtocolClient(unsigned short uiProtocolID) : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(uiProtocolID, new MonitoringProtocolClient());
}

void MonitoringProtocolClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
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

void MonitoringProtocolClient::ProtoReturn_PingDaemon(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedReturnValue;
    bool* pReturnValue = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedReturnValue;
    if (pReturnValue != 0)
        pDestination = pReturnValue;
    *pMessage >> *pDestination;
}

bool MonitoringProtocolClient::CallPingDaemon(ProtocolCallContext* pCallContext, bool* pReturnValue)
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

bool MonitoringProtocolClient::PingDaemon_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
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

void MonitoringProtocolClient::ProtoReturn_GetClusterMembers(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qList<String>* pClusterMembers = static_cast<qList<String>*>(pCallContext->GetReturnValuePtr(0));
    if (pClusterMembers != 0)
    {
        _Type_std_list<_Type_string>::Extract(pMessage, pClusterMembers);
    }
    else
    {
        qList<String> discardedClusterMembers[1];
        _Type_std_list<_Type_string>::Extract(pMessage, &discardedClusterMembers[0]);
    }
}

bool MonitoringProtocolClient::CallGetClusterMembers(ProtocolCallContext* pCallContext, qList<String>* pClusterMembers)
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
    AddMethodID(&message, METHOD_GET_CLUSTER_MEMBERS);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pClusterMembers);
    }
    s_ProtoReturn[METHOD_GET_CLUSTER_MEMBERS] = ProtoReturn_GetClusterMembers;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool MonitoringProtocolClient::GetClusterMembers_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                qList<String>* pClusterMembers)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_CLUSTER_MEMBERS);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pClusterMembers);
    }
    s_ProtoReturn[METHOD_GET_CLUSTER_MEMBERS] = ProtoReturn_GetClusterMembers;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

}
}
