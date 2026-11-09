#ifndef GAME_WORLD_BLOCK_HOPPERBLOCK_HPP
#define GAME_WORLD_BLOCK_HOPPERBLOCK_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BaseEntityBlock.hpp"

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Entity;
class IconRegister;
class ItemInstance;
class Level;
class LevelSource;
class Mirror;
class DirectionProperty;
class Player;
class Property;
class Rotation;
class TextureAtlasSprite;
class LivingEntity;

class HopperBlock : public BaseEntityBlock
{
public:
    static const std::wstring TEXTURE_OUTSIDE;
    static const std::wstring TEXTURE_INSIDE;
    static DirectionProperty* FACING;
    static Property* ENABLED;
    static const AABB* BASE_AABB;
    static const AABB* SOUTH_AABB;
    static const AABB* NORTH_AABB;
    static const AABB* WEST_AABB;
    static const AABB* EAST_AABB;

    HopperBlock();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    static void blockStaticCtor();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual bool isTopSolidBlocking(const BlockState* state);
    void checkPoweredState(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    static const Direction* getAttachedFace(int data);
    static bool isTurnedOn(int data);
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    static TextureAtlasSprite* getTexture(const std::wstring& name);
    virtual std::wstring getTileItemIconName();
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual bool canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType supportType);

private:
    TextureAtlasSprite* mSideTexture;
    TextureAtlasSprite* mTopTexture;
    TextureAtlasSprite* mBottomTexture;
};

#endif
