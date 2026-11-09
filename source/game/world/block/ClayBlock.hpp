#ifndef GAME_WORLD_BLOCK_CLAYBLOCK_HPP
#define GAME_WORLD_BLOCK_CLAYBLOCK_HPP

#include "game/world/block/Block.hpp"
#include "game/world/block/BonemealableBlock.hpp"

class BlockPos;
class BlockState;
class BlockTooltipDataHolder;
class Level;
class Random;

class ClayBlock : public Block, public BonemealableBlock
{
public:
    ClayBlock();

    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCount(Random* random);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
};

#endif
