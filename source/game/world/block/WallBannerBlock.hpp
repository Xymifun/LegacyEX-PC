#ifndef GAME_WORLD_BLOCK_WALLBANNERBLOCK_HPP
#define GAME_WORLD_BLOCK_WALLBANNERBLOCK_HPP

#include "game/world/block/BannerBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Level;
class LevelSource;
class Mirror;
class Rotation;

class WallBannerBlock : public BannerBlock
{
public:
    WallBannerBlock();

    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
};

#endif
