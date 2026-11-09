#include "game/server/ServerScoreboard.hpp"

#include "game/core/DamageEvent.hpp"
#include "game/core/GameStats.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/WeaponDamageStatTracker.hpp"
#include "game/network/ClientboundSetPlayerTeamPacket.hpp"
#include "game/network/ScoreboardPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Math.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/PlayerStats.hpp"

#include <vector>

ServerScoreboard::ServerScoreboard(MinecraftServer* server) : Scoreboard(), mPlayerStats()
{
    mServer = server;
    mPlayerStats = PlayerStatsMap();
}

Scoreboard::PlayerStats* ServerScoreboard::GetPlayerStats(const StatsUID& suid, int round)
{
    Scoreboard::PlayerStats* stats = new Scoreboard::PlayerStats();
    GameStats& gameStats = mServer->mMasterGameMode->mGameStats;
    ::PlayerStats* source = gameStats.GetPlayerStats(suid);
    if (round == -1)
        round = mServer->mMasterGameMode->mGameStats.mCurrentRound;
    if (source == NULL)
        return stats;

    stats->mName = source->mName;
    stats->StatsUID::operator=(source->mStatsUID);
    stats->mMedal = source->mAwardedMedal;
    stats->mFavouriteWeapon = source->mWeaponDamage[0]->getValue();
    stats->mWeaknessWeapon = source->mWeaponDamage[1]->getValue();

    ::PlayerStats::RoundStats* sourceRound = source->getRoundStats(round);
    Scoreboard::PlayerStats::_RoundStats& roundStats = stats->mRoundStats[round];
    roundStats.mActive = sourceRound->mWon;
    roundStats.mFinished = sourceRound->mParticipated;
    roundStats.mPlacement = sourceRound->mLifeSpan;
    roundStats.mScore = sourceRound->mKills;

    int lives = sourceRound->getNumberLifeStats();
    for (int life = 0; life < lives; ++life)
    {
        if (life == (int)roundStats.mPerLife.size())
        {
            Scoreboard::PlayerStats::_PerLife perLife;
            roundStats.mPerLife.push_back(perLife);
        }
        roundStats.mPerLife[life].mKillerName = sourceRound->getLifeStats(life)->mKillerName;
    }

    roundStats.mKillerWeapon = sourceRound->mFatalWeapon;
    roundStats.mKills = Math::round(source->getDamageDealt(round));
    roundStats.mDeaths = Math::round(source->getDamageTaken(round));
    roundStats.mLives = sourceRound->mFatalDamage != NULL ? sourceRound->mFatalDamage->mDamageSource
                                                          : Scoreboard::DAMAGE_SOURCE_UNKNOWN;
    roundStats.mBlocksPlaced = sourceRound->mJumpCount;
    roundStats.mMobKills = sourceRound->mTargetsReached;
    roundStats.mBestTime = sourceRound->mTopSpeed;
    roundStats.mItemsCollected = sourceRound->mPoints;
    roundStats.mDistanceTravelled = sourceRound->mSequenceProgress;
    roundStats.mBlocksMined = sourceRound->mUnbankedPoints;
    roundStats.mTimePlayed = sourceRound->mLastBankedPoints;
    roundStats.mTotalTime = sourceRound->mLastPointsAdded;
    roundStats.mLastCheckpointId = sourceRound->mLastCheckpointID;
    roundStats.mLastCheckpointSequenceId = sourceRound->mLastCheckpointSequenceID;
    roundStats.mLastBankedCheckpointId = sourceRound->mLastBankedCheckpointID;
    roundStats.mLapsCompleted = sourceRound->mLapsCompleted;
    roundStats.mCheckpointsPassed = sourceRound->mDeaths;
    roundStats.mWinner = sourceRound->mReachedEnd;
    roundStats.mDamageDealt = sourceRound->mBlocksPlaced.mCount;
    roundStats.mDamageTaken = sourceRound->mBlocksDestroyed.mCount;
    roundStats.mTimeAlive = sourceRound->mBlocksPresent.mCount;
    roundStats.mLongestShot = sourceRound->mNearestDistance;
    roundStats.mFallDistance = sourceRound->mProgress;
    roundStats.mArrowsFired = source->getTotalDistanceMoved(round);

    for (boost::unordered_map<int, int>::iterator it = sourceRound->mCheckpointTimes.begin();
         it != sourceRound->mCheckpointTimes.end(); ++it)
    {
        roundStats.mCheckpointTimes[it->first] = it->second;
    }

    for (int i = 0; i < ::PlayerStats::RoundStats::BUILD_SCORE_COUNT; ++i)
    {
        roundStats.mScoresGiven[i] = sourceRound->mScoresGiven[i];
        roundStats.mScoresReceived[i] = sourceRound->mScoresReceived[i];
    }
    roundStats.mLastKillerName = gameStats.GetWinningThemeChoice();
    roundStats.mLastVictimName = sourceRound->mThemeVote;
    return stats;
}

void ServerScoreboard::OnPlayerStatsChanged(const StatsUID& suid, int life)
{
    Scoreboard::PlayerStats* stats = GetPlayerStats(suid, -1);
    Scoreboard::PlayerStats* previous = mPlayerStats[suid];
    GameStats& gameStats = mServer->mMasterGameMode->mGameStats;
    int round = gameStats.mCurrentRound;
    if (previous != NULL)
    {
        ScoreboardPacket::Data* data = new ScoreboardPacket::UpdatePlayerStatsData(new Scoreboard::PlayerStats(*stats),
                                                                                   previous, round, life);
        boost::shared_ptr<Packet> packet(new ScoreboardPacket(ScoreboardPacket::UPDATE_PLAYER_STATS, data));
        gMinecraftServer->getPlayers()->broadcastAll(packet);
    }
    else
    {
        ScoreboardPacket::Data* data = new ScoreboardPacket::AddPlayerStatsData(new Scoreboard::PlayerStats(*stats),
                                                                                round, life);
        boost::shared_ptr<Packet> packet(new ScoreboardPacket(ScoreboardPacket::ADD_PLAYER_STATS, data));
        gMinecraftServer->getPlayers()->broadcastAll(packet);
    }
    mPlayerStats[suid] = stats;
    delete previous;
}

