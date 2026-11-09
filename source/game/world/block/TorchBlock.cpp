#include "game/world/block/TorchBlock.hpp"

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
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/FenceBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty* TorchBlock::FACING = NULL;

static const AABB* TORCH_SHAPE = AABB::newPermanent(0.4000000059604645, 0.0, 0.4000000059604645, 0.6000000238418579,
                                                    0.6000000238418579, 0.6000000238418579);
static const AABB* TORCH_SHAPE_NORTH = AABB::newPermanent(0.3499999940395355, 0.20000000298023224, 0.699999988079071,
                                                          0.6499999761581421, 0.800000011920929, 1.0);
static const AABB* TORCH_SHAPE_SOUTH = AABB::newPermanent(0.3499999940395355, 0.20000000298023224, 0.0,
                                                          0.6499999761581421, 0.800000011920929, 0.30000001192092896);
static const AABB* TORCH_SHAPE_WEST = AABB::newPermanent(0.699999988079071, 0.20000000298023224, 0.3499999940395355,
                                                         1.0, 0.800000011920929, 0.6499999761581421);
static const AABB* TORCH_SHAPE_EAST = AABB::newPermanent(0.0, 0.20000000298023224, 0.3499999940395355,
                                                         0.30000001192092896, 0.800000011920929, 0.6499999761581421);

TorchBlock::TorchBlock() : Block(Material::DECORATION)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(FACING, Direction::UP));
    setTicking(true);
}

void TorchBlock::blockStaticCtor()
{
    FACING = DirectionProperty::create(
        L"facing", new Predicates<const Direction*>::FunctionPredicate(DirectionProperty::NOT_DOWN));
}

int TorchBlock::getSpawnResourcesAuxValue(const BlockState*)
{
    return 5;
}

const AABB* TorchBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(FACING)->get3DDataValue())
    {
    case 5:
        return TORCH_SHAPE_EAST;
    case 4:
        return TORCH_SHAPE_WEST;
    case 3:
        return TORCH_SHAPE_SOUTH;
    case 2:
        return TORCH_SHAPE_NORTH;
    default:
        return TORCH_SHAPE;
    }
}

const AABB* TorchBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL_AABB;
}
bool TorchBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool TorchBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool TorchBlock::waterSpreadCausesSpawn()
{
    return true;
}
bool TorchBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

bool TorchBlock::canStandOn(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlockState(pos)->getBlock();
    bool isEndGateway = block == Blocks::end_gateway;
    if (level->getBlockState(pos)->isTopSolidBlocking())
        return !isEndGateway;

    bool canStand = dynamic_cast<FenceBlock*>(block) != NULL || block == Blocks::sea_lantern
                    || block == Blocks::glowstone || block == Blocks::ice || block == Blocks::beacon
                    || block == Blocks::glass || block == Blocks::cobblestone_wall || block == Blocks::stained_glass;
    return canStand && !isEndGateway;
}

bool TorchBlock::canBePlacedOn(Level* level, const BlockPos& pos, const Direction* direction)
{
    BlockPos supportPos = pos.relative(direction->getOpposite());
    const BlockState* supportState = level->getBlockState(supportPos);
    Block* support = supportState->getBlock();
    EBlockFaceShape faceShape = supportState->getBlockFaceShape(level, supportPos, direction);
    if (direction == Direction::UP && canStandOn(level, supportPos))
        return true;
    if (direction == Direction::UP || direction == Direction::DOWN)
        return false;
    return !Block::isExceptionBlockForAttachingPlusPiston(support) && faceShape == BLOCK_FACE_SOLID;
}

bool TorchBlock::mayPlace(Level* level, const BlockPos& pos)
{
    Material* material = level->getBlockState(pos)->getMaterial();
    Material* extraMaterial = level->getExtraBlockState(pos)->getMaterial();
    if (material == Material::WATER || material == Material::LAVA || material == Material::BUBBLE_COLUMN
        || extraMaterial == Material::WATER)
        return false;
    boost::unordered_set<const Direction*> possibleValues(11);
    FACING->getUnboxedPossibleValues(possibleValues);
    for (boost::unordered_set<const Direction*>::const_iterator it = possibleValues.begin(); it != possibleValues.end();
         ++it)
    {
        if (canBePlacedOn(level, pos, *it))
            return true;
    }
    return false;
}

const BlockState* TorchBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float,
                                                   float, float, int, boost::shared_ptr<LivingEntity>)
{
    if (canBePlacedOn(level, pos, direction))
        return defaultBlockState()->setValue(FACING, direction);
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const Direction* candidate = directions.data[i];
        if (canBePlacedOn(level, pos, candidate))
            return defaultBlockState()->setValue(FACING, candidate);
    }
    return defaultBlockState();
}

void TorchBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    checkCanSurvive(level, pos, state);
}

void TorchBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    checkDoPop(level, pos, state);
}

bool TorchBlock::checkDoPop(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!checkCanSurvive(level, pos, state))
        return true;

    const Direction* direction = state->getValue(FACING);
    Direction::Axis* axis = direction->getAxis();
    BlockPos supportPos = pos.relative(direction->getOpposite());
    if ((axis->isHorizontal()
         && level->getBlockState(supportPos)->getBlockFaceShape(level, supportPos, direction) != BLOCK_FACE_SOLID)
        || (axis->isVertical() && !canStandOn(level, supportPos)))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
        return true;
    }
    return false;
}

bool TorchBlock::checkCanSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (state->getBlock() == this && canBePlacedOn(level, pos, state->getValue(FACING)))
        return true;
    if (level->getBlockState(pos)->getBlock() == this)
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
    return false;
}

void TorchBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random*)
{
    const Direction* direction = state->getValue(FACING);
    double x = pos.x + 0.5;
    double y = pos.y + 0.7;
    double z = pos.z + 0.5;
    double heightOffset = 0.22;
    double wallOffset = 0.27;
    if (direction->getAxis()->isHorizontal())
    {
        const Direction* opposite = direction->getOpposite();
        level->addParticle(PARTICLE_SMOKE, x + wallOffset * opposite->getStepX(), y + heightOffset,
                           z + wallOffset * opposite->getStepZ(), 0.0, 0.0, 0.0, arrayWithLength<int>());
        level->addParticle(PARTICLE_FLAME, x + wallOffset * opposite->getStepX(), y + heightOffset,
                           z + wallOffset * opposite->getStepZ(), 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
    else
    {
        level->addParticle(PARTICLE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
        level->addParticle(PARTICLE_FLAME, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
}

int TorchBlock::getRenderLayer()
{
    return 0;
}

const BlockState* TorchBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    switch (data)
    {
    case 1:
        state = state->setValue(FACING, Direction::EAST);
        break;
    case 2:
        state = state->setValue(FACING, Direction::WEST);
        break;
    case 3:
        state = state->setValue(FACING, Direction::SOUTH);
        break;
    case 4:
        state = state->setValue(FACING, Direction::NORTH);
        break;
    default:
        state = state->setValue(FACING, Direction::UP);
        break;
    }
    return state;
}

int TorchBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    const Direction* direction = state->getValue(FACING);
    int data;
    switch (direction->get3DDataValue())
    {
    case 2:
        data = 4;
        break;
    case 3:
        data = 3;
        break;
    case 4:
        data = 2;
        break;
    case 5:
        data = 1;
        break;
    default:
        data = 5;
        break;
    }
    return data;
}

const BlockState* TorchBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* TorchBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* TorchBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING};
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape TorchBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape TorchBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_TORCH;
}
