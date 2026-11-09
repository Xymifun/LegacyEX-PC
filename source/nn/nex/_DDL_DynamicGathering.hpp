#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DYNAMICGATHERING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DYNAMICGATHERING_HPP

#include <stdint.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Gathering.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DynamicGathering : public Gathering
{
public:
    _DDL_DynamicGathering() : Gathering(), m_uiDynamicStructureVersion(0), m_Buffer() {}

    virtual ~_DDL_DynamicGathering() {}

    virtual Gathering* Clone() const;
    virtual String GetGatheringType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_DynamicGathering& rSource);
    static void Extract(Message* pMessage, _DDL_DynamicGathering* pDestination);

    uint8_t m_uiDynamicStructureVersion;
    Buffer m_Buffer;
};

}
}

#endif
