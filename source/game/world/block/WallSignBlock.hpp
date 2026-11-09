#ifndef GAME_WORLD_BLOCK_WALLSIGNBLOCK_HPP
#define GAME_WORLD_BLOCK_WALLSIGNBLOCK_HPP

#include "game/world/block/SignBlock.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class DirectionProperty;
class Level;
class LevelSource;
class Mirror;
class Rotation;

class WallSignBlock : public SignBlock
{
public:
    WallSignBlock();

    static DirectionProperty* FACING;
    static void blockStaticCtor();

    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
};

#endif
