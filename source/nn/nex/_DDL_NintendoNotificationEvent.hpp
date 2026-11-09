#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_NINTENDONOTIFICATIONEVENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_NINTENDONOTIFICATIONEVENT_HPP

#include <stdint.h>

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_NintendoNotificationEvent : public RootObject
{
public:
    _DDL_NintendoNotificationEvent() : m_uiStructureVersion(0), m_Data() { m_uiType = 0; }

    virtual ~_DDL_NintendoNotificationEvent() {}

    _DDL_NintendoNotificationEvent& operator=(const _DDL_NintendoNotificationEvent& rSource);
    bool operator==(const _DDL_NintendoNotificationEvent& rSource) const;

    static void Add(Message* pMessage, const _DDL_NintendoNotificationEvent& rSource);
    static void Extract(Message* pMessage, _DDL_NintendoNotificationEvent* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiType;
    uint32_t m_uiValue;
    AnyObjectHolder<Data, String> m_Data;
};

}
}

#endif
