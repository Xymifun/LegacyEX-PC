#include "game/world/block/PoweredRailBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<BaseRailBlock::RailShape*>* PoweredRailBlock::SHAPE = NULL;
BooleanProperty* PoweredRailBlock::POWERED = static_cast<BooleanProperty*>(BooleanProperty::create(L"powered"));

void PoweredRailBlock::blockStaticCtor()
{
    std::wstring name;
    name = L"shape";
    Predicates<BaseRailBlock::RailShape*>::FunctionPredicate* filter =
        new Predicates<BaseRailBlock::RailShape*>::FunctionPredicate(railfilter);
    EnumProperty<BaseRailBlock::RailShape*>* shape;
    {
        boost::unordered_set<BaseRailBlock::RailShape*> values;
        for (int i = 0; i < 10; ++i)
            values.insert(RailShape::VALUES[i]);
        shape = EnumProperty<BaseRailBlock::RailShape*>::create(name, typeid(BaseRailBlock::RailShape), values, filter);
    }
    PoweredRailBlock::SHAPE = shape;
}

PoweredRailBlock::PoweredRailBlock() : BaseRailBlock(true)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_SOUTH)
                             ->setValue(PoweredRailBlock::POWERED, false));
}

bool PoweredRailBlock::findPoweredRailSignal(Level* level, const BlockPos& pos, const BlockState* state, bool forward,
                                             int distance)
{
    if (distance >= 8)
        return false;

    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    bool checkBelow = true;
    BaseRailBlock::RailShape* shape = state->getValue(PoweredRailBlock::SHAPE);
    switch (shape->getData())
    {
    case 0:
        if (forward)
            ++z;
        else
            --z;
        break;
    case 1:
        if (forward)
            --x;
        else
            ++x;
        break;
    case 2:
        if (forward)
        {
            --x;
        }
        else
        {
            ++x;
            ++y;
            checkBelow = false;
        }
        shape = RailShape::EAST_WEST;
        break;
    case 3:
        if (forward)
        {
            --x;
            ++y;
            checkBelow = false;
        }
        else
        {
            ++x;
        }
        shape = RailShape::EAST_WEST;
        break;
    case 4:
        if (forward)
        {
            ++z;
        }
        else
        {
            --z;
            ++y;
            checkBelow = false;
        }
        shape = RailShape::NORTH_SOUTH;
        break;
    case 5:
        if (forward)
        {
            ++z;
            ++y;
            checkBelow = false;
        }
        else
        {
            --z;
        }
        shape = RailShape::NORTH_SOUTH;
        break;
    }

    if (isSameRailWithPower(level, BlockPos(x, y, z), forward, distance, shape))
        return true;
    if (checkBelow && isSameRailWithPower(level, BlockPos(x, y - 1, z), forward, distance, shape))
        return true;
    return false;
}

bool PoweredRailBlock::isSameRailWithPower(Level* level, const BlockPos& pos, bool forward, int distance,
                                           BaseRailBlock::RailShape* shape)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() == this)
    {
        BaseRailBlock::RailShape* otherShape = state->getValue(PoweredRailBlock::SHAPE);
        if (shape == RailShape::EAST_WEST
            && (otherShape == RailShape::NORTH_SOUTH || otherShape == RailShape::ASCENDING_NORTH
                || otherShape == RailShape::ASCENDING_SOUTH))
            return false;
        if (shape == RailShape::NORTH_SOUTH
            && (otherShape == RailShape::EAST_WEST || otherShape == RailShape::ASCENDING_EAST
                || otherShape == RailShape::ASCENDING_WEST))
            return false;
        if (state->getValue(PoweredRailBlock::POWERED))
        {
            if (level->hasNeighborSignal(pos))
                return true;
            return findPoweredRailSignal(level, pos, state, forward, distance + 1);
        }
    }
    return false;
}

