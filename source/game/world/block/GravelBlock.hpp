#ifndef GAME_WORLD_BLOCK_GRAVELBLOCK_HPP
#define GAME_WORLD_BLOCK_GRAVELBLOCK_HPP

#include "game/world/block/BonemealableBlock.hpp"
#include "game/world/block/FallingBlock.hpp"

class BlockPos;
class BlockState;
class BlockTooltipDataHolder;
class Level;
class LevelSource;
class Random;

class GravelBlock : public FallingBlock, public BonemealableBlock
{
public:
    GravelBlock();

    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual unsigned int getDustColor(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
};

#endif
