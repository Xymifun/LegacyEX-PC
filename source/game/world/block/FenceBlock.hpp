#ifndef GAME_WORLD_BLOCK_FENCEBLOCK_HPP
#define GAME_WORLD_BLOCK_FENCEBLOCK_HPP

#include "game/world/block/Block.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Material;
class MaterialColor;
class Mirror;
class Rotation;

class FenceBlock : public Block
{
public:
    static AbstractProperty<bool>* NORTH;
    static AbstractProperty<bool>* EAST;
    static AbstractProperty<bool>* SOUTH;
    static AbstractProperty<bool>* WEST;
    static const AABB* BOUNDING_BOXES[16];
    static const AABB* PILLAR_AABB;
    static const AABB* SOUTH_AABB;
    static const AABB* WEST_AABB;
    static const AABB* NORTH_AABB;
    static const AABB* EAST_AABB;

    FenceBlock(const std::wstring& name, Material* material, const MaterialColor* materialColor);
    FenceBlock(const std::wstring& name, Material* material);

    void _init();
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    static unsigned int getAABBIndex(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    static bool isExceptionBlockForAttachingPlusPiston(Block* block);
    virtual bool connectsTo(LevelSource* levelSource, const BlockPos& pos, const Direction* direction);
    virtual bool canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType supportType);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual bool canContainLiquid(const BlockState* state) const;

private:
    std::wstring mTextureName;
};

#endif
