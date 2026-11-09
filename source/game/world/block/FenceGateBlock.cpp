#include "game/world/block/FenceGateBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

AbstractProperty<bool>* FenceGateBlock::OPEN = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"open"));
AbstractProperty<bool>* FenceGateBlock::POWERED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"powered"));
AbstractProperty<bool>* FenceGateBlock::IN_WALL = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"in_wall"));
AABB* FenceGateBlock::Z_AXIS_AABB = AABB::newPermanent(0.0, 0.0, 0.375, 1.0, 1.0, 0.625);
AABB* FenceGateBlock::X_AXIS_AABB = AABB::newPermanent(0.375, 0.0, 0.0, 0.625, 1.0, 1.0);
AABB* FenceGateBlock::Z_AXIS_IN_WALL_AABB = AABB::newPermanent(0.0, 0.0, 0.375, 1.0, 0.8125, 0.625);
AABB* FenceGateBlock::X_AXIS_IN_WALL_AABB = AABB::newPermanent(0.375, 0.0, 0.0, 0.625, 0.8125, 1.0);
AABB* FenceGateBlock::Z_AXIS_COLLISION_AABB = AABB::newPermanent(0.0, 0.0, 0.375, 1.0, 1.5, 0.625);
AABB* FenceGateBlock::X_AXIS_COLLISION_AABB = AABB::newPermanent(0.375, 0.0, 0.0, 0.625, 1.5, 1.0);

FenceGateBlock::FenceGateBlock(const std::wstring& name, PlanksBlock::Variant* variant)
    : HorizontalDirectionalBlock(Material::WOOD, variant->getMaterialColor())
{
    mIsFenceGate = true;
    mBlockType = EBLOCK_TYPE_FENCE_GATE;
    DerivedInit();
    registerDefaultState(
        mStateDefinition->any()->setValue(OPEN, false)->setValue(POWERED, false)->setValue(IN_WALL, false));
    mTextureName = name;
}

const AABB* FenceGateBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    state = fillVirtualBlockStateProperties(state, levelSource, pos);
    if (state->getValue(IN_WALL))
    {
        return state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->getAxis() == Direction::Axis::X
                   ? X_AXIS_IN_WALL_AABB
                   : Z_AXIS_IN_WALL_AABB;
    }
    return state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->getAxis() == Direction::Axis::X ? X_AXIS_AABB
                                                                                                         : Z_AXIS_AABB;
}

const BlockState* FenceGateBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                                  const BlockPos& pos)
{
    const Direction::Axis* axis = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->getAxis();
    const bool inWall = (axis == Direction::Axis::Z
                         && (levelSource->getBlockState(pos.west())->getBlock() == Blocks::cobblestone_wall
                             || levelSource->getBlockState(pos.east())->getBlock() == Blocks::cobblestone_wall))
                        || (axis == Direction::Axis::X
                            && (levelSource->getBlockState(pos.north())->getBlock() == Blocks::cobblestone_wall
                                || levelSource->getBlockState(pos.south())->getBlock() == Blocks::cobblestone_wall));
    if (inWall)
        state = state->setValue(IN_WALL, true);
    return state;
}

const BlockState* FenceGateBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(
        HorizontalDirectionalBlock::FACING_PROPERTY,
        rotation->rotate(state->getTypedBoxed(HorizontalDirectionalBlock::FACING_PROPERTY)->getValue()));
}

const BlockState* FenceGateBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = HorizontalDirectionalBlock::FACING_PROPERTY;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

bool FenceGateBlock::mayPlace(Level* level, const BlockPos& pos)
{
    if (level->getBlockState(pos.below())->getBlock()->mMaterial->isSolid())
        return Block::mayPlace(level, pos);
    return false;
}

const AABB* FenceGateBlock::getClipAABB(const BlockState* state, LevelSource*, const BlockPos&)
{
    if (state->getValue(OPEN))
        return NULL_AABB;
    return state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->getAxis() == Direction::Axis::Z
               ? Z_AXIS_COLLISION_AABB
               : X_AXIS_COLLISION_AABB;
}

