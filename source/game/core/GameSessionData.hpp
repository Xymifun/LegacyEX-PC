#ifndef GAME_CORE_GAMESESSIONDATA_HPP
#define GAME_CORE_GAMESESSIONDATA_HPP

#include <string.h>

class GameSessionData
{
public:
    enum
    {
#ifdef MINECRAFT_PC
        MAX_SESSION_PLAYERS = 25,
        MAX_HUMAN_PLAYERS = 24,
        MAX_MINI_GAME_PLAYERS = 8,
#else
        MAX_SESSION_PLAYERS = 8,
#endif
        MAX_GAMERTAG_LENGTH = 21
    };

    class Gamertag
    {
    public:
        Gamertag() { memset(m_name, 0, sizeof(m_name)); }

        bool operator==(const Gamertag& other) const { return strcmp(m_name, other.m_name) == 0; }

        bool operator!=(const Gamertag& other) const { return !(*this == other); }

        char m_name[MAX_GAMERTAG_LENGTH];
    };

    GameSessionData();

    unsigned short m_version;
    Gamertag m_hostGamertag;
    Gamertag m_playerGamertags[MAX_SESSION_PLAYERS];
    unsigned int m_gameHostOptions;
    unsigned int m_texturePackParentId;
    unsigned char m_subTexturePackId;
    bool m_bPublicGame;
    unsigned char m_numPlayers;
    bool m_bGameStarted;
    unsigned int m_gameModeId : 4;
    unsigned int m_miniGameId : 4;
    unsigned int m_customGameModeHostOptions;
#ifdef MINECRAFT_PC
    unsigned char m_pcDedicated;
#endif
};

#endif
