#ifndef GAME_WORLD_BLOCK_STANDINGSIGNBLOCK_HPP
#define GAME_WORLD_BLOCK_STANDINGSIGNBLOCK_HPP

#include "game/world/block/SignBlock.hpp"

template <typename T>
class AbstractProperty;

class BlockState;
class BlockStateDefinition;
class Block;
class BlockPos;
class Level;
class Mirror;
class Rotation;

class StandingSignBlock : public SignBlock
{
public:
    StandingSignBlock();

    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();

    static AbstractProperty<int>* ROTATION;
};

#endif
