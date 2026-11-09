#include "game/world/block/NewStoneSlabBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Direction.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<NewStoneSlabBlock::Variant*>* NewStoneSlabBlock::VARIANT = NULL;
NewStoneSlabBlock::Variant* NewStoneSlabBlock::Variant::RED_SANDSTONE = NULL;
static Property* gNewStoneSlabSeamlessProperty = BooleanProperty::create(L"seamless");
static NewStoneSlabBlock::Variant* gDefaultVariant;

NewStoneSlabBlock::Variant::Variant(int data, const std::wstring& name, int translationKey,
                                    const MaterialColor* materialColor)
    : mData(data),
      mMaterialColor(materialColor)
{
    mName = name;
    mTranslationKey = translationKey;
}

NewStoneSlabBlock::Variant* NewStoneSlabBlock::Variant::byData(int data)
{
    return gDefaultVariant;
}

int NewStoneSlabBlock::Variant::getData()
{
    return mData;
}

const MaterialColor* NewStoneSlabBlock::Variant::getMaterialColor()
{
    return mMaterialColor;
}

std::wstring NewStoneSlabBlock::Variant::toString() const
{
    return mName;
}

std::wstring NewStoneSlabBlock::Variant::getSerializedName() const
{
    return mName;
}

NewStoneSlabBlock::NewStoneSlabBlock() : HalfSlabBlock(Material::STONE)
{
}

void NewStoneSlabBlock::DerivedInit()
{
    HalfSlabBlock::DerivedInit();

    const BlockState* state = mStateDefinition->any();
    bool fullSize = isFullSize();
    if (fullSize)
    {
        state = state->setValue(static_cast<const AbstractProperty<bool>*>(gNewStoneSlabSeamlessProperty), false);
    }
    else
    {
        state = state->setValue(TYPE, BOTTOM);
    }

    registerDefaultState(state->setValue(VARIANT, gDefaultVariant));
}

void NewStoneSlabBlock::blockStaticCtor()
{
    gDefaultVariant = new Variant(0, L"red_sandstone", IDS_TILE_STONESLAB_REDSANDSTONE,
                                  SandBlock::Variant::RED_SAND->getMaterialColor());
    Variant::RED_SANDSTONE = gDefaultVariant;

    std::wstring propertyName(L"variant");
    boost::unordered_set<Variant*> values;
    values.insert(Variant::RED_SANDSTONE);
    VARIANT = EnumProperty<Variant*>::create(propertyName, typeid(Variant), values, Predicates<Variant*>::alwaysTrue);
}

std::wstring NewStoneSlabBlock::getName()
{
    return L10N::GetString(IDS_TILE_STONESLAB);
}

Item* NewStoneSlabBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Item::byBlock(Blocks::stone_slab2);
}

not_null_ptr<ItemInstance> NewStoneSlabBlock::getCloneItemInstance(Level* level, const BlockPos& pos,
                                                                   const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::stone_slab2)));
}

int NewStoneSlabBlock::getAuxName(int auxValue)
{
    return Block::getDescriptionId(auxValue);
}

Property* NewStoneSlabBlock::getTypeProperty()
{
    return VARIANT;
}

Boxed* NewStoneSlabBlock::getTypeForItem(not_null_ptr<ItemInstance> itemInstance)
{
    return new TypedBoxed<Variant*>(Variant::byData(itemInstance->getAuxValue() & 7));
}

const BlockState* NewStoneSlabBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState()->setValue(VARIANT, Variant::byData(data & 7));

    if (isFullSize())
    {
        state = state->setValue(static_cast<const AbstractProperty<bool>*>(gNewStoneSlabSeamlessProperty),
                                (data & 8) != 0);
    }
    else
    {
        state = state->setValue(TYPE, (data & 8) == 0 ? BOTTOM : TOP);
    }

    return state;
}

int NewStoneSlabBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();

    if (isFullSize())
    {
        if (state->getValue(static_cast<const AbstractProperty<bool>*>(gNewStoneSlabSeamlessProperty)))
        {
            data |= 8;
        }
    }
    else if (state->getValue(TYPE) == TOP)
    {
        data |= 8;
    }

    return data;
}

BlockStateDefinition* NewStoneSlabBlock::createBlockStateDefinition()
{
    if (isFullSize())
    {
        static const Property* fullProperties[2] = {gNewStoneSlabSeamlessProperty, VARIANT};
        return new BlockStateDefinition(this, fullProperties, 2);
    }

    static const Property* halfProperties[2] = {TYPE, VARIANT};
    return new BlockStateDefinition(this, halfProperties, 2);
}

const MaterialColor* NewStoneSlabBlock::getMapColor(const BlockState* state, LevelSource* levelSource,
                                                    const BlockPos& pos)
{
    return state->getValue(VARIANT)->getMaterialColor();
}

int NewStoneSlabBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

TextureAtlasSprite* NewStoneSlabBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP || direction == Direction::DOWN)
    {
        return mIcon;
    }

    return mSideTexture;
}

void NewStoneSlabBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"red_sandstone_top");
    mSideTexture = iconRegister->registerIcon(L"red_sandstone_normal");
}
