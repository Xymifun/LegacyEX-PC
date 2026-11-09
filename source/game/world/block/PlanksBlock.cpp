#include "game/world/block/PlanksBlock.hpp"

#include <cstring>
#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

PlanksBlock::Variant* PlanksBlock::Variant::OAK = NULL;
PlanksBlock::Variant* PlanksBlock::Variant::SPRUCE = NULL;
PlanksBlock::Variant* PlanksBlock::Variant::BIRCH = NULL;
PlanksBlock::Variant* PlanksBlock::Variant::JUNGLE = NULL;
PlanksBlock::Variant* PlanksBlock::Variant::ACACIA = NULL;
PlanksBlock::Variant* PlanksBlock::Variant::DARK_OAK = NULL;

PlanksBlock::Variant* PlanksBlock::Variant::BY_DATA[6];
EnumProperty<PlanksBlock::Variant*>* PlanksBlock::VARIANT = NULL;

std::wstring PlanksBlock::TEXTURE_NAMES[6] = {std::wstring(L"oak"),    std::wstring(L"spruce"),
                                              std::wstring(L"birch"),  std::wstring(L"jungle"),
                                              std::wstring(L"acacia"), std::wstring(L"big_oak")};

const int gPlankTranslationKeys[6] = {
    PlanksBlock::Variant::eTranslationKeyOak,    PlanksBlock::Variant::eTranslationKeySpruce,
    PlanksBlock::Variant::eTranslationKeyBirch,  PlanksBlock::Variant::eTranslationKeyJungle,
    PlanksBlock::Variant::eTranslationKeyAcacia, PlanksBlock::Variant::eTranslationKeyDarkOak};

PlanksBlock::Variant::Variant(int data, const std::wstring& name, int translationKey,
                              const MaterialColor* materialColor)
    : mData(data),
      mName(name),
      mTranslationKey(translationKey),
      mMaterialColor(materialColor)
{
}

std::wstring PlanksBlock::Variant::toString() const
{
    return mName;
}

std::wstring PlanksBlock::Variant::getSerializedName() const
{
    return mName;
}

int PlanksBlock::Variant::getData()
{
    return mData;
}

int PlanksBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

const MaterialColor* PlanksBlock::Variant::getMaterialColor()
{
    return mMaterialColor;
}

PlanksBlock::Variant* PlanksBlock::Variant::byData(int data)
{
    if (static_cast<unsigned int>(data) >= 6)
    {
        data = 0;
    }
    return Variant::BY_DATA[data];
}

void PlanksBlock::blockStaticCtor()
{
    Variant::OAK = new Variant(Variant::eDataOak, L"oak", Variant::eTranslationKeyOak, MaterialColor::WOOD);
    Variant::SPRUCE = new Variant(Variant::eDataSpruce, L"spruce", Variant::eTranslationKeySpruce,
                                  MaterialColor::PODZOL);
    Variant::BIRCH = new Variant(Variant::eDataBirch, L"birch", Variant::eTranslationKeyBirch, MaterialColor::SAND);
    Variant::JUNGLE = new Variant(Variant::eDataJungle, L"jungle", Variant::eTranslationKeyJungle, MaterialColor::DIRT);
    Variant::ACACIA = new Variant(Variant::eDataAcacia, L"acacia", Variant::eTranslationKeyAcacia,
                                  MaterialColor::COLOR_ORANGE);
    Variant::DARK_OAK = new Variant(Variant::eDataDarkOak, L"dark_oak", Variant::eTranslationKeyDarkOak,
                                    MaterialColor::COLOR_BROWN);

    Variant::BY_DATA[4] = Variant::ACACIA;
    Variant::BY_DATA[2] = Variant::BIRCH;
    Variant::BY_DATA[1] = Variant::SPRUCE;
    Variant::BY_DATA[0] = Variant::OAK;
    Variant::BY_DATA[3] = Variant::JUNGLE;
    Variant::BY_DATA[5] = Variant::DARK_OAK;

    std::wstring name(L"variant");
    boost::unordered_set<Variant*> values;
    values.insert(Variant::BY_DATA[0]);
    values.insert(Variant::BY_DATA[1]);
    values.insert(Variant::BY_DATA[2]);
    values.insert(Variant::BY_DATA[3]);
    values.insert(Variant::BY_DATA[4]);
    values.insert(Variant::BY_DATA[5]);
    VARIANT = EnumProperty<Variant*>::create(name, typeid(Variant), values, Predicates<Variant*>::alwaysTrue);
}

PlanksBlock::PlanksBlock() : Block(Material::WOOD)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, Variant::OAK));
    memset(mIcons, 0, sizeof(mIcons));
}

int PlanksBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

const BlockState* PlanksBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(VARIANT, Variant::byData(data));
}

const MaterialColor* PlanksBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return state->getValue(VARIANT)->getMaterialColor();
}

int PlanksBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

BlockStateDefinition* PlanksBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {VARIANT};
    return new BlockStateDefinition(this, properties, 1);
}

int PlanksBlock::getDescriptionId(int auxValue)
{
    if (static_cast<unsigned int>(auxValue) >= 6)
    {
        auxValue = 0;
    }
    return gPlankTranslationKeys[auxValue];
}

TextureAtlasSprite* PlanksBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcons[state->getValue(VARIANT)->getData()];
}

void PlanksBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 6; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_NAMES[i]);
    }
}
