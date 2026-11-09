#include "nn/nex/NotificationEventManager.hpp"

#include "nn/nex/Scheduler.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

NotificationEventManager::NotificationEventManager(NotificationEventManager* pParent)
    : _Proto_NotificationProtocolServer(1),
      m_handlers(),
      m_pParent(pParent),
      m_events(),
      m_uiPrincipalID(0),
      m_uiConnectionID(0)
{
    SetFlag(8);
    SetName(String(L"NotificationEventManager"));
}

NotificationEventManager::~NotificationEventManager()
{
    m_handlers.clear();
}

Protocol* NotificationEventManager::Clone() const
{
    return new NotificationEventManager(const_cast<NotificationEventManager*>(this));
}

bool NotificationEventManager::RegisterNotificationEventHandler(NotificationEventHandler* pHandler)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_handlers.find(pHandler) != m_handlers.end())
        return false;
    m_handlers.insert(pHandler);
    return true;
}

bool NotificationEventManager::UnregisterNotificationEventHandler(NotificationEventHandler* pHandler)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    HandlerSet::iterator it = m_handlers.find(pHandler);
    if (it == m_handlers.end())
        return false;
    m_handlers.erase(it);
    return true;
}

uint32_t NotificationEventManager::IsEventValid(const NotificationEvent& rEvent)
{
    if (rEvent.GetType() / 1000 == 4 && !String::IsEqual(rEvent.m_StringParam, L""))
    {
        unsigned long long uiEventID = rEvent.m_StringParam.ToUInt64();
        if (uiEventID != 0)
        {
            for (EventList::iterator it = m_events.begin(); it != m_events.end(); ++it)
            {
                const NotificationEvent& rExisting = *it;
                if (rEvent.GetParam1() == rExisting.GetParam1())
                {
                    if (uiEventID <= rExisting.m_StringParam.ToUInt64())
                        return 0;
                    m_events.erase(it);
                    break;
                }
            }
            m_events.push_front(rEvent);
            if (m_events.size() > 5)
                m_events.pop_back();
        }
    }
    return 1;
}

void NotificationEventManager::ProcessNotificationEvent(const NotificationEvent&)
{
}

void NotificationEventManager::ProcessNotificationEvent(unsigned int uiPrincipalID, unsigned int uiConnectionID,
                                                        const NotificationEvent& rEvent)
{
    if (uiPrincipalID != m_uiPrincipalID || uiConnectionID != m_uiConnectionID)
        return;
    if (IsEventValid(rEvent) == 0)
        return;

    for (HandlerSet::iterator it = m_handlers.begin(); it != m_handlers.end(); ++it)
    {
        if (rEvent.GetType() / 1000 != 115 || m_bDispatchServiceEvents)
            (*it)->ProcessNotificationEvent(rEvent);
    }
}

}
}
