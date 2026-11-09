#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDOPRESENCE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDOPRESENCE_HPP

#include <stdint.h>

#include "nn/nex/GameKey.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/_DDL_NintendoPresence.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class NintendoPresence : public _DDL_NintendoPresence
{
public:
    NintendoPresence() : _DDL_NintendoPresence() { Clear(); }

    virtual ~NintendoPresence() {}

    void Clear()
    {
        m_uiChangedFlags = 0;
        m_GameKey = GameKey();
        m_strGameModeDescription = L"";
        m_uiJoinAvailabilityFlag = 0;
        m_uiMatchmakeSystemType = 0;
        m_uiJoinGameID = 0;
        m_uiJoinGameMode = 0;
        m_uiOwnerPrincipalID = 0;
        m_uiJoinGroupID = 0;
        m_ApplicationArg.clear();
    }

    void SetGameKey(const GameKey& rGameKey);
    void SetGameModeDescription(const String& rDescription);
    void SetJoinAvailabilityFlag(unsigned int uiFlag);
    void SetMatchmakeSystemType(unsigned char uiType);
    void SetJoinGameID(unsigned int uiGameID);
    void SetJoinGameMode(unsigned int uiGameMode);
    void SetOwnerPrincipalID(unsigned int uiPrincipalID);
    void SetJoinGroupID(unsigned int uiGroupID);
    void SetApplicationArg(const qVector<unsigned char>& rApplicationArg);

    void Set(const NintendoPresence& rSource);
    void Merge(const NintendoPresence& rSource);
    bool Compares(const NintendoPresence& rOther, unsigned int* pChangedFlags) const;
};

}
}

#endif
