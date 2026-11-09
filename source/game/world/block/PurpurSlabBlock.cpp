#include "game/world/block/PurpurSlabBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Boxed.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<PurpurSlabBlock::Variant*>* PurpurSlabBlock::VARIANT = NULL;
PurpurSlabBlock::Variant* PurpurSlabBlock::Variant::DEFAULT = new PurpurSlabBlock::Variant(L"default");
static PurpurSlabBlock::Variant* gPurpurSlabVariants[1] = {PurpurSlabBlock::Variant::DEFAULT};

PurpurSlabBlock::Variant::Variant(std::wstring name)
{
    mName = name;
}

std::wstring PurpurSlabBlock::Variant::toString() const
{
    return mName;
}

std::wstring PurpurSlabBlock::Variant::getSerializedName() const
{
    return mName;
}

void PurpurSlabBlock::blockStaticCtor()
{
    std::wstring name(L"variant");
    EnumProperty<Variant*>* variant;
    Predicate<Variant*>* filter = Predicates<Variant*>::alwaysTrue;
    {
        boost::unordered_set<Variant*> values;
        values.insert(gPurpurSlabVariants[0]);
        variant = EnumProperty<Variant*>::create(name, typeid(Variant), values, filter);
    }
    VARIANT = variant;
}

PurpurSlabBlock::PurpurSlabBlock() : HalfSlabBlock(Material::STONE, MaterialColor::COLOR_MAGENTA)
{
}

void PurpurSlabBlock::DerivedInit()
{
    HalfSlabBlock::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    if (!isFullSize())
    {
        state = state->setValue(TYPE, BOTTOM);
    }
    registerDefaultState(state->setValue(VARIANT, Variant::DEFAULT));
}

Item* PurpurSlabBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::purpur_slab);
}

not_null_ptr<ItemInstance> PurpurSlabBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::purpur_slab));
}

const BlockState* PurpurSlabBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState()->setValue(VARIANT, Variant::DEFAULT);
    if (!isFullSize())
    {
        state = state->setValue(TYPE, (data & 8) == 0 ? BOTTOM : TOP);
    }
    return state;
}

int PurpurSlabBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = 0;
    if (!isFullSize() && state->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP)
    {
        data = 8;
    }
    return data;
}

int PurpurSlabBlock::getAuxName(int)
{
    return getDescriptionId(-1);
}

Property* PurpurSlabBlock::getTypeProperty()
{
    return VARIANT;
}

Boxed* PurpurSlabBlock::getTypeForItem(not_null_ptr<ItemInstance>)
{
    return new TypedBoxed<Variant*>(Variant::DEFAULT);
}

BlockStateDefinition* PurpurSlabBlock::createBlockStateDefinition()
{
    if (isFullSize())
    {
        static const Property* properties[1] = {VARIANT};
        return new BlockStateDefinition(this, properties, 1);
    }

    static const Property* properties[2] = {HalfSlabBlock::TYPE, VARIANT};
    return new BlockStateDefinition(this, properties, 2);
}
