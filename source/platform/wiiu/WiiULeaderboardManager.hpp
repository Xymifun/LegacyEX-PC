#ifndef PLATFORM_WIIU_WIIULEADERBOARDMANAGER_HPP
#define PLATFORM_WIIU_WIIULEADERBOARDMANAGER_HPP

#include "game/core/LeaderboardManager.hpp"

class WiiULeaderboardManager : public LeaderboardManager
{
public:
    WiiULeaderboardManager();
    virtual ~WiiULeaderboardManager();
    virtual void Tick();
    virtual bool OpenSession();
    virtual void CloseSession();
    virtual void DeleteSession();
    virtual bool WriteStats(unsigned int uiNumScores, RegisterScore* pScores);
    virtual void FlushStats();
    virtual void CancelOperation();
    virtual bool isIdle();
};

#endif
