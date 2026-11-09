#include "game/world/block/WallSignBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty* WallSignBlock::FACING = NULL;

static const AABB* WALL_SIGN_SHAPE_EAST = AABB::newPermanent(0.0, 0.25, 0.0, 0.125, 0.75, 1.0);
static const AABB* WALL_SIGN_SHAPE_WEST = AABB::newPermanent(0.875, 0.25, 0.0, 1.0, 0.75, 1.0);
static const AABB* WALL_SIGN_SHAPE_SOUTH = AABB::newPermanent(0.0, 0.25, 0.0, 1.0, 0.75, 0.125);
static const AABB* WALL_SIGN_SHAPE_NORTH = AABB::newPermanent(0.0, 0.25, 0.875, 1.0, 0.75, 1.0);

WallSignBlock::WallSignBlock() : SignBlock()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(FACING, Direction::NORTH));
}

void WallSignBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

const AABB* WallSignBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(FACING)->get3DDataValue())
    {
    case Direction::eDirection_North:
    default:
        return WALL_SIGN_SHAPE_NORTH;
    case Direction::eDirection_South:
        return WALL_SIGN_SHAPE_SOUTH;
    case Direction::eDirection_West:
        return WALL_SIGN_SHAPE_WEST;
    case Direction::eDirection_East:
        return WALL_SIGN_SHAPE_EAST;
    }
}

void WallSignBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                    const BlockPos& fromPos)
{
    const Direction* facing = state->getValue(FACING);
    const BlockState* support = level->getBlockState(pos.relative(facing->getOpposite()));
    if (!support->getBlock()->getCraftingMaterial()->isSolid())
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
    Block::neighborChanged(state, level, pos, block, fromPos);
}

const BlockState* WallSignBlock::getBlockState(int data)
{
    const Direction* facing = Direction::from3DDataValue(data);
    if (facing->getAxis() == Direction::Axis::Y)
        facing = Direction::NORTH;
    return defaultBlockState()->setValue(FACING, facing);
}

int WallSignBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get3DDataValue();
}

const BlockState* WallSignBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* WallSignBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* WallSignBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING};
    return new BlockStateDefinition(this, properties, 1);
}
