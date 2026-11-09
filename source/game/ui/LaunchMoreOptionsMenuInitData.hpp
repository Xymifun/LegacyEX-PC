#ifndef GAME_UI_LAUNCHMOREOPTIONSMENUINITDATA_HPP
#define GAME_UI_LAUNCHMOREOPTIONSMENUINITDATA_HPP

#include <string>
#include <vector>

#include "game/minigame/EMiniGameId.hpp"
#include "game/world/generation/MiniGamePlayerSettings.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/ui/SuperflatMenu.hpp"
#include "game/world/LevelRules.hpp"

class LaunchMoreOptionsMenuInitData
{
public:
    LaunchMoreOptionsMenuInitData();

    friend class MasterGameMode;
    friend class IUIScene_StartGame;
    friend class MiniGameDef;
    friend class UIScene_LoadMenu;
    friend class UIScene_LaunchMoreOptionsMenu;
    friend class UIScene_MapSelectMenu;
    friend class UIScene_BiomePreviewMenu;
    friend class UIScene_InGameSettingsListMenu;
    friend class UIScene_CreateWorldMenu;
    friend class UIScene_SuperFlatMenu;
    friend class SuperflatMenu::CustomSuperflatBlocks;

private:
    bool mOnlineGame;
    bool mInviteOnly;
#ifdef MINECRAFT_PC
    bool mPublicWorldTitle;
#endif
    bool mAllowFriendsOfFriends;
    bool mNewWorld;
    bool mStructures;
    bool mFlatWorld;
    bool mBonusChest;
    bool mWorldTypeLocked;
    bool mPvP;
    bool mTrustPlayers;
    bool mFireSpreads;
    bool mTNT;
    bool mHostPrivileges;
    bool mResetNether;
    bool mResetEnd;
    bool mMobGriefing;
    bool mKeepInventory;
    bool mDoMobSpawning;
    bool mDoMobLoot;
    bool mDoTileDrops;
    bool mNaturalRegeneration;
    bool mDoDaylightCycle;
    bool mDoWeatherCycle;
    bool mOnlineSettingChangedBySystem;
    int mPad;
    int mTexturePackId;
    std::wstring mSeed;
    bool mFindBalancedSeed;
    int mValue44;
    bool mDisableSaving;
    int mWorldSize;
    int mCurrentWorldSize;
    bool mFlag54;
    bool mSettingsChanged;
    bool mHostMiniGame;
    bool mJoinMiniGame;
    bool mMatchmaking;
    EMiniGameId mMiniGameId;
    int mMatchmakingFilters[2];
    bool mTimeTrial;
    MiniGamePlayerSettings mPlayerSettings;
    unsigned char mBiomeScale;
    int mBiomeCentreXChunk;
    int mBiomeCentreZChunk;
    SuperflatConfig mSuperflatConfig;
    std::vector<LevelRules::LevelRulesetGroup*> mLevelRulesetGroups;
    int mValue13C;
};

#endif
