#include "game/world/block/RepeaterBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/item/Items.hpp"

const double RepeaterBlock::DELAY_TORCH_OFFSETS[4] = {-0.0625, 0.0625, 0.1875, 0.3125};
AbstractProperty<bool>* RepeaterBlock::LOCKED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"locked"));
AbstractProperty<int>* RepeaterBlock::DELAY = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"delay", 1, 4));
static const eGameHostOption REPEATER_INTERACTION_HOST_OPTION = eGameHostOption_MiniGame;

RepeaterBlock::RepeaterBlock(bool on) : DiodeBlock(on)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, Direction::NORTH)
                             ->setValue(DELAY, 1)
                             ->setValue(LOCKED, false));
}

std::wstring RepeaterBlock::getName()
{
    return L10N::GetString(IDS_ITEM_DIODE);
}

const BlockState* RepeaterBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                                 const BlockPos& pos)
{
    AbstractProperty<bool>* lockedProperty = LOCKED;
    TypedBoxed<bool> locked(isLocked(levelSource, pos, state));
    return state->setBoxedValue(lockedProperty, &locked);
}

const BlockState* RepeaterBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(
        HorizontalDirectionalBlock::FACING_PROPERTY,
        rotation->rotate(state->getTypedBoxed(HorizontalDirectionalBlock::FACING_PROPERTY)->getValue()));
}

const BlockState* RepeaterBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return state->rotate(mirror->getRotation(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
}

bool RepeaterBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                        InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
        return false;
    if (!player->mAbilities.mMayBuild)
        return false;
    int priority = 3;
    if (gApp.GetGameHostOption(REPEATER_INTERACTION_HOST_OPTION) != 0)
        priority = 0x25;
    level->setBlock(pos, state->cycle(DELAY), priority, false);
    return true;
}

int RepeaterBlock::getTurnOnDelay(const BlockState* state)
{
    return state->getValue(DELAY) << 1;
}

const BlockState* RepeaterBlock::getOnBlock(const BlockState* state)
{
    int delayValue = state->getValue(DELAY);
    bool lockedValue = state->getValue(LOCKED);
    const Direction* facingValue = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    return Blocks::powered_repeater->defaultBlockState()
        ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, facingValue)
        ->setValue(DELAY, delayValue)
        ->setValue(LOCKED, lockedValue);
}

const BlockState* RepeaterBlock::getOffBlock(const BlockState* state)
{
    int delayValue = state->getValue(DELAY);
    bool lockedValue = state->getValue(LOCKED);
    const Direction* facingValue = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    return Blocks::unpowered_repeater->defaultBlockState()
        ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, facingValue)
        ->setValue(DELAY, delayValue)
        ->setValue(LOCKED, lockedValue);
}

Item* RepeaterBlock::getResource(const BlockState*, Random*, int)
{
    return Items::repeater;
}

not_null_ptr<ItemInstance> RepeaterBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::repeater));
}

bool RepeaterBlock::isLocked(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return getAlternateSignal(levelSource, pos, state) > 0;
}

bool RepeaterBlock::isAlternateInput(const BlockState* state)
{
    return DiodeBlock::isDiode(state);
}

void RepeaterBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (mOn)
    {
        const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
        double x = (double)((float)pos.x + 0.5f) + (double)(random->nextFloat() - 0.5f) * 0.2;
        double y = (double)((float)pos.y + 0.4f) + (double)(random->nextFloat() - 0.5f) * 0.2;
        double z = (double)((float)pos.z + 0.5f) + (double)(random->nextFloat() - 0.5f) * 0.2;
        float offset = -5.0f;
        if (random->nextBoolean())
        {
            float delay = (float)state->getValue(DELAY);
            offset = (delay + delay) - 1.0f;
        }
        double xOffset = (double)(offset * 0.0625f * (float)facing->getStepX());
        double zOffset = (double)(offset * 0.0625f * (float)facing->getStepZ());
        level->addParticle(PARTICLE_RED_DUST, x + xOffset, y, z + zOffset, 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
}

void RepeaterBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onRemove(level, pos, state);
    updateNeighborsInFront(level, pos, state);
}

const BlockState* RepeaterBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, Direction::from2DDataValue(data))
        ->setValue(LOCKED, false)
        ->setValue(DELAY, (data >> 2) + 1);
}

int RepeaterBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get2DDataValue();
    return facing | (state->getValue(DELAY) * 4 - 4);
}

unsigned int RepeaterBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_USE;
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* RepeaterBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {HorizontalDirectionalBlock::FACING_PROPERTY, DELAY, LOCKED};
    return new BlockStateDefinition(this, properties, 3);
}

bool RepeaterBlock::TestUse()
{
    return true;
}

EBlockRenderShape RepeaterBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_REPEATER;
}
