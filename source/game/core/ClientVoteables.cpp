#include "game/core/ClientVoteables.hpp"

ClientVoteables::ClientVoteables() : Voteables()
{
    init();
}

ClientVoteables::~ClientVoteables()
{
}

bool ClientVoteables::isServer()
{
    return false;
}
