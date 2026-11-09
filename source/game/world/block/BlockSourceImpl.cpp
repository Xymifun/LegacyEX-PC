#include "game/world/block/BlockSourceImpl.hpp"

#include "game/world/Level.hpp"

BlockSourceImpl::BlockSourceImpl(Level* level, const BlockPos& pos)
{
    mLevel = level;
    mPos = pos;
}

Level* BlockSourceImpl::getWorld()
{
    return mLevel;
}

double BlockSourceImpl::getX()
{
    return static_cast<double>(mPos.x) + 0.5;
}

double BlockSourceImpl::getY()
{
    return static_cast<double>(mPos.y) + 0.5;
}

double BlockSourceImpl::getZ()
{
    return static_cast<double>(mPos.z) + 0.5;
}

BlockPos BlockSourceImpl::getPos()
{
    return mPos;
}

BlockState* BlockSourceImpl::getBlockState()
{
    return const_cast<BlockState*>(mLevel->getBlockState(mPos));
}

Material* BlockSourceImpl::getMaterial()
{
    return mLevel->getBlockState(mPos)->getMaterial();
}

boost::shared_ptr<BlockEntity> BlockSourceImpl::getEntity()
{
    return mLevel->getBlockEntity(mPos);
}
