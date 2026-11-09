#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDONOTIFICATIONEVENTMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDONOTIFICATIONEVENTMANAGER_HPP

#include <stdint.h>
#include <functional>
#include <set>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/NintendoNotificationEventHandler.hpp"
#include "nn/nex/NintendoNotificationEventProtocolServer.hpp"

namespace nn
{
namespace nex
{

class NintendoNotificationEventManager : public _Proto_NintendoNotificationEventProtocolServer
{
public:
    explicit NintendoNotificationEventManager(NintendoNotificationEventManager* pParent);
    virtual ~NintendoNotificationEventManager();

    virtual Protocol* Clone() const;
    virtual void ProcessNintendoNotificationEvent(const NintendoNotificationEvent& rEvent);

    uint32_t RegisterNintendoNotificationEventHandler(NintendoNotificationEventHandler* pHandler);
    uint32_t UnregisterNintendoNotificationEventHandler(NintendoNotificationEventHandler* pHandler);

private:
    typedef std::set<NintendoNotificationEventHandler*, std::less<NintendoNotificationEventHandler*>,
                     MemAllocator<NintendoNotificationEventHandler*> >
        HandlerSet;

    HandlerSet m_handlers;
    NintendoNotificationEventManager* m_pParent;
};

}
}

#endif
