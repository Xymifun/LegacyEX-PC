#include "nn/nex/RemoteLogDeviceProtocolClient.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/String.hpp"

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
const unsigned int METHOD_ID_COUNT = 2;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;
enum MethodId
{
    METHOD_LOG = 1
};
ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

RemoteLogDeviceProtocolClient::RemoteLogDeviceProtocolClient(unsigned short uiProtocolID) : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(uiProtocolID,
                                                                         new RemoteLogDeviceProtocolClient());
}

void RemoteLogDeviceProtocolClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
{
    const unsigned int uiMethodID = Protocol::ExtractMethodID(pMessage) & METHOD_ID_MASK;
    if (uiMethodID - 1 > 0)
    {
        ClientProtocol::SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
    if (s_ProtoReturn[uiMethodID] != 0)
        s_ProtoReturn[uiMethodID](pMessage, pCallContext);
}

bool RemoteLogDeviceProtocolClient::CallLog(ProtocolCallContext* pCallContext, const String& rMessage)
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
    AddMethodID(&message, METHOD_LOG);
    _Type_string::Add(&message, rMessage);
    s_ProtoReturn[METHOD_LOG] = ProtoReturn_Log;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool RemoteLogDeviceProtocolClient::Log_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                       const String& rMessage)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_LOG);
    _Type_string::Add(pMessage, rMessage);
    s_ProtoReturn[METHOD_LOG] = ProtoReturn_Log;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

}
}
