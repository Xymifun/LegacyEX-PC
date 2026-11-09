#include "game/world/block/ChestBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Direction.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/CompoundContainer.hpp"
#include "game/item/Container.hpp"
#include "game/item/Containers.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/LockableContainer.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/MemSect.hpp"

DirectionProperty* ChestBlock::FACING = NULL;

static const AABB* NORTH_CHEST_SHAPE = AABB::newPermanent(1.0 / 16.0, 0.0, 0.0, 15.0 / 16.0, 14.0 / 16.0, 15.0 / 16.0);
static const AABB* SOUTH_CHEST_SHAPE = AABB::newPermanent(1.0 / 16.0, 0.0, 1.0 / 16.0, 15.0 / 16.0, 14.0 / 16.0, 1.0);
static const AABB* WEST_CHEST_SHAPE = AABB::newPermanent(0.0, 0.0, 1.0 / 16.0, 15.0 / 16.0, 14.0 / 16.0, 15.0 / 16.0);
static const AABB* EAST_CHEST_SHAPE = AABB::newPermanent(1.0 / 16.0, 0.0, 1.0 / 16.0, 1.0, 14.0 / 16.0, 15.0 / 16.0);
static const AABB* SINGLE_CHEST_SHAPE = AABB::newPermanent(1.0 / 16.0, 0.0, 1.0 / 16.0, 15.0 / 16.0, 14.0 / 16.0,
                                                           15.0 / 16.0);

ChestBlock::ChestBlock(Type type) : BaseEntityBlock(Material::WOOD)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(FACING, Direction::NORTH));
    mType = type;
}

boost::shared_ptr<BlockEntity> ChestBlock::newBlockEntity(Level* level, int)
{
    MemSect(0x32);
    boost::shared_ptr<ChestBlockEntity> blockEntity(new ChestBlockEntity());
    blockEntity->setLevel(level);
    MemSect(0);
    return blockEntity;
}

void ChestBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

bool ChestBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool ChestBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool ChestBlock::hasCustomBreakingProgress(const BlockState*)
{
    return true;
}

const AABB* ChestBlock::getShape(const BlockState*, LevelSource* levelSource, const BlockPos& pos)
{
    if (levelSource->getBlockState(pos.north())->getBlock() == this)
    {
        return NORTH_CHEST_SHAPE;
    }
    else if (levelSource->getBlockState(pos.south())->getBlock() == this)
    {
        return SOUTH_CHEST_SHAPE;
    }
    else if (levelSource->getBlockState(pos.west())->getBlock() == this)
    {
        return WEST_CHEST_SHAPE;
    }
    else if (levelSource->getBlockState(pos.east())->getBlock() == this)
    {
        return EAST_CHEST_SHAPE;
    }
    return SINGLE_CHEST_SHAPE;
}

void ChestBlock::onPlace(Level*, const BlockPos&, const BlockState*)
{
}

const BlockState* ChestBlock::getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float, float, int,
                                                   boost::shared_ptr<LivingEntity> placer)
{
    return defaultBlockState()->setValue(FACING, placer->getDirection());
}

