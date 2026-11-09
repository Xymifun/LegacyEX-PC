#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDONOTIFICATIONEVENTPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDONOTIFICATIONEVENTPROTOCOLCLIENT_HPP

#include "nn/nex/ClientProtocol.hpp"

namespace nn
{
namespace nex
{

class NintendoNotificationEvent;

class NintendoNotificationEventProtocolClient : public ClientProtocol
{
public:
    explicit NintendoNotificationEventProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);

    virtual Protocol* CreateResponder() const { return new NintendoNotificationEventProtocolClient(); }

    static void ProtoReturn_ProcessNintendoNotificationEvent(Message* pMessage, ProtocolCallContext* pCallContext) {}

    bool CallProcessNintendoNotificationEvent(ProtocolCallContext* pCallContext,
                                              const NintendoNotificationEvent& rEvent);

    static bool ProcessNintendoNotificationEvent_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                const NintendoNotificationEvent& rEvent);
};

}
}

#endif
