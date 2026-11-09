#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_SIMPLEPLAYINGSESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_SIMPLEPLAYINGSESSION_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_SimplePlayingSession : public RootObject
{
public:
    _DDL_SimplePlayingSession() : m_uiStructureVersion(0) {}

    virtual ~_DDL_SimplePlayingSession() {}

    _DDL_SimplePlayingSession& operator=(const _DDL_SimplePlayingSession& rSource);

    static void Add(Message* pMessage, const _DDL_SimplePlayingSession& rSource);
    static void Extract(Message* pMessage, _DDL_SimplePlayingSession* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiPrincipalID;
    uint32_t m_uiGatheringID;
    uint32_t m_uiValue1;
    uint32_t m_uiValue2;
};

}
}

#endif
