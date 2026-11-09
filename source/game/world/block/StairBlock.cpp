#include "game/world/block/StairBlock.hpp"
#include "game/core/Direction.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/IconRegister.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"
#include <typeinfo>
#include <vector>
#include <boost/unordered_set.hpp>

DirectionProperty* StairBlock::FACING = NULL;
EnumProperty<StairBlock::Half*>* StairBlock::HALF = NULL;
EnumProperty<StairBlock::Shape*>* StairBlock::SHAPE = NULL;
static const AABB* gStairTopSlabAABB = AABB::newPermanent(0.0, 0.5, 0.0, 1.0, 1.0, 1.0);
static const AABB* gStairTopWestAABB = AABB::newPermanent(0.0, 0.5, 0.0, 0.5, 1.0, 1.0);
static const AABB* gStairTopEastAABB = AABB::newPermanent(0.5, 0.5, 0.0, 1.0, 1.0, 1.0);
static const AABB* gStairTopNorthAABB = AABB::newPermanent(0.0, 0.5, 0.0, 1.0, 1.0, 0.5);
static const AABB* gStairTopSouthAABB = AABB::newPermanent(0.0, 0.5, 0.5, 1.0, 1.0, 1.0);
static const AABB* gStairTopNorthWestAABB = AABB::newPermanent(0.0, 0.5, 0.0, 0.5, 1.0, 0.5);
static const AABB* gStairTopNorthEastAABB = AABB::newPermanent(0.5, 0.5, 0.0, 1.0, 1.0, 0.5);
static const AABB* gStairTopSouthWestAABB = AABB::newPermanent(0.0, 0.5, 0.5, 0.5, 1.0, 1.0);
static const AABB* gStairTopSouthEastAABB = AABB::newPermanent(0.5, 0.5, 0.5, 1.0, 1.0, 1.0);
static const AABB* gStairBottomSlabAABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0);
static const AABB* gStairBottomWestAABB = AABB::newPermanent(0.0, 0.0, 0.0, 0.5, 0.5, 1.0);
static const AABB* gStairBottomEastAABB = AABB::newPermanent(0.5, 0.0, 0.0, 1.0, 0.5, 1.0);
static const AABB* gStairBottomNorthAABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 0.5);
static const AABB* gStairBottomSouthAABB = AABB::newPermanent(0.0, 0.0, 0.5, 1.0, 0.5, 1.0);
static const AABB* gStairBottomNorthWestAABB = AABB::newPermanent(0.0, 0.0, 0.0, 0.5, 0.5, 0.5);
static const AABB* gStairBottomNorthEastAABB = AABB::newPermanent(0.5, 0.0, 0.0, 1.0, 0.5, 0.5);
static const AABB* gStairBottomSouthWestAABB = AABB::newPermanent(0.0, 0.0, 0.5, 0.5, 0.5, 1.0);
static const AABB* gStairBottomSouthEastAABB = AABB::newPermanent(0.5, 0.0, 0.5, 1.0, 0.5, 1.0);
StairBlock::Half* StairBlock::Half::TOP = new StairBlock::Half(L"top");
StairBlock::Half* StairBlock::Half::BOTTOM = new StairBlock::Half(L"bottom");
static const int STAIR_OPAQUE_LIGHT_BLOCK = 0xff;
StairBlock::Half* gStairHalfValues[2] = {StairBlock::Half::TOP, StairBlock::Half::BOTTOM};
static StairBlock::Shape* gStairStraight = new StairBlock::Shape(L"straight", 0);
static StairBlock::Shape* gStairInnerLeft = new StairBlock::Shape(L"inner_left", 1);
static StairBlock::Shape* gStairInnerRight = new StairBlock::Shape(L"inner_right", 2);
static StairBlock::Shape* gStairOuterLeft = new StairBlock::Shape(L"outer_left", 3);
static StairBlock::Shape* gStairOuterRight = new StairBlock::Shape(L"outer_right", 4);
StairBlock::Shape* gStairShapeValues[5] = {gStairStraight, gStairInnerLeft, gStairInnerRight, gStairOuterLeft,
                                           gStairOuterRight};

