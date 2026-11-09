#include "game/world/block/StoneBrickBlock.hpp"

#include <cstring>
#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Material.hpp"

StoneBrickBlock::Variant* StoneBrickBlock::Variant::DEFAULT = new StoneBrickBlock::Variant(
    StoneBrickBlock::Variant::eDataDefault, L"stonebrick", IDS_TILE_STONE_BRICK_SMOOTH);
StoneBrickBlock::Variant* StoneBrickBlock::Variant::MOSSY = new StoneBrickBlock::Variant(
    StoneBrickBlock::Variant::eDataMossy, L"mossy_stonebrick", IDS_TILE_STONE_BRICK_SMOOTH_MOSSY);
StoneBrickBlock::Variant* StoneBrickBlock::Variant::CRACKED = new StoneBrickBlock::Variant(
    StoneBrickBlock::Variant::eDataCracked, L"cracked_stonebrick", IDS_TILE_STONE_BRICK_SMOOTH_CRACKED);
StoneBrickBlock::Variant* StoneBrickBlock::Variant::CHISELED = new StoneBrickBlock::Variant(
    StoneBrickBlock::Variant::eDataChiseled, L"chiseled_stonebrick", IDS_TILE_STONE_BRICK_SMOOTH_CHISELED);
StoneBrickBlock::Variant* StoneBrickBlock::Variant::BY_DATA[4] = {
    StoneBrickBlock::Variant::DEFAULT, StoneBrickBlock::Variant::MOSSY, StoneBrickBlock::Variant::CRACKED,
    StoneBrickBlock::Variant::CHISELED};

std::wstring StoneBrickBlock::TEXTURE_NAMES[4] = {std::wstring(L""), std::wstring(L"mossy"), std::wstring(L"cracked"),
                                                  std::wstring(L"carved")};

int StoneBrickBlock::DEFAULT_META = -1;
int StoneBrickBlock::MOSSY_META = -1;
int StoneBrickBlock::CRACKED_META = -1;
int StoneBrickBlock::CHISELED_META = -1;
EnumProperty<StoneBrickBlock::Variant*>* StoneBrickBlock::VARIANT = NULL;

StoneBrickBlock::Variant::Variant(int data, const std::wstring& name, int translationKey)
    : mData(data),
      mName(name),
      mTranslationKey(translationKey)
{
}

int StoneBrickBlock::Variant::getData()
{
    return mData;
}

StoneBrickBlock::Variant* StoneBrickBlock::Variant::byData(int data)
{
    if (data < 0 || data > 3)
    {
        data = 0;
    }
    return Variant::BY_DATA[data];
}

int StoneBrickBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

std::wstring StoneBrickBlock::Variant::toString() const
{
    return mName;
}

std::wstring StoneBrickBlock::Variant::getSerializedName() const
{
    return mName;
}

void StoneBrickBlock::blockStaticCtor()
{
    {
        std::wstring name(L"variant");
        const Predicate<Variant*>* filter = Predicates<Variant*>::alwaysTrue;
        EnumProperty<Variant*>* variant;
        {
            boost::unordered_set<Variant*> values;
            for (int i = 0; i < 4; ++i)
            {
                values.insert(Variant::BY_DATA[i]);
            }
            variant = EnumProperty<Variant*>::create(name, typeid(Variant), values, filter);
        }
        VARIANT = variant;
    }
    DEFAULT_META = Variant::DEFAULT->getData();
    MOSSY_META = Variant::MOSSY->getData();
    CRACKED_META = Variant::CRACKED->getData();
    CHISELED_META = Variant::CHISELED->getData();
}

StoneBrickBlock::StoneBrickBlock() : Block(Material::STONE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, Variant::DEFAULT));
    memset(mTextures, 0, sizeof(mTextures));
}

int StoneBrickBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

const BlockState* StoneBrickBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(VARIANT, Variant::byData(data));
}

int StoneBrickBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

BlockStateDefinition* StoneBrickBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {VARIANT};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* StoneBrickBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return mTextures[state->getValue(VARIANT)->getData()];
}

void StoneBrickBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 4; ++i)
    {
        std::wstring name = getIconName();
        if (!TEXTURE_NAMES[i].empty())
        {
            name += L"_" + TEXTURE_NAMES[i];
        }
        mTextures[i] = iconRegister->registerIcon(name);
    }
}

int StoneBrickBlock::getDescriptionId(int auxValue)
{
    return getBlockState(auxValue)->getValue(VARIANT)->getTranslationKey();
}
