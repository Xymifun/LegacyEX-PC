#include "game/world/block/FlowerBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockId.hpp"

#include <cstddef>
#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/ItemId.hpp"

FlowerBlock::FlowerCategory* FlowerBlock::FlowerCategory::YELLOW = new FlowerBlock::FlowerCategory();
FlowerBlock::FlowerCategory* FlowerBlock::FlowerCategory::RED = new FlowerBlock::FlowerCategory();

FlowerBlock::Flower* FlowerBlock::Flower::DANDELION = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::YELLOW, 0, L"dandelion", IDS_TILE_DANDELION, IDS_DESC_DANDELION, L"flower_dandelion",
    FlowerBlock::Flower::FLOWER_DANDELION);

FlowerBlock::Flower* FlowerBlock::Flower::POPPY = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 0, L"poppy", IDS_TILE_POPPY, IDS_DESC_POPPY, L"flower_rose",
    FlowerBlock::Flower::FLOWER_POPPY);

FlowerBlock::Flower* FlowerBlock::Flower::BLUE_ORCHID = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 1, L"blue_orchid", IDS_TILE_BLUE_ORCHID, IDS_DESC_ORCHID, L"flower_blue_orchid",
    FlowerBlock::Flower::FLOWER_BLUE_ORCHID);

FlowerBlock::Flower* FlowerBlock::Flower::ALLIUM = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 2, L"allium", IDS_TILE_ALLIUM, IDS_DESC_ALLIUM, L"flower_alium",
    FlowerBlock::Flower::FLOWER_BLUE_ORCHID);

FlowerBlock::Flower* FlowerBlock::Flower::HOUSTONIA = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 3, L"houstonia", IDS_TILE_AZURE_BLUET, IDS_DESC_AZURE_BLUET, L"flower_houstonia",
    FlowerBlock::Flower::FLOWER_HOUSTONIA);

FlowerBlock::Flower* FlowerBlock::Flower::RED_TULIP = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 4, L"red_tulip", IDS_TILE_TULIP_RED, IDS_DESC_TULIP_RED, L"flower_tulip_red",
    FlowerBlock::Flower::FLOWER_RED_TULIP);

FlowerBlock::Flower* FlowerBlock::Flower::ORANGE_TULIP = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 5, L"orange_tulip", IDS_TILE_TULIP_ORANGE, IDS_DESC_TULIP_ORANGE,
    L"flower_tulip_orange", FlowerBlock::Flower::FLOWER_ORANGE_TULIP);

FlowerBlock::Flower* FlowerBlock::Flower::WHITE_TULIP = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 6, L"white_tulip", IDS_TILE_TULIP_WHITE, IDS_DESC_TULIP_WHITE,
    L"flower_tulip_white", FlowerBlock::Flower::FLOWER_WHITE_TULIP);

FlowerBlock::Flower* FlowerBlock::Flower::PINK_TULIP = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 7, L"pink_tulip", IDS_TILE_TULIP_PINK, IDS_DESC_TULIP_PINK, L"flower_tulip_pink",
    FlowerBlock::Flower::FLOWER_PINK_TULIP);

FlowerBlock::Flower* FlowerBlock::Flower::OXEYE_DAISY = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 8, L"oxeye_daisy", IDS_TILE_OXEYE_DAISY, IDS_DESC_DAISY, L"flower_oxeye_daisy",
    FlowerBlock::Flower::FLOWER_OXEYE_DAISY);

FlowerBlock::Flower* FlowerBlock::Flower::FLOWERS[FlowerBlock::Flower::FLOWER_COUNT] = {
    FlowerBlock::Flower::DANDELION,    FlowerBlock::Flower::POPPY,       FlowerBlock::Flower::BLUE_ORCHID,
    FlowerBlock::Flower::ALLIUM,       FlowerBlock::Flower::HOUSTONIA,   FlowerBlock::Flower::RED_TULIP,
    FlowerBlock::Flower::ORANGE_TULIP, FlowerBlock::Flower::WHITE_TULIP, FlowerBlock::Flower::PINK_TULIP,
    FlowerBlock::Flower::OXEYE_DAISY};

