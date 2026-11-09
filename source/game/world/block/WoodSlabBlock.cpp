#include "game/world/block/Predicates.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/WoodSlabBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<PlanksBlock::Variant*>* WoodSlabBlock::VARIANT = NULL;
const int gWoodSlabDescriptionIds[6] = {(int)IDS_TILE_STONESLAB_OAK,    (int)IDS_TILE_STONESLAB_SPRUCE,
                                        (int)IDS_TILE_STONESLAB_BIRCH,  (int)IDS_TILE_STONESLAB_JUNGLE,
                                        (int)IDS_TILE_STONESLAB_ACACIA, (int)IDS_TILE_STONESLAB_DARK_OAK};

void WoodSlabBlock::blockStaticCtor()
{
    std::wstring name(L"variant");
    Predicate<PlanksBlock::Variant*>* filter = Predicates<PlanksBlock::Variant*>::alwaysTrue;
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

WoodSlabBlock::WoodSlabBlock() : HalfSlabBlock(Material::WOOD)
{
}

void WoodSlabBlock::DerivedInit()
{
    HalfSlabBlock::DerivedInit();

    const BlockState* state = mStateDefinition->any();
    if (!isFullSize())
    {
        state = state->setValue(TYPE, BOTTOM);
    }

    registerDefaultState(state->setValue(VARIANT, PlanksBlock::Variant::OAK));
}

const MaterialColor* WoodSlabBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(VARIANT)->getMaterialColor();
}

Item* WoodSlabBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::wooden_slab);
}

not_null_ptr<ItemInstance> WoodSlabBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wooden_slab, 1, data));
}

int WoodSlabBlock::getAuxName(int auxValue)
{
    return getDescriptionId(auxValue);
}

Property* WoodSlabBlock::getTypeProperty()
{
    return VARIANT;
}

Boxed* WoodSlabBlock::getTypeForItem(not_null_ptr<ItemInstance> itemInstance)
{
    return new TypedBoxed<PlanksBlock::Variant*>(PlanksBlock::Variant::byData(itemInstance->getAuxValue() & 7));
}

const BlockState* WoodSlabBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState()->setValue(VARIANT, PlanksBlock::Variant::byData(data & 7));

    if (!isFullSize())
    {
        state = state->setValue(TYPE, (data & 8) == 0 ? BOTTOM : TOP);
    }

    return state;
}

int WoodSlabBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();

    if (!isFullSize() && state->getValue(TYPE) == TOP)
    {
        data |= 8;
    }

    return data;
}

BlockStateDefinition* WoodSlabBlock::createBlockStateDefinition()
{
    if (isFullSize())
    {
        static const Property* properties[1] = {VARIANT};
        return new BlockStateDefinition(this, properties, 1);
    }

    static const Property* properties[2] = {TYPE, VARIANT};
    return new BlockStateDefinition(this, properties, 2);
}

int WoodSlabBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

TextureAtlasSprite* WoodSlabBlock::getTexture(const Direction* direction, const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    return Blocks::planks->getTexture(direction, Blocks::planks->getBlockState(data));
}

void WoodSlabBlock::registerIcons(IconRegister*)
{
}

int WoodSlabBlock::getDescriptionId(int auxValue)
{
    const BlockState* state = getBlockState(auxValue);
    int data = state->getValue(VARIANT)->getData();
    if (data == -1)
    {
        data = 0;
    }
    return gWoodSlabDescriptionIds[data];
}
