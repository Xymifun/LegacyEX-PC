#ifndef GAME_WORLD_BLOCK_SHULKERBOXBLOCK_HPP
#define GAME_WORLD_BLOCK_SHULKERBOXBLOCK_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/world/block/BaseEntityBlock.hpp"
#include "game/item/not_null_ptr.hpp"

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class DyeColor;
class HtmlString;
class Item;
class Level;
class LevelSource;
class Mirror;
class Player;
class Rotation;
class LivingEntity;

class ShulkerBoxBlock : public BaseEntityBlock
{
public:
    explicit ShulkerBoxBlock(const DyeColor* color);
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isViewBlocking(const BlockState* state);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool hasCustomBreakingProgress(const BlockState* state);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    bool isEntityOnChest(Level* level, const AABB* box);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual bool playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    const DyeColor* getDyeColor();
    static const DyeColor* getColorFromBlock(Block* block);
    static Block* getBlockByColor(const DyeColor* color);
    static not_null_ptr<ItemInstance> getColoredItemInstance(const DyeColor* color);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    static const DyeColor* getColorFromItem(Item* item);

private:
    const DyeColor* mColor;
};

#endif
