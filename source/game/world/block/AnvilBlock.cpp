#include "game/world/block/AnvilBlock.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"

#include <cstring>
#include <string>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/ui/AnvilMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/world/block/Blocks.hpp"

DirectionProperty* AnvilBlock::FACING = NULL;
AbstractProperty<int>* AnvilBlock::DAMAGE = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"damage", 0, 2));

static const AABB* ANVIL_X_SHAPE = AABB::newPermanent(0.0, 0.0, 0.125, 1.0, 0.75, 0.875);
static const AABB* ANVIL_Z_SHAPE = AABB::newPermanent(0.125, 0.0, 0.0, 0.875, 0.75, 1.0);

namespace
{
std::wstring gAnvilTopIconNames[3] = {L"anvil_top", L"anvil_top_damaged_1", L"anvil_top_damaged_2"};
}

AnvilBlock::AnvilBlock() : FallingBlock(Material::HEAVY_METAL)
{
    Block::DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(FACING, Direction::NORTH)->setValue(DAMAGE, 0));
    mTextureType = 0;
    setLightBlock(0);
    memset(mTopIcons, 0, sizeof(mTopIcons));
}

void AnvilBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

bool AnvilBlock::isCubeShaped(const BlockState*)
{
    return false;
}

EBlockFaceShape AnvilBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

bool AnvilBlock::isSolidRender(const BlockState*) const
{
    return false;
}

const BlockState* AnvilBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer)
{
    const Direction* facing = placer->getDirection()->getClockWise();
    return Block::getStateForPlacement(level, pos, direction, hitX, hitY, hitZ, auxValue, placer)
        ->setValue(FACING, facing)
        ->setValue(DAMAGE, auxValue >> 2);
}

bool AnvilBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand, const Direction*, float, float, float, bool)
{
    if (!level->mIsClientSide)
    {
        Anvil anvil(level, pos);
        player->startBlockInteraction(anvil);
    }
    return true;
}

int AnvilBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(DAMAGE);
}

const AABB* AnvilBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(FACING)->getAxis() == Direction::Axis::X ? ANVIL_X_SHAPE : ANVIL_Z_SHAPE;
}

bool AnvilBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

void AnvilBlock::falling(boost::shared_ptr<FallingBlockEntity> entity)
{
    entity->setHurtsEntities(true);
}

void AnvilBlock::onLand(Level* level, const BlockPos& pos, const BlockState*, const BlockState*)
{
    level->levelEvent(LevelEvent::SOUND_ANVIL_LAND, pos, 0);
}

void AnvilBlock::onBroken(Level* level, const BlockPos& pos)
{
    level->levelEvent(LevelEvent::SOUND_ANVIL_BROKEN, pos, 0);
}

bool AnvilBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*)
{
    return true;
}

const BlockState* AnvilBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(FACING, Direction::from2DDataValue(data & 3))
        ->setValue(DAMAGE, (data & 0xf) >> 2);
}

int AnvilBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int facing = state->getValue(FACING)->get2DDataValue();
    return facing | (state->getValue(DAMAGE) << 2);
}

unsigned int AnvilBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_USE;
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* AnvilBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (state->getBlock() != this)
    {
        return state;
    }
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

BlockStateDefinition* AnvilBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING, DAMAGE};
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* AnvilBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (mTextureType == 3 && direction == Direction::UP)
        return mTopIcons[state->getValue(DAMAGE)];
    return mIcon;
}

void AnvilBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"anvil_base");
    for (int i = 0; i < 3; i++)
    {
        mTopIcons[i] = iconRegister->registerIcon(gAnvilTopIconNames[i]);
    }
}

EBlockRenderShape AnvilBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_ANVIL;
}

AnvilBlock::Anvil::Anvil(Level* level, const BlockPos& pos) : InteractionObject()
{
    mLevel = level;
    mBlockPos = pos;
}

AbstractContainerMenu* AnvilBlock::Anvil::createMenu(boost::shared_ptr<Inventory> inventory,
                                                     boost::shared_ptr<Player> player)
{
    return new AnvilMenu(inventory, mLevel, mBlockPos, player);
}

int AnvilBlock::Anvil::getTypeId(bool)
{
    return ClientboundContainerOpenPacket::REPAIR_TABLE;
}

std::wstring AnvilBlock::Anvil::getName()
{
    return L"anvil";
}

std::wstring AnvilBlock::Anvil::getCustomName()
{
    return L"anvil";
}

bool AnvilBlock::Anvil::hasCustomName()
{
    return false;
}

std::wstring AnvilBlock::Anvil::getDisplayName()
{
    return std::wstring(L10N::GetString(Blocks::anvil->getDescriptionId(-1)));
}
