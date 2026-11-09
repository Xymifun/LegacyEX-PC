#include "game/world/block/WallBannerBlock.hpp"

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
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const AABB* WALL_BANNER_SHAPE_NORTH = AABB::newPermanent(0.0, 0.0, 0.75, 1.0, 1.0, 1.0);
static const AABB* WALL_BANNER_SHAPE_SOUTH = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.25);
static const AABB* WALL_BANNER_SHAPE_WEST = AABB::newPermanent(0.75, 0.0, 0.0, 1.0, 1.0, 1.0);
static const AABB* WALL_BANNER_SHAPE_EAST = AABB::newPermanent(0.0, 0.0, 0.0, 0.25, 1.0, 1.0);

WallBannerBlock::WallBannerBlock() : BannerBlock()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(BannerBlock::FACING, Direction::NORTH));
}

const BlockState* WallBannerBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* WallBannerBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = BannerBlock::FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

const AABB* WallBannerBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    const Direction* direction = state->getValue(BannerBlock::FACING);
    switch (direction->get3DDataValue())
    {
    case 2:
    default:
        return WALL_BANNER_SHAPE_NORTH;
    case 3:
        return WALL_BANNER_SHAPE_SOUTH;
    case 4:
        return WALL_BANNER_SHAPE_WEST;
    case 5:
        return WALL_BANNER_SHAPE_EAST;
    }
}

void WallBannerBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                      const BlockPos& fromPos)
{
    const Direction* direction = state->getValue(BannerBlock::FACING);
    if (!level->getBlockState(pos.relative(direction->getOpposite()))->getMaterial()->isSolid())
    {
        static_cast<Block*>(this)->spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
    Block::neighborChanged(state, level, pos, block, fromPos);
}

const BlockState* WallBannerBlock::getBlockState(int data)
{
    const Direction* direction = Direction::from3DDataValue(data);
    if (direction->getAxis() == Direction::Axis::Y)
        direction = Direction::NORTH;
    return defaultBlockState()->setValue(BannerBlock::FACING, direction);
}

int WallBannerBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(BannerBlock::FACING)->get3DDataValue();
}

BlockStateDefinition* WallBannerBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {BannerBlock::FACING};
    return new BlockStateDefinition(this, properties, 1);
}
