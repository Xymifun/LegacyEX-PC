#ifndef GAME_CORE_LEADERBOARDINTERFACE_HPP
#define GAME_CORE_LEADERBOARDINTERFACE_HPP

#include "game/core/LeaderboardManager.hpp"
#include "game/world/generation/PlayerUID.hpp"

class LeaderboardReadListener;

class LeaderboardInterface
{
public:
    LeaderboardInterface(LeaderboardManager* pManager);
    ~LeaderboardInterface();

    void tick();

    void ReadStats_Friends(LeaderboardReadListener* pReadListener, int iUserIndex,
                           LeaderboardManager::EStatsType eStatsType, PlayerUID uid, unsigned int uiStartRank,
                           unsigned int uiNumRows);
    void ReadStats_MyScore(LeaderboardReadListener* pReadListener, int iUserIndex,
                           LeaderboardManager::EStatsType eStatsType, PlayerUID uid, unsigned int uiNumRows);
    void ReadStats_TopRank(LeaderboardReadListener* pReadListener, int iUserIndex,
                           LeaderboardManager::EStatsType eStatsType, unsigned int uiStartRank, unsigned int uiNumRows);

    void CancelOperation();

private:
    void tryOpenSession();
    bool callManager();

    LeaderboardManager* m_pManager;
    bool m_bRequestPending;
    bool m_bSessionOpen;
    LeaderboardManager::EReadType m_eReadType;
    LeaderboardReadListener* m_pReadListener;
    int m_iUserIndex;
    LeaderboardManager::EStatsType m_eStatsType;
    PlayerUID m_uid;
    unsigned int m_uiStartRank;
    unsigned int m_uiNumRows;
};

#endif
