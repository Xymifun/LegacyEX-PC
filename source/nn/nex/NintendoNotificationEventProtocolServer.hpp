#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDONOTIFICATIONEVENTPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDONOTIFICATIONEVENTPROTOCOLSERVER_HPP

#include "nn/nex/ServerProtocol.hpp"

namespace nn
{
namespace nex
{

class NintendoNotificationEvent;

class _Proto_NintendoNotificationEventProtocolServer : public ServerProtocol
{
public:
    enum _MethodID
    {
        METHOD_PROCESS_NINTENDO_NOTIFICATION_EVENT = 1
    };

    explicit _Proto_NintendoNotificationEventProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual ~_Proto_NintendoNotificationEventProtocolServer() {}

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual void ProcessNintendoNotificationEvent(const NintendoNotificationEvent& rEvent) = 0;
};

}
}

#endif