FlowerBlock::Flower* FlowerBlock::Flower::YELLOW_FLOWERS[FlowerBlock::Flower::YELLOW_FLOWER_COUNT] = {
    FlowerBlock::Flower::DANDELION};

FlowerBlock::Flower* FlowerBlock::Flower::RED_FLOWERS[FlowerBlock::Flower::RED_FLOWER_COUNT] = {
    FlowerBlock::Flower::POPPY,       FlowerBlock::Flower::BLUE_ORCHID, FlowerBlock::Flower::ALLIUM,
    FlowerBlock::Flower::HOUSTONIA,   FlowerBlock::Flower::RED_TULIP,   FlowerBlock::Flower::ORANGE_TULIP,
    FlowerBlock::Flower::WHITE_TULIP, FlowerBlock::Flower::PINK_TULIP,  FlowerBlock::Flower::OXEYE_DAISY};

FlowerBlock::Flower::Flower(FlowerCategory* category, int data, const std::wstring& name, int nameId, int descriptionId,
                            const std::wstring& iconName, EFlower id)
    : mCategory(category),
      mData(data),
      mName(name),
      mNameId(nameId),
      mDescriptionId(descriptionId),
      mIconName(iconName),
      mIcon(NULL),
      mId(id)
{
}

std::wstring FlowerBlock::Flower::toString() const
{
    return mName;
}

std::wstring FlowerBlock::Flower::getSerializedName() const
{
    return mName;
}

FlowerBlock::FlowerCategory* FlowerBlock::Flower::getCategory() const
{
    return mCategory;
}

int FlowerBlock::Flower::getData() const
{
    return mData;
}

FlowerBlock::Flower::EFlower FlowerBlock::Flower::getId() const
{
    return mId;
}

int FlowerBlock::Flower::getNameId() const
{
    return mNameId;
}

int FlowerBlock::Flower::getDescriptionId() const
{
    return mDescriptionId;
}

TextureAtlasSprite* FlowerBlock::Flower::getTexture() const
{
    return mIcon;
}

void FlowerBlock::Flower::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mIconName);
}

std::pair<FlowerBlock::Flower**, int> FlowerBlock::Flower::byCategory(const FlowerCategory* category)
{
    if (category == FlowerCategory::YELLOW)
    {
        return std::make_pair(YELLOW_FLOWERS, (int)YELLOW_FLOWER_COUNT);
    }

    return std::make_pair(RED_FLOWERS, (int)RED_FLOWER_COUNT);
}

FlowerBlock::Flower* FlowerBlock::Flower::byData(const FlowerCategory* category, int data)
{
    std::pair<Flower**, int> flowers = byCategory(category);

    if (data < 0 || data >= flowers.second)
    {
        data = 0;
    }

    return flowers.first[data];
}

Block* FlowerBlock::FlowerCategory::getBlock()
{
    return this == YELLOW ? Blocks::yellow_flower : Blocks::red_flower;
}

FlowerBlock::FlowerCategoryPredicate::FlowerCategoryPredicate(FlowerBlock* block) : mBlock(block)
{
}

bool FlowerBlock::FlowerCategoryPredicate::apply(FlowerBlock::Flower* flower) const
{
    return flower->getCategory() == mBlock->getFlowerCategory();
}

FlowerBlock::FlowerBlock() : mTypeProperty(NULL)
{
}

void FlowerBlock::DerivedInit()
{
    Block::DerivedInit();

    BlockStateDefinition::BlockStateImpl* state = mStateDefinition->any();
    AbstractProperty<Flower*>* property = getFlowerProperty();

    registerDefaultState(
        state->setValue(property, getFlowerCategory() == FlowerCategory::RED ? Flower::POPPY : Flower::DANDELION));
}

AbstractProperty<FlowerBlock::Flower*>* FlowerBlock::getFlowerProperty()
{
    if (mTypeProperty == NULL)
    {
        std::wstring name = L"type";

        FlowerCategoryPredicate* filter = new FlowerCategoryPredicate(this);
        AbstractProperty<Flower*>* property;

        {
            boost::unordered_set<Flower*> values;
            for (int i = 0; i < Flower::FLOWER_COUNT; ++i)
            {
                values.insert(Flower::FLOWERS[i]);
            }

            property = EnumProperty<Flower*>::create(name, typeid(Flower), values, filter);
        }

        mTypeProperty = property;
    }

    return mTypeProperty;
}

