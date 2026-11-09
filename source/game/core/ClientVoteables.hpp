#ifndef GAME_CORE_CLIENTVOTEABLES_HPP
#define GAME_CORE_CLIENTVOTEABLES_HPP

#include "game/core/Voteables.hpp"

class ClientVoteables : public Voteables
{
public:
    ClientVoteables();
    virtual ~ClientVoteables();
    virtual bool isServer();
};

#endif
