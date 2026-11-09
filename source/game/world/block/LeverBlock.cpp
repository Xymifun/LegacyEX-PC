#include "game/world/block/LeverBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/ButtonBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

LeverBlock::LeverDirection* LeverBlock::LeverDirection::DOWN_X = NULL;
LeverBlock::LeverDirection* LeverBlock::LeverDirection::EAST = NULL;
LeverBlock::LeverDirection* LeverBlock::LeverDirection::WEST = NULL;
LeverBlock::LeverDirection* LeverBlock::LeverDirection::SOUTH = NULL;
LeverBlock::LeverDirection* LeverBlock::LeverDirection::NORTH = NULL;
LeverBlock::LeverDirection* LeverBlock::LeverDirection::UP_Z = NULL;
LeverBlock::LeverDirection* LeverBlock::LeverDirection::UP_X = NULL;
LeverBlock::LeverDirection* LeverBlock::LeverDirection::DOWN_Z = NULL;
LeverBlock::LeverDirection* LeverBlock::LeverDirection::BY_DATA[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

AbstractProperty<LeverBlock::LeverDirection*>* LeverBlock::FACING = NULL;
AbstractProperty<bool>* LeverBlock::POWERED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"powered"));
const AABB* LeverBlock::AABB_NORTH = AABB::newPermanent(0.3125, 0.2f, 0.625, 0.6875, 0.8f, 1.0);
const AABB* LeverBlock::AABB_SOUTH = AABB::newPermanent(0.3125, 0.2f, 0.0, 0.6875, 0.8f, 0.375);
const AABB* LeverBlock::AABB_WEST = AABB::newPermanent(0.625, 0.2f, 0.3125, 1.0, 0.8f, 0.6875);
const AABB* LeverBlock::AABB_EAST = AABB::newPermanent(0.0, 0.2f, 0.3125, 0.375, 0.8f, 0.6875);
const AABB* LeverBlock::AABB_UP = AABB::newPermanent(0.25, 0.0, 0.25, 0.75, 0.6f, 0.75);
const AABB* LeverBlock::AABB_DOWN = AABB::newPermanent(0.25, 0.4f, 0.25, 0.75, 1.0, 0.75);

LeverBlock::LeverDirection::LeverDirection(int data, const std::wstring& name, const Direction* direction)
    : mData(data),
      mName(name),
      mDirection(direction)
{
}

int LeverBlock::LeverDirection::getData()
{
    return mData;
}

void LeverBlock::LeverDirection::staticCtor()
{
    DOWN_X = new LeverDirection(0, L"down_x", Direction::DOWN);
    EAST = new LeverDirection(1, L"east", Direction::EAST);
    WEST = new LeverDirection(2, L"west", Direction::WEST);
    SOUTH = new LeverDirection(3, L"south", Direction::SOUTH);
    NORTH = new LeverDirection(4, L"north", Direction::NORTH);
    UP_Z = new LeverDirection(5, L"up_z", Direction::UP);
    UP_X = new LeverDirection(6, L"up_x", Direction::UP);
    DOWN_Z = new LeverDirection(7, L"down_z", Direction::DOWN);
    BY_DATA[DOWN_X->getData()] = DOWN_X;
    BY_DATA[EAST->getData()] = EAST;
    BY_DATA[WEST->getData()] = WEST;
    BY_DATA[SOUTH->getData()] = SOUTH;
    BY_DATA[NORTH->getData()] = NORTH;
    BY_DATA[UP_Z->getData()] = UP_Z;
    BY_DATA[UP_X->getData()] = UP_X;
    BY_DATA[DOWN_Z->getData()] = DOWN_Z;
}

LeverBlock::LeverDirection* LeverBlock::LeverDirection::fromFacing(const Direction* face, const Direction* direction)
{
    switch (face->get3DDataValue())
    {
    case 0:
        switch (direction->getAxis()->getId())
        {
        case Direction::Axis::eAxis_X:
            return DOWN_X;
        case Direction::Axis::eAxis_Z:
            return DOWN_Z;
        }
    case 1:
        switch (direction->getAxis()->getId())
        {
        case Direction::Axis::eAxis_X:
            return UP_X;
        case Direction::Axis::eAxis_Z:
            return UP_Z;
        }
    case 2:
        return NORTH;
    case 3:
        return SOUTH;
    case 4:
        return WEST;
    case 5:
        return EAST;
    }
    return NULL;
}

