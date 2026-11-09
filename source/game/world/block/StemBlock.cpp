#include "game/world/block/StemBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CropBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TorchBlock.hpp"
#include "game/item/ItemId.hpp"

#include <algorithm>
#include <string>

static const std::wstring STEM_BENT_ICON_NAME = L"stem_bent";
AbstractProperty<int>* StemBlock::AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 7));
const AABB* STEM_SHAPES[8] = {AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.125, 0.625),
                              AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.25, 0.625),
                              AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.375, 0.625),
                              AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.5, 0.625),
                              AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.625, 0.625),
                              AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.75, 0.625),
                              AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.875, 0.625),
                              AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 1.0, 0.625)};
DirectionProperty* StemBlock::FACING = NULL;

StemBlock::StemBlock(Block* fruitType) : Bush()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(AGE, 0)->setValue(FACING, Direction::UP));
    mFruitType = fruitType;
    setTicking(true);
}

bool StemBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState* state, bool)
{
    return state->getValue(AGE) != 7;
}

bool StemBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

void StemBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState* state)
{
    growCrops(level, pos, state);
}

void StemBlock::blockStaticCtor()
{
    FACING = TorchBlock::FACING;
}

const AABB* StemBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return STEM_SHAPES[state->getValue(AGE)];
}

const BlockState* StemBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                             const BlockPos& pos)
{
    int age = state->getValue(AGE);
    state = state->setValue(FACING, Direction::UP);
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const Direction* direction = directions.data[i];
        if (levelSource->getBlockState(pos.relative(direction))->getBlock() == mFruitType && age == 7)
        {
            state = state->setValue(FACING, direction);
            break;
        }
    }
    return state;
}

bool StemBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    return state->getBlock() == Blocks::farmland;
}

void StemBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    BlockPos stemPos = pos;
    Block::tick(level, stemPos, state, random);
    BlockPos abovePos = stemPos.above();
    if (level->getRawBrightness(abovePos) >= 9)
    {
        float growthSpeed = CropBlock::getGrowthSpeed(this, level, stemPos);
        if (random->nextInt(static_cast<int>(25.0f / growthSpeed) + 1) == 0)
        {
            int age = state->getValue(AGE);
            if (age < 7)
            {
                state = state->setValue(AGE, age + 1);
                level->setBlock(stemPos, state, Block::UPDATE_CLIENTS, false);
            }
            else
            {
                arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
                for (unsigned int i = 0; i < directions.length; ++i)
                {
                    if (level->getBlockState(stemPos.relative(directions.data[i]))->getBlock() == mFruitType)
                        return;
                }

                stemPos = stemPos.relative(Direction::Plane::HORIZONTAL->getRandomDirection(*random));
                Block* below = level->getBlockState(stemPos.below())->getBlock();
                if (level->getBlockState(stemPos)->getBlock()->mMaterial == Material::AIR
                    && (below == Blocks::farmland || below == Blocks::dirt || below == Blocks::grass))
                {
                    level->setBlockAndUpdate(stemPos, mFruitType->defaultBlockState());
                }
            }
        }
    }
}

void StemBlock::growCrops(Level* level, const BlockPos& pos, const BlockState* state)
{
    int age = state->getValue(AGE) + Mth::nextInt(level->mRandom, 2, 5);
    level->setBlock(pos, state->setValue(AGE, std::min(age, 7)), Block::UPDATE_CLIENTS, false);
}

int StemBlock::getColor(const BlockState* state)
{
    if (state->getBlock() != this)
        return Block::getColor(state);
    int age = state->getValue(AGE);
    int red = age * 32;
    int green = 255 - age * 8;
    int blue = age * 4;
    return (red << 16) | (green << 8) | blue;
}

int StemBlock::getColor(LevelSource* levelSource, const BlockPos& pos)
{
    return getColor(levelSource->getBlockState(pos));
}

int StemBlock::getColor(LevelSource*, const BlockPos&, const BlockState* state)
{
    return getColor(state);
}

bool StemBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

Item* StemBlock::getSeedItem()
{
    if (mFruitType == Blocks::pumpkin)
        return Items::pumpkinSeeds;
    if (mFruitType == Blocks::melon_block)
        return Items::melonSeeds;
    return NULL;
}

void StemBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
    Block::spawnResources(level, pos, state, chance, lootBonus);
    if (level->mIsClientSide)
        return;
    Item* seed = getSeedItem();
    if (seed == NULL)
        return;
    int age = state->getValue(AGE);
    for (int i = 0; i < 3; ++i)
    {
        if (level->mRandom->nextInt(15) <= age)
            Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(seed)));
    }
}

Item* StemBlock::getResource(const BlockState*, Random*, int)
{
    return Items::air;
}

not_null_ptr<ItemInstance> StemBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    Item* seed = getSeedItem();
    return seed == NULL ? not_null_ptr<ItemInstance>(ItemInstance::EMPTY)
                        : not_null_ptr<ItemInstance>(boost::shared_ptr<ItemInstance>(new ItemInstance(seed)));
}

const BlockState* StemBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(AGE, data);
}

int StemBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(AGE);
}

unsigned int StemBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0
        && isValidBonemealTarget(tooltipData.m_pLevel, tooltipData.m_blockPos, tooltipData.m_pBlockState, true))
        return IDS_TOOLTIPS_GROW;
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* StemBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {AGE, FACING};
    return new BlockStateDefinition(this, properties, 2);
}

void StemBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"stem_straight");
    mAngledTexture = iconRegister->registerIcon(STEM_BENT_ICON_NAME);
}

TextureAtlasSprite* StemBlock::getAngledTexture()
{
    return mAngledTexture;
}

EBlockRenderShape StemBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_STEM;
}
