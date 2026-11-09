#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_GATHERING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_GATHERING_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Gathering;
class Message;

class _DDL_Gathering : public RootObject
{
public:
    _DDL_Gathering() : m_uiStructureVersion(0), m_Description() {}

    virtual ~_DDL_Gathering() {}

    _DDL_Gathering& operator=(const _DDL_Gathering& rSource);

    virtual Gathering* Clone() const;
    virtual String GetGatheringType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_Gathering& rSource);
    static void Extract(Message* pMessage, _DDL_Gathering* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiValue1;
    uint32_t m_uiValue2;
    uint32_t m_uiValue3;
    uint16_t m_uiShortValue1;
    uint16_t m_uiShortValue2;
    uint32_t m_uiParticipationPolicy;
    uint32_t m_uiPolicyArgument;
    uint32_t m_uiValue4;
    uint32_t m_uiValue5;
    String m_Description;
};

}
}

#endif
