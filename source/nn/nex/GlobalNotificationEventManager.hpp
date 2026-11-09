#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_GLOBALNOTIFICATIONEVENTMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_GLOBALNOTIFICATIONEVENTMANAGER_HPP

#include <stdint.h>
#include <functional>
#include <set>

#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/NotificationEventManager.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

class EndPoint;
class Message;

class GlobalNotificationEventManager : public NotificationEventManager
{
public:
    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual void ProcessNotificationEvent(unsigned int uiPrincipalID, unsigned int uiConnectionID,
                                          const NotificationEvent& rEvent);

    uint32_t RegisterGlobalNotificationEventManager(NotificationEventManager* pManager);
    uint32_t UnregisterGlobalNotificationEventManager(NotificationEventManager* pManager);

private:
    typedef std::set<NotificationEventManager*, std::less<NotificationEventManager*>,
                     MemAllocator<NotificationEventManager*> >
        ManagerSet;

    ManagerSet m_managers;
};

}
}

#endif
