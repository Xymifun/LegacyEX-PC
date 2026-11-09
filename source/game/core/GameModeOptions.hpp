#ifndef GAME_CORE_GAMEMODEOPTIONS_HPP
#define GAME_CORE_GAMEMODEOPTIONS_HPP

#include <string>
#include "game/core/maybe.hpp"

enum eCustomGameModeHostOption
{
    eCustomGameModeHostOption_NoSmallSkins,
    eCustomGameModeHostOption_SpectateMode,
    eCustomGameModeHostOption_RoundLength,
    eCustomGameModeHostOption_MaxPlayers,
    eCustomGameModeHostOption_CentreSpawn,
    eCustomGameModeHostOption_PointsActive,
    eCustomGameModeHostOption_PowerupsActive,
    eCustomGameModeHostOption_EnableVoting,
    eCustomGameModeHostOption_IsTeams,
    eCustomGameModeHostOption_Nothing,
    eCustomGameModeHostOption_All,
    eCustomGameModeHostOption_MapVariant,
    eCustomGameModeHostOption_ModeVariant,
    eCustomGameModeHostOption_GameVariant,
    eCustomGameModeHostOption_TimeTrial
};

class GameModeOptions
{
public:
    GameModeOptions();
    GameModeOptions(unsigned int uOptions);

    unsigned int Get(eCustomGameModeHostOption eOption) const;
    void Set(eCustomGameModeHostOption eOption, unsigned int uValue);

    static maybe<eCustomGameModeHostOption> parse(std::wstring sName);

private:
    unsigned int m_uOptions;
};

#endif
