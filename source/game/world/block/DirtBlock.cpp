#include "game/world/block/DirtBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/SeaGrass.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemId.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

namespace
{
const int kWaterDepth = 0x10e;
const unsigned int kHoeTooltip = IDS_TOOLTIPS_TILL;
const unsigned int kDyeTooltip = IDS_TOOLTIPS_GROW;
}

EnumProperty<DirtBlock::Variant*>* DirtBlock::VARIANT = NULL;
DirtBlock::Variant* DirtBlock::Variant::DIRT = NULL;
DirtBlock::Variant* DirtBlock::Variant::COARSE_DIRT = NULL;
DirtBlock::Variant* DirtBlock::Variant::PODZOL = NULL;
static DirtBlock::Variant* gVariantsByData[3];
static AbstractProperty<bool>* gSnowyProperty = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"snowy"));

DirtBlock::Variant::Variant(int data, const std::wstring& name, int nameId, int descriptionId,
                            const MaterialColor* materialColor, const std::wstring& texture,
                            const std::wstring& topTexture, const std::wstring& sideTexture)
{
    mData = data;
    mName = name;
    mNameId = nameId;
    mDescriptionId = descriptionId;
    mMaterialColor = materialColor;
    mTexture = texture;
    mTopTexture = topTexture;
    mSideTexture = sideTexture;
    mIcon = NULL;
    mTopIcon = NULL;
    mSideIcon = NULL;
}

const MaterialColor* DirtBlock::Variant::getMaterialColor()
{
    return mMaterialColor;
}

int DirtBlock::Variant::getData()
{
    return mData;
}

DirtBlock::Variant* DirtBlock::Variant::byData(int data)
{
    if (data < 0 || data > 2)
    {
        data = 0;
    }

    return gVariantsByData[data];
}

TextureAtlasSprite* DirtBlock::Variant::getTexture(const Direction* direction)
{
    if (mTopIcon != NULL && direction == Direction::UP)
    {
        return mTopIcon;
    }
    if (mSideIcon != NULL && direction != Direction::UP && direction != Direction::DOWN)
    {
        return mSideIcon;
    }
    return mIcon;
}

void DirtBlock::Variant::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mTexture);
    if (!mTopTexture.empty())
    {
        mTopIcon = iconRegister->registerIcon(mTopTexture);
    }
    if (!mSideTexture.empty())
    {
        mSideIcon = iconRegister->registerIcon(mSideTexture);
    }
}

int DirtBlock::Variant::getNameId()
{
    return mNameId;
}

int DirtBlock::Variant::getDescriptionId()
{
    return mDescriptionId;
}

std::wstring DirtBlock::Variant::toString() const
{
    return mName;
}

std::wstring DirtBlock::Variant::getSerializedName() const
{
    return mName;
}

DirtBlock::DirtBlock() : Block(Material::DIRT)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, Variant::DIRT)->setValue(gSnowyProperty, false));
}

bool DirtBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool DirtBlock::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    return SeaGrass::trySpawnSeaGrass(true, level, random, pos, state);
}

void DirtBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    SeaGrass::trySpawnSeaGrass(false, level, random, pos, state);
}

void DirtBlock::blockStaticCtor()
{
    Variant::DIRT = new Variant(0, L"dirt", IDS_TILE_DIRT, IDS_DESC_DIRT, MaterialColor::DIRT, L"dirt", L"", L"");
    Variant::COARSE_DIRT = new Variant(1, L"coarse_dirt", IDS_TILE_DIRT_COARSE, IDS_DESC_DIRT_COARSE,
                                       MaterialColor::DIRT, L"coarse_dirt", L"", L"");
    Variant::PODZOL = new Variant(2, L"podzol", IDS_TILE_DIRT_PODZOL, IDS_DESC_DIRT_PODZOL, MaterialColor::PODZOL,
                                  L"dirt", L"dirt_podzol_top", L"dirt_podzol_side");

    gVariantsByData[0] = Variant::DIRT;
    gVariantsByData[1] = Variant::COARSE_DIRT;
    gVariantsByData[2] = Variant::PODZOL;

    std::wstring name = L"variant";
    boost::unordered_set<Variant*> variants;
    for (int i = 0; i < 3; i++)
    {
        variants.insert(gVariantsByData[i]);
    }
    VARIANT = EnumProperty<Variant*>::create(name, typeid(Variant), variants, Predicates<Variant*>::alwaysTrue);
}

const MaterialColor* DirtBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return state->getValue(VARIANT)->getMaterialColor();
}

const BlockState* DirtBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                             const BlockPos& pos)
{
    if (state->getValue(VARIANT) == Variant::PODZOL)
    {
        Block* above = levelSource->getBlockState(pos.above())->getBlock();
        state = state->setValue(gSnowyProperty, above == Blocks::snow || above == Blocks::snow_layer);
    }
    return state;
}

not_null_ptr<ItemInstance> DirtBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, state->getValue(VARIANT)->getData()));
}

const BlockState* DirtBlock::getBlockState(int data)
{
    static const BlockState* states[3] = {defaultBlockState()->setValue(VARIANT, Variant::byData(0)),
                                          defaultBlockState()->setValue(VARIANT, Variant::byData(1)),
                                          defaultBlockState()->setValue(VARIANT, Variant::byData(2))};
    if (data < 0 || data > 2)
    {
        data = 0;
    }
    return states[data];
}

int DirtBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

unsigned int DirtBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    switch (Item::getId(tooltipData.m_pItemInstance->getItem()))
    {
    case ItemId::WOODEN_HOE:
    case ItemId::STONE_HOE:
    case ItemId::IRON_HOE:
    case ItemId::DIAMOND_HOE:
    case ItemId::GOLDEN_HOE:
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success && tooltipData.m_bCanPlace
            && tooltipData.m_pBlockState->getValue(VARIANT) != Variant::PODZOL)
        {
            return kHoeTooltip;
        }
        break;
    case ItemId::DYE:
    {
        BlockPos above = tooltipData.m_blockPos.above();
        if (((Level*)tooltipData.m_pLevel)->isUnderWater(above, kWaterDepth, 0)
            && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
            && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        {
            return kDyeTooltip;
        }
    }
    break;
    default:
        return Block::GetInteractTooltip(tooltipData);
    }
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* DirtBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {VARIANT, gSnowyProperty};
    BlockStateDefinition* definition = new BlockStateDefinition(this, properties, 2);
    return definition;
}

int DirtBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    return (variant == Variant::PODZOL ? Variant::DIRT : variant)->getData();
}

TextureAtlasSprite* DirtBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return state->getValue(VARIANT)->getTexture(direction);
}

void DirtBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 3; ++i)
    {
        gVariantsByData[i]->registerIcons(iconRegister);
    }
}
