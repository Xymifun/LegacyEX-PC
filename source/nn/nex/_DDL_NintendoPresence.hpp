#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_NINTENDOPRESENCE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_NINTENDOPRESENCE_HPP

#include <stdint.h>

#include "nn/nex/Data.hpp"
#include "nn/nex/GameKey.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_NintendoPresence : public Data
{
public:
    _DDL_NintendoPresence()
        : Data(),
          m_uiStructureVersion(0),
          m_GameKey(),
          m_strGameModeDescription(),
          m_ApplicationArg(0)
    {
    }

    virtual ~_DDL_NintendoPresence() {}

    _DDL_NintendoPresence& operator=(const _DDL_NintendoPresence& rSource);
    bool operator==(const _DDL_NintendoPresence& rSource) const;

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_NintendoPresence& rSource);
    static void Extract(Message* pMessage, _DDL_NintendoPresence* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiChangedFlags;
    GameKey m_GameKey;
    String m_strGameModeDescription;
    uint32_t m_uiJoinAvailabilityFlag;
    uint8_t m_uiMatchmakeSystemType;
    uint32_t m_uiJoinGameID;
    uint32_t m_uiJoinGameMode;
    uint32_t m_uiOwnerPrincipalID;
    uint32_t m_uiJoinGroupID;
    qVector<uint8_t> m_ApplicationArg;
};

}
}

#endif
