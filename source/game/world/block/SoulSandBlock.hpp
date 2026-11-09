#ifndef GAME_WORLD_BLOCK_SOULSANDBLOCK_HPP
#define GAME_WORLD_BLOCK_SOULSANDBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class Level;
class LevelSource;
class Random;

class SoulSandBlock : public Block
{
public:
    SoulSandBlock();

    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
};

#endif
