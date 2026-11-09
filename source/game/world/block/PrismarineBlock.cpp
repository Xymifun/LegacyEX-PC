#include "game/world/block/PrismarineBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

PrismarineBlock::Variant* PrismarineBlock::Variant::ROUGH = new Variant(0, L"prismarine", IDS_TILE_PRISMARINE,
                                                                        IDS_DESC_PRISMARINE, L"prismarine_rough");
PrismarineBlock::Variant* PrismarineBlock::Variant::BRICKS = new Variant(
    1, L"prismarine_bricks", IDS_TILE_BRICKS_PRISMARINE, IDS_DESC_BRICKS_PRISMARINE, L"prismarine_bricks");
PrismarineBlock::Variant* PrismarineBlock::Variant::DARK = new Variant(2, L"dark_prismarine", IDS_TILE_PRISMARINE_DARK,
                                                                       IDS_DESC_PRISMARINE_DARK, L"prismarine_dark");

static PrismarineBlock::Variant* gPrismarineVariants[3] = {
    PrismarineBlock::Variant::ROUGH, PrismarineBlock::Variant::BRICKS, PrismarineBlock::Variant::DARK};
EnumProperty<PrismarineBlock::Variant*>* PrismarineBlock::VARIANT = NULL;
static Predicate<PrismarineBlock::Variant*>* gPrismarineVariantPredicate =
    new Predicates<PrismarineBlock::Variant*>::ConstantPredicate(true);
int PrismarineBlock::ROUGH_DATA;
int PrismarineBlock::BRICKS_DATA;
int PrismarineBlock::DARK_DATA;

PrismarineBlock::Variant::Variant(int data, const std::wstring& name, int nameId, int descriptionId,
                                  const std::wstring& serializedName)
{
    mData = data;
    mName = name;
    mNameId = nameId;
    mDescriptionId = descriptionId;
    mSerializedName = serializedName;
}

std::wstring PrismarineBlock::Variant::toString() const
{
    return mName;
}

std::wstring PrismarineBlock::Variant::getSerializedName() const
{
    return mName;
}

int PrismarineBlock::Variant::getData()
{
    return mData;
}

PrismarineBlock::Variant* PrismarineBlock::Variant::byData(int data)
{
    if (data > 2 || data < 0)
    {
        data = 0;
    }

    return gPrismarineVariants[data];
}

TextureAtlasSprite* PrismarineBlock::Variant::getTexture()
{
    return mTexture;
}

void PrismarineBlock::Variant::registerIcons(IconRegister* iconRegister)
{
    mTexture = iconRegister->registerIcon(mSerializedName);
}

int PrismarineBlock::Variant::getNameId()
{
    return mNameId;
}

int PrismarineBlock::Variant::getDescriptionId()
{
    return mDescriptionId;
}

void PrismarineBlock::blockStaticCtor()
{
    {
        std::wstring name = L"variant";
        const Predicate<Variant*>* filter = gPrismarineVariantPredicate;
        EnumProperty<Variant*>* variant;
        {
            boost::unordered_set<Variant*> values;
            int i = 0;
            do
            {
                values.insert(gPrismarineVariants[i]);
                ++i;
            } while (i < 3);
            variant = EnumProperty<Variant*>::create(name, typeid(Variant), values, filter);
        }
        VARIANT = variant;
    }
    ROUGH_DATA = Variant::ROUGH->getData();
    BRICKS_DATA = Variant::BRICKS->getData();
    DARK_DATA = Variant::DARK->getData();
}

PrismarineBlock::PrismarineBlock() : Block(Material::STONE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, Variant::ROUGH));
}

std::wstring PrismarineBlock::getName()
{
    return L10N::GetString(IDS_TILE_PRISMARINE);
}

const MaterialColor* PrismarineBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(VARIANT) == Variant::ROUGH ? MaterialColor::COLOR_CYAN : MaterialColor::DIAMOND;
}

int PrismarineBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

int PrismarineBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

BlockStateDefinition* PrismarineBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {VARIANT};
    return new BlockStateDefinition(this, properties, 1);
}

const BlockState* PrismarineBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(VARIANT, Variant::byData(data));
}

TextureAtlasSprite* PrismarineBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(VARIANT)->getTexture();
}

void PrismarineBlock::registerIcons(IconRegister* iconRegister)
{
    gPrismarineVariants[0]->registerIcons(iconRegister);
    gPrismarineVariants[1]->registerIcons(iconRegister);
    gPrismarineVariants[2]->registerIcons(iconRegister);
}
