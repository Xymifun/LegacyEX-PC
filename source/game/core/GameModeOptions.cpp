#include "game/core/GameModeOptions.hpp"

static const unsigned int NO_SMALL_SKINS_MASK = 0x00000001;
static const unsigned int NO_SMALL_SKINS_SHIFT = 0;
static const unsigned int CENTRE_SPAWN_MASK = 0x00000004;
static const unsigned int CENTRE_SPAWN_SHIFT = 2;
static const unsigned int ENABLE_VOTING_MASK = 0x00000008;
static const unsigned int ENABLE_VOTING_SHIFT = 3;
static const unsigned int ROUND_LENGTH_MASK = 0x000000F0;
static const unsigned int MAX_PLAYERS_MASK = 0x00000F00;
static const unsigned int GAME_VARIANT_MASK = 0x0000F000;
static const unsigned int IS_TEAMS_MASK = 0x00010000;
static const unsigned int IS_TEAMS_SHIFT = 16;
static const unsigned int POINTS_ACTIVE_MASK = 0x00020000;
static const unsigned int POINTS_ACTIVE_SHIFT = 17;
static const unsigned int POWERUPS_ACTIVE_MASK = 0x00040000;
static const unsigned int POWERUPS_ACTIVE_SHIFT = 18;
static const unsigned int TIME_TRIAL_MASK = 0x00080000;
static const unsigned int TIME_TRIAL_SHIFT = 19;
static const unsigned int SPECTATE_MODE_MASK = 0x00F00000;

static inline unsigned int GetShift(unsigned int mask)
{
    unsigned int shift = 0;
    while ((mask & 1) == 0)
    {
        mask >>= 1;
        ++shift;
    }
    return shift;
}

static const int HOST_OPTION_NAME_COUNT = 10;

static const std::wstring HOST_OPTION_NAMES[HOST_OPTION_NAME_COUNT] = {
    L"NoSmallSkins", L"SpectateMode",   L"RoundLength",  L"MaxPlayers", L"CentreSpawn",
    L"PointsActive", L"PowerupsActive", L"EnableVoting", L"IsTeams",    L"Nothing"};

GameModeOptions::GameModeOptions() : m_uOptions(0)
{
}

GameModeOptions::GameModeOptions(unsigned int uOptions) : m_uOptions(uOptions)
{
}

unsigned int GameModeOptions::Get(eCustomGameModeHostOption eOption) const
{
    switch (eOption)
    {
    case eCustomGameModeHostOption_All:
        return m_uOptions;

    case eCustomGameModeHostOption_NoSmallSkins:
        return (m_uOptions & NO_SMALL_SKINS_MASK) >> NO_SMALL_SKINS_SHIFT;

    case eCustomGameModeHostOption_SpectateMode:
    {
        unsigned int shift = GetShift(SPECTATE_MODE_MASK);
        return (m_uOptions & SPECTATE_MODE_MASK) >> shift;
    }

    case eCustomGameModeHostOption_CentreSpawn:
        return (m_uOptions & CENTRE_SPAWN_MASK) >> CENTRE_SPAWN_SHIFT;

    case eCustomGameModeHostOption_EnableVoting:
        return (m_uOptions & ENABLE_VOTING_MASK) >> ENABLE_VOTING_SHIFT;

    case eCustomGameModeHostOption_RoundLength:
    {
        unsigned int shift = GetShift(ROUND_LENGTH_MASK);
        return (m_uOptions & ROUND_LENGTH_MASK) >> shift;
    }

    case eCustomGameModeHostOption_GameVariant:
    {
        unsigned int shift = GetShift(GAME_VARIANT_MASK);
        return (m_uOptions & GAME_VARIANT_MASK) >> shift;
    }

    case eCustomGameModeHostOption_MaxPlayers:
    {
        unsigned int shift = GetShift(MAX_PLAYERS_MASK);
        return (m_uOptions & MAX_PLAYERS_MASK) >> shift;
    }

    case eCustomGameModeHostOption_IsTeams:
        return (m_uOptions & IS_TEAMS_MASK) >> IS_TEAMS_SHIFT;

    case eCustomGameModeHostOption_PointsActive:
        return (m_uOptions & POINTS_ACTIVE_MASK) >> POINTS_ACTIVE_SHIFT;

    case eCustomGameModeHostOption_PowerupsActive:
        return (m_uOptions & POWERUPS_ACTIVE_MASK) >> POWERUPS_ACTIVE_SHIFT;

    case eCustomGameModeHostOption_TimeTrial:
        return (m_uOptions & TIME_TRIAL_MASK) >> TIME_TRIAL_SHIFT;

    default:
        break;
    }

    return 0;
}

