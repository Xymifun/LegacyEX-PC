#include "game/world/block/NewLogBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/Direction.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<PlanksBlock::Variant*>* NewLogBlock::VARIANT = NULL;

bool newleaf_predicate(PlanksBlock::Variant* value)
{
    return value->getData() >= 4;
}

void NewLogBlock::blockStaticCtor()
{
    std::wstring name(L"variant");
    Predicates<PlanksBlock::Variant*>::FunctionPredicate* filter =
        new Predicates<PlanksBlock::Variant*>::FunctionPredicate(newleaf_predicate);
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

NewLogBlock::NewLogBlock() : LogBlock()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, PlanksBlock::Variant::ACACIA)->setValue(AXIS, Y));
}

const MaterialColor* NewLogBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    PlanksBlock::Variant* variant = state->getValue(VARIANT);
    LogBlock::LogAxis* axis = state->getValue(AXIS);
    if (axis->getId() != 1)
    {
        switch (variant->getData())
        {
        default:
            return MaterialColor::STONE;
        case 5:
            return PlanksBlock::Variant::DARK_OAK->getMaterialColor();
        }
    }
    return variant->getMaterialColor();
}

const BlockState* NewLogBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState()->setValue(VARIANT, PlanksBlock::Variant::byData((data & 3) + 4));
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

int NewLogBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    data -= 4;
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

BlockStateDefinition* NewLogBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {VARIANT, AXIS};
    return new BlockStateDefinition(this, properties, 2);
}

not_null_ptr<ItemInstance> NewLogBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(
        new ItemInstance(Item::byBlock(this), 1, state->getValue(VARIANT)->getData() - 4));
}

int NewLogBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData() - 4;
}

int NewLogBlock::getWoodType(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

bool NewLogBlock::isStrippable()
{
    return true;
}

const BlockState* NewLogBlock::getStrippedBlock(const BlockState* state)
{
    const BlockState* stripped = NULL;
    switch (getWoodType(state))
    {
    case 4:
        stripped = Blocks::stripped_log_acacia->defaultBlockState();
        break;
    case 5:
        stripped = Blocks::stripped_log_dark_oak->defaultBlockState();
        break;
    }
    if (stripped != NULL)
    {
        LogBlock::LogAxis* axis = state->getValue(AXIS);
        switch (axis->getId())
        {
        case 0:
            stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::X);
            break;
        case 2:
            stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Z);
            break;
        case 1:
            stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Y);
            break;
        }
        return stripped;
    }
    return state;
}
