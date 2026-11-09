#include "game/network/DebugReadListener.hpp"

DebugReadListener::~DebugReadListener()
{
}

bool DebugReadListener::OnStatsReadComplete(LeaderboardManager::eStatsReturn eResult, int iUserIndex,
                                            LeaderboardManager::ReadView readView)
{
    LeaderboardManager::printStats(readView);
    return true;
}