void ServerScoreboard::OnPlayerStatsRemoved(const StatsUID& suid, const std::wstring& name)
{
    boost::shared_ptr<Packet> packet(
        new ScoreboardPacket(ScoreboardPacket::REMOVE_PLAYER_STATS, new ScoreboardPacket::RemovePlayerStatsData(suid)));
    gMinecraftServer->getPlayers()->broadcastAll(packet);

    Scoreboard::PlayerStats*& stats = mPlayerStats[suid];
    if (stats != NULL)
    {
        delete stats;
    }
    mPlayerStats.erase(suid);
}

void ServerScoreboard::SyncPlayerScores(std::vector<boost::shared_ptr<ServerPlayer> >* players,
                                        boost::shared_ptr<ServerPlayer> player)
{
    unsigned int currentRound = mServer->mMasterGameMode->mGameStats.mCurrentRound;
    for (unsigned int round = 0; round <= currentRound; ++round)
    {
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin(); it != players->end(); ++it)
        {
            if (mServer->mMasterGameMode->mGameStats.GetPlayerStats((*it)->getSUID()) == NULL)
                continue;

            Scoreboard::PlayerStats* stats = GetPlayerStats((*it)->getSUID(), round);
            int lives = (int)stats->mRoundStats[round].mPerLife.size();
            for (int life = 0; life <= lives; ++life)
            {
                boost::shared_ptr<Packet> packet(new ScoreboardPacket(
                    ScoreboardPacket::ADD_PLAYER_STATS,
                    new ScoreboardPacket::AddPlayerStatsData(new Scoreboard::PlayerStats(*stats), round, life)));
                player->mConnection->send(packet);
            }
            delete stats;
        }
    }
}

void ServerScoreboard::ResetPlayerScores()
{
    boost::shared_ptr<Packet> packet(
        new ScoreboardPacket(ScoreboardPacket::CLEAR_PLAYER_STATS, new ScoreboardPacket::Data()));
    gMinecraftServer->getPlayers()->broadcastAll(packet);

    boost::unordered_map<StatsUID, Scoreboard::PlayerStats*, boost::hash<StatsUID>, std::equal_to<StatsUID> >::iterator
        it = mPlayerStats.begin();
    while (it != mPlayerStats.end())
    {
        delete it->second;
        ++it;
    }
    mPlayerStats.clear();
}

void ServerScoreboard::removePlayerFromTeam(PlayerUID uid, PlayerTeam* pTeam)
{
    Scoreboard::removePlayerFromTeam(uid, pTeam);

    std::vector<PlayerUID> players;
    players.push_back(uid);
    mServer->getPlayers()->broadcastAll(
        boost::shared_ptr<Packet>(new ClientboundSetPlayerTeamPacket(pTeam, &players, 4)));
}

void ServerScoreboard::addPlayerToTeam(PlayerUID uid, PlayerTeam* pTeam)
{
    PlayerTeam* currentTeam = getPlayersTeam(uid);
    if (currentTeam != NULL)
    {
        Scoreboard::removePlayerFromTeam(uid, currentTeam);
    }
    Scoreboard::addPlayerToTeam(uid, pTeam);

    std::vector<PlayerUID> players;
    players.push_back(uid);
    mServer->getPlayers()->broadcastAll(
        boost::shared_ptr<Packet>(new ClientboundSetPlayerTeamPacket(pTeam, &players, 3)));
}

void ServerScoreboard::onTeamAdded(PlayerTeam* pTeam)
{
    Scoreboard::onTeamAdded(pTeam);
    mServer->getPlayers()->broadcastAll(boost::shared_ptr<Packet>(new ClientboundSetPlayerTeamPacket(pTeam, 0)));
}

void ServerScoreboard::onTeamChanged(PlayerTeam* pTeam)
{
    Scoreboard::onTeamChanged(pTeam);
}

void ServerScoreboard::onTeamRemoved(PlayerTeam* pTeam)
{
    Scoreboard::onTeamRemoved(pTeam);
    mServer->getPlayers()->broadcastAll(boost::shared_ptr<Packet>(new ClientboundSetPlayerTeamPacket(pTeam, 1)));
}

void ServerScoreboard::syncTeamChanges(PlayerTeam* pTeam)
{
    mServer->getPlayers()->broadcastAll(boost::shared_ptr<Packet>(new ClientboundSetPlayerTeamPacket(pTeam, 2)));
}

int ServerScoreboard::GetLastCheckpointID(const StatsUID& suid, int round)
{
    return gMinecraftServer->mMasterGameMode->mGameStats.GetLastCheckpointID(suid);
}

int ServerScoreboard::GetLapsCompleted(const StatsUID& suid, int round)
{
    return gMinecraftServer->mMasterGameMode->mGameStats.GetLapsCompleted(suid);
}

bool ServerScoreboard::GetEndZoneEntered(const StatsUID& suid, int round)
{
    return gMinecraftServer->mMasterGameMode->mGameStats.GetEndZoneEntered(suid);
}

int ServerScoreboard::GetLastCheckpointSequenceID(const StatsUID& suid, int round)
{
    return gMinecraftServer->mMasterGameMode->mGameStats.GetLastCheckpointSequenceID(suid);
}
