#ifndef GAME_CORE_VOTEABLES_HPP
#define GAME_CORE_VOTEABLES_HPP

#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

class ServerPlayer;
class Voteable;

class Voteables
{
public:
    enum EVoteableId
    {
        eVoteable_Map,
        eVoteable_Kick,
        eVoteable_BuildOffScoring,
        eVoteable_BuildOffTheme
    };

    Voteables();
    virtual ~Voteables();
    virtual bool isServer() = 0;
    Voteable* getVoteable(unsigned int voteableId);
    void init();
    void addVoteable(Voteable* voteable);
    void syncAllVoteablesToPlayer(boost::shared_ptr<ServerPlayer> player);
    void removeVoteable(int voteableId);

private:
    boost::unordered_map<int, Voteable*> mVoteables;
};

#endif
