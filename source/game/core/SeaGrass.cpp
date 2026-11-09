#include "game/core/SeaGrass.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Coral.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/ItemId.hpp"
#include "game/world/block/BlockId.hpp"

AbstractProperty<int>* SeaGrass::TYPE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"type", 0, 2));
const AABB* SeaGrass::SHAPE = AABB::newPermanent(0.10000000149011612, 0.0, 0.10000000149011612, 0.8999999761581421,
                                                 0.800000011920929, 0.8999999761581421);

SeaGrass::SeaGrass() : Bush(Material::REPLACEABLE_PLANT)
{
    DerivedInit();
    registerDefaultState(defaultBlockState()->setValue(TYPE, static_cast<int>(TYPE_SINGLE)));
    setTicking(true);
}

bool SeaGrass::isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool clientSide)
{
    return true;
}

bool SeaGrass::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    if (state->getBlock() == this && state->getValue(TYPE) == TYPE_SINGLE)
        return true;
    return false;
}

void SeaGrass::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    if (state->getValue(TYPE) == TYPE_SINGLE)
    {
        const BlockState* aboveState = level->getBlockState(pos.above());
        Block* aboveBlock = aboveState->getBlock();
        if (aboveBlock->isLiquidBlock() && aboveBlock->mMaterial == Material::WATER
            && aboveState->getValue(LiquidBlock::LEVEL) == 0)
        {
            const BlockState* topState = defaultBlockState()->setValue(TYPE, static_cast<int>(TYPE_TALL_TOP));
            const BlockState* bottomState = defaultBlockState()->setValue(TYPE, static_cast<int>(TYPE_TALL_BOTTOM));
            level->setBlock(pos.above(), topState, Block::UPDATE_ALL, false);
            level->setBlock(pos, bottomState, Block::UPDATE_ALL, false);
        }
    }
}

void SeaGrass::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!level->mIsClientSide)
    {
        checkAlive(level, pos, state);
        level->getBlock(pos);
    }
}

void SeaGrass::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"seagrass");
    mCarriedIcon = iconRegister->registerIcon(L"seagrass_carried");
    mDoubleTallTopIcon = iconRegister->registerIcon(L"seagrass_doubletall_top");
    mDoubleTallBottomIcon = iconRegister->registerIcon(L"seagrass_doubletall_bottom");
}

TextureAtlasSprite* SeaGrass::getTexture(const Direction* face, const BlockState* state)
{
    if (face == Direction::UP)
        return mCarriedIcon;

    int type = state->getValue(TYPE);
    switch (type)
    {
    default:
        return mIcon;
    case TYPE_TALL_TOP:
        return mDoubleTallTopIcon;
    case TYPE_TALL_BOTTOM:
        return mDoubleTallBottomIcon;
    }
}

EBlockRenderShape SeaGrass::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_SEAGRASS;
}

int SeaGrass::getColor(const BlockState* state)
{
    return Blocks::grass->getColor(state);
}

int SeaGrass::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return Blocks::grass->getColor(levelSource, pos, state);
}

const MaterialColor* SeaGrass::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return Blocks::grass->getMapColor(state, levelSource, pos);
}

Item* SeaGrass::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return NULL;
}

const BlockState* SeaGrass::getBlockState(int data)
{
    return defaultBlockState()->setValue(TYPE, data);
}

int SeaGrass::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(TYPE);
}

unsigned int SeaGrass::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0
        && tooltipData.m_pBlockState->getBlock() == this && tooltipData.m_pBlockState->getValue(TYPE) == TYPE_SINGLE)
    {
        return IDS_TOOLTIPS_GROW;
    }
    return Block::GetInteractTooltip(tooltipData);
}

EBlockOffsetType SeaGrass::getOffsetType()
{
    return BLOCK_OFFSET_XZ;
}

const AABB* SeaGrass::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return SHAPE;
}

void SeaGrass::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                               const BlockPos& neighborPos)
{
    bool clientSide = level->mIsClientSide;
    if (!clientSide)
    {
        bool survives = canSurvive(level, pos, state);
        if (!survives)
            level->addToTickNextTick(pos, this, 2);
    }
}

bool SeaGrass::mayPlace(Level* level, const BlockPos& pos, bool clientSide)
{
    Block* below = level->getBlock(pos.below());
    if (below != Blocks::magma && !below->mIsFence && below != Blocks::soul_sand
        && level->isSolidBlockingBlock(pos.below()))
    {
        if (clientSide)
            return true;
        if (level->isUnderWater(pos, BlockId::SEA_GRASS, 0) && level->getLiquidBlock(pos)->mMaterial == Material::WATER)
        {
            int waterLevel = level->getLiquidBlockState(pos)->getValue(LiquidBlock::LEVEL);
            if (waterLevel == 0)
                return true;
            if (waterLevel > 7)
                return true;
        }
    }
    return false;
}

