#ifndef GAME_WORLD_BLOCK_CHESTBLOCK_HPP
#define GAME_WORLD_BLOCK_CHESTBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/world/block/BaseEntityBlock.hpp"

class BlockEntity;
class Container;
class DirectionProperty;
class LockableContainer;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class LivingEntity;
class Mirror;
class Player;
class Rotation;
class ItemInstance;

class ChestBlock : public BaseEntityBlock
{
public:
    enum Type
    {
        TYPE_SINGLE,
        TYPE_LEFT,
        TYPE_RIGHT
    };

    static DirectionProperty* FACING;

    explicit ChestBlock(Type type);
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    static void blockStaticCtor();
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool hasCustomBreakingProgress(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    const BlockState* recalcLockDir(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    const BlockState* reorient(Level* level, const BlockPos& pos, const BlockState* state);
    bool isFullChest(Level* level, const BlockPos& pos);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool TestUse();
    static bool isBlockedChestByBlock(Level* level, const BlockPos& pos);
    static bool isEntityOnChest(Level* level, const BlockPos& pos);
    static bool isChestBlockedAt(Level* level, const BlockPos& pos);
    boost::shared_ptr<LockableContainer> getContainer(Level* level, const BlockPos& pos, bool force);
    boost::shared_ptr<LockableContainer> getContainer(Level* level, const BlockPos& pos);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual bool isSignalSource(const BlockState* state);
    virtual int getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction);
    virtual int getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction);
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);

    Type mType;
};

#endif