void GameModeOptions::Set(eCustomGameModeHostOption eOption, unsigned int uValue)
{
    switch (eOption)
    {
    case eCustomGameModeHostOption_All:
        m_uOptions = uValue;
        break;

    case eCustomGameModeHostOption_NoSmallSkins:
        if (uValue != 0)
        {
            m_uOptions |= NO_SMALL_SKINS_MASK;
        }
        else
        {
            m_uOptions &= ~NO_SMALL_SKINS_MASK;
        }
        break;

    case eCustomGameModeHostOption_SpectateMode:
    {
        unsigned int shift = GetShift(SPECTATE_MODE_MASK);
        m_uOptions &= ~SPECTATE_MODE_MASK;
        m_uOptions |= (uValue << shift) & SPECTATE_MODE_MASK;
        break;
    }

    case eCustomGameModeHostOption_CentreSpawn:
        if (uValue != 0)
        {
            m_uOptions |= CENTRE_SPAWN_MASK;
        }
        else
        {
            m_uOptions &= ~CENTRE_SPAWN_MASK;
        }
        break;

    case eCustomGameModeHostOption_EnableVoting:
        if (uValue != 0)
        {
            m_uOptions |= ENABLE_VOTING_MASK;
        }
        else
        {
            m_uOptions &= ~ENABLE_VOTING_MASK;
        }
        break;

    case eCustomGameModeHostOption_RoundLength:
    {
        unsigned int shift = GetShift(ROUND_LENGTH_MASK);
        m_uOptions &= ~ROUND_LENGTH_MASK;
        m_uOptions |= (uValue << shift) & ROUND_LENGTH_MASK;
        break;
    }

    case eCustomGameModeHostOption_GameVariant:
    {
        unsigned int shift = GetShift(GAME_VARIANT_MASK);
        m_uOptions &= ~GAME_VARIANT_MASK;
        m_uOptions |= (uValue << shift) & GAME_VARIANT_MASK;
        break;
    }

    case eCustomGameModeHostOption_IsTeams:
        if (uValue != 0)
        {
            m_uOptions |= IS_TEAMS_MASK;
        }
        else
        {
            m_uOptions &= ~IS_TEAMS_MASK;
        }
        break;

    case eCustomGameModeHostOption_MaxPlayers:
    {
        unsigned int shift = GetShift(MAX_PLAYERS_MASK);
        m_uOptions &= ~MAX_PLAYERS_MASK;
        m_uOptions |= (uValue << shift) & MAX_PLAYERS_MASK;
        break;
    }

    case eCustomGameModeHostOption_PointsActive:
        if (uValue != 0)
        {
            m_uOptions |= POINTS_ACTIVE_MASK;
        }
        else
        {
            m_uOptions &= ~POINTS_ACTIVE_MASK;
        }
        break;

    case eCustomGameModeHostOption_PowerupsActive:
        if (uValue != 0)
        {
            m_uOptions |= POWERUPS_ACTIVE_MASK;
        }
        else
        {
            m_uOptions &= ~POWERUPS_ACTIVE_MASK;
        }
        break;

    case eCustomGameModeHostOption_TimeTrial:
        if (uValue != 0)
        {
            m_uOptions |= TIME_TRIAL_MASK;
        }
        else
        {
            m_uOptions &= ~TIME_TRIAL_MASK;
        }
        break;

    default:
        break;
    }
}

maybe<eCustomGameModeHostOption> GameModeOptions::parse(std::wstring sName)
{
    if (sName.at(0) == L'$')
    {
        sName.erase(0, 1);
    }

    int nOption = 0;
    const std::wstring* name = HOST_OPTION_NAMES;
    while (nOption != HOST_OPTION_NAME_COUNT)
    {
        if (sName == *name)
        {
            return maybe<eCustomGameModeHostOption>(static_cast<eCustomGameModeHostOption>(nOption));
        }
        ++nOption;
        ++name;
    }

    return maybe<eCustomGameModeHostOption>();
}