const Direction* LeverBlock::LeverDirection::getDirection()
{
    return mDirection;
}

LeverBlock::LeverDirection* LeverBlock::LeverDirection::byData(int data)
{
    unsigned int index = data;
    return BY_DATA[index < 8 ? index : 0];
}

std::wstring LeverBlock::LeverDirection::toString() const
{
    return mName;
}

std::wstring LeverBlock::LeverDirection::getSerializedName() const
{
    return mName;
}

LeverBlock::LeverBlock() : Block(Material::DECORATION)
{
    Block::DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(FACING, LeverDirection::NORTH)->setValue(POWERED, false));
}

void LeverBlock::blockStaticCtor()
{
    LeverDirection::staticCtor();
    std::wstring name(L"facing");
    AbstractProperty<LeverDirection*>* property;
    {
        boost::unordered_set<LeverDirection*> directions;
        for (int data = 0; data < 8; ++data)
        {
            directions.insert(LeverDirection::BY_DATA[data]);
        }
        property = EnumProperty<LeverDirection*>::create(name, typeid(LeverDirection*), directions,
                                                         Predicates<LeverDirection*>::alwaysTrue);
    }
    FACING = property;
}

const AABB* LeverBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL_AABB;
}

bool LeverBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool LeverBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool LeverBlock::canAttachTo(Level* level, const BlockPos& pos, const Direction* direction)
{
    return ButtonBlock::canAttachTo(level, pos, direction);
}

bool LeverBlock::mayPlace(Level* level, const BlockPos& pos, const Direction* direction)
{
    return canAttachTo(level, pos, direction);
}

bool LeverBlock::mayPlace(Level* level, const BlockPos& pos)
{
    for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
         ++it)
    {
        const Direction* direction = *it;
        if (canAttachTo(level, pos, direction))
            return true;
    }
    return false;
}

const BlockState* LeverBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float,
                                                   float, float, int, boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* state = defaultBlockState()->setValue(POWERED, false);
    if (canAttachTo(level, pos, direction))
    {
        TypedBoxed<LeverDirection*> facing(LeverDirection::fromFacing(direction, placer->getDirection()));
        return state->setBoxedValue(FACING, &facing);
    }
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int index = 0; index < directions.length; ++index)
    {
        const Direction* candidate = directions.data[index];
        if (candidate != direction && canAttachTo(level, pos, candidate))
        {
            TypedBoxed<LeverDirection*> facing(LeverDirection::fromFacing(candidate, placer->getDirection()));
            return state->setBoxedValue(FACING, &facing);
        }
    }
    if (level->getBlockState(pos.below())->isTopSolidBlocking())
    {
        TypedBoxed<LeverDirection*> facing(LeverDirection::fromFacing(Direction::UP, placer->getDirection()));
        return state->setBoxedValue(FACING, &facing);
    }
    return state;
}

int LeverBlock::getLeverFacing(const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case 0:
        return 0;
    case 1:
        return 5;
    case 2:
        return 4;
    case 3:
        return 3;
    case 4:
        return 2;
    case 5:
        return 1;
    }
    return -1;
}

void LeverBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*,
                                 const BlockPos& fromPos)
{
    if (canSurvive(level, pos, state))
    {
        LeverDirection* facing = state->getValue(FACING);
        fromPos != pos.relative(facing->getDirection());
        if (!canAttachTo(level, pos, facing->getDirection()))
        {
            spawnResources(level, pos, state, 0);
            level->removeBlock(pos);
        }
    }
}

bool LeverBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    if (mayPlace(level, pos))
        return true;
    spawnResources(level, pos, level->getBlockState(pos), 0);
    level->removeBlock(pos);
    return false;
}

int LeverBlock::getSpawnResourcesAuxValue(const BlockState*)
{
    return getLeverFacing(Direction::NORTH);
}

const AABB* LeverBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(FACING)->getData())
    {
    case 1:
        return AABB_EAST;
    case 2:
        return AABB_WEST;
    case 3:
        return AABB_SOUTH;
    case 5:
    case 6:
        return AABB_UP;
    case 0:
    case 7:
        return AABB_DOWN;
    default:
        return AABB_NORTH;
    case 4:
        return AABB_NORTH;
    }
}

bool LeverBlock::TestUse()
{
    return true;
}

