#include "game/world/block/FurnaceBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/render/ParticleType.hpp"

#include "game/core/Boxed.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/Containers.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"

DirectionProperty* FurnaceBlock::FACING = NULL;

bool FurnaceBlock::sKeepInventory = false;

FurnaceBlock::FurnaceBlock(bool lit) : BaseEntityBlock(Material::STONE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any());
    mLit = lit;
}

boost::shared_ptr<BlockEntity> FurnaceBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> blockEntity = boost::shared_ptr<FurnaceBlockEntity>(new FurnaceBlockEntity());
    blockEntity->setLevel(level);
    return blockEntity;
}

void FurnaceBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

Item* FurnaceBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::furnace);
}

void FurnaceBlock::recalcLockDir(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->mIsClientSide)
        return;
    const BlockState* north = level->getBlockState(pos.north());
    const BlockState* south = level->getBlockState(pos.south());
    const BlockState* west = level->getBlockState(pos.west());
    const BlockState* east = level->getBlockState(pos.east());
    const Direction* facing = state->getValue(FACING);
    if (facing == Direction::NORTH && north->isSolid() && !south->isSolid())
        facing = Direction::SOUTH;
    else if (facing == Direction::SOUTH && south->isSolid() && !north->isSolid())
        facing = Direction::NORTH;
    else if (facing == Direction::WEST && west->isSolid() && !east->isSolid())
        facing = Direction::EAST;
    else if (facing == Direction::EAST && east->isSolid() && !west->isSolid())
        facing = Direction::WEST;
    level->setBlock(pos, state->setValue(FACING, facing), Block::UPDATE_CLIENTS, false);
}

void FurnaceBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    recalcLockDir(level, pos, state);
}

void FurnaceBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (!mLit)
        return;
    const Direction* facing = state->getValue(FACING);
    const double x = pos.x + 0.5;
    const double y = pos.y + random->nextDouble() * 6.0 / 16.0;
    const double z = pos.z + 0.5;
    const double offset = 0.52;
    const double side = random->nextDouble() * 0.6 - 0.3;
    if (random->nextDouble() < 0.1)
    {
        level->playLocalSound(pos.x + 0.5, pos.y, pos.z + 0.5, SoundEvents::FURNACE_FIRE_CRACKLE,
                              SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, false, 16.0f);
    }
    switch (facing->get3DDataValue())
    {
    case Direction::eDirection_West:
        level->addParticle(PARTICLE_SMOKE, x - offset, y, z + side, 0.0, 0.0, 0.0, arrayWithLength<int>());
        level->addParticle(PARTICLE_FLAME, x - offset, y, z + side, 0.0, 0.0, 0.0, arrayWithLength<int>());
        break;
    case Direction::eDirection_East:
        level->addParticle(PARTICLE_SMOKE, x + offset, y, z + side, 0.0, 0.0, 0.0, arrayWithLength<int>());
        level->addParticle(PARTICLE_FLAME, x + offset, y, z + side, 0.0, 0.0, 0.0, arrayWithLength<int>());
        break;
    case Direction::eDirection_North:
        level->addParticle(PARTICLE_SMOKE, x + side, y, z - offset, 0.0, 0.0, 0.0, arrayWithLength<int>());
        level->addParticle(PARTICLE_FLAME, x + side, y, z - offset, 0.0, 0.0, 0.0, arrayWithLength<int>());
        break;
    case Direction::eDirection_South:
        level->addParticle(PARTICLE_SMOKE, x + side, y, z + offset, 0.0, 0.0, 0.0, arrayWithLength<int>());
        level->addParticle(PARTICLE_FLAME, x + side, y, z + offset, 0.0, 0.0, 0.0, arrayWithLength<int>());
        break;
    default:
        break;
    }
}

bool FurnaceBlock::TestUse()
{
    return true;
}

bool FurnaceBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                       InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
        return false;
    if (!level->mIsClientSide)
    {
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        bool isFurnace = eTYPE_DERIVED_FROM(BlockEntityType::FURNACE, blockEntity->GetType());
        if (isFurnace)
            player->openContainer(boost::static_pointer_cast<FurnaceBlockEntity>(blockEntity));
    }
    return true;
}

void FurnaceBlock::setLit(bool lit, Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    sKeepInventory = true;
    if (lit)
    {
        level->setBlock(pos, Blocks::lit_furnace->defaultBlockState()->setValue(FACING, state->getValue(FACING)),
                        Block::UPDATE_ALL, false);
        level->setBlock(pos, Blocks::lit_furnace->defaultBlockState()->setValue(FACING, state->getValue(FACING)),
                        Block::UPDATE_ALL, false);
    }
    else
    {
        level->setBlock(pos, Blocks::furnace->defaultBlockState()->setValue(FACING, state->getValue(FACING)),
                        Block::UPDATE_ALL, false);
        level->setBlock(pos, Blocks::furnace->defaultBlockState()->setValue(FACING, state->getValue(FACING)),
                        Block::UPDATE_ALL, false);
    }
    sKeepInventory = false;
    if (blockEntity != NULL)
    {
        sKeepInventory = false;
        blockEntity->clearRemoved();
        level->setBlockEntity(pos, blockEntity);
    }
}

void FurnaceBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                               boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
    DirectionProperty* property = FACING;
    level->setBlock(pos, state->setValue(property, placer->getDirection()->getOpposite()), Block::UPDATE_CLIENTS,
                    false);
    if (itemInstance->hasCustomHoverName())
    {
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        bool isFurnace = eTYPE_DERIVED_FROM(BlockEntityType::FURNACE, blockEntity->GetType());
        if (isFurnace)
        {
            boost::shared_ptr<FurnaceBlockEntity> furnace = boost::dynamic_pointer_cast<FurnaceBlockEntity>(
                blockEntity);
            furnace->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                   itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
        }
    }
}

void FurnaceBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!sKeepInventory)
    {
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        if (blockEntity.get() != NULL)
        {
            bool isFurnace = eTYPE_DERIVED_FROM(BlockEntityType::FURNACE, blockEntity->GetType());
            if (isFurnace)
            {
                Containers::dropContents(level, pos, boost::dynamic_pointer_cast<FurnaceBlockEntity>(blockEntity));
                level->updateNeighbourForOutputSignal(pos, this);
            }
        }
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

bool FurnaceBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int FurnaceBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    return AbstractContainerMenu::getRedstoneSignalFromBlockEntity(level->getBlockEntity(pos));
}

not_null_ptr<ItemInstance> FurnaceBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::furnace)));
}

EBlockRenderShape FurnaceBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_MODEL;
}

const BlockState* FurnaceBlock::getInventoryRenderState(const BlockState*)
{
    return defaultBlockState()->setValue(FACING, Direction::SOUTH);
}

const BlockState* FurnaceBlock::getBlockState(int data)
{
    const Direction* direction = Direction::from3DDataValue(data);
    if (direction->getAxis() == Direction::Axis::Y)
        direction = Direction::NORTH;
    return defaultBlockState()->setValue(FACING, direction);
}

int FurnaceBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get3DDataValue();
}

unsigned int FurnaceBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_USE;
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* FurnaceBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* FurnaceBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* FurnaceBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* FurnaceBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP || direction == Direction::DOWN)
        return mTopIcon;
    return direction != state->getValue(FACING) ? mIcon : mFrontIcon;
}

void FurnaceBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"furnace_side");
    mFrontIcon = iconRegister->registerIcon(mLit ? L"furnace_front_lit" : L"furnace_front");
    mTopIcon = iconRegister->registerIcon(L"furnace_top");
}
