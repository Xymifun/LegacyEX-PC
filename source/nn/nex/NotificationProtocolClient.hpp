#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONPROTOCOLCLIENT_HPP

#include "nn/nex/ClientProtocol.hpp"

namespace nn
{
namespace nex
{

class NotificationEvent;

class NotificationProtocolClient : public ClientProtocol
{
public:
    explicit NotificationProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);

    virtual Protocol* CreateResponder() const { return new NotificationProtocolClient(); }

    static void ProtoReturn_ProcessNotificationEvent(Message* pMessage, ProtocolCallContext* pCallContext) {}

    bool CallProcessNotificationEvent(ProtocolCallContext* pCallContext, const NotificationEvent& rEvent);

    static bool ProcessNotificationEvent_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        const NotificationEvent& rEvent);
};

}
}

#endif
