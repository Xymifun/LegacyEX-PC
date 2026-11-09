#ifndef GAME_WORLD_BLOCK_FURNACEBLOCK_HPP
#define GAME_WORLD_BLOCK_FURNACEBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/world/block/BaseEntityBlock.hpp"

class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class Mirror;
class Random;
class Rotation;
class TextureAtlasSprite;
class Item;
class ItemInstance;
class LivingEntity;
class Player;
template <class T> class not_null_ptr;

class DirectionProperty;

class FurnaceBlock : public BaseEntityBlock
{
public:
    static DirectionProperty* FACING;

    FurnaceBlock(bool lit);
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    static void blockStaticCtor();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    void recalcLockDir(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual bool TestUse();
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    static void setLit(bool lit, Level* level, const BlockPos& pos);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual const BlockState* getInventoryRenderState(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    static bool sKeepInventory;

    bool mLit;
    TextureAtlasSprite* mTopIcon;
    TextureAtlasSprite* mFrontIcon;
};

#endif
