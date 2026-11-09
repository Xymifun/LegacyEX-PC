#ifndef GAME_SERVER_SERVERSCOREBOARD_HPP
#define GAME_SERVER_SERVERSCOREBOARD_HPP

#include <functional>
#include <string>

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/Scoreboard.hpp"
#include "game/core/StatsUID.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/world/generation/PlayerUID.hpp"

class MinecraftServer;
class ServerPlayer;

class ServerScoreboard : public Scoreboard
{
public:
    explicit ServerScoreboard(MinecraftServer* server);

    Scoreboard::PlayerStats* GetPlayerStats(const StatsUID& suid, int round);
    void OnPlayerStatsChanged(const StatsUID& suid, int life);
    void OnPlayerStatsRemoved(const StatsUID& suid, const std::wstring& name);
    void SyncPlayerScores(std::vector<boost::shared_ptr<ServerPlayer> >* players,
                          boost::shared_ptr<ServerPlayer> player);
    virtual void ResetPlayerScores();
    virtual void removePlayerFromTeam(PlayerUID uid, PlayerTeam* pTeam);
    virtual void addPlayerToTeam(PlayerUID uid, PlayerTeam* pTeam);
    virtual void onTeamAdded(PlayerTeam* pTeam);
    virtual void onTeamChanged(PlayerTeam* pTeam);
    virtual void onTeamRemoved(PlayerTeam* pTeam);
    virtual void syncTeamChanges(PlayerTeam* pTeam);
    virtual int GetLastCheckpointID(const StatsUID& suid, int round);
    virtual int GetLapsCompleted(const StatsUID& suid, int round);
    virtual bool GetEndZoneEntered(const StatsUID& suid, int round);
    virtual int GetLastCheckpointSequenceID(const StatsUID& suid, int round);

private:
    typedef boost::unordered_map<StatsUID, Scoreboard::PlayerStats*, boost::hash<StatsUID>, std::equal_to<StatsUID> >
        PlayerStatsMap;

    MinecraftServer* mServer;
    PlayerStatsMap mPlayerStats;
};

#endif
