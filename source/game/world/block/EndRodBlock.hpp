#ifndef GAME_WORLD_BLOCK_ENDRODBLOCK_HPP
#define GAME_WORLD_BLOCK_ENDRODBLOCK_HPP

#include "game/world/block/DirectionalBlock.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class Level;
class LevelSource;
class Mirror;
class Random;
class Rotation;
class LivingEntity;

class EndRodBlock : public DirectionalBlock
{
public:
    EndRodBlock();
    virtual void DerivedInit();
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int data,
                                                   boost::shared_ptr<LivingEntity> entity);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    const AABB* getShapeRod(const BlockState* state);
    const AABB* getShapeBase(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
};

#endif