BlockStateDefinition* FlowerBlock::createBlockStateDefinition()
{
    const Property* properties[] = {getFlowerProperty()};

    return new BlockStateDefinition(this, properties, 1);
}

const BlockState* FlowerBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    AbstractProperty<Flower*>* property = getFlowerProperty();
    FlowerCategory* category = getFlowerCategory();
    TypedBoxed<Flower*> flower(Flower::byData(category, data));
    const BlockState* result = state->setBoxedValue(property, &flower);

    return result;
}

int FlowerBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(getFlowerProperty())->getData();
}

int FlowerBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(getFlowerProperty())->getData();
}

EBlockOffsetType FlowerBlock::getOffsetType()
{
    return BLOCK_OFFSET_XZ;
}

const AABB* FlowerBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    const Vec3* offset = state->getOffset(levelSource, pos);

    return Bush::getShape(state, levelSource, pos)->move(*offset);
}

bool FlowerBlock::canContainLiquid(const BlockState* state) const
{
    return false;
}

bool FlowerBlock::mayPlace(Level* level, const BlockPos& pos)
{
    Block* liquidBlock = level->getLiquidBlock(pos);

    if (liquidBlock->mMaterial == Material::WATER || liquidBlock->mMaterial == Material::LAVA)
    {
        return false;
    }

    return Bush::mayPlace(level, pos);
}

TextureAtlasSprite* FlowerBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return state->getValue(mTypeProperty)->getTexture();
}

void FlowerBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < Flower::FLOWER_COUNT; ++i)
    {
        Flower* flower = Flower::FLOWERS[i];
        flower->registerIcons(iconRegister);
    }
}

unsigned int FlowerBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());

    if (itemId == ItemId::DYE && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail)
    {
        const DyeColor* color = DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue());

        if (color->getBlockData() == 0)
        {
            return IDS_TOOLTIPS_GROW;
        }
    }

    return Block::GetInteractTooltip(tooltipData);
}

bool FlowerBlock::isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide)
{
    return true;
}

bool FlowerBlock::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    return performBonemeal(true, level, random, pos, state);
}

void FlowerBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    performBonemeal(false, level, random, pos, state);
}

bool FlowerBlock::performBonemeal(bool checkOnly, Level* level, Random* random, const BlockPos& pos,
                                  const BlockState* state)
{
    Random* levelRandom = level->mRandom;
    bool success = false;

    for (int distance = 16; distance < 64; ++distance)
    {
        MutableBlockPos flowerPos(pos);

        if (!randomWalk(level, flowerPos, distance)
            || level->getBlockId(flowerPos.x, flowerPos.y, flowerPos.z) != BlockId::AIR)
        {
            continue;
        }

        int roll = levelRandom->nextInt(8);
        Block* flowerBlock = Blocks::red_flower;
        if (roll == 0)
        {
            if (this == flowerBlock)
            {
                flowerBlock = Blocks::yellow_flower;
            }
        }
        else if (this != flowerBlock)
        {
            flowerBlock = Blocks::yellow_flower;
        }

        if (flowerBlock != NULL && flowerBlock->canSurvive(level, flowerPos, state))
        {
            if (!checkOnly)
            {
                level->setBlockAndData(flowerPos.x, flowerPos.y, flowerPos.z, Block::getId(flowerBlock), 0,
                                       Block::UPDATE_ALL);
            }

            success = true;
        }
    }

    return success;
}

bool FlowerBlock::randomWalk(Level* level, MutableBlockPos& pos, int distance) const
{
    Random* random = level->mRandom;
    int steps = distance / 16;

    for (int step = 0; step < steps; ++step)
    {
        pos.addX(random->nextInt(3) - 1);
        int yDirection = random->nextInt(3) - 1;
        pos.addY(yDirection * random->nextInt(3) / 2);
        pos.addZ(random->nextInt(3) - 1);

        if (level->getBlockId(pos.x, pos.y - 1, pos.z) != BlockId::GRASS)
        {
            return false;
        }

        if (level->isSolidBlockingBlockInLoadedChunk(pos, true))
        {
            return false;
        }
    }

    return steps > 0;
}
