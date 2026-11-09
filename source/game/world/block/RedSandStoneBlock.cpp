#include "game/world/block/RedSandStoneBlock.hpp"
#include "game/core/StringIds.hpp"

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
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

RedSandStoneBlock::Variant* RedSandStoneBlock::Variant::DEFAULT = new RedSandStoneBlock::Variant(0, L"red_sandstone",
                                                                                                 IDS_TILE_REDSANDSTONE);
RedSandStoneBlock::Variant* RedSandStoneBlock::Variant::CHISELED = new RedSandStoneBlock::Variant(
    1, L"chiseled_red_sandstone", 0xD03F0851);
RedSandStoneBlock::Variant* RedSandStoneBlock::Variant::SMOOTH = new RedSandStoneBlock::Variant(
    2, L"smooth_red_sandstone", 0x40496822);
static RedSandStoneBlock::Variant* gRedSandStoneVariants[3] = {
    RedSandStoneBlock::Variant::DEFAULT, RedSandStoneBlock::Variant::CHISELED, RedSandStoneBlock::Variant::SMOOTH};
EnumProperty<RedSandStoneBlock::Variant*>* RedSandStoneBlock::TYPE = NULL;

static std::wstring gRedSandStoneTopTexture = L"red_sandstone_top";
static std::wstring gRedSandStoneBottomTexture = L"red_sandstone_bottom";
static std::wstring gRedSandStoneSideTextures[3] = {L"red_sandstone_normal", L"red_sandstone_carved",
                                                    L"red_sandstone_smooth"};

RedSandStoneBlock::Variant::Variant(int data, const std::wstring& name, int translationKey)
{
    mData = data;
    mName = name;
    mTranslationKey = translationKey;
}

int RedSandStoneBlock::Variant::getData()
{
    return mData;
}

RedSandStoneBlock::Variant* RedSandStoneBlock::Variant::byData(int data)
{
    if (data < 0 || data > 2)
    {
        data = 0;
    }
    return gRedSandStoneVariants[data];
}

std::wstring RedSandStoneBlock::Variant::toString() const
{
    return mName;
}

std::wstring RedSandStoneBlock::Variant::getSerializedName() const
{
    return mName;
}

int RedSandStoneBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

void RedSandStoneBlock::blockStaticCtor()
{
    std::wstring name = L"type";
    Predicate<Variant*>* predicate = Predicates<Variant*>::alwaysTrue;
    EnumProperty<Variant*>* type;
    {
        boost::unordered_set<Variant*> values;
        for (int i = 0; i < 3; i++)
        {
            values.insert(gRedSandStoneVariants[i]);
        }
        type = EnumProperty<Variant*>::create(name, typeid(Variant), values, predicate);
    }
    TYPE = type;
}

RedSandStoneBlock::RedSandStoneBlock() : Block(Material::STONE, SandBlock::Variant::RED_SAND->getMaterialColor())
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(TYPE, Variant::DEFAULT));
    memset(mTextures, 0, sizeof(mTextures));
}

int RedSandStoneBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

const BlockState* RedSandStoneBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(TYPE, Variant::byData(data));
}

int RedSandStoneBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

BlockStateDefinition* RedSandStoneBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = {TYPE};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* RedSandStoneBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(TYPE);
    if (direction == Direction::UP)
    {
        return mTopTexture;
    }
    if (direction == Direction::DOWN)
    {
        if (variant == Variant::CHISELED || variant == Variant::SMOOTH)
        {
            return mTopTexture;
        }
        return mBottomTexture;
    }
    return mTextures[variant->getData()];
}

void RedSandStoneBlock::registerIcons(IconRegister* iconRegister)
{
    mTextures[0] = iconRegister->registerIcon(gRedSandStoneSideTextures[0]);
    mTextures[1] = iconRegister->registerIcon(gRedSandStoneSideTextures[1]);
    mTextures[2] = iconRegister->registerIcon(gRedSandStoneSideTextures[2]);
    mTopTexture = iconRegister->registerIcon(gRedSandStoneTopTexture);
    mBottomTexture = iconRegister->registerIcon(gRedSandStoneBottomTexture);
}
