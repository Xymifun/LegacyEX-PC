#ifndef GAME_NETWORK_LEADERBOARDREADLISTENER_HPP
#define GAME_NETWORK_LEADERBOARDREADLISTENER_HPP

#include "game/core/LeaderboardManager.hpp"

class LeaderboardReadListener
{
public:
    virtual ~LeaderboardReadListener() {}

    virtual bool OnStatsReadComplete(LeaderboardManager::eStatsReturn eResult, int iUserIndex,
                                     LeaderboardManager::ReadView readView) = 0;
};

#endif
