#include "nn/nex/NintendoNotificationEventProtocolServer.hpp"

#include "nn/nex/Message.hpp"
#include "nn/nex/NintendoNotificationEvent.hpp"

namespace nn
{
namespace nex
{

void _Proto_NintendoNotificationEventProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                                             bool* pResponseRequired, EndPoint*)
{
    const uint16_t uiMethodID = static_cast<uint16_t>(ExtractMethodID(pRequest));
    if (uiMethodID == METHOD_PROCESS_NINTENDO_NOTIFICATION_EVENT)
    {
        {
            NintendoNotificationEvent event;
            _DDL_NintendoNotificationEvent::Extract(pRequest, &event);
            if (!pRequest->HasError())
            {
                ProcessNintendoNotificationEvent(event);
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
