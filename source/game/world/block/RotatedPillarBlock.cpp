#include "game/world/block/RotatedPillarBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Boxed.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<Direction::Axis*>* gRotatedPillarBlockAxisProperty = NULL;

void RotatedPillarBlock::blockStaticCtor()
{
    std::wstring name = L"axis";
    Predicate<Direction::Axis*>* filter = Predicates<Direction::Axis*>::alwaysTrue;

    EnumProperty<Direction::Axis*>* property;
    {
        boost::unordered_set<Direction::Axis*> values;
        for (int i = 0; i < 3; ++i)
        {
            values.insert(Direction::Axis::VALUES[i]);
        }

        property = EnumProperty<Direction::Axis*>::create(name, typeid(Direction::Axis), values, filter);
    }
    gRotatedPillarBlockAxisProperty = property;
}

RotatedPillarBlock::RotatedPillarBlock(Material* material) : Block(material, material->getColor())
{
}

RotatedPillarBlock::RotatedPillarBlock(Material* material, const MaterialColor* materialColor)
    : Block(material, materialColor)
{
}

const BlockState* RotatedPillarBlock::rotate(const BlockState* state, Rotation* rotation)
{
    Rotation::eTYPE type = rotation->getType();
    switch (type)
    {
    case Rotation::eTYPE_CounterClockwise90:
    case Rotation::eTYPE_Clockwise90:
        switch (state->getValue(gRotatedPillarBlockAxisProperty)->getId())
        {
        case Direction::Axis::eAxis_X:
            return state->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Z);
        case Direction::Axis::eAxis_Z:
            return state->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::X);
        default:
            return state;
        }
    default:
        return state;
    }
}

const BlockState* RotatedPillarBlock::getBlockState(int data)
{
    Direction::Axis* axis = Direction::Axis::Y;
    if ((data & 0xC) == 4)
    {
        axis = Direction::Axis::X;
    }
    else if ((data & 0xC) == 8)
    {
        axis = Direction::Axis::Z;
    }

    return defaultBlockState()->setValue(gRotatedPillarBlockAxisProperty, axis);
}

int RotatedPillarBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = 0;
    Direction::Axis* axis = state->getValue(gRotatedPillarBlockAxisProperty);
    if (axis == Direction::Axis::X)
    {
        data = 4;
    }
    else if (axis == Direction::Axis::Z)
    {
        data = 8;
    }
    return data;
}

BlockStateDefinition* RotatedPillarBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {gRotatedPillarBlockAxisProperty};
    return new BlockStateDefinition(this, properties, 1);
}

not_null_ptr<ItemInstance> RotatedPillarBlock::getSilkTouchItemInstance(const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this)));
}

const BlockState* RotatedPillarBlock::getStateForPlacement(Level* level, const BlockPos& pos,
                                                           const Direction* direction, float hitX, float hitY,
                                                           float hitZ, int auxValue,
                                                           boost::shared_ptr<LivingEntity> placer)
{
    return Block::getStateForPlacement(level, pos, direction, hitX, hitY, hitZ, auxValue, placer)
        ->setValue(gRotatedPillarBlockAxisProperty, direction->getAxis());
}

TextureAtlasSprite* RotatedPillarBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Direction::Axis* axis = getAxis(state);
    if (axis == Direction::Axis::Y && (direction == Direction::UP || direction == Direction::DOWN))
    {
        return getTopTexture(state);
    }
    if (axis == Direction::Axis::X && (direction == Direction::EAST || direction == Direction::WEST))
    {
        return getTopTexture(state);
    }
    if (axis == Direction::Axis::Z && (direction == Direction::NORTH || direction == Direction::SOUTH))
    {
        return getTopTexture(state);
    }

    return getTypeTexture(state);
}

TextureAtlasSprite* RotatedPillarBlock::getTypeTexture(const BlockState*)
{
    return mIcon;
}

TextureAtlasSprite* RotatedPillarBlock::getTopTexture(const BlockState*)
{
    return mTopTexture;
}

EBlockRenderShape RotatedPillarBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_TREE;
}
