#include "game/core/GameSessionData.hpp"

#include <string.h>

GameSessionData::GameSessionData()
{
    m_version = 0;
    memset(m_playerGamertags, 0, sizeof(m_playerGamertags));
    m_bPublicGame = true;
    m_gameHostOptions = 0;
    m_texturePackParentId = 0;
    m_subTexturePackId = 0;
    m_numPlayers = 0;
    m_bGameStarted = false;
    m_gameModeId = 0;
    m_miniGameId = 0;
    m_customGameModeHostOptions = 0;
#ifdef MINECRAFT_PC
    m_pcDedicated = 0;
#endif
}