StairBlock::Half::Half(const std::wstring& name) : mName(name)
{
}

std::wstring StairBlock::Half::toString() const
{
    return mName;
}

std::wstring StairBlock::Half::getSerializedName() const
{
    return mName;
}

StairBlock::Shape::Shape(const std::wstring& name, int id) : mName(name), mId(id)
{
}

int StairBlock::Shape::getId() const
{
    return mId;
}

std::wstring StairBlock::Shape::toString() const
{
    return mName;
}

std::wstring StairBlock::Shape::getSerializedName() const
{
    return mName;
}

void StairBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;

    {
        std::wstring name(L"half");
        const Predicate<Half*>* filter = Predicates<Half*>::alwaysTrue;
        boost::unordered_set<Half*> halves;
        for (int i = 0; i < 2; ++i)
        {
            halves.insert(gStairHalfValues[i]);
        }
        HALF = EnumProperty<Half*>::create(name, typeid(Half), halves, filter);
    }

    {
        std::wstring name(L"shape");
        const Predicate<Shape*>* filter = Predicates<Shape*>::alwaysTrue;
        boost::unordered_set<Shape*> shapes;
        for (int i = 0; i < 5; ++i)
        {
            shapes.insert(gStairShapeValues[i]);
        }
        SHAPE = EnumProperty<Shape*>::create(name, typeid(Shape), shapes, filter);
    }
}

StairBlock::StairBlock(const BlockState* modelState) : Block(modelState->getBlock()->getCraftingMaterial())
{
    mBlockType = static_cast<EBlockType>(EBLOCK_TYPE_NONE + 7);
    DerivedInit();

    const BlockState* state = mStateDefinition->any()
                                  ->setValue(FACING, Direction::NORTH)
                                  ->setValue(HALF, Half::BOTTOM)
                                  ->setValue(SHAPE, gStairStraight);
    registerDefaultState(state);

    mModelBlock = modelState->getBlock();
    mModelState = modelState;
    mIsModelState = false;
    mShapeState = 0;
    setDestroyTime(mModelBlock->mDestroyTime);
    setExplodeable(mModelBlock->mExplosionResistance / 3.0f);
    setSoundType(mModelBlock->mSoundType);
    setLightBlock(STAIR_OPAQUE_LIGHT_BLOCK);
}

void StairBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* box,
                                   std::vector<AABB*>* result, boost::shared_ptr<Entity> entity, bool actual)
{
    if (!actual)
    {
        state = fillVirtualBlockStateProperties(state, level, pos);
    }

    const arrayWithLength<const AABB*>* shapes = getShapes(state, level, pos);
    for (unsigned int i = 0; i < shapes->length; ++i)
    {
        if (shapes->data[i] != NULL)
        {
            addCollisionAABB(pos, box, result, shapes->data[i]);
        }
    }
}

const AABB* StairBlock::getStepAABB(const BlockState* state)
{
    bool top = state->getValue(HALF) == Half::TOP;
    switch (state->getValue(FACING)->get3DDataValue())
    {
    default:
        return top ? gStairBottomNorthAABB : gStairTopNorthAABB;
    case 3:
        return top ? gStairBottomSouthAABB : gStairTopSouthAABB;
    case 4:
        return top ? gStairBottomWestAABB : gStairTopWestAABB;
    case 5:
        return top ? gStairBottomEastAABB : gStairTopEastAABB;
    }
}

const AABB* StairBlock::getCornerAABB(const BlockState* state)
{
    const Direction* facing = state->getValue(FACING);
    const Direction* cornerFacing;
    switch (state->getValue(SHAPE)->getId())
    {
    case 3:
    default:
        cornerFacing = facing;
        break;
    case 4:
        cornerFacing = facing->getClockWise();
        break;
    case 2:
        cornerFacing = facing->getOpposite();
        break;
    case 1:
        cornerFacing = facing->getCounterClockWise();
        break;
    }

    bool top = state->getValue(HALF) == Half::TOP;
    switch (cornerFacing->get3DDataValue())
    {
    default:
        return top ? gStairBottomNorthWestAABB : gStairTopNorthWestAABB;
    case 3:
        return top ? gStairBottomSouthEastAABB : gStairTopSouthEastAABB;
    case 4:
        return top ? gStairBottomSouthWestAABB : gStairTopSouthWestAABB;
    case 5:
        return top ? gStairBottomNorthEastAABB : gStairTopNorthEastAABB;
    }
}

