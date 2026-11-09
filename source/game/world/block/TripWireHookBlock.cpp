#include "game/world/block/TripWireHookBlock.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TripWireBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty* TripWireHookBlock::FACING = NULL;
AbstractProperty<bool>* TripWireHookBlock::POWERED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"powered"));
AbstractProperty<bool>* TripWireHookBlock::ATTACHED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"attached"));
AbstractProperty<bool>* TripWireHookBlock::SUSPENDED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"suspended"));
const AABB* TripWireHookBlock::AABB_DOWN = AABB::newPermanent(0.0, 0.0, 0.375, 1.0, 0.125, 0.625);
const AABB* TripWireHookBlock::AABB_UP = AABB::newPermanent(0.0, 0.875, 0.375, 1.0, 1.0, 0.625);
const AABB* TripWireHookBlock::AABB_NORTH = AABB::newPermanent(0.3125, 0.0, 0.625, 0.6875, 0.625, 1.0);
const AABB* TripWireHookBlock::AABB_SOUTH = AABB::newPermanent(0.3125, 0.0, 0.0, 0.6875, 0.625, 0.375);
const AABB* TripWireHookBlock::AABB_WEST = AABB::newPermanent(0.625, 0.0, 0.3125, 1.0, 0.625, 0.6875);
const AABB* TripWireHookBlock::AABB_EAST = AABB::newPermanent(0.0, 0.0, 0.3125, 0.375, 0.625, 0.6875);

TripWireHookBlock::TripWireHookBlock() : Block(Material::DECORATION)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(FACING, Direction::NORTH)
                             ->setValue(POWERED, false)
                             ->setValue(ATTACHED, false)
                             ->setValue(SUSPENDED, false));
    setTicking(true);
}

void TripWireHookBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

const AABB* TripWireHookBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(FACING)->get3DDataValue())
    {
    default:
        return AABB_EAST;
    case 4:
        return AABB_WEST;
    case 3:
        return AABB_SOUTH;
    case 2:
        return AABB_NORTH;
    }
}

const AABB* TripWireHookBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL_AABB;
}
bool TripWireHookBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool TripWireHookBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool TripWireHookBlock::mayPlace(Level* level, const BlockPos& pos, const Direction* direction)
{
    BlockPos supportPos = pos.relative(direction->getOpposite());
    const BlockState* supportState = level->getBlockState(supportPos);
    bool pistonException = Block::isExceptionForPistonAttaching(supportState->getBlock());
    bool exception = Block::isExceptionBlockForAttaching(supportState->getBlock()) && !pistonException;
    return !exception && direction->getAxis()->isHorizontal()
           && (supportState->getBlockFaceShape(level, supportPos, direction) == BLOCK_FACE_SOLID || pistonException)
           && !supportState->isSignalSource();
}

bool TripWireHookBlock::mayPlace(Level* level, const BlockPos& pos)
{
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        if (mayPlace(level, pos, directions.data[i]))
            return true;
    }
    return false;
}

const BlockState* TripWireHookBlock::getStateForPlacement(Level*, const BlockPos&, const Direction* direction, float,
                                                          float, float, int, boost::shared_ptr<LivingEntity>)
{
    const BlockState* state = defaultBlockState()->setValue(POWERED, false)->setValue(ATTACHED, false);
    if (direction->getAxis()->isHorizontal())
    {
        state = state->setValue(FACING, direction);
    }
    return state;
}

void TripWireHookBlock::notifyNeighbors(Level* level, const BlockPos& pos, const Direction* direction)
{
    level->updateNeighborsAt(pos, this, false);
    level->updateNeighborsAt(pos.relative(direction->getOpposite()), this, false);
}

void TripWireHookBlock::playSound(Level* level, const BlockPos& pos, bool attached, bool powered, bool oldAttached,
                                  bool oldPowered)
{
    if (powered)
    {
        if (!oldPowered)
        {
            level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::TRIPWIRE_CLICK_ON,
                             SoundSource::eSoundSource_Blocks, 0.4f, 0.6f, 16.0f);
            return;
        }
    }
    else if (oldPowered)
    {
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::TRIPWIRE_CLICK_OFF,
                         SoundSource::eSoundSource_Blocks, 0.4f, 0.5f, 16.0f);
        return;
    }

    if (attached)
    {
        if (!oldAttached)
        {
            level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::TRIPWIRE_ATTACH,
                             SoundSource::eSoundSource_Blocks, 0.4f, 0.7f, 16.0f);
            return;
        }
    }
    else if (oldAttached)
    {
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::TRIPWIRE_DETACH,
                         SoundSource::eSoundSource_Blocks, 0.4f, 1.2f / (level->mRandom->nextFloat() * 0.2f + 0.9f),
                         16.0f);
        return;
    }
}

