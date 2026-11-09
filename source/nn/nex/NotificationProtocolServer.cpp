#include "nn/nex/NotificationProtocolServer.hpp"

#include "nn/nex/Message.hpp"
#include "nn/nex/NotificationEvent.hpp"

namespace nn
{
namespace nex
{

void _Proto_NotificationProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                                bool* pResponseRequired, EndPoint*)
{
    const uint16_t uiMethodID = static_cast<uint16_t>(ExtractMethodID(pRequest));
    if (uiMethodID == METHOD_PROCESS_NOTIFICATION_EVENT)
    {
        {
            NotificationEvent event;
            _DDL_NotificationEvent::Extract(pRequest, &event);
            if (!pRequest->HasError())
            {
                ProcessNotificationEvent(event);
                AddMethodID(pResponse, 0x8001);
            }
        }

        *pResponseRequired = true;
        return;
    }

    SetCallError(qResult(static_cast<int>(0x80010002u)));
}

}
}
