#ifndef GAME_SERVER_SERVERVOTEABLES_HPP
#define GAME_SERVER_SERVERVOTEABLES_HPP

#include "game/core/Voteables.hpp"

class ServerVoteables : public Voteables
{
public:
    ServerVoteables();
    virtual ~ServerVoteables();
    virtual bool isServer();
};

#endif
