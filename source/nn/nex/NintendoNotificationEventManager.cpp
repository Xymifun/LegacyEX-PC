#include "nn/nex/NintendoNotificationEventManager.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

NintendoNotificationEventManager::NintendoNotificationEventManager(NintendoNotificationEventManager* pParent)
    : _Proto_NintendoNotificationEventProtocolServer(1),
      m_handlers(),
      m_pParent(pParent)
{
    SetFlag(8);
    SetName(String(L"NintendoNotificationEventManager"));
}

NintendoNotificationEventManager::~NintendoNotificationEventManager()
{
    m_handlers.clear();
}

Protocol* NintendoNotificationEventManager::Clone() const
{
    return new NintendoNotificationEventManager(const_cast<NintendoNotificationEventManager*>(this));
}

uint32_t NintendoNotificationEventManager::RegisterNintendoNotificationEventHandler(
    NintendoNotificationEventHandler* pHandler)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_handlers.find(pHandler) == m_handlers.end())
    {
        m_handlers.insert(pHandler);
        return 1;
    }
    return 0;
}

uint32_t NintendoNotificationEventManager::UnregisterNintendoNotificationEventHandler(
    NintendoNotificationEventHandler* pHandler)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    HandlerSet::iterator it = m_handlers.find(pHandler);
    if (it != m_handlers.end())
    {
        m_handlers.erase(it);
        return 1;
    }
    return 0;
}

void NintendoNotificationEventManager::ProcessNintendoNotificationEvent(const NintendoNotificationEvent& rEvent)
{
    if (m_pParent != 0)
    {
        m_pParent->ProcessNintendoNotificationEvent(rEvent);
        return;
    }

    for (HandlerSet::iterator it = m_handlers.begin(); it != m_handlers.end(); ++it)
        (*it)->ProcessNintendoNotificationEvent(rEvent);
}

}
}
