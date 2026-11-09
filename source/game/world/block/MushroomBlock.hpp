#ifndef GAME_WORLD_BLOCK_MUSHROOMBLOCK_HPP
#define GAME_WORLD_BLOCK_MUSHROOMBLOCK_HPP

#include "game/core/Bush.hpp"
#include "game/world/block/BonemealableBlock.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockTooltipDataHolder;
class Level;
class LevelSource;
class Random;

class MushroomBlock : public Bush, public BonemealableBlock
{
public:
    MushroomBlock();

    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);

    bool growMushroom(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual bool canContainLiquid(const BlockState* state) const;
};

#endif
