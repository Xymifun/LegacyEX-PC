#ifndef GAME_CORE_VOTEABLE_HPP
#define GAME_CORE_VOTEABLE_HPP

#include <stdint.h>
#include <map>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

class MultiplayerLocalPlayer;
class ServerPlayer;

class Voteable
{
public:
    struct KeyHasher
    {
        std::size_t operator()(const std::pair<int, int>& key) const
        {
            return static_cast<std::size_t>(key.first ^ key.second);
        }
    };

    static const char NO_VOTE = -1;

    Voteable(int voteableId, bool isServer, bool oneVotePerPlayer);
    ~Voteable();
    std::map<int, int> getVotesFromPlayer(int playerId);
    std::map<int, int> getVotesToVoteable(int voteableId);
    boost::unordered_map<int, int> getTotalledVotes();
    int getWinningVote(int& voteCount);
    bool getOnlyVote(int playerId, int* voteableId) const;
    static std::pair<int, int> getKey(int playerId, int voteableId);
    int getVote(int playerId, int voteableId);
    int getVote(boost::shared_ptr<MultiplayerLocalPlayer> player, int voteableId);
    void setVote(int playerId, int voteableId, char vote);
    void setVoteAndSync(int playerId, int voteableId, char vote);
    void setIncludedVotesAndSync(int playerId, char vote);
    void setIncludedVotesAndSync(int playerId, int voteableId, char vote);
    void syncAllVotesToPlayer(boost::shared_ptr<ServerPlayer> player);
    void clearVotes();
    void setVote(int, char);
    void setVoteAndSync(int, char);

private:
    friend class Voteables;
    int mVoteableId;
    bool mIsServer;
    bool mOneVotePerPlayer;
    boost::unordered_map<std::pair<int, int>, char, KeyHasher> mVotes;
};

#endif
