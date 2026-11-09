#ifndef GAME_WORLD_BLOCK_SNOWBLOCK_HPP
#define GAME_WORLD_BLOCK_SNOWBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class Level;
class Random;

class SnowBlock : public Block
{
public:
    SnowBlock();

    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCount(Random* random);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual bool shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool waterSpreadCausesSpawn();
};

#endif
