#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MYNOTIFICATIONEVENTHANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MYNOTIFICATIONEVENTHANDLER_HPP

#include <stdint.h>

#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/NotificationEventFilterInterface.hpp"
#include "nn/nex/NotificationEventHandler.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class MyNotificationEventHandler : public NotificationEventHandler, public NotificationEventFilterInterface
{
public:
    struct NotificationEventEqualParam1
    {
        explicit NotificationEventEqualParam1(uint32_t uiParam1) : m_uiParam1(uiParam1) {}

        bool operator()(const NotificationEvent& rEvent) const { return rEvent.GetParam1() == m_uiParam1; }

        uint32_t m_uiParam1;
    };

    struct NotificationEventEqualUInt64Param1
    {
        explicit NotificationEventEqualUInt64Param1(uint64_t ui64Param1) : m_ui64Param1(ui64Param1) {}

        bool operator()(const NotificationEvent& rEvent) const
        {
            return ((static_cast<uint64_t>(rEvent.GetParam1()) << 32) | rEvent.GetParam2()) == m_ui64Param1;
        }

        uint64_t m_ui64Param1;
    };

    MyNotificationEventHandler(uint32_t uiEventType, uint32_t uiEventSubType, NotificationEvent* pEvent)
        : m_bEventReceived(false),
          m_bUseParam1(false),
          m_bUseUInt64Param1(false),
          m_uiEventType(uiEventType),
          m_uiEventSubType(uiEventSubType),
          m_uiParam1(0),
          m_pEvent(pEvent)
    {
    }

    virtual ~MyNotificationEventHandler() {}

    virtual void ProcessNotificationEvent(const NotificationEvent& rEvent);

    virtual void SetEventParam1(uint32_t uiParam1);
    virtual void SetEventUInt64Param1(uint64_t ui64Param1);

private:
    friend class JobWaitForNotification;

    typedef qVector<NotificationEvent> EventList;

    bool m_bEventReceived;
    bool m_bUseParam1;
    bool m_bUseUInt64Param1;
    uint32_t m_uiEventType;
    uint32_t m_uiEventSubType;
    uint32_t m_uiParam1;
    uint64_t m_ui64Param1;
    EventList m_pendingEvents;
    NotificationEvent* m_pEvent;
};

}
}

#endif
