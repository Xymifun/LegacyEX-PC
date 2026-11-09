#ifndef GAME_CORE_SCOREBOARD_HPP
#define GAME_CORE_SCOREBOARD_HPP

#include <stdint.h>
#include <functional>
#include <string>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/StatsUID.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/generation/PlayerUID.hpp"

class DamageSource;
class DataInputStream;
class DataOutputStream;
class GameMedal;
class Item;
class ItemInstance;
class PlayerTeam;
class Potion;
class ServerPlayer;

class Scoreboard
{
public:
    typedef boost::unordered_map<PlayerUID, PlayerTeam*, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >
        PlayerTeamByUIDMap;

    enum EDamageSource
    {
        DAMAGE_SOURCE_FIRE,
        DAMAGE_SOURCE_LAVA,
        DAMAGE_SOURCE_DROWN,
        DAMAGE_SOURCE_FALL,
        DAMAGE_SOURCE_CACTUS,
        DAMAGE_SOURCE_STARVE,
        DAMAGE_SOURCE_MAGIC,
        DAMAGE_SOURCE_THORNS,
        DAMAGE_SOURCE_POISON,
        DAMAGE_SOURCE_HARM,
        DAMAGE_SOURCE_MELEE,
        DAMAGE_SOURCE_ARROW,
        DAMAGE_SOURCE_EXPLOSION,
        DAMAGE_SOURCE_FLY_INTO_WALL,
        DAMAGE_SOURCE_UNKNOWN,
        DAMAGE_SOURCE_COUNT
    };

    enum eStringArgIndex
    {
        eStringArg_PlayerName,
        eStringArg_KillerName,
        eStringArg_Count
    };

    enum eBoolArgIndex
    {
        eBoolArg_Active,
        eBoolArg_Finished,
        eBoolArg_Winner,
        eBoolArg_Count
    };

    enum eFloatArgIndex
    {
        eFloatArg_DistanceTravelled,
        eFloatArg_LongestShot,
        eFloatArg_FallDistance,
        eFloatArg_BestTime,
        eFloatArg_Count
    };

    enum eIntArgIndex
    {
        eIntArg_MedalId,
        eIntArg_FavouriteWeaponItem,
        eIntArg_FavouriteWeaponPotion,
        eIntArg_Score,
        eIntArg_Placement,
        eIntArg_KillerWeaponItem,
        eIntArg_KillerWeaponPotion,
        eIntArg_Kills,
        eIntArg_Deaths,
        eIntArg_Lives,
        eIntArg_DamageDealt,
        eIntArg_DamageTaken,
        eIntArg_ArrowsFired,
        eIntArg_ArrowsHit,
        eIntArg_BlocksPlaced,
        eIntArg_MobKills,
        eIntArg_ItemsCollected,
        eIntArg_BlocksMined,
        eIntArg_TimePlayed,
        eIntArg_TotalTime,
        eIntArg_LastCheckpointId,
        eIntArg_LastBankedCheckpointId,
        eIntArg_CheckpointTime,
        eIntArg_LastCheckpointSequenceId,
        eIntArg_LapsCompleted,
        eIntArg_CheckpointsPassed,
        eIntArg_KillsMelee,
        eIntArg_KillsRanged,
        eIntArg_KillsExplosive,
        eIntArg_KillsOther,
        eIntArg_DeathsMelee,
        eIntArg_DeathsRanged,
        eIntArg_DeathsExplosive,
        eIntArg_DeathsOther,
        eIntArg_WeaknessWeaponItem,
        eIntArg_WeaknessWeaponPotion,
        eIntArg_Count
    };

    enum eInt64ArgIndex
    {
        eInt64Arg_Count
    };

    class Weapon
    {
    public:
        Weapon();
        Weapon(int itemId);
        Weapon(int itemId, Potion* pPotion);

        uint32_t GetKey() const;
        bool HasItem() const;
        Item* GetItem() const;
        not_null_ptr<ItemInstance> GetItemInstance() const;

        int mItemId;
        int mPotionId;
    };

    class PlayerStats : public StatsUID
    {
    public:
        class _RoundStats
        {
        public:
            struct _PerLife
            {
                std::wstring mKillerName;
            };

            _RoundStats();