void PoweredRailBlock::updateState(const BlockState* state, Level* level, const BlockPos& pos, Block*)
{
    bool wasPowered = state->getValue(PoweredRailBlock::POWERED);
    bool powered = level->hasNeighborSignal(pos) || findPoweredRailSignal(level, pos, state, true, 0)
                   || findPoweredRailSignal(level, pos, state, false, 0);
    if (powered != wasPowered)
    {
        level->setBlock(pos, state->setValue(PoweredRailBlock::POWERED, powered), Block::UPDATE_ALL, false);
        level->updateNeighborsAt(pos.below(), this, false);
        if (state->getValue(PoweredRailBlock::SHAPE)->isAscending())
            level->updateNeighborsAt(pos.above(), this, false);
    }
}

const AbstractProperty<BaseRailBlock::RailShape*>* PoweredRailBlock::getShapeProperty()
{
    return PoweredRailBlock::SHAPE;
}

const BlockState* PoweredRailBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(PoweredRailBlock::SHAPE, BaseRailBlock::RailShape::byData(data & 7))
        ->setValue(PoweredRailBlock::POWERED, (data & 8) > 0);
}

int PoweredRailBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(PoweredRailBlock::SHAPE)->getData();
    if (state->getValue(PoweredRailBlock::POWERED))
        data |= 8;
    return data;
}

const BlockState* PoweredRailBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        switch (state->getValue(PoweredRailBlock::SHAPE)->getData())
        {
        case 2:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_WEST);
        case 3:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_EAST);
        case 4:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_SOUTH);
        case 5:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_NORTH);
        case 6:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_WEST);
        case 7:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_EAST);
        case 8:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::SOUTH_EAST);
        case 9:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::SOUTH_WEST);
        }
    case Rotation::eTYPE_CounterClockwise90:
        switch (state->getValue(PoweredRailBlock::SHAPE)->getData())
        {
        case 0:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::EAST_WEST);
        case 1:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_SOUTH);
        case 2:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_NORTH);
        case 3:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_SOUTH);
        case 4:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_WEST);
        case 5:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_EAST);
        case 6:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_EAST);
        case 7:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::SOUTH_EAST);
        case 8:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::SOUTH_WEST);
        case 9:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_WEST);
        }
    case Rotation::eTYPE_Clockwise90:
        switch (state->getValue(PoweredRailBlock::SHAPE)->getData())
        {
        case 0:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::EAST_WEST);
        case 1:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_SOUTH);
        case 2:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_SOUTH);
        case 3:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_NORTH);
        case 4:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_EAST);
        case 5:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_WEST);
        case 6:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::SOUTH_WEST);
        case 7:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_WEST);
        case 8:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_EAST);
        case 9:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::SOUTH_EAST);
        }
    default:
        break;
    }

    return state;
}

const BlockState* PoweredRailBlock::mirror(const BlockState* state, Mirror* mirror)
{
    BaseRailBlock::RailShape* shape = state->getValue(PoweredRailBlock::SHAPE);
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
        switch (shape->getData())
        {
        case 4:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_SOUTH);
        case 5:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_NORTH);
        case 6:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_EAST);
        case 7:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_WEST);
        case 8:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::SOUTH_WEST);
        case 9:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::SOUTH_EAST);
        }
        break;
    case Mirror::eTYPE_FrontBack:
        switch (shape->getData())
        {
        case 2:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_WEST);
        case 3:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::ASCENDING_EAST);
        case 6:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::SOUTH_WEST);
        case 7:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::SOUTH_EAST);
        case 8:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_EAST);
        case 9:
            return state->setValue(PoweredRailBlock::SHAPE, RailShape::NORTH_WEST);
        }
        break;
    }

    return Block::mirror(state, mirror);
}

BlockStateDefinition* PoweredRailBlock::createBlockStateDefinition()
{
    static const Property* properties[2] = {PoweredRailBlock::SHAPE, PoweredRailBlock::POWERED};
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* PoweredRailBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(PoweredRailBlock::POWERED) ? mPoweredTexture : mIcon;
}

void PoweredRailBlock::registerIcons(IconRegister* iconRegister)
{
    Block::registerIcons(iconRegister);
    mPoweredTexture = iconRegister->registerIcon(getIconName() + L"_powered");
}
