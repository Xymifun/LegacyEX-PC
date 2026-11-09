#include "game/world/block/DaylightDetectorBlock.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/Boxed.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DaylightDetectorBlockEntity.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/util/Math.hpp"

static AbstractProperty<int>* gDaylightDetectorPower = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"power", 0, 15));
static const AABB* gDaylightDetectorShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.375, 1.0);
static const unsigned int DAYLIGHT_DETECTOR_ADVENTURE_TOOLTIP = IDS_TOOLTIP_INVERT;

DaylightDetectorBlock::DaylightDetectorBlock(bool inverted) : BaseEntityBlock(Material::WOOD)
{
    DerivedInit();
    mInverted = inverted;
    registerDefaultState(mStateDefinition->any()->setValue(gDaylightDetectorPower, 0));
    setDestroyTime(0.2f);
    setSoundType(SoundType::WOOD);
}

boost::shared_ptr<BlockEntity> DaylightDetectorBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> blockEntity = boost::shared_ptr<DaylightDetectorBlockEntity>(
        new DaylightDetectorBlockEntity());
    blockEntity->setLevel(level);
    return blockEntity;
}

const AABB* DaylightDetectorBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gDaylightDetectorShape;
}

int DaylightDetectorBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    return state->getValue(gDaylightDetectorPower);
}

void DaylightDetectorBlock::updateSignalStrength(Level* level, const BlockPos& pos)
{
    if (level->mDimension->isHasSkyLight())
    {
        const BlockState* oldState = level->getBlockState(pos);
        int strength = level->getBrightness(LightLayer::SKY, pos) - level->getSkyDarken();
        float sunAngle = level->getSunAngle(1.0f);
        if (mInverted)
            strength = 15 - strength;
        if (strength > 0 && !mInverted)
        {
            float target = sunAngle < 3.1415927f ? 0.0f : 6.2831855f;
            sunAngle += (target - sunAngle) * 0.2f;
            strength = Math::round((float)strength * Mth::cos(sunAngle));
        }
        int value = Mth::clamp(strength, 0, Block::SIGNAL_MAX);
        if (oldState->getValue(gDaylightDetectorPower) != value)
        {
            level->setBlock(pos, oldState->setValue(gDaylightDetectorPower, value),
                            Block::UPDATE_ALL | Block::UPDATE_NO_OBSERVERS, false);
        }
    }
}

bool DaylightDetectorBlock::use(Level* level, const BlockPos& pos, const BlockState* state,
                                boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand,
                                const Direction* direction, float hitX, float hitY, float hitZ, bool clientSide)
{
    if (player->mayBuild())
    {
        if (!level->mIsClientSide)
        {
            if (mInverted)
            {
                level->setBlock(pos,
                                Blocks::daylight_detector->defaultBlockState()->setValue(
                                    gDaylightDetectorPower, state->getValue(gDaylightDetectorPower)),
                                Block::UPDATE_INVISIBLE, false);
                static_cast<DaylightDetectorBlock*>(Blocks::daylight_detector)->updateSignalStrength(level, pos);
            }
            else
            {
                level->setBlock(pos,
                                Blocks::daylight_detector_inverted->defaultBlockState()->setValue(
                                    gDaylightDetectorPower, state->getValue(gDaylightDetectorPower)),
                                Block::UPDATE_INVISIBLE, false);
                static_cast<DaylightDetectorBlock*>(Blocks::daylight_detector_inverted)
                    ->updateSignalStrength(level, pos);
            }
        }
        return true;
    }
    return Block::use(level, pos, state, player, hand, direction, hitX, hitY, hitZ, false);
}

Item* DaylightDetectorBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::daylight_detector);
}

not_null_ptr<ItemInstance> DaylightDetectorBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::daylight_detector)));
}

bool DaylightDetectorBlock::isInverted()
{
    return mInverted;
}

bool DaylightDetectorBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool DaylightDetectorBlock::isSolidRender(const BlockState*) const
{
    return false;
}

EBlockRenderShape DaylightDetectorBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_MODEL;
}

bool DaylightDetectorBlock::isSignalSource(const BlockState*)
{
    return true;
}

const BlockState* DaylightDetectorBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(gDaylightDetectorPower, data);
}

int DaylightDetectorBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gDaylightDetectorPower);
}

unsigned int DaylightDetectorBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock)
    {
        if (!tooltipData.m_pPlayer->GetGameMode()->isAdventure())
        {
            return DAYLIGHT_DETECTOR_ADVENTURE_TOOLTIP;
        }
    }
    return Block::GetInteractTooltip(tooltipData);
}

unsigned int DaylightDetectorBlock::GetHitTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_bCanDestroy && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_MINE;
    return IDS_NONE;
}

BlockStateDefinition* DaylightDetectorBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = {gDaylightDetectorPower};
    return new BlockStateDefinition(this, properties, 1);
}

EPistonPushReaction DaylightDetectorBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_NORMAL;
}

EBlockFaceShape DaylightDetectorBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&,
                                                         const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* DaylightDetectorBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::UP)
        return mInverted ? mInvertedTopIcon : mTopIcon;
    return mSideIcon;
}

void DaylightDetectorBlock::registerIcons(IconRegister* iconRegister)
{
    mTopIcon = iconRegister->registerIcon(getIconName() + L"_top");
    mInvertedTopIcon = iconRegister->registerIcon(getIconName() + L"_inverted_top");
    mSideIcon = iconRegister->registerIcon(getIconName() + L"_side");
}
