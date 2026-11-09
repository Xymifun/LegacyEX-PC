#include "game/world/block/EndPortalFrameBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockInWorld.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/BlockPatternBuilder.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BlockStatePredicate.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/item/Item.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty* EndPortalFrameBlock::FACING = NULL;
BooleanProperty* EndPortalFrameBlock::EYE = static_cast<BooleanProperty*>(BooleanProperty::create(L"eye"));

static const AABB* END_PORTAL_FRAME_SHAPE = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.8125, 1.0);
static const AABB* END_PORTAL_FRAME_EYE_SHAPE = AABB::newPermanent(0.25, 0.8125, 0.25, 0.75, 1.0, 0.75);
static BlockPattern* END_PORTAL_SHAPE = NULL;

EndPortalFrameBlock::EndPortalFrameBlock() : Block(Material::STONE, MaterialColor::COLOR_GREEN)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(FACING, Direction::NORTH)->setValue(EYE, false));
}

void EndPortalFrameBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

bool EndPortalFrameBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

const AABB* EndPortalFrameBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return END_PORTAL_FRAME_SHAPE;
}

void EndPortalFrameBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos,
                                            const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                            boost::shared_ptr<Entity> entity, bool isActualState)
{
    if (state->getBlock() == this)
    {
        addCollisionAABB(pos, entityBox, collidingBoxes, END_PORTAL_FRAME_SHAPE);
        if (state->getValue(EYE))
        {
            addCollisionAABB(pos, entityBox, collidingBoxes, END_PORTAL_FRAME_EYE_SHAPE);
        }
    }
}

Item* EndPortalFrameBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Items::air;
}

const BlockState* EndPortalFrameBlock::getStateForPlacement(Level* level, const BlockPos& pos,
                                                            const Direction* direction, float hitX, float hitY,
                                                            float hitZ, int auxValue,
                                                            boost::shared_ptr<LivingEntity> placer)
{
    return defaultBlockState()->setValue(FACING, placer->getDirection()->getOpposite())->setValue(EYE, false);
}

bool EndPortalFrameBlock::hasAnalogOutputSignal(const BlockState* state)
{
    return true;
}

int EndPortalFrameBlock::getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos)
{
    return state->getValue(EYE) ? SIGNAL_MAX : SIGNAL_NONE;
}

const BlockState* EndPortalFrameBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(EYE, ((data >> 2) & 1) != 0)
        ->setValue(FACING, Direction::from2DDataValue(data & 3));
}

int EndPortalFrameBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->get2DDataValue();
    if (state->getValue(EYE))
    {
        data |= 4;
    }
    return data;
}

const BlockState* EndPortalFrameBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* EndPortalFrameBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* EndPortalFrameBlock::createBlockStateDefinition()
{
    static const Property* properties[2] = {FACING, EYE};
    return new BlockStateDefinition(this, properties, 2);
}

bool EndPortalFrameBlock::isCubeShaped(const BlockState* state)
{
    return false;
}

BlockPattern* EndPortalFrameBlock::getOrCreatePortalShape()
{
    if (END_PORTAL_SHAPE == NULL)
    {
        std::wstring rows[5] = {L"?vvv?", L">?\?\?<", L">?\?\?<", L">?\?\?<", L"?^^^?"};
        BlockStatePredicate* south = BlockStatePredicate::forBlock(Blocks::end_portal_frame);
        south = south->where(
            EYE, new BlockStatePredicate::PredicateConversion<bool>(new Predicates<bool>::EqualToValuePredicate(true)));
        south = south->where(FACING, new BlockStatePredicate::PredicateConversion<const Direction*>(
                                         new Predicates<const Direction*>::EqualToValuePredicate(Direction::SOUTH)));
        BlockStatePredicate* west = BlockStatePredicate::forBlock(Blocks::end_portal_frame);
        west = west->where(
            EYE, new BlockStatePredicate::PredicateConversion<bool>(new Predicates<bool>::EqualToValuePredicate(true)));
        west = west->where(FACING, new BlockStatePredicate::PredicateConversion<const Direction*>(
                                       new Predicates<const Direction*>::EqualToValuePredicate(Direction::WEST)));
        BlockStatePredicate* north = BlockStatePredicate::forBlock(Blocks::end_portal_frame);
        north = north->where(
            EYE, new BlockStatePredicate::PredicateConversion<bool>(new Predicates<bool>::EqualToValuePredicate(true)));
        north = north->where(FACING, new BlockStatePredicate::PredicateConversion<const Direction*>(
                                         new Predicates<const Direction*>::EqualToValuePredicate(Direction::NORTH)));
        BlockStatePredicate* east = BlockStatePredicate::forBlock(Blocks::end_portal_frame);
        east = east->where(
            EYE, new BlockStatePredicate::PredicateConversion<bool>(new Predicates<bool>::EqualToValuePredicate(true)));
        east = east->where(FACING, new BlockStatePredicate::PredicateConversion<const Direction*>(
                                       new Predicates<const Direction*>::EqualToValuePredicate(Direction::EAST)));
        END_PORTAL_SHAPE = BlockPatternBuilder::start()
                               ->aisle(rows)
                               ->where(L'?', BlockInWorld::hasState(Predicates<const BlockState*>::alwaysTrue))
                               ->where(L'^', BlockInWorld::hasState(south))
                               ->where(L'>', BlockInWorld::hasState(west))
                               ->where(L'v', BlockInWorld::hasState(north))
                               ->where(L'<', BlockInWorld::hasState(east))
                               ->build(Direction::SOUTH, Direction::DOWN);
    }
    return END_PORTAL_SHAPE;
}

EBlockFaceShape EndPortalFrameBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                                       const BlockPos& pos, const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* EndPortalFrameBlock::getEye()
{
    return mEyeIcon;
}

TextureAtlasSprite* EndPortalFrameBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP)
    {
        return mTopIcon;
    }
    if (direction == Direction::DOWN)
    {
        return Blocks::end_stone->getTexture(direction);
    }
    return mIcon;
}

void EndPortalFrameBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"endframe_side");
    mTopIcon = iconRegister->registerIcon(L"endframe_top");
    mEyeIcon = iconRegister->registerIcon(L"endframe_eye");
}

EBlockRenderShape EndPortalFrameBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_AIR_PORTAL_FRAME;
}
