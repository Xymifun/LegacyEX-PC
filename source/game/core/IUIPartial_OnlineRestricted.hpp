#ifndef GAME_CORE_IUIPARTIAL_ONLINERESTRICTED_HPP
#define GAME_CORE_IUIPARTIAL_ONLINERESTRICTED_HPP

class MiniGameDef;

class IUIPartial_OnlineRestricted
{
public:
    virtual unsigned int GetCustomGameModeID() = 0;

    bool IsUGCRestrictedMode();
    bool IsUGCRestrictedMode(int userIndex);
    MiniGameDef* GetMiniGame();
};

#endif
