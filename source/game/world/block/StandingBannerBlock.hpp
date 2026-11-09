#ifndef GAME_WORLD_BLOCK_STANDINGBANNERBLOCK_HPP
#define GAME_WORLD_BLOCK_STANDINGBANNERBLOCK_HPP

#include "game/world/block/BannerBlock.hpp"

class Block;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Level;
class LevelSource;
class Mirror;
class Rotation;

class StandingBannerBlock : public BannerBlock
{
public:
    StandingBannerBlock();

    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();

    static AbstractProperty<int>* ROTATION;
};

#endif
