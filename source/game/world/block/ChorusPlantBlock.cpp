#include "game/world/block/ChorusPlantBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* ChorusPlantBlock::NORTH_PROPERTY = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"north"));
AbstractProperty<bool>* ChorusPlantBlock::EAST_PROPERTY = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"east"));
AbstractProperty<bool>* ChorusPlantBlock::SOUTH_PROPERTY = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"south"));
AbstractProperty<bool>* ChorusPlantBlock::WEST_PROPERTY = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"west"));
AbstractProperty<bool>* ChorusPlantBlock::UP_PROPERTY = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"up"));
AbstractProperty<bool>* ChorusPlantBlock::DOWN_PROPERTY = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"down"));

static inline AABB* createChorusShape(double minX, double minY, double minZ, double maxX, double maxY, double maxZ)
{
    return AABB::newPermanent(minX, minY, minZ, maxX, maxY, maxZ)->divideInternalsBy(16.0);
}

static inline AABB* createTempChorusShape(double minX, double minY, double minZ, double maxX, double maxY, double maxZ)
{
    return AABB::newTemp(minX, minY, minZ, maxX, maxY, maxZ)->divideInternalsBy(16.0);
}

static const AABB* CHORUS_NORTH_SHAPE = createChorusShape(4.0, 4.0, 0.0, 12.0, 12.0, 4.0);
static const AABB* CHORUS_SOUTH_SHAPE = createChorusShape(4.0, 4.0, 12.0, 12.0, 12.0, 16.0);
static const AABB* CHORUS_EAST_SHAPE = createChorusShape(12.0, 4.0, 4.0, 16.0, 12.0, 12.0);
static const AABB* CHORUS_WEST_SHAPE = createChorusShape(0.0, 4.0, 4.0, 4.0, 12.0, 12.0);
static const AABB* CHORUS_CENTER_SHAPE = createChorusShape(4.0, 4.0, 4.0, 12.0, 12.0, 12.0);

ChorusPlantBlock::ChorusPlantBlock() : Block(Material::PLANT, MaterialColor::COLOR_PURPLE)
{
    DerivedInit();
}

void ChorusPlantBlock::DerivedInit()
{
    Block::DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(NORTH_PROPERTY, false)
                             ->setValue(EAST_PROPERTY, false)
                             ->setValue(SOUTH_PROPERTY, false)
                             ->setValue(WEST_PROPERTY, false)
                             ->setValue(UP_PROPERTY, false)
                             ->setValue(DOWN_PROPERTY, false));
}

const BlockState* ChorusPlantBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                                    const BlockPos& pos)
{
    Block* below = levelSource->getBlock(pos.below());
    Block* above = levelSource->getBlock(pos.above());
    Block* north = levelSource->getBlock(pos.north());
    Block* east = levelSource->getBlock(pos.east());
    Block* south = levelSource->getBlock(pos.south());
    Block* west = levelSource->getBlock(pos.west());
    const BlockState* result = state->setValue(DOWN_PROPERTY, below == this || below == Blocks::chorus_flower
                                                                  || below == Blocks::end_stone);
    result = result->setValue(UP_PROPERTY, above == this || above == Blocks::chorus_flower);
    result = result->setValue(NORTH_PROPERTY, north == this || north == Blocks::chorus_flower);
    result = result->setValue(EAST_PROPERTY, east == this || east == Blocks::chorus_flower);
    result = result->setValue(SOUTH_PROPERTY, south == this || south == Blocks::chorus_flower);
    return result->setValue(WEST_PROPERTY, west == this || west == Blocks::chorus_flower);
}

const AABB* ChorusPlantBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (levelSource != NULL)
        state = state->fillVirtualBlockStateProperties(levelSource, pos);
    float minX = state->getValue(WEST_PROPERTY) ? 0.0f : 0.1875f;
    float minY = state->getValue(DOWN_PROPERTY) ? 0.0f : 0.1875f;
    float minZ = state->getValue(NORTH_PROPERTY) ? 0.0f : 0.1875f;
    float maxX = state->getValue(EAST_PROPERTY) ? 1.0f : 0.8125f;
    float maxY = state->getValue(UP_PROPERTY) ? 1.0f : 0.8125f;
    float maxZ = state->getValue(SOUTH_PROPERTY) ? 1.0f : 0.8125f;
    return AABB::newTemp(minX, minY, minZ, maxX, maxY, maxZ);
}

void ChorusPlantBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos,
                                         const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                         boost::shared_ptr<Entity> entity, bool isActualState)
{
    if (!isActualState)
        state = state->fillVirtualBlockStateProperties(level, pos);
    addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.1875, 0.1875, 0.8125, 0.8125, 0.8125));
    if (state->getValue(WEST_PROPERTY))
        addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.0, 0.1875, 0.1875, 0.1875, 0.8125, 0.8125));
    if (state->getValue(EAST_PROPERTY))
        addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.8125, 0.1875, 0.1875, 1.0, 0.8125, 0.8125));
    if (state->getValue(UP_PROPERTY))
        addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.8125, 0.1875, 0.8125, 1.0, 0.8125));
    if (state->getValue(DOWN_PROPERTY))
        addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.0, 0.1875, 0.8125, 0.1875, 0.8125));
    if (state->getValue(NORTH_PROPERTY))
        addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.1875, 0.0, 0.8125, 0.8125, 0.1875));
    if (state->getValue(SOUTH_PROPERTY))
        addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.1875, 0.8125, 0.8125, 0.8125, 1.0));
}

