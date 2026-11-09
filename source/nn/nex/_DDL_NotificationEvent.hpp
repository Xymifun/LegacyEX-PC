#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_NOTIFICATIONEVENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_NOTIFICATIONEVENT_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_NotificationEvent : public RootObject
{
public:
    _DDL_NotificationEvent() : m_uiStructureVersion(0), m_StringParam() {}

    virtual ~_DDL_NotificationEvent() {}

    _DDL_NotificationEvent& operator=(const _DDL_NotificationEvent& rSource);
    bool operator==(const _DDL_NotificationEvent& rSource) const;

    static void Add(Message* pMessage, const _DDL_NotificationEvent& rSource);
    static void Extract(Message* pMessage, _DDL_NotificationEvent* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiValue;
    uint32_t m_uiType;
    uint32_t m_uiParam1;
    uint32_t m_uiParam2;
    String m_StringParam;
    uint32_t m_uiParam3;
};

}
}

#endif
