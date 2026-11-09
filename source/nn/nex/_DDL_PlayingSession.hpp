#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_PLAYINGSESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_PLAYINGSESSION_HPP

#include <stdint.h>

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/Gathering.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_PlayingSession : public RootObject
{
public:
    _DDL_PlayingSession() : m_uiStructureVersion(0), m_Gathering() {}

    virtual ~_DDL_PlayingSession() {}

    _DDL_PlayingSession& operator=(const _DDL_PlayingSession& rSource);

    static void Add(Message* pMessage, const _DDL_PlayingSession& rSource);
    static void Extract(Message* pMessage, _DDL_PlayingSession* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiPrincipalID;
    AnyObjectHolder<Gathering, String> m_Gathering;
};

}
}

#endif