bool FenceGateBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool FenceGateBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool FenceGateBlock::isPathfindable(LevelSource* levelSource, const BlockPos& pos)
{
    LevelSource& source = *levelSource;
    return source.getBlockState(pos)->getValue(OPEN);
}

const BlockState* FenceGateBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction*, float,
                                                       float, float, int, boost::shared_ptr<LivingEntity> placer)
{
    bool powered = level->hasNeighborSignal(pos);
    return defaultBlockState()
        ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, placer->getDirection())
        ->setValue(OPEN, powered)
        ->setValue(POWERED, powered)
        ->setValue(IN_WALL, false);
}

bool FenceGateBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                         InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
    {
        level->levelEvent(
            player, state->getValue(OPEN) ? LevelEvent::SOUND_OPEN_FENCE_GATE : LevelEvent::SOUND_CLOSE_FENCE_GATE, pos,
            0);
        return false;
    }
    if (state->getValue(OPEN))
    {
        state = state->setValue(OPEN, false);
        level->setBlock(pos, state, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
    }
    else
    {
        const Direction* facing = Direction::fromYRot((double)player->mYRot);
        if (state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY) == facing->getOpposite())
        {
            state = state->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, facing);
        }
        state = state->setValue(OPEN, true);
        level->setBlock(pos, state, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
    }
    level->levelEvent(
        player, state->getValue(OPEN) ? LevelEvent::SOUND_OPEN_FENCE_GATE : LevelEvent::SOUND_CLOSE_FENCE_GATE, pos, 0);
    return true;
}

void FenceGateBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*,
                                     const BlockPos&)
{
    if (level->mIsClientSide)
        return;
    const bool powered = level->hasNeighborSignal(pos);
    if (state->getValue(POWERED) != powered)
    {
        level->setBlock(pos, state->setValue(POWERED, powered)->setValue(OPEN, powered),
                        Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        if (state->getValue(OPEN) != powered)
        {
            level->levelEvent(boost::shared_ptr<Player>(),
                              powered ? LevelEvent::SOUND_OPEN_FENCE_GATE : LevelEvent::SOUND_CLOSE_FENCE_GATE, pos, 0);
        }
    }
}

bool FenceGateBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*)
{
    return true;
}

const BlockState* FenceGateBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, Direction::from2DDataValue(data))
        ->setValue(OPEN, (data & 4) != 0)
        ->setValue(POWERED, (data & 8) != 0);
}

int FenceGateBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get2DDataValue();
    if (state->getValue(POWERED))
    {
        data |= 8;
    }
    if (state->getValue(OPEN))
    {
        data |= 4;
    }
    return data;
}

unsigned int FenceGateBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    return (tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace
               ? IDS_TOOLTIPS_USE
               : Block::GetInteractTooltip(tooltipData);
}

bool FenceGateBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

BlockStateDefinition* FenceGateBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {HorizontalDirectionalBlock::FACING_PROPERTY, OPEN, POWERED, IN_WALL};
    return new BlockStateDefinition(this, properties, 4);
}

EBlockFaceShape FenceGateBlock::getBlockFaceShape(LevelSource*, const BlockState* state, const BlockPos&,
                                                  const Direction* direction)
{
    if (direction == Direction::UP || direction == Direction::DOWN)
        return BLOCK_FACE_UNDEFINED;
    const BlockState& blockState = *state;
    return blockState.getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->getAxis()
                   == direction->getClockWise()->getAxis()
               ? BLOCK_FACE_MIDDLE_POLE
               : BLOCK_FACE_UNDEFINED;
}

bool FenceGateBlock::isFenceGate(Block* block)
{
    return block == Blocks::fence_gate || block == Blocks::spruce_fence_gate || block == Blocks::birch_fence_gate
           || block == Blocks::jungle_fence_gate || block == Blocks::acacia_fence_gate
           || block == Blocks::dark_oak_fence_gate;
}

void FenceGateBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mTextureName);
}
EBlockRenderShape FenceGateBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_FENCE_GATE;
}
