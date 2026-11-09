#include "game/world/generation/MiniGamePlayerSettings.hpp"

MiniGamePlayerSettings::~MiniGamePlayerSettings()
{
}

bool MiniGamePlayerSettings::operator==(const MiniGamePlayerSettings& other) const
{
    return mHostOptions.Get(eCustomGameModeHostOption_All) == other.mHostOptions.Get(eCustomGameModeHostOption_All)
           && mPlayerAbilities == other.mPlayerAbilities && mLivesPerRound == other.mLivesPerRound
           && mLootSetId == other.mLootSetId && mRandomLootSet == other.mRandomLootSet && mTeamCount == other.mTeamCount
           && mTeamMaxSize == other.mTeamMaxSize && mTeamBasedSpawn == other.mTeamBasedSpawn
           && mCheckpointBasedSpawn == other.mCheckpointBasedSpawn && mShowNametags == other.mShowNametags
           && mTakeEverythingEnabled == other.mTakeEverythingEnabled && mChestRefillEnabled == other.mChestRefillEnabled
           && ((!mCanTntDestroyBlocks.isSet() && !other.mCanTntDestroyBlocks.isSet())
               || mCanTntDestroyBlocks.get() == other.mCanTntDestroyBlocks.get())
           && ((!mMapSizeOverride.isSet() && !other.mMapSizeOverride.isSet())
               || mMapSizeOverride.get() == other.mMapSizeOverride.get())
           && mRandomMapSize == other.mRandomMapSize
           && ((!mArenaSizeMultiplier.isSet() && !other.mArenaSizeMultiplier.isSet())
               || mArenaSizeMultiplier.get() == other.mArenaSizeMultiplier.get())
           && ((!mLayerCount.isSet() && !other.mLayerCount.isSet()) || mLayerCount.get() == other.mLayerCount.get())
           && mRoundCount == other.mRoundCount && mSmallInventoryEnabled == other.mSmallInventoryEnabled
           && mNaturalRegeneration == other.mNaturalRegeneration && mFoodBalance == other.mFoodBalance
           && mSmallSkinsEnabled == other.mSmallSkinsEnabled && mRoundLength == other.mRoundLength
           && mMaxPlayers == other.mMaxPlayers && mCentralSpawnDisabled == other.mCentralSpawnDisabled
           && mTeamGame == other.mTeamGame && mSpectatorParticipation == other.mSpectatorParticipation;
}
