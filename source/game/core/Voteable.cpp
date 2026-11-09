#include "game/core/Voteable.hpp"

#include <map>
#include <vector>

#include "game/core/Minecraft.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/VotePacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

Voteable::Voteable(int voteableId, bool isServer, bool oneVotePerPlayer)
    : mVotes()
{
    mVoteableId = voteableId;
    mIsServer = isServer;
    mOneVotePerPlayer = oneVotePerPlayer;
}

Voteable::~Voteable() {}

std::map<int, int> Voteable::getVotesFromPlayer(int playerId)
{
    std::map<int, int> votes;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        const char vote = it->second;
        const int& voteableId = it->first.second;
        if (it->first.first == playerId) votes[voteableId] = static_cast<int>(vote);
    }
    return votes;
}

std::map<int, int> Voteable::getVotesToVoteable(int voteableId)
{
    std::map<int, int> votes;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        const char vote = it->second;
        const int& playerId = it->first.first;
        if (it->first.second == voteableId) votes[playerId] = static_cast<int>(vote);
    }
    return votes;
}

boost::unordered_map<int, int> Voteable::getTotalledVotes()
{
    boost::unordered_map<int, int> totals;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        const char vote = it->second;
        const int voteableId = it->first.second;
        if (vote >= 0) totals[voteableId] += static_cast<int>(vote);
    }
    return totals;
}

int Voteable::getWinningVote(int& voteCount)
{
    boost::unordered_map<int, int> totals = getTotalledVotes();
    Random random;
    int winningVote = -1;
    int winningCount = 0;
    for (boost::unordered_map<int, int>::iterator it = totals.begin(); it != totals.end(); ++it) {
        const int count = it->second;
        if (count > 0 && (count > winningCount || (count == winningCount && random.nextBoolean()))) {
            winningVote = it->first;
            winningCount = count;
        }
    }
    voteCount = winningCount;
    return winningVote;
}

bool Voteable::getOnlyVote(int playerId, int* voteableId) const
{
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        if (it->first.first == playerId) {
            *voteableId = it->first.second;
            return true;
        }
    }
    return false;
}

std::pair<int, int> Voteable::getKey(int playerId, int voteableId)
{
    return std::make_pair(playerId, voteableId);
}

int Voteable::getVote(int playerId, int voteableId)
{
    const boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it = mVotes.find(getKey(playerId, voteableId));
    if (it != mVotes.end())
        return static_cast<int>(it->second);
    return -1;
}

int Voteable::getVote(boost::shared_ptr<MultiplayerLocalPlayer> player, int voteableId)
{
    if (player)
    {
        int playerId = player->mPlayerIndex;
        return getVote(playerId, voteableId);
    }
    return -1;
}

void Voteable::setVote(int playerId, int voteableId, char vote)
{
    if (vote == -1) {
        std::pair<int, int> key = getKey(playerId, voteableId);
        boost::unordered_map<std::pair<int, int>, char, KeyHasher>::iterator found = mVotes.find(key);
        if (found != mVotes.end()) mVotes.erase(found);
        return;
    }
    if (mOneVotePerPlayer) {
        int previousVoteable = -1;
        if (getOnlyVote(playerId, &previousVoteable)) {
            std::pair<int, int> previousKey(playerId, previousVoteable);
            boost::unordered_map<std::pair<int, int>, char, KeyHasher>::iterator previous = mVotes.find(previousKey);
            mVotes.erase(previous);
        }
    }
    std::pair<int, int> key = getKey(playerId, voteableId);
    boost::unordered_map<std::pair<int, int>, char, KeyHasher>::iterator existing = mVotes.find(key);
    char value = vote;
    if (existing != mVotes.end()) {
        existing->second = value;
        return;
    }
    std::pair<std::pair<int, int>, char> entry(getKey(playerId, voteableId), value);
    mVotes.insert(entry);
}

void Voteable::setVoteAndSync(int playerId, int voteableId, char vote)
{
    const char previousVote = getVote(playerId, voteableId);
    if (previousVote == vote) return;
    setVote(playerId, voteableId, vote);
    if (mIsServer) {
        PlayerList* players = gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers;
        players->broadcastAll(boost::shared_ptr<VotePacket>(new VotePacket(mVoteableId, vote, playerId, voteableId)));
    } else {
        Minecraft::GetInstance()->mPlayer->mPacketListener->send(boost::shared_ptr<VotePacket>(new VotePacket(mVoteableId, vote, playerId, voteableId)));
    }
}

void Voteable::setIncludedVotesAndSync(int playerId, char vote)
{
    std::vector<std::pair<int, int> > keys;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        if (it->first.first == playerId) keys.push_back(it->first);
    }
    for (std::vector<std::pair<int, int> >::iterator it = keys.begin(); it != keys.end(); ++it) setVoteAndSync(it->first, it->second, vote);
}

void Voteable::setIncludedVotesAndSync(int playerId, int voteableId, char vote)
{
    std::vector<std::pair<int, int> > keys;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        if (it->first.first == playerId || it->first.second == voteableId) keys.push_back(it->first);
    }
    for (std::vector<std::pair<int, int> >::iterator it = keys.begin(); it != keys.end(); ++it) setVoteAndSync(it->first, it->second, vote);
}

void Voteable::syncAllVotesToPlayer(boost::shared_ptr<ServerPlayer> player)
{
    if (!mIsServer) return;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        boost::shared_ptr<ServerGamePacketListenerImpl>& connection = player->mConnection;
        connection->send(boost::shared_ptr<VotePacket>(new VotePacket(mVoteableId, it->second, it->first.first, it->first.second)));
    }
}

void Voteable::clearVotes() { mVotes.clear(); }
