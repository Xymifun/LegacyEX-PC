#ifndef GAME_WORLD_BLOCK_GLAZEDTERRACOTTABLOCK_HPP
#define GAME_WORLD_BLOCK_GLAZEDTERRACOTTABLOCK_HPP

#include "game/world/block/HorizontalDirectionalBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class DyeColor;
class Level;
class Mirror;
class Rotation;
class LivingEntity;

class GlazedTerracottaBlock : public HorizontalDirectionalBlock
{
public:
    explicit GlazedTerracottaBlock(const DyeColor* dyeColor);

    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
};

#endif
