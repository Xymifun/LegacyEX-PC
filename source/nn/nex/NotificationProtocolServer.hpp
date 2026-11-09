#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONPROTOCOLSERVER_HPP

#include "nn/nex/ServerProtocol.hpp"

namespace nn
{
namespace nex
{

class NotificationEvent;

class _Proto_NotificationProtocolServer : public ServerProtocol
{
public:
    enum _MethodID
    {
        METHOD_PROCESS_NOTIFICATION_EVENT = 1
    };

    explicit _Proto_NotificationProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}


    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual void ProcessNotificationEvent(const NotificationEvent& rEvent) = 0;
    virtual void ProcessNotificationEvent(unsigned int uiPrincipalID, unsigned int uiConnectionID,
                                          const NotificationEvent& rEvent) = 0;
};

}
}

#endif
