#include "game/world/block/DiodeBlock.hpp"

#include <algorithm>

#include "game/core/Direction.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/RedStoneWireBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"

static const AABB* gDiodeShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.125, 1.0);

DiodeBlock::DiodeBlock(bool on) : HorizontalDirectionalBlock(Material::DECORATION), mOn(on)
{
}
const AABB* DiodeBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gDiodeShape;
}
bool DiodeBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool DiodeBlock::mayPlace(Level* level, const BlockPos& pos)
{
    if (level->getBlockState(pos.below())->isTopSolidBlocking())
    {
        return Block::mayPlace(level, pos);
    }
    return false;
}

bool DiodeBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    return level->getBlockState(pos.below())->isTopSolidBlocking();
}

void DiodeBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*)
{
}

void DiodeBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!isLocked(level, pos, state))
    {
        bool turnOn = shouldTurnOn(level, pos, state);
        if (mOn && !turnOn)
        {
            level->setBlock(pos, getOffBlock(state), Block::UPDATE_CLIENTS, false);
        }
        else if (!mOn)
        {
            level->setBlock(pos, getOnBlock(state), Block::UPDATE_CLIENTS, false);
            if (!turnOn)
            {
                level->addToTickNextTick(pos, getOnBlock(state)->getBlock(), getTurnOffDelay(state), -1);
            }
        }
    }
}

bool DiodeBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction* direction)
{
    return direction->getAxis() != Direction::Axis::Y;
}

bool DiodeBlock::isOn(const BlockState*)
{
    return mOn;
}

int DiodeBlock::getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction)
{
    return state->getSignal(levelSource, pos, direction);
}

int DiodeBlock::getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction)
{
    if (isOn(state) && state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY) == direction)
        return getOutputSignal(levelSource, pos, state);
    return Block::SIGNAL_NONE;
}

void DiodeBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (canSurvive(level, pos, NULL))
    {
        checkTickOnNeighbor(level, pos, state);
    }
    else
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
        for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
             ++it)
        {
            level->updateNeighborsAt(pos.relative(*it), this, false);
        }
    }
}

void DiodeBlock::checkTickOnNeighbor(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (isLocked(level, pos, state))
    {
        return;
    }
    bool turnOn = shouldTurnOn(level, pos, state);
    if (mOn != turnOn && !level->isBlockToBeTickedAt(pos, this))
    {
        int priority = -1;
        if (shouldPrioritize(level, pos, state))
        {
            priority = -3;
        }
        else if (mOn)
        {
            priority = -2;
        }
        level->addToTickNextTick(pos, this, getTurnOnDelay(state), priority);
    }
}

bool DiodeBlock::isLocked(LevelSource*, const BlockPos&, const BlockState*)
{
    return false;
}
bool DiodeBlock::shouldTurnOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    return getInputSignal(level, pos, state) > Block::SIGNAL_NONE;
}

int DiodeBlock::getInputSignal(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    BlockPos inputPos = pos.relative(direction);
    int signal = level->getSignal(inputPos, direction);
    if (signal >= Block::SIGNAL_MAX)
    {
        return signal;
    }
    const BlockState* inputState = level->getBlockState(inputPos);
    return std::max(signal, inputState->getBlock() == Blocks::redstone_wire
                                ? inputState->getValue(RedStoneWireBlock::POWER)
                                : Block::SIGNAL_NONE);
}

int DiodeBlock::getAlternateSignal(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    const Direction* clockWise = direction->getClockWise();
    const Direction* counterClockWise = direction->getCounterClockWise();
    return std::max(getAlternateSignalAt(levelSource, pos.relative(clockWise), clockWise),
                    getAlternateSignalAt(levelSource, pos.relative(counterClockWise), counterClockWise));
}

int DiodeBlock::getAlternateSignalAt(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = levelSource->getBlockState(pos);
    Block* block = state->getBlock();
    if (isAlternateInput(state))
    {
        if (block == Blocks::redstone_block)
        {
            return Block::SIGNAL_MAX;
        }
        if (block == Blocks::redstone_wire)
        {
            return state->getValue(RedStoneWireBlock::POWER);
        }
        return levelSource->getDirectSignal(pos, direction);
    }
    return Block::SIGNAL_NONE;
}

bool DiodeBlock::isSignalSource(const BlockState*)
{
    return true;
}

const BlockState* DiodeBlock::getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float, float, int,
                                                   boost::shared_ptr<LivingEntity> placer)
{
    return defaultBlockState()->setValue(HorizontalDirectionalBlock::FACING_PROPERTY,
                                         placer->getDirection()->getOpposite());
}

void DiodeBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity>, not_null_ptr<ItemInstance>)
{
    if (shouldTurnOn(level, pos, state))
        level->addToTickNextTick(pos, this, 1);
}

void DiodeBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    updateNeighborsInFront(level, pos, state);
}

void DiodeBlock::updateNeighborsInFront(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    BlockPos front = pos.relative(direction->getOpposite());
    level->neighborChanged(front, this, pos);
    level->updateNeighborsAtExceptFromFacing(front, this, direction);
}

void DiodeBlock::destroy(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (mOn)
    {
        for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
             ++it)
        {
            level->updateNeighborsAt(pos.relative(*it), this, false);
        }
    }
    Block::destroy(level, pos, state);
}

bool DiodeBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool DiodeBlock::isAlternateInput(const BlockState* state)
{
    return state->isSignalSource();
}
int DiodeBlock::getOutputSignal(LevelSource*, const BlockPos&, const BlockState*)
{
    return Block::SIGNAL_MAX;
}

bool DiodeBlock::isDiode(const BlockState* state)
{
    return static_cast<DiodeBlock*>(Blocks::unpowered_repeater)->isSameDiode(state)
           || static_cast<DiodeBlock*>(Blocks::unpowered_comparator)->isSameDiode(state);
}

bool DiodeBlock::isSameDiode(const BlockState* state)
{
    Block* block = state->getBlock();
    return block == getOnBlock(defaultBlockState())->getBlock()
           || block == getOffBlock(defaultBlockState())->getBlock();
}

bool DiodeBlock::shouldPrioritize(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    const Direction* direction = facing->getOpposite();
    BlockPos neighborPos = pos.relative(direction);
    if (isDiode(level->getBlockState(neighborPos)))
    {
        return level->getBlockState(neighborPos)->getValue(HorizontalDirectionalBlock::FACING_PROPERTY) != direction;
    }
    return false;
}

int DiodeBlock::getTurnOffDelay(const BlockState* state)
{
    return getTurnOnDelay(state);
}
bool DiodeBlock::isMatching(Block* block)
{
    return isSameDiode(block->defaultBlockState());
}
int DiodeBlock::getRenderLayer()
{
    return 0;
}

EBlockFaceShape DiodeBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&,
                                              const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape DiodeBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_DIODE;
}

TextureAtlasSprite* DiodeBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::DOWN)
    {
        if (mOn)
        {
            return Blocks::redstone_torch->getTexture(direction);
        }
        return Blocks::unlit_redstone_torch->getTexture(direction);
    }
    if (direction == Direction::UP)
    {
        return mIcon;
    }
    return Blocks::stone_slab->getTexture(Direction::UP);
}