int ChorusPlantBlock::convertBlockStateToLegacyData(const BlockState*)
{
    return 0;
}

void ChorusPlantBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!canSurvive(level, pos, NULL))
        level->destroyBlock(pos, true);
}

Item* ChorusPlantBlock::getResource(const BlockState*, Random*, int)
{
    return Items::chorusFruit;
}
int ChorusPlantBlock::getResourceCount(Random* random)
{
    return random->nextInt(2);
}
bool ChorusPlantBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool ChorusPlantBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool ChorusPlantBlock::mayPlace(Level* level, const BlockPos& pos)
{
    if (Block::mayPlace(level, pos))
        return canSurvive(level, pos, NULL);
    return false;
}

void ChorusPlantBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!canSurvive(level, pos, NULL))
        level->addToTickNextTick(pos, this, 1);
}

bool ChorusPlantBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    bool aboveEmpty = level->isEmptyBlock(pos.above());
    bool belowEmpty = level->isEmptyBlock(pos.below());

    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    const Direction** end = directions.data + directions.length;
    for (const Direction** it = directions.data; it != end; ++it)
    {
        BlockPos side = pos.relative(*it);
        if (level->getBlock(side) == this)
        {
            if (!aboveEmpty && !belowEmpty)
            {
                return false;
            }
            Block* sideBelow = level->getBlock(side.below());
            if (sideBelow == this || sideBelow == Blocks::end_stone)
            {
                return true;
            }
        }
    }

    Block* below = level->getBlock(pos.below());
    return below == this || below == Blocks::end_stone;
}

int ChorusPlantBlock::getRenderLayer()
{
    return 0;
}
bool ChorusPlantBlock::isPathfindable(LevelSource*, const BlockPos&)
{
    return false;
}
EBlockFaceShape ChorusPlantBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}
EBlockRenderShape ChorusPlantBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_CHORUS_PLANT;
}

const AABB* ChorusPlantBlock::getVerticalShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    bool down = state->getValue(DOWN_PROPERTY);
    bool up = state->getValue(UP_PROPERTY);
    if (down)
    {
        if (up)
        {
            return createTempChorusShape(4.0, 0.0, 4.0, 12.0, 16.0, 12.0);
        }
        return createTempChorusShape(4.0, 0.0, 4.0, 12.0, 12.0, 12.0);
    }
    if (up)
    {
        return createTempChorusShape(4.0, 4.0, 4.0, 12.0, 16.0, 12.0);
    }
    return CHORUS_CENTER_SHAPE;
}

const AABB* ChorusPlantBlock::getHorizontalShape(const BlockState*, const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case Direction::eDirection_North:
        return CHORUS_NORTH_SHAPE;
    case Direction::eDirection_South:
        return CHORUS_SOUTH_SHAPE;
    case Direction::eDirection_West:
        return CHORUS_WEST_SHAPE;
    case Direction::eDirection_East:
        return CHORUS_EAST_SHAPE;
    default:
        return NULL;
    }
}

const AABB* ChorusPlantBlock::getRandomNoduleShape(bool large, const BlockState*, const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case Direction::eDirection_North:
        return large ? createTempChorusShape(5.0, 5.0, 2.0, 11.0, 11.0, 4.0)
                     : createTempChorusShape(4.0, 4.0, 3.0, 12.0, 12.0, 4.0);
    case Direction::eDirection_South:
        return large ? createTempChorusShape(5.0, 5.0, 11.0, 11.0, 11.0, 14.0)
                     : createTempChorusShape(4.0, 4.0, 12.0, 12.0, 12.0, 13.0);
    case Direction::eDirection_West:
        return large ? createTempChorusShape(2.0, 5.0, 5.0, 4.0, 11.0, 11.0)
                     : createTempChorusShape(3.0, 4.0, 4.0, 4.0, 12.0, 12.0);
    case Direction::eDirection_East:
        return large ? createTempChorusShape(12.0, 5.0, 5.0, 14.0, 11.0, 11.0)
                     : createTempChorusShape(12.0, 4.0, 4.0, 13.0, 12.0, 12.0);
    default:
        return NULL;
    }
}

bool ChorusPlantBlock::isWaterBlocking() const
{
    return false;
}
bool ChorusPlantBlock::waterSpreadCausesSpawn()
{
    return true;
}
bool ChorusPlantBlock::canContainLiquid(const BlockState*) const
{
    return false;
}
bool ChorusPlantBlock::canProvideSupport(const BlockState*, const Direction*, EBlockSupportType)
{
    return false;
}

BlockStateDefinition* ChorusPlantBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {NORTH_PROPERTY, EAST_PROPERTY, SOUTH_PROPERTY,
                                           WEST_PROPERTY,  UP_PROPERTY,   DOWN_PROPERTY};
    return new BlockStateDefinition(this, properties, 6);
}