bool LeverBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player>,
                     InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
    {
        const BlockState* toggledState = state->cycle(POWERED);
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::LEVER_CLICK, SoundSource::eSoundSource_Blocks,
                         0.3f, toggledState->getValue(POWERED) ? 0.6f : 0.5f, 16.0f);
        return false;
    }
    if (level->mIsClientSide)
    {
        const BlockState* toggledState = state->cycle(POWERED);
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::LEVER_CLICK, SoundSource::eSoundSource_Blocks,
                         0.3f, toggledState->getValue(POWERED) ? 0.6f : 0.5f, 16.0f);
        return true;
    }
    const BlockState* toggledState = state->cycle(POWERED);
    level->setBlock(pos, toggledState, Block::UPDATE_ALL, false);
    float pitch = toggledState->getValue(POWERED) ? 0.6f : 0.5f;
    level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::LEVER_CLICK, SoundSource::eSoundSource_Blocks, 0.3f,
                     pitch, 16.0f);
    level->updateNeighborsAt(pos, this, false);
    const Direction* facing = toggledState->getValue(FACING)->getDirection();
    level->updateNeighborsAt(pos.relative(facing->getOpposite()), this, false);
    return true;
}

void LeverBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    bool powered = state->getValue(POWERED);
    if (powered)
    {
        level->updateNeighborsAt(pos, this, false);
        const Direction* facing = state->getValue(FACING)->getDirection();
        level->updateNeighborsAt(pos.relative(facing->getOpposite()), this, false);
    }
    Block::onRemove(level, pos, state);
}

int LeverBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    return state->getValue(POWERED) ? SIGNAL_MAX : SIGNAL_NONE;
}

int LeverBlock::getDirectSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction* direction)
{
    if (state->getValue(POWERED) && state->getValue(FACING)->getDirection() == direction)
        return SIGNAL_MAX;
    return SIGNAL_NONE;
}

bool LeverBlock::isSignalSource(const BlockState*)
{
    return true;
}

const BlockState* LeverBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(FACING, LeverDirection::byData(data & 7))->setValue(POWERED, (data & 8) > 0);
}

int LeverBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->getData();
    if (state->getValue(POWERED))
        data |= 8;
    return data;
}

unsigned int LeverBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_USE;
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* LeverBlock::mirror(const BlockState* state, Mirror* mirror)
{
    AbstractProperty<LeverDirection*>* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)->getDirection()));
    return result;
}

const BlockState* LeverBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        switch (state->getValue(FACING)->getData())
        {
        case 1:
            return state->setValue(FACING, LeverDirection::WEST);
        case 2:
            return state->setValue(FACING, LeverDirection::EAST);
        case 3:
            return state->setValue(FACING, LeverDirection::NORTH);
        case 4:
            return state->setValue(FACING, LeverDirection::SOUTH);
        default:
            return state;
        }
    case Rotation::eTYPE_CounterClockwise90:
        switch (state->getValue(FACING)->getData())
        {
        case 1:
            return state->setValue(FACING, LeverDirection::NORTH);
        case 2:
            return state->setValue(FACING, LeverDirection::SOUTH);
        case 3:
            return state->setValue(FACING, LeverDirection::EAST);
        case 4:
            return state->setValue(FACING, LeverDirection::WEST);
        case 5:
            return state->setValue(FACING, LeverDirection::UP_X);
        case 6:
            return state->setValue(FACING, LeverDirection::UP_Z);
        case 0:
            return state->setValue(FACING, LeverDirection::DOWN_Z);
        case 7:
            return state->setValue(FACING, LeverDirection::DOWN_X);
        }
    case Rotation::eTYPE_Clockwise90:
        switch (state->getValue(FACING)->getData())
        {
        case 1:
            return state->setValue(FACING, LeverDirection::SOUTH);
        case 2:
            return state->setValue(FACING, LeverDirection::NORTH);
        case 3:
            return state->setValue(FACING, LeverDirection::WEST);
        case 4:
            return state->setValue(FACING, LeverDirection::EAST);
        case 5:
            return state->setValue(FACING, LeverDirection::UP_X);
        case 6:
            return state->setValue(FACING, LeverDirection::UP_Z);
        case 0:
            return state->setValue(FACING, LeverDirection::DOWN_Z);
        case 7:
            return state->setValue(FACING, LeverDirection::DOWN_X);
        }
    default:
        return state;
    }
}

BlockStateDefinition* LeverBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING, POWERED};
    return new BlockStateDefinition(this, properties, 2);
}

EBlockFaceShape LeverBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape LeverBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_LEVER;
}
