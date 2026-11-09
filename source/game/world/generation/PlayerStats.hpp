#ifndef GAME_WORLD_GENERATION_PLAYERSTATS_HPP
#define GAME_WORLD_GENERATION_PLAYERSTATS_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/core/Scoreboard.hpp"
#include "game/core/StatsUID.hpp"
#include "game/world/block/BlockPos.hpp"

class AmphibiousKillStatTracker;
class DamageDealtStatTracker;
class DamageEvent;
class DamageReceivedStatTracker;
class DamageSourceMedal;
class DamageTakenBySourceStatTracker;
class DeadshotStatTracker;
class FallKillStatTracker;
class GameMedal;
class GameStats;
class Item;
class KillsWithBowStatTracker;
class LastPlayerDamageStatTracker;
class LastPlayerFatalDamageStatTracker;
class MaxSnowballHitsWithSnowballOnPlayerStatTracker;
class SniperStatTracker;
class TotalKillsStatTracker;
class UntypedStatTracker;
class WeaponDamageStatTracker;

enum eTimeSpentCategory
{
    eTimeSpentCategory_Leading,
    eTimeSpentCategory_Trailing,
    eTimeSpentCategory_BetweenCheckpoints,
    eTimeSpentCategory_GoingBackwards,
    eTimeSpentCategory_NearBlocks,
    eTimeSpentCategory_InWater,
    eTimeSpentCategory_Count
};

class ContainerInfo
{
public:
    int mX;
    int mY;
    int mZ;
    int mType;
};

class PlayerStats
{
public:
    class RoundStats
    {
    public:
        class BlockInteractionInfo
        {
        public:
            BlockInteractionInfo();

            int mCount;
            std::map<int, int> mByBlock;
            std::map<int, int> mByHeight;
            std::map<int, int> mByTime;
        };

        class PerLifeStats
        {
        public:
            std::wstring mKillerName;
        };

        static const int NO_LAYER = -1;
        static const int LAYERS_UNUSED = -2;
        static const int RING_SIZE_COUNT = 3;
        static const int BUILD_SCORE_COUNT = 4;

        RoundStats(const StatsUID& statsUID);
        ~RoundStats();

        PerLifeStats* getLifeStats(int life);
        int getNumberLifeStats() const;
        void MedalTimestamp(eTimeSpentCategory category, bool start, long long timestamp);

        std::vector<UntypedStatTracker*> mTrackers;
        bool mWon;
        bool mParticipated;
        DamageDealtStatTracker* mDamageDealt;
        DamageReceivedStatTracker* mDamageReceived;
        LastPlayerDamageStatTracker* mLastPlayerDamage;
        LastPlayerFatalDamageStatTracker* mLastPlayerFatalDamage;
        KillsWithBowStatTracker* mKillsWithBow;
        MaxSnowballHitsWithSnowballOnPlayerStatTracker* mMaxSnowballHits;
        int mJumpCount;
        int mKills;
        std::map<int, int> mItems;
        std::vector<ContainerInfo*> mContainers;
        int mWalkDistance;
        int mSprintDistance;
        int mSneakDistance;
        int mSwimDistance;
        int mUnderwaterDistance;
        int mClimbDistance;
        std::vector<Item*> mItemsUsed;
        int mSplashPotionsUsed;
        int mPotionsUsed;
        int mSnowballsThrown;
        int mLifeSpan;
        Scoreboard::Weapon mFatalWeapon;
        const DamageEvent* mFatalDamage;
        int mField0x94;
        int mDeathLayer;
        int mLayerChanges;
        std::vector<unsigned int> mTimeOnLayer;
        int mFinalLayer;
        bool mLanded;
        int mDeathOrder;
        bool mReachedEnd;
        int mDeaths;
        int mTargetsReached;
        int mPoints;
        int mUnbankedPoints;
        int mLastBankedPoints;
        unsigned int mLastPointsAdded;
        double mTopSpeed;
        std::vector<std::wstring> mBankedTargets;
        std::vector<std::wstring> mUnbankedTargets;
        int mUnbankedRings[RING_SIZE_COUNT];
        int mBankedRings[RING_SIZE_COUNT];
        int mLastCheckpointID;
        int mLastCheckpointSequenceID;
        int mLastBankedCheckpointID;
        int mLapsCompleted;
        double mSequenceProgress;
        double mNearestDistance;
        double mProgress;
        boost::unordered_map<int, int> mCheckpointTimes;
        int mThermalsEntered;
        int mTimeSpent[eTimeSpentCategory_Count];
        long long mTimestamps[eTimeSpentCategory_Count];
        std::set<int> mSpawnedCheckpoints;
        int mFinishPosition;
        int mFinishPlayerCount;
        unsigned char mReserved0x1bc[4];
        int mBuildAreaFromY;
        unsigned char mReserved0x1c4[16];
        BlockInteractionInfo mBlocksPlaced;
        BlockInteractionInfo mBlocksDestroyed;
        BlockInteractionInfo mBlocksPresent;
        unsigned short mScoresGiven[BUILD_SCORE_COUNT];
        unsigned short mScoresReceived[BUILD_SCORE_COUNT];
        std::wstring mThemeVote;
        std::vector<PerLifeStats> mLifeStats;
    };

