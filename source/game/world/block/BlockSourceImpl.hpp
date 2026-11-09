#ifndef GAME_WORLD_BLOCK_BLOCKSOURCEIMPL_HPP
#define GAME_WORLD_BLOCK_BLOCKSOURCEIMPL_HPP

#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockPos.hpp"

class Level;

class BlockSourceImpl : public BlockSource
{
public:
    BlockSourceImpl(Level* level, const BlockPos& pos);
    virtual ~BlockSourceImpl() {}

    virtual double getX();
    virtual double getY();
    virtual double getZ();
    virtual BlockPos getPos();
    virtual BlockState* getBlockState();
    virtual Material* getMaterial();
    virtual boost::shared_ptr<BlockEntity> getEntity();
    virtual Level* getWorld();

private:
    Level* mLevel;
    BlockPos mPos;
};

#endif