void TripWireHookBlock::calculateState(Level* level, const BlockPos& pos, const BlockState* state,
                                       bool isBeingDestroyed, bool canUpdate, int wireSource,
                                       const BlockState* wireSourceState)
{
    const Direction* facing = state->getValue(FACING);
    bool wasAttached = state->getValue(ATTACHED);
    bool wasPowered = state->getValue(POWERED);
    bool attached = !isBeingDestroyed;
    bool powered = false;
    int receiverPos = 0;
    const BlockState* wireStates[42];

    for (int i = 1; i < 42; ++i)
    {
        BlockPos testPos = pos.relative(facing, i);
        const BlockState* testState = level->getBlockState(testPos);
        if (testState->getBlock() == Blocks::tripwire_hook)
        {
            if (testState->getValue(FACING) == facing->getOpposite())
                receiverPos = i;
            break;
        }

        if (testState->getBlock() == Blocks::tripwire || i == wireSource)
        {
            if (i == wireSource && wireSourceState != NULL)
                testState = wireSourceState;
            bool armed = !testState->getValue(TripWireBlock::SUSPENDED);
            bool wirePowered = testState->getValue(TripWireBlock::POWERED);
            bool active = false;
            if (armed && wirePowered)
                active = true;
            wireStates[i] = testState;
            powered = powered || active;
            if (i == wireSource)
            {
                level->addToTickNextTick(pos, this, getTickDelay(level));
                attached &= armed;
            }
        }
        else
        {
            wireStates[i] = NULL;
            attached = false;
        }
    }

    attached &= receiverPos > 1;
    powered &= attached;
    const BlockState* newState = defaultBlockState()->setValue(ATTACHED, attached)->setValue(POWERED, powered);
    if (receiverPos > 0)
    {
        BlockPos receiver = pos.relative(facing, receiverPos);
        const Direction* opposite = facing->getOpposite();
        level->setBlock(receiver, newState->setValue(FACING, opposite), Block::UPDATE_ALL, false);
        notifyNeighbors(level, receiver, opposite);
        playSound(level, receiver, attached, powered, wasAttached, wasPowered);
    }

    playSound(level, pos, attached, powered, wasAttached, wasPowered);
    if (!isBeingDestroyed)
    {
        level->setBlock(pos, newState->setValue(FACING, facing), Block::UPDATE_ALL, false);
        if (canUpdate)
            notifyNeighbors(level, pos, facing);
    }

    if (wasAttached != attached)
    {
        for (int i = 1; i < receiverPos; ++i)
        {
            BlockPos wirePos = pos.relative(facing, i);
            const BlockState* wireState = wireStates[i];
            if (wireState != NULL && level->getBlockState(wirePos)->getMaterial() != Material::AIR)
            {
                level->setBlock(wirePos, wireState->setValue(TripWireBlock::ATTACHED, attached), Block::UPDATE_ALL,
                                false);
            }
        }
    }
}

void TripWireHookBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                    boost::shared_ptr<LivingEntity>, not_null_ptr<ItemInstance>)
{
    calculateState(level, pos, state, false, false, -1, NULL);
}

bool TripWireHookBlock::checkCanSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!mayPlace(level, pos))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
        return false;
    }
    return true;
}

void TripWireHookBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                        const BlockPos&)
{
    if (block != this && checkCanSurvive(level, pos, state))
    {
        const Direction* facing = state->getValue(FACING);
        if (!mayPlace(level, pos, facing))
        {
            spawnResources(level, pos, state, 0);
            level->removeBlock(pos);
        }
    }
}

void TripWireHookBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*)
{
}
void TripWireHookBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    calculateState(level, pos, state, false, true, -1, NULL);
}

void TripWireHookBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    bool attached = state->getValue(ATTACHED);
    bool powered = state->getValue(POWERED);
    if (attached || powered)
    {
        calculateState(level, pos, state, true, false, -1, NULL);
        if (powered)
        {
            level->updateNeighborsAt(pos, this, false);
            level->updateNeighborsAt(pos.relative(state->getValue(FACING)->getOpposite()), this, false);
        }
    }
    Block::onRemove(level, pos, state);
}

int TripWireHookBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    if (state->getValue(POWERED))
        return Block::SIGNAL_MAX;
    return Block::SIGNAL_NONE;
}
int TripWireHookBlock::getDirectSignal(const BlockState* state, LevelSource*, const BlockPos&,
                                       const Direction* direction)
{
    return state->getValue(POWERED) && direction == state->getValue(FACING) ? Block::SIGNAL_MAX : Block::SIGNAL_NONE;
}
bool TripWireHookBlock::isSignalSource(const BlockState*)
{
    return true;
}
int TripWireHookBlock::getRenderLayer()
{
    return 0;
}

const BlockState* TripWireHookBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(FACING, Direction::from2DDataValue(data & 3))
        ->setValue(POWERED, (data & 8) > 0)
        ->setValue(ATTACHED, (data & 4) > 0);
}

int TripWireHookBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->get2DDataValue();
    if (state->getValue(POWERED))
        data |= 8;
    if (state->getValue(ATTACHED))
        data |= 4;
    return data;
}

const BlockState* TripWireHookBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* TripWireHookBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* TripWireHookBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING, POWERED, ATTACHED, SUSPENDED};
    return new BlockStateDefinition(this, properties, 4);
}

EBlockFaceShape TripWireHookBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}
EBlockRenderShape TripWireHookBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_TRIPWIRE_SOURCE;
}
