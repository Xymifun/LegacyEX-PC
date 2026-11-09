#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_SIMPLECOMMUNITY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_SIMPLECOMMUNITY_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_SimpleCommunity : public RootObject
{
public:
    _DDL_SimpleCommunity() : m_uiStructureVersion(0) {}

    virtual ~_DDL_SimpleCommunity() {}

    _DDL_SimpleCommunity& operator=(const _DDL_SimpleCommunity& rSource);

    static void Add(Message* pMessage, const _DDL_SimpleCommunity& rSource);
    static void Extract(Message* pMessage, _DDL_SimpleCommunity* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiGatheringID;
    uint32_t m_uiMatchmakeSessionCount;
};

}
}

#endif
