#ifndef GAME_WORLD_GENERATION_MINIGAMEPLAYERSETTINGS_HPP
#define GAME_WORLD_GENERATION_MINIGAMEPLAYERSETTINGS_HPP

#include "game/core/Abilities.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/maybe.hpp"

class MiniGamePlayerSettings
{
public:
    MiniGamePlayerSettings()
        : mHostOptions(),
          mPlayerAbilities(),
          mLivesPerRound(1),
          mLootSetId(0),
          mRandomLootSet(false),
          mTeamCount(0),
          mTeamMaxSize(1),
          mTeamBasedSpawn(false),
          mCheckpointBasedSpawn(false),
          mShowNametags(false),
          mTakeEverythingEnabled(true),
          mChestRefillEnabled(true),
          mCanTntDestroyBlocks(),
          mMapSizeOverride(),
          mRandomMapSize(false),
          mArenaSizeMultiplier(),
          mLayerCount(),
          mRoundCount(0),
          mSmallInventoryEnabled(true),
          mNaturalRegeneration(true),
          mFoodBalance(0),
          mSmallSkinsEnabled(true),
          mRoundLength(0),
          mMaxPlayers(8),
          mCentralSpawnDisabled(false),
          mTeamGame(false),
          mSpectatorParticipation(false)
    {
    }
    ~MiniGamePlayerSettings();

    bool operator==(const MiniGamePlayerSettings& other) const;

    GameModeOptions mHostOptions;
    Abilities mPlayerAbilities;
    unsigned int mLivesPerRound;
    unsigned int mLootSetId;
    bool mRandomLootSet;
    unsigned int mTeamCount;
    unsigned int mTeamMaxSize;
    bool mTeamBasedSpawn;
    bool mCheckpointBasedSpawn;
    bool mShowNametags;
    bool mTakeEverythingEnabled;
    bool mChestRefillEnabled;
    maybe<bool> mCanTntDestroyBlocks;
    maybe<int> mMapSizeOverride;
    bool mRandomMapSize;
    maybe<float> mArenaSizeMultiplier;
    maybe<unsigned int> mLayerCount;
    int mRoundCount;
    bool mSmallInventoryEnabled;
    bool mNaturalRegeneration;
    int mFoodBalance;
    bool mSmallSkinsEnabled;
    int mRoundLength;
    int mMaxPlayers;
    bool mCentralSpawnDisabled;
    bool mTeamGame;
    bool mSpectatorParticipation;
};

#endif
