#ifndef GAME_WORLD_BLOCK_FENCEGATEBLOCK_HPP
#define GAME_WORLD_BLOCK_FENCEGATEBLOCK_HPP

#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"

class DirectionProperty;
class AABB;

class FenceGateBlock : public HorizontalDirectionalBlock
{
public:
    FenceGateBlock(const std::wstring& name, PlanksBlock::Variant* variant);

    static AbstractProperty<bool>* OPEN;
    static AbstractProperty<bool>* POWERED;
    static AbstractProperty<bool>* IN_WALL;
    static AABB* Z_AXIS_AABB;
    static AABB* X_AXIS_AABB;
    static AABB* Z_AXIS_IN_WALL_AABB;
    static AABB* X_AXIS_IN_WALL_AABB;
    static AABB* Z_AXIS_COLLISION_AABB;
    static AABB* X_AXIS_COLLISION_AABB;

    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    static bool isFenceGate(Block* block);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    std::wstring mTextureName;
};

#endif
