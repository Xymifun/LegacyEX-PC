#ifndef GAME_WORLD_BLOCK_DISPENSERBLOCK_HPP
#define GAME_WORLD_BLOCK_DISPENSERBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/BehaviorRegistry.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/world/block/BaseEntityBlock.hpp"
#include "game/util/Random.hpp"

class Block;
class BlockPos;
class BlockSource;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class DirectionProperty;
class IconRegister;
class Level;
class LivingEntity;
class Mirror;
class Position;
class Rotation;
class TextureAtlasSprite;
class ItemInstance;
class DispenseItemBehavior;
template <typename T> class AbstractProperty;

class DispenserBlock : public BaseEntityBlock
{
public:
    static BehaviorRegistry DISPENSER_REGISTRY;
    static DirectionProperty* FACING;
    static AbstractProperty<bool>* TRIGGERED;

    DispenserBlock();
    virtual ~DispenserBlock() {}

    static Position* getDispensePosition(BlockSource* blockSource);
    static void blockStaticCtor();
    virtual int getTickDelay(Level* level);
    void recalcLockDir(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool TestUse();
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void dispenseFrom(Level* level, const BlockPos& pos);
    virtual DispenseItemBehavior* getDispenseMethod(not_null_ptr<ItemInstance> itemInstance);
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    Random mRandom;

protected:
    TextureAtlasSprite* mTopIcon;
    TextureAtlasSprite* mFrontIcon;
    TextureAtlasSprite* mFrontVerticalIcon;
};

#endif
