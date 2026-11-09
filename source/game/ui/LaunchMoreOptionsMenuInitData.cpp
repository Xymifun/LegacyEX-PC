#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"

LaunchMoreOptionsMenuInitData::LaunchMoreOptionsMenuInitData()
{
    mSettingsChanged = false;
    mTimeTrial = false;
    mHostPrivileges = false;
    mMatchmakingFilters[1] = 0;
    mDoTileDrops = true;
    mMatchmakingFilters[0] = 0;
    mInviteOnly = false;
#ifdef MINECRAFT_PC
    mPublicWorldTitle = false;
#endif
    mTrustPlayers = false;
    mDoMobSpawning = true;
    mDoWeatherCycle = true;
    mResetEnd = false;
    mTexturePackId = 0;
    mDoDaylightCycle = true;
    mMiniGameId = eMiniGameId_Any;
    mStructures = false;
    mBonusChest = false;
    mNaturalRegeneration = true;
    mJoinMiniGame = false;
    mHostMiniGame = false;
    mResetNether = false;
    mFlatWorld = false;
    mNewWorld = false;
    mMobGriefing = true;
    mOnlineGame = true;
    mPad = -1;
    mWorldTypeLocked = false;
    mDoMobLoot = true;
    mAllowFriendsOfFriends = true;
    mPvP = true;
    mMatchmaking = false;
    mKeepInventory = false;
    mFireSpreads = true;
    mTNT = true;
    mSeed.assign(L"");
    mFlag54 = false;
    mValue44 = 3;
    mDisableSaving = false;
    mWorldSize = 0;
    mCurrentWorldSize = 0;
    mSuperflatConfig.makePreset(DefaultPresets::CLASSIC);
    mValue13C = 0;
    mBiomeScale = 0;
    mFindBalancedSeed = true;
    mBiomeCentreZChunk = 0;
    mBiomeCentreXChunk = 0;
}