EBlockFaceShape StairBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction)
{
    state = fillVirtualBlockStateProperties(state, levelSource, pos);
    if (direction->getAxis() == Direction::Axis::Y)
    {
        return (direction == Direction::UP) == (state->getValue(HALF) == Half::TOP) ? BLOCK_FACE_SOLID
                                                                                    : BLOCK_FACE_UNDEFINED;
    }

    Shape* shape = state->getValue(SHAPE);
    if (shape->getId() == 3 || shape->getId() == 4)
    {
        return BLOCK_FACE_UNDEFINED;
    }

    const Direction* facing = state->getValue(FACING);
    switch (shape->getId())
    {
    case 0:
        return facing == direction ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
    case 1:
        return facing == direction || facing == direction->getClockWise() ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
    case 2:
        return facing == direction || facing == direction->getCounterClockWise() ? BLOCK_FACE_SOLID
                                                                                 : BLOCK_FACE_UNDEFINED;
    default:
        return BLOCK_FACE_UNDEFINED;
    }
}

int StairBlock::getSpawnResourcesAuxValue(const BlockState*)
{
    return 2;
}

bool StairBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool StairBlock::isCubeShaped(const BlockState*)
{
    return false;
}

void StairBlock::addLights(Level* level, const BlockPos& pos)
{
    mModelBlock->addLights(level, pos);
}

void StairBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    mModelBlock->animateTick(state, level, pos, random);
}

void StairBlock::attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player)
{
    mModelBlock->attack(level, pos, player);
}

void StairBlock::destroy(Level* level, const BlockPos& pos, const BlockState* state)
{
    mModelBlock->destroy(level, pos, state);
}

int StairBlock::getLightColor(const BlockState* state, LevelSource* source, const BlockPos& pos, int light)
{
    return mModelBlock->getLightColor(state, source, pos, light);
}

float StairBlock::getExplosionResistance(boost::shared_ptr<Entity> entity)
{
    return mModelBlock->getExplosionResistance(entity);
}

int StairBlock::getRenderLayer()
{
    return mModelBlock->getRenderLayer();
}

int StairBlock::getTickDelay(Level* level)
{
    return mModelBlock->getTickDelay(level);
}

const AABB* StairBlock::getOutlineAABB(const BlockState*, Level* level, const BlockPos& pos)
{
    return mModelState->getOutlineAABB(level, pos);
}

Vec3* StairBlock::handleEntityInside(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, Vec3* motion)
{
    return mModelBlock->handleEntityInside(level, pos, entity, motion);
}

bool StairBlock::mayPick()
{
    return mModelBlock->mayPick();
}

bool StairBlock::mayPick(const BlockState* state, bool includeLiquid)
{
    return mModelBlock->mayPick(state, includeLiquid);
}

bool StairBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return mModelBlock->mayPlace(level, pos);
}

void StairBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    mModelBlock->onRemove(level, pos, state);
}

void StairBlock::prepareRender(Level* level, const BlockPos& pos)
{
    mModelBlock->prepareRender(level, pos);
}

void StairBlock::stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity)
{
    mModelBlock->stepOn(level, pos, entity);
}

void StairBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    mModelBlock->tick(level, pos, state, random);
}

