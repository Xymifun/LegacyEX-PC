#include "game/core/Voteables.hpp"

#include "game/core/Voteable.hpp"

Voteables::Voteables() : mVoteables(11)
{
    mVoteables = boost::unordered_map<int, Voteable*>(11);
}

Voteables::~Voteables()
{
    for (boost::unordered_map<int, Voteable*>::iterator it = mVoteables.begin(); it != mVoteables.end(); it++)
    {
        delete it->second;
    }
    mVoteables.clear();
}

Voteable* Voteables::getVoteable(unsigned int voteableId)
{
    if (mVoteables.find(static_cast<int>(voteableId)) == mVoteables.end())
    {
        return NULL;
    }

    return mVoteables.find(static_cast<int>(voteableId))->second;
}

void Voteables::init()
{
}

void Voteables::addVoteable(Voteable* voteable)
{
    const int voteableId = voteable->mVoteableId;
    if (mVoteables.find(voteableId) == mVoteables.end())
    {
        mVoteables.insert(std::make_pair(voteable->mVoteableId, voteable));
    }
}

void Voteables::syncAllVoteablesToPlayer(boost::shared_ptr<ServerPlayer> player)
{
    for (boost::unordered_map<int, Voteable*>::iterator it = mVoteables.begin(); it != mVoteables.end(); it++)
    {
        it->second->syncAllVotesToPlayer(player);
    }
}
