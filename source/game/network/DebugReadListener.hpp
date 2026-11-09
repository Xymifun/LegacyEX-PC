#ifndef GAME_NETWORK_DEBUGREADLISTENER_HPP
#define GAME_NETWORK_DEBUGREADLISTENER_HPP

#include "game/core/LeaderboardManager.hpp"
#include "game/network/LeaderboardReadListener.hpp"

class DebugReadListener : public LeaderboardReadListener
{
public:
    virtual ~DebugReadListener();

    virtual bool OnStatsReadComplete(LeaderboardManager::eStatsReturn eResult, int iUserIndex,
                                     LeaderboardManager::ReadView readView);
};

#endif
