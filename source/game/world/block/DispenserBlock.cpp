#include "game/world/block/DispenserBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Position.hpp"
#include "game/core/PositionImpl.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Container.hpp"
#include "game/item/Containers.hpp"
#include "game/item/DefaultDispenseItemBehavior.hpp"
#include "game/item/DispenseItemBehavior.hpp"
#include "game/item/NoOpDispenseItemBehavior.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockSourceImpl.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"

namespace
{
const unsigned int DISPENSER_INTERACT_TOOLTIP = IDS_TOOLTIPS_USE;
DispenseItemBehavior* NO_OP_BEHAVIOR = new NoOpDispenseItemBehavior();
}

AbstractProperty<bool>* DispenserBlock::TRIGGERED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"triggered"));
BehaviorRegistry DispenserBlock::DISPENSER_REGISTRY(new DefaultDispenseItemBehavior());
DirectionProperty* DispenserBlock::FACING = NULL;

DispenserBlock::DispenserBlock() : BaseEntityBlock(Material::STONE), mRandom()
{
    Block::DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(TRIGGERED, false));
}

boost::shared_ptr<BlockEntity> DispenserBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> entity = boost::shared_ptr<DispenserBlockEntity>(new DispenserBlockEntity());
    entity->setLevel(level);
    return entity;
}

Position* DispenserBlock::getDispensePosition(BlockSource* blockSource)
{
    const Direction* direction = blockSource->getBlockState()->getValue(FACING);
    double x = blockSource->getX() + 0.7 * direction->getStepX();
    double y = blockSource->getY() + 0.7 * direction->getStepY();
    double z = blockSource->getZ() + 0.7 * direction->getStepZ();
    return new PositionImpl(x, y, z);
}

void DispenserBlock::blockStaticCtor()
{
    FACING = DirectionalBlock::FACING;
}

int DispenserBlock::getTickDelay(Level*)
{
    return 4;
}

void DispenserBlock::recalcLockDir(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->mIsClientSide)
    {
        return;
    }
    const Direction* facing = state->getValue(FACING);
    bool northSolid = level->getBlockState(pos.north())->isSolid();
    bool southSolid = level->getBlockState(pos.south())->isSolid();
    const Direction* next = facing;
    if (facing == Direction::NORTH && northSolid && !southSolid)
        next = Direction::SOUTH;
    else if (facing == Direction::SOUTH && southSolid && !northSolid)
        next = Direction::NORTH;
    else
    {
        bool westSolid = level->getBlockState(pos.west())->isSolid();
        bool eastSolid = level->getBlockState(pos.east())->isSolid();
        if (facing == Direction::WEST && westSolid && !eastSolid)
            next = Direction::EAST;
        else if (facing == Direction::EAST && eastSolid && !westSolid)
            next = Direction::WEST;
    }
    level->setBlock(pos, state->setValue(FACING, next)->setValue(TRIGGERED, false), Block::UPDATE_CLIENTS, false);
}

void DispenserBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    recalcLockDir(level, pos, state);
}

bool DispenserBlock::TestUse()
{
    return true;
}

bool DispenserBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                         InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
    {
        return false;
    }
    if (!level->mIsClientSide)
    {
        boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
        if (eTYPE_DERIVED_FROM(BlockEntityType::DISPENSER, entity->GetType()))
        {
            player->openContainer(boost::static_pointer_cast<DispenserBlockEntity>(entity));
        }
    }
    return true;
}

void DispenserBlock::dispenseFrom(Level* level, const BlockPos& pos)
{
    BlockSourceImpl source(level, pos);
    boost::shared_ptr<DispenserBlockEntity> dispenser = boost::dynamic_pointer_cast<DispenserBlockEntity>(
        source.getEntity());
    if (!dispenser)
    {
        return;
    }
    int slot = dispenser->getRandomSlot();
    if (slot < 0)
    {
        level->levelEvent(LevelEvent::SOUND_DISPENSER_FAIL, pos, 0);
        return;
    }
    not_null_ptr<ItemInstance> item = dispenser->getItem(static_cast<unsigned int>(slot));
    DispenseItemBehavior* behavior = getDispenseMethod(item);
    if (behavior != NO_OP_BEHAVIOR)
    {
        dispenser->setItem(static_cast<unsigned int>(slot), behavior->dispense(&source, item));
    }
}

