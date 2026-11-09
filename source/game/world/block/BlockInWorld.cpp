#include "game/world/block/BlockInWorld.hpp"

#include "game/world/Level.hpp"

BlockInWorld::BlockInWorld(Level* level, const BlockPos& pos, bool loadChunks)
    : mLevel(level),
      mPos(pos),
      mLoadChunks(loadChunks),
      mEntity()
{
    mState = NULL;
    mEntity = boost::shared_ptr<BlockEntity>();
    mEntityCached = false;
}

const BlockState* BlockInWorld::getState()
{
    if (mState == NULL && (mLoadChunks || mLevel->hasChunkAt(mPos)))
    {
        mState = mLevel->getBlockState(mPos);
    }
    return mState;
}

boost::shared_ptr<BlockEntity> BlockInWorld::getEntity()
{
    if (mEntity.get() == NULL && !mEntityCached)
    {
        mEntity = mLevel->getBlockEntity(mPos);
        mEntityCached = true;
    }
    return mEntity;
}

BlockPos BlockInWorld::getPos()
{
    return mPos;
}

Predicate<BlockInWorld*>* BlockInWorld::hasState(const Predicate<const BlockState*>* predicate)
{
    return new BlockInWorldPredicate(predicate);
}

BlockInWorld::BlockInWorldPredicate::BlockInWorldPredicate(const Predicate<const BlockState*>* predicate)
    : mPredicate(predicate)
{
}

bool BlockInWorld::BlockInWorldPredicate::apply(BlockInWorld* blockInWorld) const
{
    return blockInWorld != NULL && mPredicate->apply(blockInWorld->getState());
}

BlockInWorld::BlockInWorldPredicate::~BlockInWorldPredicate()
{
}