            bool mActive;
            bool mFinished;
            int mScore;
            int mPlacement;
            int mKills;
            int mDeaths;
            Weapon mKillerWeapon;
            int mLives;
            std::vector<_PerLife> mPerLife;
            int mDamageDealt;
            int mDamageTaken;
            int mTimeAlive;
            bool mQuit;
            bool mWinner;
            int mArrowsFired;
            int mArrowsHit;
            int mBlocksPlaced;
            unsigned int mLastCheckpointId;
            int mLastCheckpointSequenceId;
            int mLastBankedCheckpointId;
            int mLapsCompleted;
            double mDistanceTravelled;
            double mLongestShot;
            double mFallDistance;
            boost::unordered_map<int, int> mCheckpointTimes;
            int mMobKills;
            int mCheckpointsPassed;
            int mItemsCollected;
            int mBlocksMined;
            int mTimePlayed;
            int mTotalTime;
            double mBestTime;
            unsigned short mScoresGiven[4];
            unsigned short mScoresReceived[4];
            std::wstring mLastKillerName;
            std::wstring mLastVictimName;
        };

        typedef _RoundStats::_PerLife _PerLife;
        typedef boost::unordered_map<int, _RoundStats> RoundStatsMap;

        enum eRecordIndex
        {
            eRecord_SUID = 0,
            eRecord_StringFirst = 1,
            eRecord_BoolFirst = eRecord_StringFirst + eStringArg_Count,
            eRecord_FloatFirst = eRecord_BoolFirst + eBoolArg_Count,
            eRecord_IntFirst = eRecord_FloatFirst + eFloatArg_Count,
            eRecord_Int64First = eRecord_IntFirst + eIntArg_Count,
            eRecord_Count = eRecord_Int64First + eInt64Arg_Count
        };

        PlayerStats();

        StatsUID getRecordSUID();
        void setRecordSUID(const StatsUID& suid);

        std::wstring getRecord(eStringArgIndex index, int round, int life);
        bool getRecord(eBoolArgIndex index, int round);
        float getRecord(eFloatArgIndex index, int round);
        int getRecord(eIntArgIndex index, int round);
        long long getRecord(eInt64ArgIndex index, int round);

        void setRecord(eStringArgIndex index, int round, int life, std::wstring value);
        void setRecord(eBoolArgIndex index, int round, bool value);
        void setRecord(eFloatArgIndex index, int round, float value);
        void setRecord(eIntArgIndex index, int round, int value);
        void setRecord(eInt64ArgIndex index, int round, long long value);

        bool isRecordSame(PlayerStats* pOther, int record, int round, int life);
        unsigned long long getChangedBitfield(PlayerStats* pOther, int round, int life);
        void updateFromOtherStats(PlayerStats* pOther, int round, int life, unsigned long long changed);

        void readRecord(DataInputStream* pInput, int record, int round, int life);
        void writeRecord(DataOutputStream* pOutput, int record, int round, int life);

        std::wstring mName;
        const GameMedal* mMedal;
        Weapon mFavouriteWeapon;
        Weapon mWeaknessWeapon;
        RoundStatsMap mRoundStats;
    };

    Scoreboard();
    virtual ~Scoreboard();

    virtual void ResetPlayerScores() = 0;
    virtual void removePlayerFromTeam(PlayerUID uid, PlayerTeam* pTeam);
    virtual void addPlayerToTeam(PlayerUID uid, PlayerTeam* pTeam);
    virtual void onTeamAdded(PlayerTeam* pTeam);
    virtual void onTeamChanged(PlayerTeam* pTeam);
    virtual void onTeamRemoved(PlayerTeam* pTeam);
    virtual void syncTeamChanges(PlayerTeam* pTeam);
    virtual int GetLastCheckpointID(const StatsUID& suid, int round) = 0;
    virtual int GetLapsCompleted(const StatsUID& suid, int round) = 0;
    virtual bool GetEndZoneEntered(const StatsUID& suid, int round) = 0;
    virtual int GetLastCheckpointSequenceID(const StatsUID& suid, int round) = 0;

    PlayerTeam* getPlayerTeam(const std::wstring& name);
    PlayerTeam* addPlayerTeam(const std::wstring& name);
    void removePlayerTeam(PlayerTeam* pTeam);
    void removeAllPlayerTeams();
    PlayerTeam* getPlayersTeam(PlayerUID uid);
    bool removePlayerFromTeam(PlayerUID uid);
    std::vector<PlayerTeam*>* getPlayerTeams();
    std::vector<std::wstring>* getTeamNames();
    PlayerTeam* getTeamByIndex(int index) const;
    boost::shared_ptr<ServerPlayer> getPlayer(PlayerUID uid);

    static EDamageSource GetDamageSource(const DamageSource* pSource);
    static Potion* GetPotionByDamageSource(EDamageSource damageSource);

    boost::unordered_map<std::wstring, PlayerTeam*> mTeamsByName;
    PlayerTeamByUIDMap mTeamsByPlayer;
    int mCurrentRound;
};

#endif
