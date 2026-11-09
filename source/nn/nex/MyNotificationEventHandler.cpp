#include "nn/nex/MyNotificationEventHandler.hpp"

#include <algorithm>

namespace nn
{
namespace nex
{

void MyNotificationEventHandler::ProcessNotificationEvent(const NotificationEvent& rEvent)
{
    uint32_t uiType = rEvent.GetType();
    if (uiType / 1000 != m_uiEventType)
    {
        return;
    }

    if (m_bUseParam1)
    {
        if (uiType % 1000 == m_uiEventSubType && rEvent.GetParam1() == m_uiParam1)
        {
            *m_pEvent = rEvent;
            m_bEventReceived = true;
        }
    }
    else if (m_bUseUInt64Param1)
    {
        if (uiType % 1000 == m_uiEventSubType
            && ((static_cast<uint64_t>(rEvent.GetParam1()) << 32) | rEvent.GetParam2()) == m_ui64Param1)
        {
            *m_pEvent = rEvent;
            m_bEventReceived = true;
        }
    }
    else if (uiType % 1000 == m_uiEventSubType)
    {
        m_pendingEvents.push_back(rEvent);
        if (m_pendingEvents.size() > 16)
        {
            m_pendingEvents.erase(m_pendingEvents.begin());
        }
    }
}

void MyNotificationEventHandler::SetEventParam1(uint32_t uiParam1)
{
    m_uiParam1 = uiParam1;
    m_bUseParam1 = true;

    EventList::iterator it = std::find_if(m_pendingEvents.begin(), m_pendingEvents.end(),
                                          NotificationEventEqualParam1(uiParam1));
    if (it != m_pendingEvents.end())
    {
        *m_pEvent = *it;
        m_bEventReceived = true;
    }
    m_pendingEvents.clear();
}

void MyNotificationEventHandler::SetEventUInt64Param1(uint64_t ui64Param1)
{
    m_ui64Param1 = ui64Param1;
    m_bUseUInt64Param1 = true;

    EventList::iterator it = std::find_if(m_pendingEvents.begin(), m_pendingEvents.end(),
                                          NotificationEventEqualUInt64Param1(ui64Param1));
    if (it != m_pendingEvents.end())
    {
        *m_pEvent = *it;
        m_bEventReceived = true;
    }
    m_pendingEvents.clear();
}

}
}
