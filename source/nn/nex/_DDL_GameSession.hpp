#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_GAMESESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_GAMESESSION_HPP

#include <stdint.h>

#include "nn/nex/Gathering.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_GameSession : public Gathering
{
public:
    _DDL_GameSession() : Gathering(), m_uiStructureVersion(0) {}

    virtual ~_DDL_GameSession() {}

    _DDL_GameSession& operator=(const _DDL_GameSession& rSource);

    virtual Gathering* Clone() const;
    virtual String GetGatheringType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_GameSession& rSource);
    static void Extract(Message* pMessage, _DDL_GameSession* pDestination);

    uint8_t m_uiStructureVersion;
};

}
}

#endif
