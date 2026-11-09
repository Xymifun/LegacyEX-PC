#include "game/world/block/SandStoneBlock.hpp"

#include <cstring>
#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<SandStoneBlock::Variant*>* SandStoneBlock::TYPE = NULL;
SandStoneBlock::Variant* SandStoneBlock::SANDSTONE = new SandStoneBlock::Variant(0, L"sandstone", 0);
SandStoneBlock::Variant* SandStoneBlock::CHISELED = new SandStoneBlock::Variant(1, L"chiseled_sandstone", 1);
SandStoneBlock::Variant* SandStoneBlock::SMOOTH = new SandStoneBlock::Variant(2, L"smooth_sandstone", 2);
static SandStoneBlock::Variant* gSandStoneVariants[3] = {SandStoneBlock::SANDSTONE, SandStoneBlock::CHISELED,
                                                         SandStoneBlock::SMOOTH};
static std::wstring gSandStoneTopTexture = L"sandstone_top";
static std::wstring gSandStoneBottomTexture = L"sandstone_bottom";
static std::wstring gSandStoneSideTextures[3] = {L"sandstone_side", L"sandstone_carved", L"sandstone_smooth"};

SandStoneBlock::Variant::Variant(int data, const std::wstring& name, int translationKey)
{
    mData = data;
    mName = name;
    mTranslationKey = translationKey;
}

int SandStoneBlock::Variant::getData()
{
    return mData;
}

SandStoneBlock::Variant* SandStoneBlock::Variant::byData(int data)
{
    if (data < 0 || data > 2)
    {
        data = 0;
    }
    return gSandStoneVariants[data];
}

std::wstring SandStoneBlock::Variant::toString() const
{
    return mName;
}

std::wstring SandStoneBlock::Variant::getSerializedName() const
{
    return mName;
}

int SandStoneBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

void SandStoneBlock::blockStaticCtor()
{
    std::wstring name = L"type";
    Predicate<Variant*>* predicate = Predicates<Variant*>::alwaysTrue;
    EnumProperty<Variant*>* type;
    {
        boost::unordered_set<Variant*> values;
        for (int i = 0; i < 3; i++)
        {
            values.insert(gSandStoneVariants[i]);
        }
        type = EnumProperty<Variant*>::create(name, typeid(Variant), values, predicate);
    }
    TYPE = type;
}

SandStoneBlock::SandStoneBlock() : Block(Material::STONE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(TYPE, SANDSTONE));
    memset(mTextures, 0, sizeof(mTextures));
}

int SandStoneBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

const MaterialColor* SandStoneBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return MaterialColor::SAND;
}

const BlockState* SandStoneBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(TYPE, Variant::byData(data));
}

int SandStoneBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

BlockStateDefinition* SandStoneBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = {TYPE};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* SandStoneBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(TYPE);
    if (direction == Direction::UP || (direction == Direction::DOWN && (variant == CHISELED || variant == SMOOTH)))
    {
        return mTopTexture;
    }
    if (direction == Direction::DOWN)
    {
        return mBottomTexture;
    }
    return mTextures[variant->getData()];
}

void SandStoneBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 3; i++)
    {
        mTextures[i] = iconRegister->registerIcon(gSandStoneSideTextures[i]);
    }
    mTopTexture = iconRegister->registerIcon(gSandStoneTopTexture);
    mBottomTexture = iconRegister->registerIcon(gSandStoneBottomTexture);
}
