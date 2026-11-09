#include "game/core/LeaderboardManager.hpp"

LeaderboardManager::LeaderboardManager() : m_uid(0)
{
    zeroReadParameters();
    m_uid = PlayerUID(0);
}

void LeaderboardManager::zeroReadParameters()
{
    m_iUserIndex = -1;
    m_eStatsType = eStatsType_Count;
    m_uiStartRank = 0;
    m_pReadListener = NULL;
    m_eReadType = eReadType_None;
    m_uiNumRows = 0;
}

bool LeaderboardManager::ReadStats_Friends(LeaderboardReadListener* pReadListener, int iUserIndex,
                                           EStatsType eStatsType, PlayerUID uid, unsigned int uiStartRank,
                                           unsigned int uiNumRows)
{
    zeroReadParameters();

    m_pReadListener = pReadListener;
    m_iUserIndex = iUserIndex;
    m_eStatsType = eStatsType;
    m_eReadType = eReadType_Friends;

    return true;
}

bool LeaderboardManager::ReadStats_MyScore(LeaderboardReadListener* pReadListener, int iUserIndex,
                                           EStatsType eStatsType, PlayerUID uid, unsigned int uiNumRows)
{
    zeroReadParameters();

    m_pReadListener = pReadListener;
    m_iUserIndex = iUserIndex;
    m_eStatsType = eStatsType;
    m_uiNumRows = uiNumRows;
    m_eReadType = eReadType_MyScore;

    return true;
}

bool LeaderboardManager::ReadStats_TopRank(LeaderboardReadListener* pReadListener, int iUserIndex,
                                           EStatsType eStatsType, unsigned int uiStartRank, unsigned int uiNumRows)
{
    zeroReadParameters();

    m_pReadListener = pReadListener;
    m_iUserIndex = iUserIndex;
    m_eStatsType = eStatsType;
    m_uiStartRank = uiStartRank;
    m_uiNumRows = uiNumRows;
    m_eReadType = eReadType_TopRank;

    return true;
}

void LeaderboardManager::DeleteInstance()
{
    delete g_pLeaderboardManager;
    g_pLeaderboardManager = NULL;
}

void LeaderboardManager::printStats(ReadView& readView)
{
    for (unsigned int uiScore = 0; uiScore < readView.m_uiNumScores; ++uiScore)
    {
        ReadScore readScore(readView.m_pScores[uiScore]);
    }
}
