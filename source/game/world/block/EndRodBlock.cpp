#include "game/world/block/EndRodBlock.hpp"
#include "game/world/block/Blocks.hpp"

#include "game/client/render/ParticleType.hpp"
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
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const AABB* END_ROD_SHAPE_Y = AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 1.0, 0.625);
static const AABB* END_ROD_SHAPE_Z = AABB::newPermanent(0.375, 0.375, 0.0, 0.625, 0.625, 1.0);
static const AABB* END_ROD_SHAPE_X = AABB::newPermanent(0.0, 0.375, 0.375, 1.0, 0.625, 0.625);

static inline const AABB* endRodPart(double x0, double y0, double z0, double x1, double y1, double z1)
{
    return AABB::newPermanent(x0, y0, z0, x1, y1, z1)->divideInternalsBy(16.0);
}

static const AABB* END_ROD_UP_ROD = endRodPart(7.0, 1.0, 7.0, 9.0, 16.0, 9.0);
static const AABB* END_ROD_UP_BASE = endRodPart(6.0, 0.0, 6.0, 10.0, 1.0, 10.0);
static const AABB* END_ROD_DOWN_ROD = endRodPart(7.0, 0.0, 7.0, 9.0, 15.0, 9.0);
static const AABB* END_ROD_DOWN_BASE = endRodPart(6.0, 15.0, 6.0, 10.0, 16.0, 10.0);
static const AABB* END_ROD_SOUTH_ROD = endRodPart(7.0, 7.0, 1.0, 9.0, 9.0, 16.0);
static const AABB* END_ROD_SOUTH_BASE = endRodPart(6.0, 6.0, 0.0, 10.0, 10.0, 1.0);
static const AABB* END_ROD_NORTH_ROD = endRodPart(7.0, 7.0, 0.0, 9.0, 9.0, 15.0);
static const AABB* END_ROD_NORTH_BASE = endRodPart(6.0, 6.0, 15.0, 10.0, 10.0, 16.0);
static const AABB* END_ROD_EAST_ROD = endRodPart(1.0, 7.0, 7.0, 16.0, 9.0, 9.0);
static const AABB* END_ROD_EAST_BASE = endRodPart(0.0, 6.0, 6.0, 1.0, 10.0, 10.0);
static const AABB* END_ROD_WEST_ROD = endRodPart(0.0, 7.0, 7.0, 15.0, 9.0, 9.0);
static const AABB* END_ROD_WEST_BASE = endRodPart(15.0, 6.0, 6.0, 16.0, 10.0, 10.0);

EndRodBlock::EndRodBlock() : DirectionalBlock(Material::DECORATION)
{
    DerivedInit();
}

void EndRodBlock::DerivedInit()
{
    Block::DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(DirectionalBlock::FACING_PROPERTY, Direction::UP));
}

const BlockState* EndRodBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(DirectionalBlock::FACING_PROPERTY,
                           rotation->rotate(state->getTypedBoxed(DirectionalBlock::FACING_PROPERTY)->getValue()));
}

const BlockState* EndRodBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return state->setValue(DirectionalBlock::FACING_PROPERTY,
                           mirror->mirror(state->getTypedBoxed(DirectionalBlock::FACING_PROPERTY)->getValue()));
}

const AABB* EndRodBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(DirectionalBlock::FACING_PROPERTY)->getAxis()->getId())
    {
    case Direction::Axis::eAxis_X:
    default:
        return END_ROD_SHAPE_X;
    case Direction::Axis::eAxis_Z:
        return END_ROD_SHAPE_Z;
    case Direction::Axis::eAxis_Y:
        return END_ROD_SHAPE_Y;
    }
}

bool EndRodBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool EndRodBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool EndRodBlock::mayPlace(Level*, const BlockPos&)
{
    return true;
}

const BlockState* EndRodBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                    float, float, float, int, boost::shared_ptr<LivingEntity>)
{
    const BlockState* neighbor = level->getBlockState(pos.relative(direction->getOpposite()));
    if (neighbor->getBlock() == Blocks::end_rod)
    {
        const Direction* neighborFacing = neighbor->getValue(DirectionalBlock::FACING_PROPERTY);
        if (neighborFacing == direction)
        {
            return defaultBlockState()->setValue(DirectionalBlock::FACING_PROPERTY, direction->getOpposite());
        }
    }
    return defaultBlockState()->setValue(DirectionalBlock::FACING_PROPERTY, direction);
}

void EndRodBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    const Direction* direction = state->getValue(DirectionalBlock::FACING_PROPERTY);
    double x = static_cast<double>(pos.x) + 0.55 - static_cast<double>(random->nextFloat() * 0.1F);
    double y = static_cast<double>(pos.y) + 0.55 - static_cast<double>(random->nextFloat() * 0.1F);
    double z = static_cast<double>(pos.z) + 0.55 - static_cast<double>(random->nextFloat() * 0.1F);
    double spread = 0.4F - (random->nextFloat() + random->nextFloat()) * 0.4F;
    if (random->nextInt(5) == 0)
    {
        level->addParticle(PARTICLE_END_ROD, x + static_cast<double>(direction->getStepX()) * spread,
                           y + static_cast<double>(direction->getStepY()) * spread,
                           z + static_cast<double>(direction->getStepZ()) * spread, random->nextGaussian() * 0.005,
                           random->nextGaussian() * 0.005, random->nextGaussian() * 0.005, arrayWithLength<int>());
    }
}

int EndRodBlock::getRenderLayer()
{
    return 0;
}

const BlockState* EndRodBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(DirectionalBlock::FACING_PROPERTY, Direction::from3DDataValue(data));
}

int EndRodBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(DirectionalBlock::FACING_PROPERTY)->get3DDataValue();
}

EPistonPushReaction EndRodBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_NORMAL;
}
EBlockFaceShape EndRodBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}
EBlockRenderShape EndRodBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_END_ROD;
}

const AABB* EndRodBlock::getShapeRod(const BlockState* state)
{
    switch (state->getValue(DirectionalBlock::FACING_PROPERTY)->get3DDataValue())
    {
    case Direction::eDirection_Up:
        return END_ROD_UP_ROD;
    default:
        return END_ROD_UP_ROD;
    case Direction::eDirection_Down:
        return END_ROD_DOWN_ROD;
    case Direction::eDirection_East:
        return END_ROD_EAST_ROD;
    case Direction::eDirection_West:
        return END_ROD_WEST_ROD;
    case Direction::eDirection_North:
        return END_ROD_NORTH_ROD;
    case Direction::eDirection_South:
        return END_ROD_SOUTH_ROD;
    }
}

const AABB* EndRodBlock::getShapeBase(const BlockState* state)
{
    switch (state->getValue(DirectionalBlock::FACING_PROPERTY)->get3DDataValue())
    {
    case Direction::eDirection_Up:
        return END_ROD_UP_BASE;
    default:
        return END_ROD_UP_BASE;
    case Direction::eDirection_Down:
        return END_ROD_DOWN_BASE;
    case Direction::eDirection_East:
        return END_ROD_EAST_BASE;
    case Direction::eDirection_West:
        return END_ROD_WEST_BASE;
    case Direction::eDirection_North:
        return END_ROD_NORTH_BASE;
    case Direction::eDirection_South:
        return END_ROD_SOUTH_BASE;
    }
}

BlockStateDefinition* EndRodBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {DirectionalBlock::FACING_PROPERTY};
    return new BlockStateDefinition(this, properties, 1);
}

int EndRodBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return Direction::UP->get3DDataValue();
}
