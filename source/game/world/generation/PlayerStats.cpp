#include "game/world/generation/PlayerStats.hpp"

#include <climits>
#include <cmath>
#include <cstring>

#include "game/core/AmphibiousKillStatTracker.hpp"
#include "game/core/DamageDealtStatTracker.hpp"
#include "game/core/DamageReceivedStatTracker.hpp"
#include "game/core/DamageTakenBySourceStatTracker.hpp"
#include "game/core/DeadshotStatTracker.hpp"
#include "game/core/FallKillStatTracker.hpp"
#include "game/core/KillsWithBowStatTracker.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/SniperStatTracker.hpp"
#include "game/core/System.hpp"
#include "game/core/TotalKillsStatTracker.hpp"
#include "game/core/WeaponDamageStatTracker.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/LastPlayerDamageStatTracker.hpp"
#include "game/world/generation/LastPlayerFatalDamageStatTracker.hpp"
#include "game/world/generation/MaxSnowballHitsWithSnowballOnPlayerStatTracker.hpp"

namespace
{
const double BLOCK_INTERACTION_TIME_BUCKET_MS = 10000.0;
}

PlayerStats::RoundStats::BlockInteractionInfo::BlockInteractionInfo() : mCount(0), mByBlock(), mByHeight(), mByTime()
{
}

PlayerStats::PlayerStats(const StatsUID& statsUID)
    : mStatsUID(statsUID),
      mName(L""),
      mAwardedMedal(NULL),
      mMedals(),
      mMedalWeight(-1),
      mBlocksDestroyedFromBelow(0),
      mSnowballDirectHits(0),
      mHitsBySnowball(0),
      mTrackers(),
      mRoundStats()
{
    mDamageDealt = new DamageDealtStatTracker(mStatsUID);
    mTrackers.push_back(mDamageDealt);
    mDamageReceived = new DamageReceivedStatTracker(mStatsUID);
    mTrackers.push_back(mDamageReceived);
    mFallKills = new FallKillStatTracker(mStatsUID);
    mTrackers.push_back(mFallKills);
    mAmphibiousKills = new AmphibiousKillStatTracker(mStatsUID);
    mTrackers.push_back(mAmphibiousKills);
    mTotalKills = new TotalKillsStatTracker(mStatsUID);
    mTrackers.push_back(mTotalKills);
    mSniper = new SniperStatTracker(mStatsUID);
    mTrackers.push_back(mSniper);

    for (int source = 0; source < DAMAGE_SOURCE_COUNT; ++source)
    {
        mDamageTakenBySource[source] = new DamageTakenBySourceStatTracker(
            mStatsUID, static_cast<Scoreboard::EDamageSource>(source));
        mTrackers.push_back(mDamageTakenBySource[source]);
    }

    mWeaponDamage[WeaponDamageStatTracker::TRACKING_RECEIVED] = new WeaponDamageStatTracker(
        mStatsUID, WeaponDamageStatTracker::TRACKING_RECEIVED);
    mWeaponDamage[WeaponDamageStatTracker::TRACKING_DEALT] = new WeaponDamageStatTracker(
        mStatsUID, WeaponDamageStatTracker::TRACKING_DEALT);
    mTrackers.push_back(mWeaponDamage[WeaponDamageStatTracker::TRACKING_RECEIVED]);
    mTrackers.push_back(mWeaponDamage[WeaponDamageStatTracker::TRACKING_DEALT]);

    mDeadshot = new DeadshotStatTracker(mStatsUID);
    mTrackers.push_back(mDeadshot);
}

PlayerStats::~PlayerStats()
{
    for (std::vector<UntypedStatTracker*>::iterator tracker = mTrackers.begin(); tracker != mTrackers.end(); ++tracker)
        delete *tracker;

    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        delete round->second;
}

