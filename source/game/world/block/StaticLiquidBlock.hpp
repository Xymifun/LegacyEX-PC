#ifndef GAME_WORLD_BLOCK_STATICLIQUIDBLOCK_HPP
#define GAME_WORLD_BLOCK_STATICLIQUIDBLOCK_HPP

#include "game/world/block/LiquidBlock.hpp"

class Block;
class BlockPos;
class BlockState;
class Level;
class Material;
class Random;

class StaticLiquidBlock : public LiquidBlock
{
public:
    StaticLiquidBlock(Material* material);

    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void setDynamic(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
};

#endif
