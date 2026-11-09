#include "game/world/block/LadderBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty* LadderBlock::FACING = NULL;

static const AABB* LADDER_SHAPE_EAST = AABB::newPermanent(0.0, 0.0, 0.0, 0.125, 1.0, 1.0);
static const AABB* LADDER_SHAPE_WEST = AABB::newPermanent(0.875, 0.0, 0.0, 1.0, 1.0, 1.0);
static const AABB* LADDER_SHAPE_SOUTH = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.125);
static const AABB* LADDER_SHAPE_NORTH = AABB::newPermanent(0.0, 0.0, 0.875, 1.0, 1.0, 1.0);

LadderBlock::LadderBlock() : Block(Material::DECORATION)
{
    DerivedInit();
    mWaterBlocking = false;
}

void LadderBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

const AABB* LadderBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    const Direction* direction = state->getValue(FACING);
    switch (direction->get3DDataValue())
    {
    case Direction::eDirection_North:
        return LADDER_SHAPE_NORTH;
    case Direction::eDirection_South:
        return LADDER_SHAPE_SOUTH;
    case Direction::eDirection_West:
        return LADDER_SHAPE_WEST;
    default:
        return LADDER_SHAPE_EAST;
    }
}

bool LadderBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool LadderBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool LadderBlock::canAttachTo(Level* level, const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    return !Block::isExceptionBlockForAttachingPlusPiston(block)
           && state->getBlockFaceShape(level, pos, direction) == BLOCK_FACE_SOLID && !state->isSignalSource();
}

bool LadderBlock::mayPlace(Level* level, const BlockPos& pos, const Direction* direction)
{
    if (canAttachTo(level, pos.west(), direction) || canAttachTo(level, pos.east(), direction)
        || canAttachTo(level, pos.north(), direction) || canAttachTo(level, pos.south(), direction))
        return true;
    return false;
}

bool LadderBlock::canContainLiquid(const BlockState*) const
{
    return true;
}
bool LadderBlock::waterSpreadCausesSpawn()
{
    return false;
}

const BlockState* LadderBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                    float, float, float, int, boost::shared_ptr<LivingEntity>)
{
    if (direction->getAxis()->isHorizontal() && canAttachTo(level, pos.relative(direction->getOpposite()), direction))
    {
        return defaultBlockState()->setValue(FACING, direction);
    }

    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const Direction* candidate = directions.data[i];
        if (canAttachTo(level, pos.relative(candidate->getOpposite()), candidate))
        {
            return defaultBlockState()->setValue(FACING, candidate);
        }
    }
    return defaultBlockState();
}

void LadderBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                  const BlockPos& fromPos)
{
    const Direction* direction = state->getValue(FACING);
    if (!canAttachTo(level, pos.relative(direction->getOpposite()), direction))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
    Block::neighborChanged(state, level, pos, block, fromPos);
}

int LadderBlock::getRenderLayer()
{
    return 0;
}

const BlockState* LadderBlock::getBlockState(int data)
{
    const Direction* direction = Direction::from3DDataValue(data);
    if (direction->getAxis() == Direction::Axis::Y)
        direction = Direction::NORTH;
    return defaultBlockState()->setValue(FACING, direction);
}

int LadderBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get3DDataValue();
}

const BlockState* LadderBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* LadderBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* LadderBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING};
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape LadderBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape LadderBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_LADDER;
}