PlayerStats::RoundStats::RoundStats(const StatsUID& statsUID)
{
    mParticipated = false;
    mWon = false;

    mDamageDealt = new DamageDealtStatTracker(statsUID);
    mTrackers.push_back(mDamageDealt);
    mDamageReceived = new DamageReceivedStatTracker(statsUID);
    mTrackers.push_back(mDamageReceived);
    mLastPlayerDamage = new LastPlayerDamageStatTracker(statsUID);
    mTrackers.push_back(mLastPlayerDamage);
    mLastPlayerFatalDamage = new LastPlayerFatalDamageStatTracker(statsUID);
    mTrackers.push_back(mLastPlayerFatalDamage);
    mKillsWithBow = new KillsWithBowStatTracker(statsUID);
    mTrackers.push_back(mKillsWithBow);
    mMaxSnowballHits = new MaxSnowballHitsWithSnowballOnPlayerStatTracker(statsUID);
    mTrackers.push_back(mMaxSnowballHits);

    mItems = std::map<int, int>();
    mKills = 0;
    mJumpCount = 0;
    mUnderwaterDistance = 0;
    mClimbDistance = 0;
    mSwimDistance = 0;
    mWalkDistance = 0;
    mSprintDistance = 0;
    mSneakDistance = 0;
    mItemsUsed = std::vector<Item*>();
    mSnowballsThrown = 0;
    mPotionsUsed = 0;
    mSplashPotionsUsed = 0;
    mLifeSpan = Minecraft::InMiniGame(eMiniGameId_Glide, true) ? -1 : 0;
    mTargetsReached = 0;
    mFatalDamage = NULL;
    mField0x94 = 0;

    memset(mBankedRings, 0, sizeof(mBankedRings));
    memset(mUnbankedRings, 0, sizeof(mUnbankedRings));
    memset(mTimeSpent, 0, sizeof(mTimeSpent));
    memset(mTimestamps, 0, sizeof(mTimestamps));

    mLastCheckpointSequenceID = 0;
    mLastBankedPoints = 0;
    mLastCheckpointID = 0;
    mSequenceProgress = 0.0;
    mPoints = 0;
    mUnbankedPoints = 0;
    mTopSpeed = 0.0;
    mNearestDistance = 0.0;
    mLastBankedCheckpointID = 0;
    mLapsCompleted = 0;
    mProgress = 0.0;
    mLastPointsAdded = 0;
    mDeaths = 0;
    mCheckpointTimes = boost::unordered_map<int, int>();
    mLanded = false;
    mLayerChanges = 0;
    mDeathLayer = NO_LAYER;
    mDeathOrder = 0;
    mFinalLayer = NO_LAYER;
    mReachedEnd = false;
}

PlayerStats::RoundStats::~RoundStats()
{
    for (std::vector<UntypedStatTracker*>::iterator tracker = mTrackers.begin(); tracker != mTrackers.end(); ++tracker)
        delete *tracker;

    for (std::vector<ContainerInfo*>::iterator container = mContainers.begin(); container != mContainers.end();
         ++container)
        delete *container;
}

PlayerStats::RoundStats* PlayerStats::getRoundStats(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator found = mRoundStats.find(round);
    if (found == mRoundStats.end())
    {
        RoundStats* stats = new RoundStats(mStatsUID);
        mRoundStats[round] = stats;
        return stats;
    }
    return found->second;
}

void PlayerStats::updateTrackedStats(int round, const DamageEvent* damageEvent)
{
    for (std::vector<UntypedStatTracker*>::iterator tracker = mTrackers.begin(); tracker != mTrackers.end(); ++tracker)
        (*tracker)->update(damageEvent);

    RoundStats* stats = getRoundStats(round);
    for (std::vector<UntypedStatTracker*>::iterator tracker = stats->mTrackers.begin();
         tracker != stats->mTrackers.end(); ++tracker)
        (*tracker)->update(damageEvent);
}

unsigned int PlayerStats::getTotalRounds()
{
    if (mRoundStats.empty())
        return 1;

    unsigned int highestRound = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
    {
        if (highestRound < static_cast<unsigned int>(round->first))
            highestRound = round->first;
    }
    return highestRound + 1;
}

float PlayerStats::getDamageDealt(int round)
{
    return getRoundStats(round)->mDamageDealt->getValue();
}

float PlayerStats::getDamageTaken(int round)
{
    return getRoundStats(round)->mDamageReceived->getValue();
}

int PlayerStats::getTotalDistanceMoved(unsigned int round)
{
    RoundStats* stats = getRoundStats(round);
    if (stats == NULL)
        return 0;
    return stats->mClimbDistance + stats->mSneakDistance + stats->mSprintDistance + stats->mSwimDistance
           + stats->mUnderwaterDistance + stats->mWalkDistance;
}

float PlayerStats::getAllDamageTaken()
{
    return mDamageReceived->getValue();
}

int PlayerStats::getTotalJumpCount()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mJumpCount;
    return total;
}

int PlayerStats::getTotalDeaths()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mDeaths;
    return total;
}

int PlayerStats::getDeaths(unsigned int round)
{
    if (mRoundStats.find(round) != mRoundStats.end())
        return mRoundStats[round]->mDeaths;
    return 0;
}

