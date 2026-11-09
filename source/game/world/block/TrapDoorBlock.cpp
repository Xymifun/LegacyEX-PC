#include "game/world/block/Predicates.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/TrapDoorBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

AbstractProperty<TrapDoorBlock::Half*>* TrapDoorBlock::HALF = NULL;
AbstractProperty<bool>* TrapDoorBlock::OPEN = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"open"));
AbstractProperty<const Direction*>* TrapDoorBlock::FACING = NULL;
const AABB* TrapDoorBlock::EAST_OPEN_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 0.1875, 1.0, 1.0);
const AABB* TrapDoorBlock::WEST_OPEN_AABB = AABB::newPermanent(0.8125, 0.0, 0.0, 1.0, 1.0, 1.0);
const AABB* TrapDoorBlock::SOUTH_OPEN_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.1875);
const AABB* TrapDoorBlock::NORTH_OPEN_AABB = AABB::newPermanent(0.0, 0.0, 0.8125, 1.0, 1.0, 1.0);
const AABB* TrapDoorBlock::BOTTOM_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.1875, 1.0);
const AABB* TrapDoorBlock::TOP_AABB = AABB::newPermanent(0.0, 0.8125, 0.0, 1.0, 1.0, 1.0);
TrapDoorBlock::Half* TrapDoorBlock::TOP = new TrapDoorBlock::Half(L"top");
TrapDoorBlock::Half* TrapDoorBlock::BOTTOM = new TrapDoorBlock::Half(L"bottom");
TrapDoorBlock::Half* TrapDoorBlock::HALVES[2] = {TrapDoorBlock::TOP, TrapDoorBlock::BOTTOM};

std::wstring TrapDoorBlock::Half::toString() const
{
    return mName;
}
std::wstring TrapDoorBlock::Half::getSerializedName() const
{
    return mName;
}

TrapDoorBlock::TrapDoorBlock(Material* material) : Block(material, material->getColor())
{
    DerivedInit();
    registerDefaultState(
        mStateDefinition->any()->setValue(FACING, Direction::NORTH)->setValue(OPEN, false)->setValue(HALF, BOTTOM));
}

TrapDoorBlock::TrapDoorBlock(Material* material, const MaterialColor* materialColor) : Block(material, materialColor)
{
    DerivedInit();
    registerDefaultState(
        mStateDefinition->any()->setValue(FACING, Direction::NORTH)->setValue(OPEN, false)->setValue(HALF, BOTTOM));
}

void TrapDoorBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
    std::wstring name;
    name = L"half";
    EnumProperty<Half*>* half;
    {
        Predicate<Half*>* filter = Predicates<Half*>::alwaysTrue;
        boost::unordered_set<Half*> halves;
        for (int i = 0; i < 2; ++i)
        {
            halves.insert(HALVES[i]);
        }
        half = EnumProperty<Half*>::create(name, typeid(Half*), halves, filter);
    }
    HALF = half;
}

const AABB* TrapDoorBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    const AABB* shape;
    if (state->getValue(OPEN))
    {
        switch (state->getValue(FACING)->get3DDataValue())
        {
        default:
            shape = NORTH_OPEN_AABB;
            break;
        case 3:
            shape = SOUTH_OPEN_AABB;
            break;
        case 4:
            shape = WEST_OPEN_AABB;
            break;
        case 5:
            shape = EAST_OPEN_AABB;
            break;
        }
    }
    else if (state->getValue(HALF) == TOP)
        shape = TOP_AABB;
    else
        shape = BOTTOM_AABB;
    return shape;
}

bool TrapDoorBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool TrapDoorBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool TrapDoorBlock::isPathfindable(LevelSource* levelSource, const BlockPos& pos)
{
    return !levelSource->getBlockState(pos)->getValue(OPEN);
}
bool TrapDoorBlock::TestUse()
{
    return mMaterial != Material::METAL;
}

