#include "game/world/block/StoneBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/item/Item.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

StoneBlock::Variant* StoneBlock::Variant::STONE = NULL;
StoneBlock::Variant* StoneBlock::Variant::GRANITE = NULL;
StoneBlock::Variant* StoneBlock::Variant::SMOOTH_GRANITE = NULL;
StoneBlock::Variant* StoneBlock::Variant::DIORITE = NULL;
StoneBlock::Variant* StoneBlock::Variant::SMOOTH_DIORITE = NULL;
StoneBlock::Variant* StoneBlock::Variant::ANDESITE = NULL;
StoneBlock::Variant* StoneBlock::Variant::SMOOTH_ANDESITE = NULL;
EnumProperty<StoneBlock::Variant*>* StoneBlock::VARIANT = NULL;

StoneBlock::Variant* StoneBlock::Variant::BY_DATA[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

StoneBlock::Variant::Variant(int data, const MaterialColor* materialColor, const std::wstring& name, int nameId,
                             int descriptionId, const std::wstring& serializedName, bool natural)
    : mData(data),
      mMaterialColor(materialColor)
{
    mName = name;
    mNameId = nameId;
    mDescriptionId = descriptionId;
    mSerializedName = serializedName;
    mNatural = natural;
    mTexture = NULL;
}

const MaterialColor* StoneBlock::Variant::getMaterialColor()
{
    return mMaterialColor;
}
int StoneBlock::Variant::getData()
{
    return mData;
}
StoneBlock::Variant* StoneBlock::Variant::byData(int data)
{
    if (data < 0 || data > 6)
    {
        data = 0;
    }

    return BY_DATA[data];
}
TextureAtlasSprite* StoneBlock::Variant::getTexture()
{
    return mTexture;
}
void StoneBlock::Variant::registerIcons(IconRegister* iconRegister)
{
    mTexture = iconRegister->registerIcon(mSerializedName);
}
std::wstring StoneBlock::Variant::toString() const
{
    return mName;
}
std::wstring StoneBlock::Variant::getSerializedName() const
{
    return mName;
}
int StoneBlock::Variant::getNameId()
{
    return mNameId;
}
int StoneBlock::Variant::getDescriptionId()
{
    return mDescriptionId;
}
bool StoneBlock::Variant::isNatural() const
{
    return mNatural;
}
std::wstring StoneBlock::Variant::getName()
{
    return mName;
}

void StoneBlock::blockStaticCtor()
{
    Variant::STONE = new Variant(0, MaterialColor::STONE, L"stone", IDS_TILE_STONE, IDS_DESC_STONE, L"stone", true);
    Variant::GRANITE = new Variant(1, MaterialColor::DIRT, L"granite", IDS_TILE_STONE_GRANITE, IDS_DESC_STONE_GRANITE,
                                   L"stone_granite", true);
    Variant::SMOOTH_GRANITE = new Variant(2, MaterialColor::DIRT, L"smooth_granite", IDS_TILE_STONE_GRANITE_POLISHED,
                                          IDS_DESC_STONE_GRANITE_POLISHED, L"stone_granite_smooth", false);
    Variant::DIORITE = new Variant(3, MaterialColor::QUARTZ, L"diorite", IDS_TILE_STONE_DIORITE, IDS_DESC_STONE_DIORITE,
                                   L"stone_diorite", true);
    Variant::SMOOTH_DIORITE = new Variant(4, MaterialColor::QUARTZ, L"smooth_diorite", IDS_TILE_STONE_DIORITE_POLISHED,
                                          IDS_DESC_STONE_DIORITE_POLISHED, L"stone_diorite_smooth", false);
    Variant::ANDESITE = new Variant(5, MaterialColor::STONE, L"andesite", IDS_TILE_STONE_ANDESITE,
                                    IDS_DESC_STONE_ANDESITE, L"stone_andesite", true);
    Variant::SMOOTH_ANDESITE = new Variant(6, MaterialColor::STONE, L"smooth_andesite",
                                           IDS_TILE_STONE_ANDESITE_POLISHED, IDS_DESC_STONE_ANDESITE_POLISHED,
                                           L"stone_andesite_smooth", false);
    Variant::BY_DATA[0] = Variant::STONE;
    Variant::BY_DATA[1] = Variant::GRANITE;
    Variant::BY_DATA[2] = Variant::SMOOTH_GRANITE;
    Variant::BY_DATA[3] = Variant::DIORITE;
    Variant::BY_DATA[4] = Variant::SMOOTH_DIORITE;
    Variant::BY_DATA[5] = Variant::ANDESITE;
    Variant::BY_DATA[6] = Variant::SMOOTH_ANDESITE;
    std::wstring name(L"variant");
    EnumProperty<Variant*>* variant;
    {
        boost::unordered_set<Variant*> values;
        for (int i = 0; i < 7; ++i)
            values.insert(Variant::BY_DATA[i]);
        variant = EnumProperty<Variant*>::create(name, typeid(Variant), values, Predicates<Variant*>::alwaysTrue);
    }
    VARIANT = variant;
}

StoneBlock::StoneBlock() : Block(Material::STONE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue<Variant*>(VARIANT, Variant::STONE));
}

std::wstring StoneBlock::getName()
{
    return L10N::GetString(IDS_TILE_STONE);
}

const MaterialColor* StoneBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(VARIANT)->getMaterialColor();
}

Item* StoneBlock::getResource(const BlockState* state, Random*, int)
{
    if (state->getValue(VARIANT) == Variant::STONE)
    {
        return Item::byBlock(Blocks::cobblestone);
    }
    return Item::byBlock(Blocks::stone);
}

int StoneBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}
const BlockState* StoneBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue<Variant*>(VARIANT, Variant::byData(data));
}
int StoneBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}
BlockStateDefinition* StoneBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {VARIANT};
    return new BlockStateDefinition(this, properties, 1);
}
TextureAtlasSprite* StoneBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(VARIANT)->getTexture();
}
void StoneBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 7; ++i)
        Variant::BY_DATA[i]->registerIcons(iconRegister);
}