int PlayerStats::getDeathsInAnyRound(int deaths)
{
    int result = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
    {
        const int roundDeaths = round->second->mDeaths;
        if (Mth::abs(roundDeaths - deaths) < INT_MAX)
            result = roundDeaths;
    }
    return result;
}

int PlayerStats::getTotalKills()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mKills;
    return total;
}

void PlayerStats::getAllItems(std::map<int, int>& items)
{
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        items.insert(round->second->mItems.begin(), round->second->mItems.end());
}

int PlayerStats::getTotalWalkDistance()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mWalkDistance;
    return total;
}

int PlayerStats::getTotalSprintDistance()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mSprintDistance;
    return total;
}

int PlayerStats::getTotalSneakDistance()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mSneakDistance;
    return total;
}

int PlayerStats::getTotalSwimDistance()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mSwimDistance;
    return total;
}

int PlayerStats::getTotalUnderwaterDistance()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mUnderwaterDistance;
    return total;
}

int PlayerStats::getTotalClimbDistance()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mClimbDistance;
    return total;
}

void PlayerStats::getAllItemsUsed(std::vector<Item*>& items)
{
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
    {
        std::vector<Item*>::iterator first = round->second->mItemsUsed.begin();
        std::vector<Item*>::iterator last = round->second->mItemsUsed.end();
        items.insert(items.end(), first, last);
    }
}

int PlayerStats::getTotalRoundsWon()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
    {
        if (round->second->mWon)
            ++total;
    }
    return total;
}

bool PlayerStats::getRoundWon(unsigned int round)
{
    if (mRoundStats.find(round) != mRoundStats.end())
        return mRoundStats[round]->mWon;
    return false;
}

int PlayerStats::getTotalBlocksDestroyed()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mBlocksDestroyed.mCount;
    return total;
}

int PlayerStats::getTotalSplashPotionsUsed()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mSplashPotionsUsed;
    return total;
}

int PlayerStats::getTotalPotionsUsed()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mPotionsUsed;
    return total;
}

int PlayerStats::getTotalLifeSpan()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mLifeSpan;
    return total;
}

unsigned int PlayerStats::getLifeSpan(unsigned int round)
{
    unsigned int lifeSpan = 0;
    if (mRoundStats.find(round) != mRoundStats.end())
        lifeSpan = mRoundStats[round]->mLifeSpan;
    return lifeSpan;
}

unsigned int PlayerStats::getBestLifeSpan(bool highest, bool reachedEndOnly)
{
    unsigned int best = highest ? 0 : UINT_MAX;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
    {
        RoundStats* stats = round->second;
        if (highest)
        {
            if (reachedEndOnly && !stats->mReachedEnd)
                continue;

            const unsigned int lifeSpan = stats->mLifeSpan;
            if (best < lifeSpan)
                best = lifeSpan;
        }
        else if (!reachedEndOnly || stats->mReachedEnd)
        {
            const unsigned int lifeSpan = stats->mLifeSpan;
            if (lifeSpan < best)
                best = lifeSpan;
        }
    }
    return best;
}

int PlayerStats::getTotalPoints()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mPoints + round->second->mUnbankedPoints;
    return total;
}

int PlayerStats::getPoints(unsigned int round)
{
    int points = 0;
    if (mRoundStats.find(round) != mRoundStats.end())
    {
        points = mRoundStats[round]->mPoints;
        points += mRoundStats[round]->mUnbankedPoints;
    }
    return points;
}

unsigned int PlayerStats::getBestPoints(bool highest, bool reachedEndOnly)
{
    unsigned int best = highest ? 0 : UINT_MAX;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
    {
        RoundStats* stats = round->second;
        if (reachedEndOnly && !stats->mReachedEnd)
            continue;

        const unsigned int points = stats->mPoints + stats->mUnbankedPoints;
        if (!highest)
        {
            if (points < best)
                best = points;
        }
        else if (best < points)
            best = points;
    }
    return best;
}

bool PlayerStats::getReachedEnd(unsigned int round)
{
    if (mRoundStats.find(round) != mRoundStats.end())
        return mRoundStats[round]->mReachedEnd;
    return false;
}

int PlayerStats::getTotalThermalsEntered()
{
    int total = 0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
        total += round->second->mThermalsEntered;
    return total;
}

bool PlayerStats::hasSpawnedAtCheckpoint(int checkpoint, unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator found = mRoundStats.find(round);
    if (found != mRoundStats.end())
    {
        const RoundStats* stats = found->second;
        return stats->mSpawnedCheckpoints.find(checkpoint) != stats->mSpawnedCheckpoints.end();
    }
    return false;
}

