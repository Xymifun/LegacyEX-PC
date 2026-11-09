#include "game/world/block/OldLogBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

bool oldlog_filter(PlanksBlock::Variant* value)
{
    return value->getData() < 4;
}

EnumProperty<PlanksBlock::Variant*>* OldLogBlock::VARIANT = NULL;

void OldLogBlock::blockStaticCtor()
{
    std::wstring name(L"variant");
    Predicates<PlanksBlock::Variant*>::FunctionPredicate* filter =
        new Predicates<PlanksBlock::Variant*>::FunctionPredicate(oldlog_filter);
    EnumProperty<PlanksBlock::Variant*>* variant;
    {
        boost::unordered_set<PlanksBlock::Variant*> values;
        for (int i = 0; i < 6; ++i)
        {
            values.insert(PlanksBlock::Variant::BY_DATA[i]);
        }
        variant = EnumProperty<PlanksBlock::Variant*>::create(name, typeid(PlanksBlock::Variant), values, filter);
    }
    VARIANT = variant;
}

OldLogBlock::OldLogBlock() : LogBlock()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, PlanksBlock::Variant::OAK)->setValue(AXIS, Y));
}

const MaterialColor* OldLogBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    PlanksBlock::Variant* variant = state->getValue(VARIANT);
    LogBlock::LogAxis* axis = state->getValue(AXIS);
    if (axis->getId() != 1)
    {
        switch (variant->getData())
        {
        case 1:
            return PlanksBlock::Variant::DARK_OAK->getMaterialColor();
        case 2:
            return MaterialColor::QUARTZ;
        default:
            return PlanksBlock::Variant::SPRUCE->getMaterialColor();
        }
    }
    return variant->getMaterialColor();
}

const BlockState* OldLogBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState()->setValue(VARIANT, PlanksBlock::Variant::byData((data & 3) % 4));
    switch (data & 0xC)
    {
    case 0:
        state = state->setValue(AXIS, Y);
        break;
    case 4:
        state = state->setValue(AXIS, X);
        break;
    case 8:
        state = state->setValue(AXIS, Z);
        break;
    default:
        state = state->setValue(AXIS, NONE);
    }
    return state;
}

int OldLogBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    LogBlock::LogAxis* axis = state->getValue(AXIS);
    switch (axis->getId())
    {
    case 0:
        data |= 4;
        break;
    case 2:
        data |= 8;
        break;
    case 3:
        data |= 12;
        break;
    }
    return data;
}

BlockStateDefinition* OldLogBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {VARIANT, AXIS};
    return new BlockStateDefinition(this, properties, 2);
}

not_null_ptr<ItemInstance> OldLogBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1, state->getValue(VARIANT)->getData()));
}

int OldLogBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

int OldLogBlock::getWoodType(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}
