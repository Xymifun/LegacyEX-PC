#include "nn/nex/NotificationProtocolClient.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/_DDL_NotificationEvent.hpp"

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
    METHOD_PROCESS_NOTIFICATION_EVENT = 1
};
ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

NotificationProtocolClient::NotificationProtocolClient(unsigned short uiProtocolID) : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(uiProtocolID,
                                                                         new NotificationProtocolClient());
}

void NotificationProtocolClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
{
    const unsigned int uiMethodID = Protocol::ExtractMethodID(pMessage) & METHOD_ID_MASK;
    if (uiMethodID < 1 || uiMethodID >= METHOD_ID_COUNT)
    {
        ClientProtocol::SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
    if (s_ProtoReturn[uiMethodID] != 0)
        s_ProtoReturn[uiMethodID](pMessage, pCallContext);
}

bool NotificationProtocolClient::CallProcessNotificationEvent(ProtocolCallContext* pCallContext,
                                                              const NotificationEvent& rEvent)
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
    AddMethodID(&message, METHOD_PROCESS_NOTIFICATION_EVENT);
    _DDL_NotificationEvent::Add(&message, rEvent);
    s_ProtoReturn[METHOD_PROCESS_NOTIFICATION_EVENT] = ProtoReturn_ProcessNotificationEvent;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool NotificationProtocolClient::ProcessNotificationEvent_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                         Message* pMessage,
                                                                         const NotificationEvent& rEvent)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_PROCESS_NOTIFICATION_EVENT);
    _DDL_NotificationEvent::Add(pMessage, rEvent);
    s_ProtoReturn[METHOD_PROCESS_NOTIFICATION_EVENT] = ProtoReturn_ProcessNotificationEvent;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

}
}
