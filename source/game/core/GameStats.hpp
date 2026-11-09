#ifndef GAME_CORE_GAMESTATS_HPP
#define GAME_CORE_GAMESTATS_HPP

#include <string>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>

#include "game/core/StatsUID.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/GlideRingGenerator.hpp"
#include "game/world/generation/PlayerStats.hpp"

class ClientboundChatPacket;
class CombatEntry;
class DamageEvent;
class Entity;
class ItemInstance;
class ServerPlayer;

class GameStats
{
    friend class CheckpointCommand;
    friend class MasterGameMode;
    friend class ServerScoreboard;
    friend class FastestTimeAndHightestScoreInARoundCondition;
    friend class NotFastestTimeButWonAnywayCondition;

public:
    typedef boost::unordered_map<StatsUID, PlayerStats*, boost::hash<StatsUID> > StatsMap;

    GameStats();
    ~GameStats();

    PlayerStats* GetPlayerStats(const StatsUID& uid);
    boost::shared_ptr<ServerPlayer> GetFirstPlace();
    unsigned int GetShortestLifespan(unsigned int round);
    StatsMap* GetPlayerStats();
    unsigned int GetDeathCount(const StatsUID& uid);
    void RecordBlockPresent(const StatsUID& uid, const BlockPos& pos, int blockId);
    unsigned int GetRoundTime();
    void AwardMG01Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players);
    static bool SORT_MG02_STATS(PlayerStats* first, PlayerStats* second);
    static boost::weak_ptr<ServerPlayer> getPlayerFromStatsVec(const std::vector<PlayerStats*>& stats,
                                                               unsigned int index);
    void AwardMG02Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players);
    static bool SORT_MG03_STATS_FALLBACK(PlayerStats* first, PlayerStats* second);
    static bool SORT_MG03_STATS_POINTS(PlayerStats* first, PlayerStats* second);
    static bool SORT_MG03_STATS_TIME(PlayerStats* first, PlayerStats* second);
    static bool SORT_MG03_STATS_POINTS_CURRENT_ROUND(PlayerStats* first, PlayerStats* second);
    static bool SORT_MG03_STATS_TIME_CURRENT_ROUND(PlayerStats* first, PlayerStats* second);
    static bool SORT_MG03_STATS_FALLBACK_CURRENT_ROUND(PlayerStats* first, PlayerStats* second);
    void AwardMG03Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players);
    void AwardMG04Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players);
    void CalculateStats(const std::vector<boost::shared_ptr<ServerPlayer> >* players, bool newRound);
    void AddPlayerStats(const boost::shared_ptr<ServerPlayer>& player);
    void AddPlayers(const std::vector<boost::shared_ptr<ServerPlayer> >* players);
    void RemovePlayerStats(const boost::shared_ptr<ServerPlayer>& player);
    void Clear();
    void OnRoundStart();
    void OnRoundEnd();
    void OnGameStart();
    void OnGameEnd();
    unsigned int GetDamageTakenThisRound(const StatsUID& uid);
    unsigned int GetKillsWithBowThisRound(const StatsUID& uid);
    unsigned int GetMaxSnowballHitsOnAnyPlayer(const StatsUID& uid);
    void RecordDamage(boost::shared_ptr<Entity> entity, CombatEntry* entry,
                      boost::shared_ptr<ClientboundChatPacket> deathMessage);
    const DamageEvent* GetPlayerFatalDamage(const StatsUID& uid);
    void RecordDeath(boost::shared_ptr<Entity> entity);
    void ResetDeaths(const StatsUID& uid);
    int GetFavouriteWeapon(const StatsUID& uid);
    int GetWeaponRecievedMostDamageFrom(const StatsUID& uid);
    void RecordJump(const StatsUID& uid);
    unsigned int GetJumpCount(const StatsUID& uid);
    void RecordItem(const StatsUID& uid, not_null_ptr<ItemInstance> item);
    std::vector<int> GetItems(const StatsUID& uid);
    void RecordContainer(const StatsUID& uid, int type, int x, int y, int z);
    std::vector<int> GetContainers(const StatsUID& uid);
    void RecordWalk(const StatsUID& uid, int distance, bool sprinting, bool sneaking);
    void RecordSwim(const StatsUID& uid, int distance, bool underwater);
    void RecordClimb(const StatsUID& uid, int distance);
    void RecordGainPoints(const StatsUID& uid, int points);
    void ResetTempPoints(const StatsUID& uid);
    bool RecordSecondaryTargetReached(const StatsUID& uid, const std::wstring& target, int points,
                                      GlideRingGenerator::eGlideRingSize size);
    void ResetSecondaryTargets(const StatsUID& uid);
    unsigned int GetSecondaryTargetCount(const StatsUID& uid);
    int GetPoints(const StatsUID& uid);
    void TakeTimestamp(const StatsUID& uid, eTimeSpentCategory category, bool start);
    void RecordUseItem(const StatsUID& uid, not_null_ptr<ItemInstance> item);
    void RecordTimeOnLayer(const StatsUID& uid, int layer, bool enteredLayer);
    void RecordSnowballDirectHit(const StatsUID& uid);
    void RecordHitBySnowball(const StatsUID& uid);
    void RecordLanding(const StatsUID& uid);
    void RecordBuildArea(const StatsUID& uid, const BlockPos& from, const BlockPos& to);
    void RecordBlockPlaced(const StatsUID& uid, const BlockPos& pos, int blockId);
    void RecordBlockDestroyed(const StatsUID& uid, const BlockPos& pos, int blockId);
    bool RecordLastCheckpoint(const StatsUID& uid, int checkpoint, int sequence, bool banked);
    void RecordLapCompleted(const StatsUID& uid);
    unsigned int GetLapsCompleted(const StatsUID& uid);
    unsigned int GetLastCheckpointID(const StatsUID& uid);
    unsigned int GetLastCheckpointSequenceID(const StatsUID& uid);
    unsigned int GetLastBankedCheckpointID(const StatsUID& uid);
    void RecordThermalEntered(const StatsUID& uid);
    bool RecordProgress(const StatsUID& uid, double sequenceProgress, double nearestDistance, double progress,
                        double speed);
    void RecordSpawnedAtCheckpoint(const StatsUID& uid, int checkpoint);
    void RecordTimeSpentInPosition(bool pointsActive);
    bool RecordEndZoneEntered(const StatsUID& uid);
    bool GetEndZoneEntered(const StatsUID& uid);
    unsigned int GetTimeToCheckpoint(const StatsUID& uid, int checkpoint);
    boost::shared_ptr<ServerPlayer> GetLeadingPlayer();
    boost::shared_ptr<ServerPlayer> GetTrailingPlayer(const boost::shared_ptr<ServerPlayer>& player);
    void increasePlayerRoundStartTimeMs(long long milliseconds);
    long long GetStartTime();
    long long GetPlayerLifeTime(const StatsUID& uid);
    boost::shared_ptr<ServerPlayer> GetRoundWinner();
    boost::shared_ptr<ServerPlayer> GetSecondPlace();
    boost::shared_ptr<ServerPlayer> GetThirdPlace();
    std::wstring GetWinningThemeChoice() const;
    int GetMedal(const StatsUID& uid);
    int GetBestMedal(const StatsUID& uid);
    unsigned int GetRoundsWon(const StatsUID& uid);
    unsigned int GetMostBlocksDestroyed();
    unsigned int GetSecondMostBlocksDestroyed();
    unsigned int GetThirdMostBlocksDestroyed();
    unsigned int GetFastestPlayerTime();
    unsigned int GetHighestScoringPlayerScore();
    unsigned int GetFarthestCheckpointReached();

private:
    int mRoundDeathCount;
    bool mAchievementExploitDetected;
    unsigned int mCurrentRound;
    bool mLandingRecorded;
    unsigned int mFarthestCheckpointReached;
    StatsMap mPlayerStats;
    long long mRoundStartTime;
    long long mRoundEndTime;
    boost::shared_ptr<ServerPlayer> mRoundWinner;
    boost::weak_ptr<ServerPlayer> mFirstPlace;
    boost::weak_ptr<ServerPlayer> mSecondPlace;
    boost::weak_ptr<ServerPlayer> mThirdPlace;
    std::wstring mWinningThemeChoice;
};

#endif
