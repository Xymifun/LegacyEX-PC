#include "game/server/ObserverBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"

AbstractProperty<bool>* ObserverBlock::POWERED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"powered"));

ObserverBlock::ObserverBlock() : DirectionalBlock(Material::STONE)
{
    Block::DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue<const Direction*>(DirectionalBlock::FACING_PROPERTY, Direction::SOUTH)
                             ->setValue<bool>(POWERED, false));
}

bool ObserverBlock::TestUse()
{
    return false;
}

BlockStateDefinition* ObserverBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {DirectionalBlock::FACING_PROPERTY, POWERED};
    return new BlockStateDefinition(this, properties, 2);
}

const BlockState* ObserverBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue<const Direction*>(
        DirectionalBlock::FACING_PROPERTY,
        rotation->rotate(state->getTypedBoxed(DirectionalBlock::FACING_PROPERTY)->getValue()));
}

const BlockState* ObserverBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return state->rotate(mirror->getRotation(state->getValue(DirectionalBlock::FACING_PROPERTY)));
}

void ObserverBlock::updateNeighborsInFront(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* facing = state->getValue(DirectionalBlock::FACING_PROPERTY);
    BlockPos front = pos.relative(facing->getOpposite());
    level->neighborChanged(front, this, pos);
    level->updateNeighborsAtExceptFromFacing(front, this, facing);
}

void ObserverBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (state->getValue(POWERED))
    {
        level->setBlock(pos, state->setValue<bool>(POWERED, false), Block::UPDATE_CLIENTS, false);
    }
    else
    {
        level->setBlock(pos, state->setValue<bool>(POWERED, true), Block::UPDATE_CLIENTS, false);
        level->addToTickNextTick(pos, this, 2);
    }
    updateNeighborsInFront(level, pos, state);
}

void ObserverBlock::neighborChanged(const BlockState*, Level*, const BlockPos&, Block*, const BlockPos&)
{
}

void ObserverBlock::startSignal(const BlockState* state, Level* level, const BlockPos& pos)
{
    if (!state->getValue(POWERED) && !level->hasTickNextTick(pos, this))
        level->addToTickNextTick(pos, this, 2);
}

void ObserverBlock::observedNeighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*,
                                            const BlockPos& fromPos)
{
    if (!level->mIsClientSide)
    {
        BlockPos front = pos.relative(state->getValue(DirectionalBlock::FACING_PROPERTY));
        if (front.equals(fromPos))
            startSignal(state, level, pos);
    }
}

bool ObserverBlock::isSignalSource(const BlockState*)
{
    return true;
}

int ObserverBlock::getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                   const Direction* direction)
{
    return state->getSignal(levelSource, pos, direction);
}

int ObserverBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction* direction)
{
    if (state->getValue(POWERED) && state->getValue(DirectionalBlock::FACING_PROPERTY) == direction)
        return Block::SIGNAL_MAX;
    return Block::SIGNAL_NONE;
}

void ObserverBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->mIsClientSide)
        return;
    if (state->getValue(POWERED))
        tick(level, pos, state, level->mRandom);
    startSignal(state, level, pos);
}

void ObserverBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (state->getValue(POWERED) && level->hasTickNextTick(pos, this))
    {
        updateNeighborsInFront(level, pos, state->setValue<bool>(POWERED, false));
    }
}

const BlockState* ObserverBlock::getStateForPlacement(Level*, const BlockPos& pos, const Direction*, float, float,
                                                      float, int, boost::shared_ptr<LivingEntity> placer)
{
    return defaultBlockState()->setValue<const Direction*>(
        DirectionalBlock::FACING_PROPERTY, Direction::getDirectionFromLivingEntity(pos, placer)->getOpposite());
}

int ObserverBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(DirectionalBlock::FACING_PROPERTY)->get3DDataValue();
    if (state->getValue(POWERED))
        data |= 8;
    return data;
}

const BlockState* ObserverBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue<const Direction*>(DirectionalBlock::FACING_PROPERTY, Direction::from3DDataValue(data & 7))
        ->setValue<bool>(POWERED, (data & 8) > 0);
}

TextureAtlasSprite* ObserverBlock::getTexture(const Direction* direction, const BlockState* state)
{
    const Direction* facing = state->getValue(DirectionalBlock::FACING_PROPERTY);
    if (direction == facing)
        return mFrontIcon;
    if (direction != facing->getOpposite())
    {
        if (facing == Direction::UP || facing == Direction::DOWN)
        {
            if (direction == Direction::NORTH || direction == Direction::SOUTH)
                return mTopIcon;
        }
        else if (direction == Direction::UP || direction == Direction::DOWN)
        {
            return mTopIcon;
        }
        return mSideIcon;
    }
    return state->getValue(POWERED) ? mBackLitIcon : mBackIcon;
}

void ObserverBlock::registerIcons(IconRegister* iconRegister)
{
    mSideIcon = iconRegister->registerIcon(L"observer_side");
    mTopIcon = iconRegister->registerIcon(L"observer_top");
    mFrontIcon = iconRegister->registerIcon(L"observer_front");
    mBackIcon = iconRegister->registerIcon(L"observer_back");
    mBackLitIcon = iconRegister->registerIcon(L"observer_back_lit");
}

EBlockRenderShape ObserverBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_OBSERVER;
}
