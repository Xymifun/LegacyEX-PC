#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENT_HPP

#include <stdint.h>

#include "nn/nex/_DDL_NotificationEvent.hpp"

namespace nn
{
namespace nex
{

class StringStream;

class NotificationEvent : public _DDL_NotificationEvent
{
public:
    NotificationEvent()
    {
        m_uiValue = 0;
        m_uiType = 0;
        m_uiParam1 = 0;
        m_uiParam2 = 0;
        m_uiParam3 = 0;
    }

    uint32_t GetType() const { return m_uiType; }
    uint32_t GetParam1() const { return m_uiParam1; }
    uint32_t GetParam2() const { return m_uiParam2; }
    uint32_t GetParam3() const { return m_uiParam3; }
};

StringStream& operator<<(StringStream& stream, const NotificationEvent& event);

}
}

#endif