const arrayWithLength<const AABB*>* StairBlock::getShapes(const BlockState* state, LevelSource* levelSource,
                                                          const BlockPos& pos)
{
    arrayWithLength<const AABB*>* shapes =
        &static_cast<Block::ThreadStorage*>(TlsGetValue(THREAD_STORAGE_INDEX))->mShapes;

    if (state->getValue(HALF) == Half::TOP)
    {
        shapes->data[0] = gStairTopSlabAABB;
    }
    else
    {
        shapes->data[0] = gStairBottomSlabAABB;
    }

    const Shape* shape = state->getValue(SHAPE);

    if (shape == gStairStraight || shape == gStairInnerLeft || shape == gStairInnerRight)
    {
        shapes->data[1] = getStepAABB(state);
    }
    else
    {
        shapes->data[1] = NULL;
    }

    shapes->data[2] = shape == gStairStraight ? NULL : getCornerAABB(state);

    return &static_cast<Block::ThreadStorage*>(TlsGetValue(THREAD_STORAGE_INDEX))->mShapes;
}

void StairBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    mModelState->neighborChanged(level, pos, Blocks::air, pos);
    mModelBlock->onPlace(level, pos, state);
}

bool StairBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction*, float, float, float, bool client)
{
    if (client)
    {
        return false;
    }

    return mModelBlock->use(level, pos, mModelState, player, hand, Direction::DOWN, 0.0f, 0.0f, 0.0f, false);
}

void StairBlock::wasExploded(Level* level, const BlockPos& pos, Explosion* explosion)
{
    mModelBlock->wasExploded(level, pos, explosion);
}

bool StairBlock::isTopSolidBlocking(const BlockState* state)
{
    return state->getValue(HALF) == Half::TOP;
}

const MaterialColor* StairBlock::getMapColor(const BlockState*, LevelSource* source, const BlockPos& pos)
{
    return mModelBlock->getMapColor(mModelState, source, pos);
}

const BlockState* StairBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* state = Block::getStateForPlacement(level, pos, direction, hitX, hitY, hitZ, auxValue, placer);
    state = state->setValue(FACING, placer->getDirection())->setValue(SHAPE, gStairStraight);

    if (direction == Direction::DOWN || (direction != Direction::UP && hitY > 0.5))
    {
        return state->setValue(HALF, Half::TOP);
    }

    return state->setValue(HALF, Half::BOTTOM);
}

bool StairBlock::_neighboringBlockCheckForCreatingBarrierInDirection(LevelSource* source, const BlockPos& pos,
                                                                     const BlockState* state,
                                                                     const Direction* direction) const
{
    const BlockState* neighbor = source->getBlockState(pos);
    if (state->getBlock() == neighbor->getBlock())
    {
        if (neighbor->getValue(FACING) == direction)
        {
            return true;
        }
    }

    return false;
}

bool StairBlock::liquidCanFlowIntoFromDirection(LevelSource* source, const BlockPos& pos,
                                                const Direction* direction) const
{
    const BlockState* state = source->getBlockState(pos);
    if (direction == Direction::UP || direction == Direction::DOWN)
    {
        const Direction* solidSide = Direction::DOWN;
        if (state->getValue(HALF) == Half::TOP)
        {
            solidSide = Direction::UP;
        }

        if (direction != solidSide)
        {
            return true;
        }
    }
    else
    {
        bool blocked;
        switch (state->getValue(FACING)->get3DDataValue())
        {
        case 5:
        {
            if (direction == Direction::EAST)
            {
                return false;
            }

            if (direction == Direction::NORTH)
            {
                BlockPos adjacent = pos.west();
                if (_neighboringBlockCheckForCreatingBarrierInDirection(source, adjacent, state, Direction::NORTH))
                {
                    return false;
                }
            }

            if (direction != Direction::SOUTH)
            {
                return true;
            }

            BlockPos neighbour = pos.west();
            blocked = _neighboringBlockCheckForCreatingBarrierInDirection(source, neighbour, state, Direction::SOUTH);
            break;
        }
        case 4:
        {
            if (direction == Direction::WEST)
            {
                return false;
            }

            if (direction == Direction::NORTH)
            {
                BlockPos adjacent = pos.east();
                if (_neighboringBlockCheckForCreatingBarrierInDirection(source, adjacent, state, Direction::NORTH))
                {
                    return false;
                }
            }

            if (direction != Direction::SOUTH)
            {
                return true;
            }

            BlockPos neighbour = pos.east();
            blocked = _neighboringBlockCheckForCreatingBarrierInDirection(source, neighbour, state, Direction::SOUTH);
            break;
        }
        case 2:
        {
            if (direction == Direction::NORTH)
            {
                return false;
            }

            if (direction == Direction::EAST)
            {
                BlockPos adjacent = pos.south();
                if (_neighboringBlockCheckForCreatingBarrierInDirection(source, adjacent, state, Direction::EAST))
                {
                    return false;
                }
            }

            if (direction != Direction::WEST)
            {
                return true;
            }

            BlockPos neighbour = pos.south();
            blocked = _neighboringBlockCheckForCreatingBarrierInDirection(source, neighbour, state, Direction::WEST);
            break;
        }
        case 3:
        {
            if (direction == Direction::SOUTH)
            {
                return false;
            }

            if (direction == Direction::EAST)
            {
                BlockPos adjacent = pos.north();
                if (_neighboringBlockCheckForCreatingBarrierInDirection(source, adjacent, state, Direction::EAST))
                {
                    return false;
                }
            }

            if (direction != Direction::WEST)
            {
                return true;
            }

            BlockPos neighbour = pos.north();
            blocked = _neighboringBlockCheckForCreatingBarrierInDirection(source, neighbour, state, Direction::WEST);
            break;
        }
        default:
            return false;
        }

        if (!blocked)
        {
            return true;
        }
    }

    return false;
}