const BlockState* ChestBlock::recalcLockDir(Level* level, const BlockPos& pos, const BlockState* state)
{
    const BlockState* north = level->getBlockState(pos.north());
    const BlockState* south = level->getBlockState(pos.south());
    const BlockState* west = level->getBlockState(pos.west());
    const BlockState* east = level->getBlockState(pos.east());
    const Direction* facing = state->getValue(FACING);

    if (north->getBlock() == this || south->getBlock() == this)
    {
        BlockPos other = north->getBlock() == this ? pos.north() : pos.south();
        const BlockState* otherWest = level->getBlockState(other.west());
        const BlockState* otherEast = level->getBlockState(other.east());
        facing = Direction::EAST;
        const Direction* otherFacing = north->getBlock() == this ? north->getValue(FACING) : south->getValue(FACING);
        if (otherFacing == Direction::WEST)
        {
            facing = Direction::WEST;
        }
        if ((west->isSolid() || otherWest->isSolid()) && !east->isSolid() && !otherEast->isSolid())
        {
            facing = Direction::EAST;
        }
        if ((east->isSolid() || otherEast->isSolid()) && !west->isSolid() && !otherWest->isSolid())
        {
            facing = Direction::WEST;
        }
    }
    else
    {
        bool northSolid = north->isSolid();
        bool southSolid = south->isSolid();
        if (west->getBlock() == this || east->getBlock() == this)
        {
            BlockPos other = west->getBlock() == this ? pos.west() : pos.east();
            const BlockState* otherNorth = level->getBlockState(other.north());
            const BlockState* otherSouth = level->getBlockState(other.south());
            facing = Direction::SOUTH;
            const Direction* otherFacing = west->getBlock() == this ? west->getValue(FACING) : east->getValue(FACING);
            if (otherFacing == Direction::NORTH)
            {
                facing = Direction::NORTH;
            }
            if ((northSolid || otherNorth->isSolid()) && !southSolid && !otherSouth->isSolid())
            {
                facing = Direction::SOUTH;
            }
            if ((southSolid || otherSouth->isSolid()) && !northSolid && !otherNorth->isSolid())
            {
                facing = Direction::NORTH;
            }
        }
    }

    state = state->setValue(FACING, facing);
    level->setBlock(pos, state, Block::UPDATE_ALL, false);
    if (north->getBlock() == this)
    {
        level->setBlock(pos.north(), north->setValue(FACING, facing), Block::UPDATE_ALL, false);
    }
    else if (south->getBlock() == this)
    {
        level->setBlock(pos.south(), south->setValue(FACING, facing), Block::UPDATE_ALL, false);
    }
    else if (east->getBlock() == this)
    {
        level->setBlock(pos.east(), east->setValue(FACING, facing), Block::UPDATE_ALL, false);
    }
    else if (west->getBlock() == this)
    {
        level->setBlock(pos.west(), west->setValue(FACING, facing), Block::UPDATE_ALL, false);
    }
    return state;
}

void ChestBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
    const Direction* facing =
        Direction::from2DDataValue(Mth::absFloor((double)(placer->mYRot * 4.0f / 360.0f) + 0.5) & 3)->getOpposite();
    state = state->setValue(FACING, facing);

    BlockPos north = pos.north();
    BlockPos south = pos.south();
    BlockPos west = pos.west();
    BlockPos east = pos.east();
    bool northIsChest = this == level->getBlockState(north)->getBlock();
    bool southIsChest = this == level->getBlockState(south)->getBlock();
    bool westIsChest = this == level->getBlockState(west)->getBlock();
    bool eastIsChest = this == level->getBlockState(east)->getBlock();

    if (northIsChest || southIsChest || westIsChest || eastIsChest)
    {
        if (facing->getAxis() == Direction::Axis::X && (northIsChest || southIsChest))
        {
            if (northIsChest)
            {
                level->setBlock(north, state, Block::UPDATE_ALL, false);
            }
            else
            {
                level->setBlock(south, state, Block::UPDATE_ALL, false);
            }
            level->setBlock(pos, state, Block::UPDATE_ALL, false);
        }
        else if (facing->getAxis() == Direction::Axis::Z && (westIsChest || eastIsChest))
        {
            if (westIsChest)
            {
                level->setBlock(west, state, Block::UPDATE_ALL, false);
            }
            else
            {
                level->setBlock(east, state, Block::UPDATE_ALL, false);
            }
            level->setBlock(pos, state, Block::UPDATE_ALL, false);
        }
        else
        {
            recalcLockDir(level, pos, state);
        }
    }
    else
    {
        level->setBlock(pos, state, Block::UPDATE_ALL, false);
    }

    if (itemInstance->hasCustomHoverName())
    {
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        bool isChest = eTYPE_DERIVED_FROM(BlockEntityType::CHEST, blockEntity->GetType());
        if (isChest)
        {
            boost::shared_ptr<ChestBlockEntity> chest = boost::dynamic_pointer_cast<ChestBlockEntity>(blockEntity);
            chest->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                 itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
        }
    }
}

