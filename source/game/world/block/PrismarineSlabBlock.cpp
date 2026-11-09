#include "game/world/block/PrismarineSlabBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/core/Direction.hpp"

PrismarineSlabBlock::Variant* PrismarineSlabBlock::Variant::ROUGH = NULL;
PrismarineSlabBlock::Variant* PrismarineSlabBlock::Variant::BRICKS = NULL;
PrismarineSlabBlock::Variant* PrismarineSlabBlock::Variant::DARK = NULL;
static PrismarineSlabBlock::Variant* gPrismarineSlabVariants[3];
EnumProperty<PrismarineSlabBlock::Variant*>* PrismarineSlabBlock::VARIANT = NULL;

PrismarineSlabBlock::Variant::Variant(int data, const std::wstring& name, int nameId, int descriptionId,
                                      const std::wstring& serializedName)
{
    mData = data;
    mName = name;
    mNameId = nameId;
    mDescriptionId = descriptionId;
    mSerializedName = serializedName;
}

int PrismarineSlabBlock::Variant::getData()
{
    return mData;
}

PrismarineSlabBlock::Variant* PrismarineSlabBlock::Variant::byData(int data)
{
    if (static_cast<unsigned int>(data) >= 3)
    {
        data = 0;
    }
    return gPrismarineSlabVariants[data];
}

int PrismarineSlabBlock::Variant::getNameId()
{
    return mNameId;
}

void PrismarineSlabBlock::Variant::registerIcons(IconRegister* iconRegister)
{
    mTexture = iconRegister->registerIcon(mSerializedName);
}

std::wstring PrismarineSlabBlock::Variant::toString() const
{
    return mName;
}

std::wstring PrismarineSlabBlock::Variant::getSerializedName() const
{
    return mName;
}

void PrismarineSlabBlock::blockStaticCtor()
{
    Variant::ROUGH = new Variant(0, L"prismarine_slab", IDS_TILE_SLAB_PRISMARINE, IDS_DESC_SLAB_PRISMARINE,
                                 L"prismarine_rough");
    Variant::BRICKS = new Variant(1, L"prismarine_bricks_slab", IDS_TILE_SLAB_PRISMARINE_BRICKS,
                                  IDS_DESC_SLAB_PRISMARINE, L"prismarine_bricks");
    Variant::DARK = new Variant(2, L"dark_prismarine_slab", IDS_TILE_SLAB_DARK_PRISMARINE, IDS_DESC_SLAB_PRISMARINE,
                                L"prismarine_dark");
    gPrismarineSlabVariants[0] = Variant::ROUGH;
    gPrismarineSlabVariants[1] = Variant::BRICKS;
    gPrismarineSlabVariants[2] = Variant::DARK;

    std::wstring name(L"variant");
    EnumProperty<Variant*>* variant;
    Predicate<Variant*>* filter = Predicates<Variant*>::alwaysTrue;
    {
        boost::unordered_set<Variant*> values;
        values.insert(gPrismarineSlabVariants[0]);
        values.insert(gPrismarineSlabVariants[1]);
        values.insert(gPrismarineSlabVariants[2]);
        variant = EnumProperty<Variant*>::create(name, typeid(Variant), values, filter);
    }
    VARIANT = variant;
}

PrismarineSlabBlock::PrismarineSlabBlock() : HalfSlabBlock(Material::STONE)
{
}

void PrismarineSlabBlock::DerivedInit()
{
    HalfSlabBlock::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    if (!isFullSize())
    {
        state = state->setValue(TYPE, BOTTOM);
    }
    registerDefaultState(state->setValue(VARIANT, Variant::ROUGH));
}

Item* PrismarineSlabBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::prismarine_slab);
}

const MaterialColor* PrismarineSlabBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(VARIANT) == Variant::ROUGH ? MaterialColor::COLOR_CYAN : MaterialColor::DIAMOND;
}

int PrismarineSlabBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

not_null_ptr<ItemInstance> PrismarineSlabBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(
        new ItemInstance(Blocks::prismarine_slab, 1, state->getValue(VARIANT)->getData()));
}

const BlockState* PrismarineSlabBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState()->setValue(VARIANT, Variant::byData(data & 7));
    if (!isFullSize())
    {
        state = state->setValue(TYPE, (data & 8) == 0 ? BOTTOM : TOP);
    }
    return state;
}

int PrismarineSlabBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    int data = variant->getData();
    if (!isFullSize() && state->getValue(TYPE) == TOP)
    {
        data |= 8;
    }
    return data;
}

int PrismarineSlabBlock::getAuxName(int auxValue)
{
    return Variant::byData(auxValue)->getNameId();
}

Property* PrismarineSlabBlock::getTypeProperty()
{
    return VARIANT;
}

Boxed* PrismarineSlabBlock::getTypeForItem(not_null_ptr<ItemInstance> itemInstance)
{
    return new TypedBoxed<Variant*>(Variant::byData(itemInstance->getAuxValue() & 7));
}

BlockStateDefinition* PrismarineSlabBlock::createBlockStateDefinition()
{
    if (isFullSize())
    {
        static const Property* properties[] = {VARIANT};
        return new BlockStateDefinition(this, properties, 1);
    }

    static const Property* properties[] = {TYPE, VARIANT};
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* PrismarineSlabBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    return Blocks::prismarine->getTexture(direction, Blocks::prismarine->getBlockState(variant->getData()));
}

void PrismarineSlabBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 3; ++i)
    {
        Variant* variant = gPrismarineSlabVariants[i];
        variant->registerIcons(iconRegister);
    }
}