    static const int DAMAGE_SOURCE_COUNT = 15;

    PlayerStats(const StatsUID& statsUID);
    ~PlayerStats();

    RoundStats* getRoundStats(unsigned int round);
    void updateTrackedStats(int round, const DamageEvent* damageEvent);
    unsigned int getTotalRounds();
    float getDamageDealt(int round);
    float getDamageTaken(int round);
    int getTotalDistanceMoved(unsigned int round);
    float getAllDamageTaken();
    int getTotalJumpCount();
    int getTotalSecondaryTargets();
    int getTotalDeaths();
    int getDeaths(unsigned int round);
    int getDeathsInAnyRound(int deaths);
    int getTotalKills();
    void getAllItems(std::map<int, int>& items);
    int getTotalWalkDistance();
    int getTotalSprintDistance();
    int getTotalSneakDistance();
    int getTotalSwimDistance();
    int getTotalUnderwaterDistance();
    int getTotalClimbDistance();
    void getAllItemsUsed(std::vector<Item*>& items);
    void getAllContainers(std::vector<ContainerInfo*>& containers);
    int getTotalRoundsWon();
    bool getRoundWon(unsigned int round);
    int getTotalBlocksDestroyed();
    int getTotalSplashPotionsUsed();
    int getTotalPotionsUsed();
    int getTotalLifeSpan();
    unsigned int getLifeSpan(unsigned int round);
    unsigned int getBestLifeSpan(bool highest, bool reachedEndOnly);
    int getTotalPoints();
    int GetLastPointsAdded(unsigned int round);
    int getPoints(unsigned int round);
    unsigned int getBestPoints(bool highest, bool reachedEndOnly);
    int getTotalRoundsReachedEnd();
    bool getReachedEnd(unsigned int round);
    int getTotalThermalsEntered();
    bool hasSpawnedAtCheckpoint(int checkpoint);
    bool hasSpawnedAtCheckpoint(int checkpoint, unsigned int round);
    bool hasSpawnedAtAllCheckpoints();
    bool hasSpawnedAtAllCheckpointsInOneRound();
    double getTopSpeed();
    int getImprovement();
    bool hasRoundStats(unsigned int round) const;

    StatsUID mStatsUID;
    std::wstring mName;
    const GameMedal* mAwardedMedal;
    std::vector<const GameMedal*> mMedals;
    int mMedalWeight;
    int mBlocksDestroyedFromBelow;
    unsigned int mSnowballDirectHits;
    unsigned int mHitsBySnowball;
    DamageDealtStatTracker* mDamageDealt;
    DamageReceivedStatTracker* mDamageReceived;
    FallKillStatTracker* mFallKills;
    AmphibiousKillStatTracker* mAmphibiousKills;
    TotalKillsStatTracker* mTotalKills;
    SniperStatTracker* mSniper;
    DamageTakenBySourceStatTracker* mDamageTakenBySource[DAMAGE_SOURCE_COUNT];
    WeaponDamageStatTracker* mWeaponDamage[2];
    DeadshotStatTracker* mDeadshot;
    std::vector<UntypedStatTracker*> mTrackers;
    boost::unordered_map<int, RoundStats*> mRoundStats;
};

int getBlocksPlaced(const PlayerStats::RoundStats& roundStats);
int getBlocksDestroyed(const PlayerStats::RoundStats& roundStats);
int getBlocksPresent(const PlayerStats::RoundStats& roundStats);
int wasWinnerWithoutDestroyingBlocks(const PlayerStats::RoundStats& roundStats);
int lostFirstWithoutDestroyingBlocks(const PlayerStats::RoundStats& roundStats);
void writeBlockInteractionData(PlayerStats::RoundStats& roundStats, PlayerStats::RoundStats::BlockInteractionInfo& info,
                               const long long& roundStartTime, const BlockPos& pos, int blockId);

#endif