HitResult* StairBlock::clip(const BlockState* state, Level* level, const BlockPos& pos, Vec3* from, Vec3* to)
{
    std::vector<HitResult*> hits;
    const arrayWithLength<const AABB*>* shapes = getShapes(fillVirtualBlockStateProperties(state, level, pos), level,
                                                           pos);
    for (unsigned int i = 0; i < shapes->length; ++i)
    {
        if (shapes->data[i] != NULL)
        {
            hits.push_back(clip(pos, from, to, shapes->data[i]));
        }
    }

    HitResult* furthest = NULL;
    double furthestDistance = 0.0;
    for (std::vector<HitResult*>::iterator it = hits.begin(); it != hits.end(); ++it)
    {
        if (*it != NULL)
        {
            double distance = (*it)->mLocation->distanceToSqr(to);
            if (distance > furthestDistance)
            {
                furthestDistance = distance;
                furthest = *it;
            }
        }
    }

    return furthest;
}

const BlockState* StairBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    state = state->setValue(HALF, (data & 4) > 0 ? Half::TOP : Half::BOTTOM);
    state = state->setValue(FACING, Direction::from3DDataValue(5 - (data & 3)));
    return state;
}

int StairBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = 0;
    if (state->getValue(HALF) == Half::TOP)
    {
        data = 4;
    }

    return data | (5 - state->getValue(FACING)->get3DDataValue());
}

bool StairBlock::isStairs(const BlockState* state)
{
    return dynamic_cast<StairBlock*>(state->getBlock()) != NULL;
}

bool StairBlock::canTakeShape(const BlockState* state, LevelSource* source, const BlockPos& pos,
                              const Direction* direction)
{
    const BlockState* neighbor = source->getBlockState(pos.relative(direction));
    bool matches = false;
    if (isStairs(neighbor))
    {
        if (neighbor->getValue(FACING) == state->getValue(FACING))
        {
            if (neighbor->getValue(HALF) == state->getValue(HALF))
            {
                matches = true;
            }
        }
    }

    return !matches;
}

StairBlock::Shape* StairBlock::getStairsShape(const BlockState* state, LevelSource* source, const BlockPos& pos)
{
    const Direction* facing = state->getValue(FACING);
    const BlockState* front = source->getBlockState(pos.relative(facing));
    if (isStairs(front) && state->getValue(HALF) == front->getValue(HALF))
    {
        const Direction* frontFacing = front->getValue(FACING);
        if (frontFacing->getAxis() != state->getValue(FACING)->getAxis()
            && canTakeShape(state, source, pos, frontFacing->getOpposite()))
        {
            if (frontFacing == facing->getCounterClockWise())
            {
                return gStairOuterLeft;
            }

            return gStairOuterRight;
        }
    }

    const BlockState* back = source->getBlockState(pos.relative(facing->getOpposite()));
    if (isStairs(back) && state->getValue(HALF) == back->getValue(HALF))
    {
        const Direction* backFacing = back->getValue(FACING);
        if (backFacing->getAxis() != state->getValue(FACING)->getAxis() && canTakeShape(state, source, pos, backFacing))
        {
            if (backFacing == facing->getCounterClockWise())
            {
                return gStairInnerLeft;
            }

            return gStairInnerRight;
        }
    }

    return gStairStraight;
}

