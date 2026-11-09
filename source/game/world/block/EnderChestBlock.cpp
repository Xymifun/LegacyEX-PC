#include "game/world/block/EnderChestBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/Blocks.hpp"

#include "game/client/render/PlayerEnderChestContainer.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/EnderChestBlockEntity.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

DirectionProperty* EnderChestBlock::FACING = NULL;

static const AABB* ENDER_CHEST_SHAPE = AABB::newPermanent(1.0 / 16.0, 0.0, 1.0 / 16.0, 15.0 / 16.0, 14.0 / 16.0,
                                                          15.0 / 16.0);

EnderChestBlock::EnderChestBlock() : BaseEntityBlock(Material::STONE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(FACING, Direction::NORTH));
}

boost::shared_ptr<BlockEntity> EnderChestBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> entity = boost::shared_ptr<EnderChestBlockEntity>(new EnderChestBlockEntity());
    entity->setLevel(level);
    return entity;
}

void EnderChestBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

const AABB* EnderChestBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return ENDER_CHEST_SHAPE;
}

bool EnderChestBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool EnderChestBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool EnderChestBlock::hasCustomBreakingProgress(const BlockState*)
{
    return true;
}

Item* EnderChestBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::obsidian);
}

int EnderChestBlock::getResourceCount(Random*)
{
    return 8;
}

bool EnderChestBlock::isSilkTouchable()
{
    return true;
}

const BlockState* EnderChestBlock::getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float, float,
                                                        int, boost::shared_ptr<LivingEntity> placer)
{
    return defaultBlockState()->setValue(FACING, placer->getDirection()->getOpposite());
}

void EnderChestBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                  boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
    DirectionProperty* facing = FACING;
    level->setBlock(pos, state->setValue(facing, placer->getDirection()->getOpposite()), Block::UPDATE_CLIENTS, false);

    if (itemInstance->hasCustomHoverName())
    {
        boost::shared_ptr<EnderChestBlockEntity> enderChest = boost::dynamic_pointer_cast<EnderChestBlockEntity>(
            level->getBlockEntity(pos));
        if (enderChest)
        {
            enderChest->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                      itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
        }
    }
}

bool EnderChestBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                          InteractionHand::EInteractionHand, const Direction*, float, float, float, bool)
{
    boost::shared_ptr<PlayerEnderChestContainer> inventory = player->getEnderChestInventory();
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (!inventory || !eTYPE_DERIVED_FROM(BlockEntityType::ENDER_CHEST, blockEntity->GetType()))
        return true;
    if (level->getBlockState(pos.above())->isSolidBlockingCubeAndNotSignalSource()
        || ChestBlock::isChestBlockedAt(level, pos))
        return true;
    if (level->mIsClientSide)
        return true;
    inventory->setActiveChest(boost::dynamic_pointer_cast<EnderChestBlockEntity>(blockEntity));
    player->openContainer(inventory);
    return true;
}

void EnderChestBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    for (int i = 0; i < 3; ++i)
    {
        int xSign = random->nextInt(2) * 2 - 1;
        int zSign = random->nextInt(2) * 2 - 1;
        double x = (double)pos.x + 0.5 + xSign * 0.25;
        double y = (float)pos.y + random->nextFloat();
        double z = (double)pos.z + 0.5 + zSign * 0.25;
        double xa = random->nextFloat() * (float)xSign;
        double ya = ((double)random->nextFloat() - 0.5) * 0.125;
        double za = random->nextFloat() * (float)zSign;
        level->addParticle(PARTICLE_PORTAL, x, y, z, xa, ya, za, arrayWithLength<int>());
    }
}

const BlockState* EnderChestBlock::getBlockState(int data)
{
    const Direction* facing = Direction::from3DDataValue(data);
    if (facing->getAxis() == Direction::Axis::Y)
    {
        facing = Direction::NORTH;
    }
    return defaultBlockState()->setValue(FACING, facing);
}

int EnderChestBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get3DDataValue();
}

unsigned int EnderChestBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
    {
        return IDS_TOOLTIPS_OPEN;
    }
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* EnderChestBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* EnderChestBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* EnderChestBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING};
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape EnderChestBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

void EnderChestBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"ender_chest_top");
}

EBlockRenderShape EnderChestBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_ENTITY;
}
