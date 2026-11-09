#include "game/core/IUIPartial_OnlineRestricted.hpp"

#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"

bool IUIPartial_OnlineRestricted::IsUGCRestrictedMode()
{
    MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(GetCustomGameModeID()), true);
    return CGameNetworkManager::sInstance.IsUGCRestricted(0xff) && miniGame->AllowsUGC();
}

bool IUIPartial_OnlineRestricted::IsUGCRestrictedMode(int userIndex)
{
    MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(GetCustomGameModeID()), true);
    return CGameNetworkManager::sInstance.IsUGCRestricted(userIndex) && miniGame->AllowsUGC();
}

MiniGameDef* IUIPartial_OnlineRestricted::GetMiniGame()
{
    return MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(GetCustomGameModeID()), true);
}
