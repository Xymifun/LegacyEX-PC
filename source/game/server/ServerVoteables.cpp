#include "game/server/ServerVoteables.hpp"

ServerVoteables::ServerVoteables() : Voteables()
{
}

ServerVoteables::~ServerVoteables()
{
}

bool ServerVoteables::isServer()
{
    return true;
}
