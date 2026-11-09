#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_GATHERINGSTATS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_GATHERINGSTATS_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_GatheringStats : public RootObject
{
public:
    _DDL_GatheringStats() : m_uiStructureVersion(0) {}

    virtual ~_DDL_GatheringStats() {}

    static void Add(Message* pMessage, const _DDL_GatheringStats& rSource);
    static void Extract(Message* pMessage, _DDL_GatheringStats* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiValue1;
    uint32_t m_uiValue2;
    qList<float> m_Values;
};

}
}

#endif