const BlockState* ChestBlock::reorient(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* open = NULL;
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const Direction* direction = directions.data[i];
        const BlockState* neighbour = level->getBlockState(pos.relative(direction));
        if (neighbour->getBlock() == this)
        {
            return state;
        }
        if (neighbour->isSolid())
        {
            if (open != NULL)
            {
                open = NULL;
                break;
            }
            open = direction;
        }
    }

    if (open != NULL)
    {
        return state->setValue(FACING, open->getOpposite());
    }

    const Direction* facing = state->getValue(FACING);
    if (level->getBlockState(pos.relative(facing))->isSolid())
    {
        facing = facing->getOpposite();
    }
    if (level->getBlockState(pos.relative(facing))->isSolid())
    {
        facing = facing->getClockWise();
    }
    if (level->getBlockState(pos.relative(facing))->isSolid())
    {
        facing = facing->getOpposite();
    }
    return state->setValue(FACING, facing);
}

bool ChestBlock::isFullChest(Level* level, const BlockPos& pos)
{
    if (level->getBlockState(pos)->getBlock() != this)
    {
        return false;
    }
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const Direction* direction = directions.data[i];
        if (level->getBlockState(pos.relative(direction))->getBlock() == this)
        {
            return true;
        }
    }
    return false;
}

bool ChestBlock::mayPlace(Level* level, const BlockPos& pos)
{
    int count = 0;
    BlockPos west = pos.west();
    BlockPos east = pos.east();
    BlockPos north = pos.north();
    BlockPos south = pos.south();

    if (level->getBlockState(west)->getBlock() == this)
    {
        if (isFullChest(level, west))
        {
            return false;
        }
        ++count;
    }
    if (level->getBlockState(east)->getBlock() == this)
    {
        if (isFullChest(level, east))
        {
            return false;
        }
        ++count;
    }
    if (level->getBlockState(north)->getBlock() == this)
    {
        if (isFullChest(level, north))
        {
            return false;
        }
        ++count;
    }
    if (level->getBlockState(south)->getBlock() == this)
    {
        if (isFullChest(level, south))
        {
            return false;
        }
        ++count;
    }
    return count <= 1;
}

void ChestBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos)
{
    Block::neighborChanged(state, level, pos, block, fromPos);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    bool isChest = eTYPE_DERIVED_FROM(BlockEntityType::CHEST, blockEntity->GetType());
    if (isChest)
    {
        blockEntity->clearCache();
    }
}

void ChestBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity.get() != NULL && eTYPE_DERIVED_FROM(BlockEntityType::CONTAINER, blockEntity->GetType()))
    {
        Containers::dropContents(level, pos, boost::dynamic_pointer_cast<Container>(blockEntity));
        level->updateNeighbourForOutputSignal(pos, this);
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

bool ChestBlock::TestUse()
{
    return true;
}

bool ChestBlock::isBlockedChestByBlock(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos.above())->isSolidBlockingCubeAndNotSignalSource();
}

bool ChestBlock::isEntityOnChest(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlock(pos);
    if (block == Blocks::chest || block == Blocks::ender_chest || block == Blocks::trapped_chest)
    {
        std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(
            typeid(LivingEntity), block->getShape(level->getBlockState(pos), level, pos)->move(pos.above()));
        bool occupied = !entities->empty();
        delete entities;
        return occupied;
    }
    return false;
}

bool ChestBlock::isChestBlockedAt(Level* level, const BlockPos& pos)
{
    return isBlockedChestByBlock(level, pos) || isEntityOnChest(level, pos);
}

