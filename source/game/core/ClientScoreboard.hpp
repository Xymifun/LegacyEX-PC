#ifndef GAME_CORE_CLIENTSCOREBOARD_HPP
#define GAME_CORE_CLIENTSCOREBOARD_HPP

#include <list>
#include <string>
#include <utility>

#include <boost/unordered_map.hpp>

#include "game/core/Scoreboard.hpp"
#include "game/minigame/EMiniGameId.hpp"

class GameMedal;

class ClientScoreboard : public Scoreboard
{
public:
    typedef boost::unordered_map<StatsUID, PlayerStats*, boost::hash<StatsUID>, std::equal_to<StatsUID> >
        PlayerStatsMap;

    ClientScoreboard();

    void SyncPlayerStats(const StatsUID& suid, PlayerStats* stats, int round);
    void UpdatePlayerStats(const StatsUID& suid, PlayerStats* stats, int round, int life, unsigned long long changed);
    PlayerStats* GetPlayerStats(const StatsUID& suid);
    PlayerStats::_RoundStats* GetPlayerRoundStats(const StatsUID& suid, int round);
    int GetLastBankedCheckpointID(const StatsUID& suid, int round);
    void RemovePlayerStats(const StatsUID& suid);
    virtual void ResetPlayerScores();
    int GetActivePlayerCount(EMiniGameId miniGameId);
    std::wstring GetDisplayName(const StatsUID& suid);
    GameMedal* GetMedal(const StatsUID& suid);
    bool IsWinner(const StatsUID& suid, int round);
    bool GetSurvived(const StatsUID& suid, int round);
    int GetKills(const StatsUID& suid, int round);
    not_null_ptr<ItemInstance> GetFavouriteWeapon(const StatsUID& suid);
    not_null_ptr<ItemInstance> GetWeaknessWeapon(const StatsUID& suid);
    std::wstring GetKiller(const StatsUID& suid, int round, int life);
    std::wstring GetNemesis(const StatsUID& suid);
    not_null_ptr<ItemInstance> GetKillerWeapon(const StatsUID& suid, int round);
    int GetDamageDealt(const StatsUID& suid, int round);
    int GetDamageReceived(const StatsUID& suid, int round);
    unsigned int GetLifeSpan(const StatsUID& suid, int round);
    unsigned int GetBestLifeSpan(const StatsUID& suid, int* round);
    unsigned int GetBestLifeSpan();
    int GetTotalLifeSpan(const StatsUID& suid);
    int GetRoundsWon(const StatsUID& suid);
    bool IsWinningOnMatchPoint(const StatsUID& suid);
    int GetBlocksDestroyed(const StatsUID& suid, int round);
    int GetBlocksPlaced(const StatsUID& suid, int round);
    int GetSecondaryTargets(const StatsUID& suid, int round);
    int GetPoints(const StatsUID& suid, int round);
    int GetLastPointsAdded(const StatsUID& suid, int round);
    unsigned int GetBestPoints(const StatsUID& suid, int* round);
    unsigned int GetBestPoints();
    void SetPersonalBest(const StatsUID& suid, int round);
    bool IsPersonalBest(const StatsUID& suid, int round);
    virtual int GetLastCheckpointID(const StatsUID& suid, int round);
    virtual int GetLapsCompleted(const StatsUID& suid, int round);
    virtual bool GetEndZoneEntered(const StatsUID& suid, int round);
    virtual int GetLastCheckpointSequenceID(const StatsUID& suid, int round);
    int GetDeaths(const StatsUID& suid, int round);
    double GetPercentageOfMapTraversed(const StatsUID& suid, int round);
    int GetTotalBlocksDestroyed(const StatsUID& suid);
    int GetTotalDistanceTravelled(const StatsUID& suid);
    int GetTotalTrapsSetOff(const StatsUID& suid);
    int GetTotalJumps(const StatsUID& suid);
    const unsigned short* GetVotesGiven(const StatsUID& suid, int round);
    const unsigned short* GetVotesReceived(const StatsUID& suid, int round);
    int GetRoundLength(unsigned int round);
    void GetPlayersSortedByPoints(std::list<std::pair<std::wstring, int> >& players, int round);
    void GetPlayersSortedByPosition(std::list<std::pair<std::wstring, int> >& players, int round);
    bool HasStats();
    bool HasStats(const StatsUID& suid);
    bool HasWinner(int round);
    std::wstring GetWinnerName(int round);
    bool IsOverallWinner(const StatsUID& suid);
    bool GlideHasOverallWinner();
    std::wstring GetGlideOverallWinner(bool usePoints);
    bool IsOverallGlideWinner(const StatsUID& suid, bool usePoints);

    std::wstring GetKillingBlowSource(const StatsUID& suid, int round);
    std::wstring GetKillingBlowMessage(const StatsUID& suid, int round);
    bool IsWinning(const StatsUID& suid);
    int GetBlocksPresent(const StatsUID& suid, int round);
    double GetDistanceTravelled(const StatsUID& suid, int round);
    int GetTrapsSetOff(const StatsUID& suid, int round);
    int GetJumps(const StatsUID& suid, int round);
    double GetDistanceFromLastCheckpoint(const StatsUID& suid, int round);
    double GetDistanceToNextCheckpoint(const StatsUID& suid, int round);
    int GetTimeToCheckpoint(const StatsUID& suid, int round, int checkpoint);
    int GetTotalSecondaryTargets(const StatsUID& suid);
    int GetTotalDeaths(const StatsUID& suid);
    int GetChosenBuildOffTheme(const StatsUID& suid, int round);
    int GetDesiredBuildOffTheme(const StatsUID& suid, int round);

    PlayerStatsMap mPlayerStats;
};

#endif