const BlockState* StairBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* source,
                                                              const BlockPos& pos)
{
    return state->setValue(SHAPE, getStairsShape(state, source, pos));
}

bool StairBlock::isStairs(int data)
{
    bool stairs = true;
    switch (data)
    {
    case 0x35:
    case 0x43:
    case 0x6c:
    case 0x6d:
    case 0x72:
    case 0x80:
    case 0x86:
    case 0x87:
    case 0x88:
    case 0x9c:
    case 0xa3:
    case 0xa4:
    case 0xb4:
    case 0xcb:
        break;
    default:
        stairs = false;
        break;
    }

    return stairs;
}

const BlockState* StairBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* StairBlock::mirror(const BlockState* state, Mirror* mirror)
{
    const Direction* facing = state->getValue(FACING);
    Shape* shape = state->getValue(SHAPE);
    if (mirror->getType() == Mirror::eTYPE_LeftRight)
    {
        if (facing->getAxis() == Direction::Axis::Z)
        {
            if (shape == gStairInnerLeft)
            {
                TypedBoxed<Shape*> value(gStairInnerRight);
                return rotate(state, Rotation::CLOCKWISE_180)->setBoxedValue(SHAPE, &value);
            }

            if (shape == gStairInnerRight)
            {
                TypedBoxed<Shape*> value(gStairInnerLeft);
                return rotate(state, Rotation::CLOCKWISE_180)->setBoxedValue(SHAPE, &value);
            }

            if (shape == gStairOuterLeft)
            {
                TypedBoxed<Shape*> value(gStairOuterRight);
                return rotate(state, Rotation::CLOCKWISE_180)->setBoxedValue(SHAPE, &value);
            }

            const BlockState* rotated = rotate(state, Rotation::CLOCKWISE_180);
            if (shape != gStairOuterRight)
            {
                return rotated;
            }

            TypedBoxed<Shape*> value(gStairOuterLeft);
            return rotated->setBoxedValue(SHAPE, &value);
        }
    }
    else if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        if (facing->getAxis() == Direction::Axis::X)
        {
            if (shape == gStairInnerLeft)
            {
                TypedBoxed<Shape*> value(gStairInnerLeft);
                return rotate(state, Rotation::CLOCKWISE_180)->setBoxedValue(SHAPE, &value);
            }

            if (shape == gStairInnerRight)
            {
                TypedBoxed<Shape*> value(gStairInnerRight);
                return rotate(state, Rotation::CLOCKWISE_180)->setBoxedValue(SHAPE, &value);
            }

            if (shape == gStairOuterLeft)
            {
                TypedBoxed<Shape*> value(gStairOuterRight);
                return rotate(state, Rotation::CLOCKWISE_180)->setBoxedValue(SHAPE, &value);
            }

            if (shape == gStairOuterRight)
            {
                TypedBoxed<Shape*> value(gStairOuterLeft);
                return rotate(state, Rotation::CLOCKWISE_180)->setBoxedValue(SHAPE, &value);
            }

            if (shape == gStairStraight)
            {
                return rotate(state, Rotation::CLOCKWISE_180);
            }
        }
    }

    return Block::mirror(state, mirror);
}

BlockStateDefinition* StairBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING, HALF, SHAPE};
    return new BlockStateDefinition(this, properties, 3);
}

TextureAtlasSprite* StairBlock::getTexture(const Direction* direction, const BlockState*)
{
    return mModelBlock->getTexture(direction, mModelState);
}

void StairBlock::registerIcons(IconRegister*)
{
}

EBlockRenderShape StairBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_STAIRS;
}