boost::shared_ptr<LockableContainer> ChestBlock::getContainer(Level* level, const BlockPos& pos, bool force)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    bool isChest = eTYPE_DERIVED_FROM(BlockEntityType::CHEST, blockEntity->GetType());
    if (!isChest)
    {
        return boost::shared_ptr<LockableContainer>();
    }

    boost::shared_ptr<LockableContainer> container = boost::dynamic_pointer_cast<ChestBlockEntity>(blockEntity);
    if (!force && isChestBlockedAt(level, pos))
    {
        return boost::shared_ptr<LockableContainer>();
    }

    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const Direction* direction = directions.data[i];
        BlockPos neighbourPos = pos.relative(direction);
        Block* block = level->getBlockState(neighbourPos)->getBlock();
        if (block == this)
        {
            if (isChestBlockedAt(level, neighbourPos))
            {
                return boost::shared_ptr<LockableContainer>();
            }
            boost::shared_ptr<BlockEntity> neighbourEntity = level->getBlockEntity(neighbourPos);
            if (eTYPE_DERIVED_FROM(BlockEntityType::CHEST, neighbourEntity->GetType()))
            {
                if (direction == Direction::WEST || direction == Direction::NORTH)
                {
                    container = boost::shared_ptr<LockableContainer>(new CompoundContainer(
                        -1, boost::static_pointer_cast<ChestBlockEntity>(neighbourEntity), container));
                }
                else
                {
                    container = boost::shared_ptr<LockableContainer>(new CompoundContainer(
                        -1, container, boost::static_pointer_cast<ChestBlockEntity>(neighbourEntity)));
                }
            }
        }
    }
    return container;
}

boost::shared_ptr<LockableContainer> ChestBlock::getContainer(Level* level, const BlockPos& pos)
{
    return getContainer(level, pos, false);
}

bool ChestBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (!clientSide && !level->mIsClientSide)
    {
        boost::shared_ptr<LockableContainer> container = getContainer(level, pos);
        if (container.get() != NULL)
        {
            GameMode* gameMode = player->GetGameMode();
            if (gameMode != NULL)
            {
                gameMode->RecordContainer(container->getTypeId(false), pos.x, pos.y, pos.z);
            }
            player->openContainer(container);
        }
    }
    return true;
}

bool ChestBlock::isSignalSource(const BlockState*)
{
    return mType == TYPE_LEFT;
}

int ChestBlock::getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction*)
{
    if (!state->isSignalSource())
    {
        return Block::SIGNAL_NONE;
    }
    int openCount = 0;
    boost::shared_ptr<BlockEntity> blockEntity = levelSource->getBlockEntity(pos);
    if (eTYPE_DERIVED_FROM(BlockEntityType::CHEST, blockEntity->GetType()))
    {
        openCount = boost::dynamic_pointer_cast<ChestBlockEntity>(blockEntity)->mOpenCount;
    }
    return Mth::clamp(openCount, Block::SIGNAL_NONE, Block::SIGNAL_MAX);
}

int ChestBlock::getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction)
{
    if (direction == Direction::UP)
    {
        return getSignal(state, levelSource, pos, direction);
    }
    return Block::SIGNAL_NONE;
}

bool ChestBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int ChestBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    return AbstractContainerMenu::getRedstoneSignalFromContainer(getContainer(level, pos));
}

const BlockState* ChestBlock::getBlockState(int data)
{
    const Direction* facing = Direction::from3DDataValue(data);
    if (facing->getAxis() == Direction::Axis::Y)
    {
        facing = Direction::NORTH;
    }
    return defaultBlockState()->setValue(FACING, facing);
}

int ChestBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get3DDataValue();
}

unsigned int ChestBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
    {
        return IDS_TOOLTIPS_OPEN;
    }
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* ChestBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* ChestBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* ChestBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING};
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape ChestBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

void ChestBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"chest_top");
}

EBlockRenderShape ChestBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_ENTITY;
}

EPistonPushReaction ChestBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_NORMAL;
}
