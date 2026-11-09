#ifndef GAME_WORLD_BLOCK_LADDERBLOCK_HPP
#define GAME_WORLD_BLOCK_LADDERBLOCK_HPP

#include "game/world/block/Block.hpp"

class DirectionProperty;

class LadderBlock : public Block
{
public:
    static DirectionProperty* FACING;

    LadderBlock();
    static void blockStaticCtor();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    bool canAttachTo(Level* level, const BlockPos& pos, const Direction* direction);
    virtual bool mayPlace(Level* level, const BlockPos& pos, const Direction* direction);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual bool waterSpreadCausesSpawn();
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
};

#endif
