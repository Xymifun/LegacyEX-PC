#include "game/world/block/CropBlock.hpp"
#include "game/item/ItemId.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/FarmBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"

#include "game/core/StringConversion.hpp"

#include <string>


static const AABB* SHAPE_BY_AGE[8] = {
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.125, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.25, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.375, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.625, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.75, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.875, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)};
AbstractProperty<int>* CropBlock::AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 7));

CropBlock::CropBlock(bool derivedInit) : Bush()
{
    if (!derivedInit)
    {
        DerivedInit();
    }
    setDestroyTime(0.0f);
    setSoundType(SoundType::GRASS);
    setNotCollectStatistics();
    sendBlockData(15);
}

bool CropBlock::isMaxAge(const BlockState* state)
{
    return state->getValue(getAgeProperty()) >= getMaxAge();
}

bool CropBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState* state, bool)
{
    return !isMaxAge(state);
}

bool CropBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

void CropBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState* state)
{
    growCrops(level, pos, state);
}

void CropBlock::DerivedInit()
{
    Block::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    registerDefaultState(state->setValue(getAgeProperty(), 0));
    setTicking(true);
}

const AABB* CropBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return SHAPE_BY_AGE[state->getValue(getAgeProperty())];
}

bool CropBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    return state->getBlock() == Blocks::farmland;
}

AbstractProperty<int>* CropBlock::getAgeProperty()
{
    return AGE;
}

int CropBlock::getMaxAge()
{
    return 7;
}

int CropBlock::getAge(const BlockState* state)
{
    return state->getValue(getAgeProperty());
}

const BlockState* CropBlock::getStateForAge(int age)
{
    const BlockState* state = defaultBlockState();
    return state->setValue(getAgeProperty(), age);
}

float CropBlock::getGrowthSpeed(Block* block, Level* level, const BlockPos& pos)
{
    float speed = 1.0f;
    BlockPos below = pos.below();
    for (int x = -1; x <= 1; ++x)
    {
        for (int z = -1; z <= 1; ++z)
        {
            float contribution = 0.0f;
            const BlockState* state = level->getBlockState(below.offset(x, 0, z));
            if (state->getBlock() == Blocks::farmland)
            {
                contribution = 1.0f;
                if (state->getValue(FarmBlock::MOISTURE) > 0)
                    contribution = 3.0f;
            }
            if (x != 0 || z != 0)
                contribution *= 0.25f;
            speed += contribution;
        }
    }

    BlockPos north = pos.north();
    BlockPos south = pos.south();
    BlockPos west = pos.west();
    BlockPos east = pos.east();
    bool eastWest = block == level->getBlockState(west)->getBlock() || block == level->getBlockState(east)->getBlock();
    bool northSouth = block == level->getBlockState(north)->getBlock()
                      || block == level->getBlockState(south)->getBlock();
    if (eastWest && northSouth)
    {
        speed *= 0.5f;
    }
    else if (block == level->getBlockState(west.north())->getBlock()
             || block == level->getBlockState(east.north())->getBlock()
             || block == level->getBlockState(east.south())->getBlock()
             || block == level->getBlockState(west.south())->getBlock())
    {
        speed *= 0.5f;
    }
    return speed;
}

void CropBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    Bush::tick(level, pos, state, random);
    if (level->getRawBrightness(pos.above()) >= 9)
    {
        int age = getAge(state);
        if (age < getMaxAge() && random->nextInt(static_cast<int>(12.0f / getGrowthSpeed(this, level, pos))) == 0)
        {
            level->setBlock(pos, getStateForAge(age + 1), Block::UPDATE_CLIENTS, false);
        }
    }
}

void CropBlock::growCrops(Level* level, const BlockPos& pos, const BlockState* state)
{
    int age = getAge(state) + getBonemealAgeIncrease(level);
    int maxAge = getMaxAge();
    if (age > maxAge)
        age = maxAge;
    level->setBlock(pos, getStateForAge(age), Block::UPDATE_CLIENTS, false);
}

int CropBlock::getBonemealAgeIncrease(Level* level)
{
    return Mth::nextInt(level->mRandom, 2, 5);
}

bool CropBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return (level->getDaytimeRawBrightness(pos) >= 8 || level->canSeeSky(pos))
           && mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()));
}

Item* CropBlock::getBaseSeedId()
{
    return Items::wheatSeeds;
}

Item* CropBlock::getBasePlantId()
{
    return Items::wheat;
}

void CropBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
    Block::spawnResources(level, pos, state, chance, 0);
    if (level->mIsClientSide)
        return;
    int age = getAge(state);
    if (age < getMaxAge())
        return;
    for (int i = 0; i < lootBonus + 3; ++i)
    {
        if (level->mRandom->nextInt(getMaxAge() * 2) <= age)
        {
            Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(getBaseSeedId(), 1, 0)));
        }
    }
}

Item* CropBlock::getResource(const BlockState* state, Random*, int)
{
    return isMaxAge(state) ? getBasePlantId() : getBaseSeedId();
}

not_null_ptr<ItemInstance> CropBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(getBaseSeedId()));
}

const BlockState* CropBlock::getBlockState(int data)
{
    return getStateForAge(data);
}

int CropBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return getAge(state);
}

unsigned int CropBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0
        && isValidBonemealTarget(tooltipData.m_pLevel, tooltipData.m_blockPos, tooltipData.m_pBlockState, true))
    {
        return IDS_TOOLTIPS_GROW;
    }
    return Block::GetInteractTooltip(tooltipData);
}

bool CropBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* CropBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {AGE};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* CropBlock::getTexture(const Direction*, const BlockState* state)
{
    return mStageIcons[Mth::clamp(state->getValue(AGE), 0, 7)];
}

void CropBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 8; ++i)
    {
        mStageIcons[i] = iconRegister->registerIcon(L"crops_" + _toString<int>(i));
    }
}

EBlockRenderShape CropBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_ROW;
}
