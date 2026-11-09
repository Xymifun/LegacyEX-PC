#include "platform/wiiu/WiiULeaderboardManager.hpp"

LeaderboardManager* g_pLeaderboardManager = new WiiULeaderboardManager();

WiiULeaderboardManager::WiiULeaderboardManager() : LeaderboardManager()
{
}

WiiULeaderboardManager::~WiiULeaderboardManager()
{
}

void WiiULeaderboardManager::Tick()
{
}

bool WiiULeaderboardManager::OpenSession()
{
    return true;
}

void WiiULeaderboardManager::CloseSession()
{
}

void WiiULeaderboardManager::DeleteSession()
{
}

bool WiiULeaderboardManager::WriteStats(unsigned int uiNumScores, RegisterScore* pScores)
{
    return true;
}

void WiiULeaderboardManager::FlushStats()
{
}

void WiiULeaderboardManager::CancelOperation()
{
}

bool WiiULeaderboardManager::isIdle()
{
    return true;
}