bool PlayerStats::hasSpawnedAtAllCheckpointsInOneRound()
{
    MasterGameMode* gameMode = gMinecraftServer->mMasterGameMode;
    if (gameMode == NULL)
        return false;

    for (unsigned int round = 0; round < static_cast<unsigned int>(getTotalRounds()); ++round)
    {
        bool spawnedAtAll = true;
        for (unsigned int index = 0; index < gameMode->mCheckpoints.size(); ++index)
        {
            CheckpointRuleDefinition* checkpoint = gameMode->mCheckpoints[index];
            if (checkpoint->hasSpawnPoint() && !hasSpawnedAtCheckpoint(checkpoint->getSequenceID(), round))
            {
                spawnedAtAll = false;
                break;
            }
        }
        if (spawnedAtAll)
            return true;
    }
    return false;
}

double PlayerStats::getTopSpeed()
{
    double topSpeed = 0.0;
    for (boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin(); round != mRoundStats.end();
         ++round)
    {
        const double speed = round->second->mTopSpeed;
        if (topSpeed < speed)
            topSpeed = speed;
    }
    return topSpeed;
}

int PlayerStats::getImprovement()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    int lastRound = miniGame->GetFinalRoundIndex();
    int firstRound = 0;

    for (int round = 0; round <= lastRound; ++round)
    {
        if (mRoundStats.find(round) != mRoundStats.end())
        {
            firstRound = round;
            break;
        }
    }
    for (int round = lastRound; firstRound <= round; --round)
    {
        if (mRoundStats.find(round) != mRoundStats.end())
        {
            lastRound = round;
            break;
        }
    }

    if (firstRound == lastRound)
        return 0;

    switch (Minecraft::GetInstance()->GetMiniGame()->GetGameVariant())
    {
    case MiniGameDef::eGlideVariant_TimeAttack:
        return getLifeSpan(firstRound) - getLifeSpan(lastRound);
    case MiniGameDef::eGlideVariant_ScoreAttack:
        return getPoints(lastRound) - getPoints(firstRound);
    default:
        return 0;
    }
}

bool PlayerStats::hasRoundStats(unsigned int round) const
{
    return mRoundStats.find(round) != mRoundStats.end();
}

PlayerStats::RoundStats::PerLifeStats* PlayerStats::RoundStats::getLifeStats(int life)
{
    if (static_cast<unsigned int>(life) >= mLifeStats.size())
        mLifeStats.push_back(PerLifeStats());
    return &mLifeStats.at(life);
}

int PlayerStats::RoundStats::getNumberLifeStats() const
{
    return static_cast<int>(mLifeStats.size());
}

void PlayerStats::RoundStats::MedalTimestamp(eTimeSpentCategory category, bool start, long long timestamp)
{
    if (!start)
    {
        if (mTimestamps[category] == 0)
            return;
        mTimeSpent[category] += static_cast<int>(timestamp - mTimestamps[category]);
        mTimestamps[category] = 0;
        return;
    }

    if (mTimestamps[category] != 0)
        return;
    mTimestamps[category] = timestamp;
}

int getBlocksPlaced(const PlayerStats::RoundStats& roundStats)
{
    return roundStats.mBlocksPlaced.mCount;
}

int getBlocksDestroyed(const PlayerStats::RoundStats& roundStats)
{
    return roundStats.mBlocksDestroyed.mCount;
}

int getBlocksPresent(const PlayerStats::RoundStats& roundStats)
{
    return roundStats.mBlocksPresent.mCount;
}

int wasWinnerWithoutDestroyingBlocks(const PlayerStats::RoundStats& roundStats)
{
    if (roundStats.mWon && roundStats.mBlocksDestroyed.mCount == 0)
        return 1;
    return 0;
}

int lostFirstWithoutDestroyingBlocks(const PlayerStats::RoundStats& roundStats)
{
    if (roundStats.mDeathOrder == 1 && roundStats.mBlocksDestroyed.mCount == 0)
        return 1;
    return 0;
}

void writeBlockInteractionData(PlayerStats::RoundStats& roundStats, PlayerStats::RoundStats::BlockInteractionInfo& info,
                               const long long& roundStartTime, const BlockPos& pos, int blockId)
{
    ++info.mCount;
    ++info.mByBlock[blockId];
    ++info.mByHeight[pos.y - roundStats.mBuildAreaFromY];
    const double elapsed = std::floor(static_cast<double>(System::processTimeInMilliSecs() - roundStartTime));
    ++info.mByTime[static_cast<int>(elapsed / BLOCK_INTERACTION_TIME_BUCKET_MS)];
}
