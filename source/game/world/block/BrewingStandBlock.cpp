#include "game/world/block/BrewingStandBlock.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/core/L10N.hpp"
#include "game/core/IconRegister.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/item/Containers.hpp"
#include "game/item/Items.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/BrewingStandBlockEntity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"

AbstractProperty<bool>* BrewingStandBlock::HAS_BOTTLE_0 = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"has_bottle_0"));
AbstractProperty<bool>* BrewingStandBlock::HAS_BOTTLE_1 = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"has_bottle_1"));
AbstractProperty<bool>* BrewingStandBlock::HAS_BOTTLE_2 = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"has_bottle_2"));

static const AABB* gBrewingStandShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.875, 1.0);
static const AABB* gBrewingStandBaseShape = AABB::newPermanent(0.125, 0.0, 0.125, 0.875, 0.125, 0.875);

BrewingStandBlock::BrewingStandBlock() : BaseEntityBlock(Material::METAL)
{
    DerivedInit();
}

boost::shared_ptr<BlockEntity> BrewingStandBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> blockEntity = boost::shared_ptr<BrewingStandBlockEntity>(
        new BrewingStandBlockEntity());
    blockEntity->setLevel(level);
    return blockEntity;
}

std::wstring BrewingStandBlock::getName()
{
    return std::wstring(L10N::GetString(IDS_BREWING_STAND));
}

bool BrewingStandBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool BrewingStandBlock::isCubeShaped(const BlockState*)
{
    return false;
}

void BrewingStandBlock::addCollisionAABBs(const BlockState*, Level*, const BlockPos& pos, const AABB* entityBox,
                                          std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity>, bool)
{
    addCollisionAABB(pos, entityBox, collidingBoxes, gBrewingStandShape);
    addCollisionAABB(pos, entityBox, collidingBoxes, gBrewingStandBaseShape);
}

const AABB* BrewingStandBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gBrewingStandBaseShape;
}

bool BrewingStandBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                            InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
        return false;
    if (!level->mIsClientSide)
    {
        boost::shared_ptr<BrewingStandBlockEntity> brewingStand = boost::dynamic_pointer_cast<BrewingStandBlockEntity>(
            level->getBlockEntity(pos));
        if (brewingStand.get() != NULL)
            player->openContainer(brewingStand);
    }
    return true;
}

void BrewingStandBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                    boost::shared_ptr<LivingEntity>, not_null_ptr<ItemInstance> itemInstance)
{
    if (!itemInstance->hasCustomHoverName())
        return;
    boost::shared_ptr<BrewingStandBlockEntity> brewingStand = boost::dynamic_pointer_cast<BrewingStandBlockEntity>(
        level->getBlockEntity(pos));
    if (brewingStand.get() != NULL)
    {
        brewingStand->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                    itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
    }
}

void BrewingStandBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    double x = pos.x + 0.4f + random->nextFloat() * 0.2f;
    double y = pos.y + 0.7f + random->nextFloat() * 0.3f;
    double z = pos.z + 0.4f + random->nextFloat() * 0.2f;
    level->addParticle(PARTICLE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

void BrewingStandBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity.get() != NULL && boost::dynamic_pointer_cast<BrewingStandBlockEntity>(blockEntity).get() != NULL)
    {
        Containers::dropContents(level, pos, boost::dynamic_pointer_cast<BrewingStandBlockEntity>(blockEntity));
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

Item* BrewingStandBlock::getResource(const BlockState*, Random*, int)
{
    return Items::brewingStand;
}

not_null_ptr<ItemInstance> BrewingStandBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::brewingStand));
}

bool BrewingStandBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int BrewingStandBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    return AbstractContainerMenu::getRedstoneSignalFromBlockEntity(level->getBlockEntity(pos));
}

int BrewingStandBlock::getRenderLayer()
{
    return 0;
}

const BlockState* BrewingStandBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    state = state->setValue(HAS_BOTTLE_0, (data & 1) != 0);
    state = state->setValue(HAS_BOTTLE_1, (data & 2) != 0);
    return state->setValue(HAS_BOTTLE_2, (data & 4) != 0);
}

int BrewingStandBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = 0;
    if (state->getValue(HAS_BOTTLE_0))
        data |= 1;
    if (state->getValue(HAS_BOTTLE_1))
        data |= 2;
    if (state->getValue(HAS_BOTTLE_2))
        data |= 4;
    return data;
}

unsigned int BrewingStandBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_USE;
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* BrewingStandBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {HAS_BOTTLE_0, HAS_BOTTLE_1, HAS_BOTTLE_2};
    return new BlockStateDefinition(this, properties, 3);
}

EBlockFaceShape BrewingStandBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

void BrewingStandBlock::registerIcons(IconRegister* iconRegister)
{
    Block::registerIcons(iconRegister);
    mBaseTexture = iconRegister->registerIcon(getIconName() + L"_base");
}

TextureAtlasSprite* BrewingStandBlock::getBaseTexture()
{
    return mBaseTexture;
}

EBlockRenderShape BrewingStandBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_BREWING_STAND;
}
