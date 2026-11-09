#include "game/world/block/Predicates.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/TallGrassBlock.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/GrassColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Stats.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"

static const AABB* gTallGrassShape = AABB::newPermanent(0.1, 0.0, 0.1, 0.9, 0.8, 0.9);
static const std::wstring gIconNames[3] = {L"deadbush", L"tallgrass", L"fern"};

AbstractProperty<TallGrassBlock::Variant*>* TallGrassBlock::VARIANT = NULL;
TallGrassBlock::Variant* TallGrassBlock::Variant::DEAD_BUSH = new TallGrassBlock::Variant(0, L"dead_bush");
TallGrassBlock::Variant* TallGrassBlock::Variant::GRASS = new TallGrassBlock::Variant(1, L"tall_grass");
TallGrassBlock::Variant* TallGrassBlock::Variant::FERN = new TallGrassBlock::Variant(2, L"fern");
TallGrassBlock::Variant* TallGrassBlock::VARIANTS[3] = {TallGrassBlock::Variant::DEAD_BUSH,
                                                        TallGrassBlock::Variant::GRASS, TallGrassBlock::Variant::FERN};

TallGrassBlock::Variant::Variant(int data, const std::wstring& name) : mData(data), mName(name)
{
}

int TallGrassBlock::Variant::getData()
{
    return mData;
}

TallGrassBlock::Variant* TallGrassBlock::Variant::byData(int data)
{
    if (data < 0 || data > 2)
    {
        data = 0;
    }
    return VARIANTS[data];
}

std::wstring TallGrassBlock::Variant::toString() const
{
    return mName;
}

std::wstring TallGrassBlock::Variant::getSerializedName() const
{
    return mName;
}

void TallGrassBlock::blockStaticCtor()
{
    std::wstring name(L"type");
    AbstractProperty<Variant*>* property;
    {
        boost::unordered_set<Variant*> values;
        for (int i = 0; i < 3; ++i)
        {
            values.insert(VARIANTS[i]);
        }
        property = EnumProperty<Variant*>::create(name, typeid(Variant), values, Predicates<Variant*>::alwaysTrue);
    }
    VARIANT = property;
}

TallGrassBlock::TallGrassBlock() : Bush(Material::REPLACEABLE_PLANT)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, Variant::GRASS));
    memset(mIcons, 0, sizeof(mIcons));
}

bool TallGrassBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState* state, bool)
{
    return state->getValue(VARIANT) != Variant::DEAD_BUSH;
}

bool TallGrassBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

void TallGrassBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState* state)
{
    DoublePlantBlock::Variant* variant = gDoublePlantGrass;
    if (state->getValue(VARIANT) == Variant::FERN)
    {
        variant = gDoublePlantFern;
    }
    if (Blocks::double_plant->mayPlace(level, pos))
    {
        DoublePlantBlock* doublePlant = static_cast<DoublePlantBlock*>(Blocks::double_plant);
        doublePlant->placeAt(level, pos, variant, 2);
    }
}

const AABB* TallGrassBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return gTallGrassShape;
}

bool TallGrassBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()));
}

bool TallGrassBlock::mayReplaceWithPlace(LevelSource*, const BlockPos&)
{
    return true;
}

int TallGrassBlock::getColor() const
{
    return GrassColor::get(0.5, 1.0);
}

int TallGrassBlock::getColor(const BlockState* state)
{
    if (state->getBlock() != this)
    {
        return Block::getColor(state);
    }
    if (state->getValue(VARIANT) != Variant::DEAD_BUSH)
    {
        return GrassColor::get(0.5, 1.0);
    }
    return 0xFFFFFF;
}

int TallGrassBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int)
{
    const BlockState* state = levelSource->getBlockState(pos);
    return getColor(levelSource, pos, state);
}

int TallGrassBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    if (state->getBlock() != this)
    {
        return levelSource->getBiome(pos)->getGrassColor(pos);
    }
    if (state->getValue(VARIANT) == Variant::DEAD_BUSH)
    {
        return 0xFFFFFF;
    }
    return levelSource->getBiome(pos)->getGrassColor(pos);
}

Item* TallGrassBlock::getResource(const BlockState*, Random* random, int)
{
    return random->nextInt(8) == 0 ? Items::wheatSeeds : Items::air;
}

int TallGrassBlock::getResourceCountForLootBonus(int lootBonus, Random* random)
{
    return random->nextInt(lootBonus * 2 + 1) + 1;
}

void TallGrassBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                   const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                   not_null_ptr<ItemInstance> tool)
{
    if (!level->mIsClientSide && tool->getItem() == Items::shears)
    {
        Player* awardee = player.operator->();
        Stat* stat = gStats->get_blocksMined(Block::getId(this));
        awardee->awardStat(stat, GenericStats::param_blocksMined(state, 1));
        Variant* variant = state->getValue(VARIANT);
        Block::popResource(level, pos,
                           not_null_ptr<ItemInstance>(new ItemInstance(Blocks::tallgrass, 1, variant->getData())));
        return;
    }
    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}

not_null_ptr<ItemInstance> TallGrassBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(
        new ItemInstance(this, 1, state->getBlock()->convertBlockStateToLegacyData(state)));
}

const BlockState* TallGrassBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(VARIANT, Variant::byData(data));
}

int TallGrassBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

unsigned int TallGrassBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
    {
        return IDS_TOOLTIPS_GROW;
    }
    return Block::GetInteractTooltip(tooltipData);
}

bool TallGrassBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* TallGrassBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = {VARIANT};
    return new BlockStateDefinition(this, properties, 1);
}

EBlockOffsetType TallGrassBlock::getOffsetType()
{
    return BLOCK_OFFSET_XYZ;
}

TextureAtlasSprite* TallGrassBlock::getTexture(const Direction*, const BlockState* state)
{
    return mIcons[state->getValue(VARIANT)->getData()];
}

void TallGrassBlock::registerIcons(IconRegister* iconRegister)
{
    mIcons[0] = iconRegister->registerIcon(gIconNames[0]);
    mIcons[1] = iconRegister->registerIcon(gIconNames[1]);
    mIcons[2] = iconRegister->registerIcon(gIconNames[2]);
}