void TrapDoorBlock::playSound(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos, bool open)
{
    if (open)
    {
        int sound = LevelEvent::SOUND_OPEN_WOODEN_TRAP_DOOR;
        if (mMaterial == Material::METAL)
            sound = LevelEvent::SOUND_OPEN_IRON_TRAP_DOOR;
        level->levelEvent(player, sound, pos, 0);
    }
    else
    {
        int sound = LevelEvent::SOUND_CLOSE_WOODEN_TRAP_DOOR;
        if (mMaterial == Material::METAL)
            sound = LevelEvent::SOUND_CLOSE_IRON_TRAP_DOOR;
        level->levelEvent(player, sound, pos, 0);
    }
}

bool TrapDoorBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                        InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (mMaterial == Material::METAL)
        return false;
    if (clientSide)
    {
        playSound(player, level, pos, state->getValue(OPEN));
        return false;
    }
    state = state->cycle(OPEN);
    level->setBlock(pos, state, Block::UPDATE_ALL, false);
    playSound(player, level, pos, state->getValue(OPEN));
    return true;
}

void TrapDoorBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                    const BlockPos&)
{
    if (level->mIsClientSide)
        return;
    bool powered = level->hasNeighborSignal(pos);
    if (!powered && !block->defaultBlockState()->isSignalSource())
        return;
    if (state->getValue(OPEN) != powered)
    {
        level->setBlock(pos, state->setValue(OPEN, powered), Block::UPDATE_ALL, false);
        playSound(boost::shared_ptr<Player>(), level, pos, powered);
    }
}

const BlockState* TrapDoorBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                      float, float hitY, float, int,
                                                      boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* state = defaultBlockState();
    if (direction->getAxis()->isHorizontal())
    {
        state = state->setValue(FACING, direction)->setValue(OPEN, false)->setValue(HALF, hitY > 0.5f ? TOP : BOTTOM);
    }
    else
    {
        state = state->setValue(FACING, placer->getDirection()->getOpposite())
                    ->setValue(OPEN, false)
                    ->setValue(HALF, direction == Direction::UP ? BOTTOM : TOP);
    }
    if (level->hasNeighborSignal(pos))
        state = state->setValue(OPEN, true);
    return state;
}

bool TrapDoorBlock::mayPlace(Level*, const BlockPos&, const Direction*)
{
    return true;
}
const Direction* TrapDoorBlock::getFacingFromData(int data)
{
    switch (data & 3)
    {
    case 0:
        return Direction::NORTH;
    case 1:
        return Direction::SOUTH;
    case 2:
        return Direction::WEST;
    default:
        return Direction::EAST;
    }
}

unsigned char TrapDoorBlock::getDataFromFacing(const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case 2:
        return 0;
    case 3:
        return 1;
    case 4:
        return 2;
    default:
        return 3;
    }
}

int TrapDoorBlock::getRenderLayer()
{
    return 0;
}

const BlockState* TrapDoorBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(FACING, getFacingFromData(data))
        ->setValue(OPEN, (data & 4) != 0)
        ->setValue(HALF, (data & 8) == 0 ? BOTTOM : TOP);
}

int TrapDoorBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = getDataFromFacing(state->getValue(FACING));
    if (state->getValue(OPEN))
        data |= 4;
    if (state->getValue(HALF) == TOP)
        data |= 8;
    return data;
}

unsigned int TrapDoorBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bCanPlace)
        return IDS_NONE;
    if (mMaterial == Material::METAL)
    {
        if (tooltipData.m_pItemInstance->isEmpty())
            return IDS_NONE;
    }
    else if (tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock)
    {
        if (tooltipData.m_pItemInstance->isEmpty() || !tooltipData.m_pPlayer->isSneaking())
            return IDS_TOOLTIPS_USE;
    }
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* TrapDoorBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* TrapDoorBlock::mirror(const BlockState* state, Mirror* mirror)
{
    AbstractProperty<const Direction*>* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* TrapDoorBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING, OPEN, HALF};
    return new BlockStateDefinition(this, properties, 3);
}

EBlockFaceShape TrapDoorBlock::getBlockFaceShape(LevelSource*, const BlockState* state, const BlockPos&,
                                                 const Direction* direction)
{
    if ((direction == Direction::UP && state->getValue(HALF) == TOP)
        || (direction == Direction::DOWN && state->getValue(HALF) == BOTTOM))
    {
        if (!state->getValue(OPEN))
            return BLOCK_FACE_SOLID;
    }
    return BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape TrapDoorBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_TRAPDOOR;
}