bool SeaGrass::canSurvive(Level* level, const BlockPos& pos, const BlockState* state, bool clientSide)
{
    Block* block = level->getBlock(pos);
    if (block != this)
        return SeaGrass::mayPlace(level, pos, clientSide);
    if (!level->isUnderWater(pos, BlockId::SEA_GRASS, convertBlockStateToLegacyData(state)))
        return false;

    int type = state->getValue(TYPE);
    if (type == TYPE_TALL_TOP)
    {
        if (!level->isUnderWater(pos, BlockId::SEA_GRASS, convertBlockStateToLegacyData(state)))
            return false;
        const BlockState* belowState = level->getBlockState(pos.below());
        if (belowState->getBlock() == this && belowState->getValue(TYPE) == TYPE_TALL_BOTTOM)
            return true;
        return false;
    }
    else if (type == TYPE_TALL_BOTTOM)
    {
        const BlockState* aboveState = level->getBlockState(pos.above());
        if (aboveState->getBlock() == this && aboveState->getValue(TYPE) == TYPE_TALL_TOP
            && level->isSolidBlockingBlock(pos.below()))
            return true;
        return false;
    }
    return SeaGrass::mayPlace(level, pos, clientSide);
}

bool SeaGrass::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return SeaGrass::canSurvive(level, pos, state, false);
}

bool SeaGrass::mayPlace(Level* level, const BlockPos& pos)
{
    return SeaGrass::mayPlace(level, pos, false);
}

void SeaGrass::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->getLiquidBlock(pos)->mMaterial == mMaterial)
        level->setExtraBlock(pos, Blocks::flowing_water->defaultBlockState(), Block::UPDATE_ALL, false);
}

void SeaGrass::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                             const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                             not_null_ptr<ItemInstance> tool)
{
    if (tool->isEmpty() || Item::getId(tool->getItem()) != ItemId::SHEARS)
        return;

    int count = 2;
    if (state->getValue(TYPE) == TYPE_SINGLE)
        count = 1;
    Block::popResource(level, pos,
                       not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::sea_grass), count,
                                                                   getSpawnResourcesAuxValue(state))));
}

bool SeaGrass::trySpawnSeaGrass(bool simulate, Level* level, Random* random, const BlockPos& pos,
                                const BlockState* state)
{
    if (!level->isUnderWater(pos.above(), BlockId::SEA_GRASS, 0))
        return false;

    bool spawned = false;
    bool tall = false;
    const BlockState* tallTopState = Blocks::sea_grass->defaultBlockState()->setValue(TYPE,
                                                                                      static_cast<int>(TYPE_TALL_TOP));

    for (int attempt = 16; attempt < 64; ++attempt)
    {
        BlockPos spawnPos = pos.above();
        const int steps = attempt / 16;
        bool reachable = true;
        for (int step = 0; step < steps; ++step)
        {
            spawnPos = BlockPos(spawnPos.x + random->nextInt(3) - 1,
                                spawnPos.y + (random->nextInt(3) - 1) * random->nextInt(3) / 2,
                                spawnPos.z + random->nextInt(3) - 1);
            Block* ground = level->getBlock(spawnPos.below());
            if ((ground != Blocks::dirt && ground != Blocks::sand && ground != Blocks::gravel && ground != Blocks::clay)
                || !level->isEmptyWaterBlock(spawnPos))
            {
                reachable = false;
                break;
            }
        }
        if (!reachable || steps == 0)
            continue;

        if (!level->isUnderWater(spawnPos.above(), BlockId::SEA_GRASS, 0)
            || level->getBlock(spawnPos) == Blocks::sea_grass || level->getBlock(spawnPos) == Blocks::coral)
            continue;

        const BlockState* spawnState;
        int roll = random->nextInt(8);
        if (roll == 0)
        {
            tall = true;
            spawnState = Blocks::sea_grass->defaultBlockState()->setValue(TYPE, static_cast<int>(TYPE_TALL_BOTTOM));
        }
        else if (roll == 2)
        {
            spawnState = Blocks::coral->defaultBlockState()->setValue(Coral::VARIANT, random->nextInt(5));
        }
        else
        {
            spawnState = Blocks::sea_grass->defaultBlockState();
        }

        bool canPlace;
        if (tall)
            canPlace = spawnState != NULL && spawnState->getBlock()->mayPlace(level, spawnPos)
                       && level->isEmptyWaterBlock(spawnPos.above());
        else
            canPlace = spawnState != NULL && spawnState->getBlock()->mayPlace(level, spawnPos);
        if (canPlace)
        {
            spawned = true;
            if (!simulate)
            {
                if (tall)
                    level->setBlockAndUpdate(spawnPos.above(), tallTopState);
                level->setBlockAndUpdate(spawnPos, spawnState);
            }
        }
    }

    return spawned;
}

BlockStateDefinition* SeaGrass::createBlockStateDefinition()
{
    static const Property* properties[] = {TYPE};
    return new BlockStateDefinition(this, properties, 1);
}

bool SeaGrass::isSilkTouchable()
{
    return false;
}