DispenseItemBehavior* DispenserBlock::getDispenseMethod(not_null_ptr<ItemInstance> itemInstance)
{
    return DISPENSER_REGISTRY.get(itemInstance->getItem());
}

void DispenserBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*,
                                     const BlockPos&)
{
    bool signal = level->hasNeighborSignal(pos) || level->hasNeighborSignal(pos.above());
    bool triggered = state->getValue(TRIGGERED);
    if (signal && !triggered)
    {
        level->addToTickNextTick(pos, this, getTickDelay(level));
        level->setBlock(pos, state->setValue(TRIGGERED, true), Block::UPDATE_INVISIBLE, false);
    }
    else if (!signal && triggered)
    {
        level->setBlock(pos, state->setValue(TRIGGERED, false), Block::UPDATE_INVISIBLE, false);
    }
}

void DispenserBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide)
    {
        dispenseFrom(level, pos);
    }
}

const BlockState* DispenserBlock::getStateForPlacement(Level*, const BlockPos& pos, const Direction*, float, float,
                                                       float, int, boost::shared_ptr<LivingEntity> placer)
{
    return defaultBlockState()
        ->setValue(FACING, Direction::getDirectionFromLivingEntity(pos, placer))
        ->setValue(TRIGGERED, false);
}

void DispenserBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                 boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
    level->setBlock(pos, state->setValue(FACING, Direction::getDirectionFromLivingEntity(pos, placer)),
                    Block::UPDATE_CLIENTS, false);
    if (itemInstance->hasCustomHoverName())
    {
        boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
        if (eTYPE_DERIVED_FROM(BlockEntityType::DISPENSER, entity->GetType()))
        {
            boost::shared_ptr<DispenserBlockEntity> dispenser = boost::static_pointer_cast<DispenserBlockEntity>(
                entity);
            dispenser->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                     itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
        }
    }
}

void DispenserBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    if (entity && eTYPE_DERIVED_FROM(BlockEntityType::DISPENSER, entity->GetType()))
    {
        Containers::dropContents(level, pos, boost::static_pointer_cast<DispenserBlockEntity>(entity));
        level->updateNeighbourForOutputSignal(pos, this);
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

bool DispenserBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int DispenserBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    return AbstractContainerMenu::getRedstoneSignalFromBlockEntity(level->getBlockEntity(pos));
}

EBlockRenderShape DispenserBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_MODEL;
}

const BlockState* DispenserBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(FACING, Direction::from3DDataValue(data & 7))
        ->setValue(TRIGGERED, (data & 8) > 0);
}

int DispenserBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->get3DDataValue();
    if (state->getValue(TRIGGERED))
        data |= 8;
    return data;
}

unsigned int DispenserBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
    {
        return DISPENSER_INTERACT_TOOLTIP;
    }
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* DispenserBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* DispenserBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* DispenserBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING, TRIGGERED};
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* DispenserBlock::getTexture(const Direction* direction, const BlockState* state)
{
    const Direction* facing = state->getValue(FACING);
    if (direction == facing)
    {
        return facing == Direction::UP || facing == Direction::DOWN ? mFrontVerticalIcon : mFrontIcon;
    }
    if (facing != Direction::UP && facing != Direction::DOWN && direction != Direction::UP
        && direction != Direction::DOWN)
    {
        return mIcon;
    }
    return mTopIcon;
}

void DispenserBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"furnace_side");
    mTopIcon = iconRegister->registerIcon(L"furnace_top");
    mFrontIcon = iconRegister->registerIcon(L"dispenser_front");
    mFrontVerticalIcon = iconRegister->registerIcon(L"dispenser_front_vertical");
}
