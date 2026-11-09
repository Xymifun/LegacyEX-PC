#include "game/core/SeaPickle.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
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
#include "game/world/block/CoralBlock.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/item/ItemId.hpp"

AbstractProperty<int>* SeaPickle::COUNT = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"count", 0, 3));
AbstractProperty<int>* SeaPickle::FLANGE = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"flange", 0, 1));

static const AABB* SHAPE = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0);
const unsigned char LIGHT_EMISSION[4] = {5, 9, 12, 15};

SeaPickle::SeaPickle() : Bush(Material::REPLACEABLE_PLANT)
{
    DerivedInit();
    mFullBlock = false;
}

bool SeaPickle::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool SeaPickle::performBonemeal(bool simulate, Level* level, Random* random, const BlockPos& pos, const BlockState*)
{
    const BlockState* belowState = level->getBlockState(pos.below());
    Block* below = belowState->getBlock();
    if (this == Blocks::sea_pickle && level->isUnderWater(pos, BlockId::SEA_PICKLE, 0) && below == Blocks::coral_block)
    {
        if (belowState->getValue(CoralBlock::DEAD))
            return false;
        int offset = 0;
        int x = pos.x - 2;
        int layer = 0;
        int width = 1;
        for (int dx = 0; dx < 5; ++dx)
        {
            for (int i = 0; i < width; ++i)
            {
                int yStart = pos.y - 1;
                int yEnd = pos.y + 1;
                for (int y = yStart; y < yEnd; ++y)
                {
                    BlockPos target(x + dx, y, pos.z - offset + i);
                    if (!(target == pos) && random->nextInt(3) == 0
                        && level->isUnderWater(target, BlockId::SEA_PICKLE, 0))
                    {
                        Block* targetBelow = level->getBlock(target.below());
                        if (targetBelow == Blocks::coral_block)
                        {
                            if (!targetBelow->defaultBlockState()->getValue(CoralBlock::DEAD)
                                && level->isEmptyWaterBlock(target))
                            {
                                Block* existing = level->getBlock(target);
                                int minCount = 0;
                                int count;
                                if (existing != Blocks::sea_pickle
                                    || (minCount = existing->defaultBlockState()->getValue(COUNT)) < 3)
                                    count = random->nextInt(minCount, 3);
                                else
                                    count = 3;
                                if (!simulate)
                                {
                                    const BlockState* grown = Blocks::sea_pickle->defaultBlockState()->setValue(COUNT,
                                                                                                                count);
                                    level->setBlock(target, grown, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
                                }
                            }
                        }
                    }
                }
            }
            if (layer < 2)
            {
                width += 2;
                ++offset;
            }
            else
            {
                width -= 2;
                --offset;
            }
            ++layer;
        }

        if (!simulate)
        {
            const BlockState* full = Blocks::sea_pickle->defaultBlockState()->setValue(COUNT, 3);
            level->setBlock(pos, full, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        }
        return true;
    }
    return false;
}

bool SeaPickle::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    return performBonemeal(true, level, random, pos, state);
}

void SeaPickle::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    performBonemeal(false, level, random, pos, state);
}

int SeaPickle::getColor(const BlockState* state)
{
    return Blocks::grass->getColor(state);
}

int SeaPickle::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return Blocks::grass->getColor(levelSource, pos, state);
}

const MaterialColor* SeaPickle::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return Blocks::grass->getMapColor(state, levelSource, pos);
}

EBlockRenderShape SeaPickle::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_SEA_PICKLE;
}

Item* SeaPickle::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::sea_pickle);
}

void SeaPickle::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float, int)
{
    if (level->mIsClientSide)
        return;
    int count = state->getValue(COUNT);
    if (count < 4)
        ++count;
    Block::popResource(level, pos,
                       not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::sea_pickle), count,
                                                                   getSpawnResourcesAuxValue(state))));
}

int SeaPickle::getLightEmission(const BlockState* state)
{
    if (state->getBlock() == Blocks::sea_pickle)
    {
        int count = state->getValue(COUNT);
        if (state->getValue(FLANGE) != 1 && (unsigned int)count <= 3)
            return LIGHT_EMISSION[count];
    }
    return 0;
}

void SeaPickle::playerDestroy(Level* level, boost::shared_ptr<Player>, const BlockPos& pos, const BlockState* state,
                              boost::shared_ptr<BlockEntity>, not_null_ptr<ItemInstance>)
{
    int count = state->getValue(COUNT);
    if (count < 4)
        ++count;
    Block::popResource(level, pos,
                       not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::sea_pickle), count,
                                                                   getSpawnResourcesAuxValue(state))));
}

bool SeaPickle::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    const BlockState* below = level->getBlockState(pos.below());
    return mayPlaceOn(level, pos.below(), below);
}

const AABB* SeaPickle::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return SHAPE;
}

bool SeaPickle::mayReplaceWithPlace(LevelSource*, const BlockPos&)
{
    return true;
}
bool SeaPickle::canContainLiquid(const BlockState*) const
{
    return true;
}
bool SeaPickle::waterSpreadCausesSpawn()
{
    return true;
}

std::wstring SeaPickle::getTileItemIconName()
{
    return L"sea_pickle";
}

int SeaPickle::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(COUNT) | state->getValue(FLANGE) << 3;
}

unsigned int SeaPickle::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE)
    {
        int count = tooltipData.m_pLevel->getBlockState(tooltipData.m_blockPos)->getValue(COUNT);
        const BlockState* belowState = tooltipData.m_pLevel->getBlockState(tooltipData.m_blockPos.below());
        Block* below = belowState->getBlock();
        if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail)
        {
            if (DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0
                && below == Blocks::coral_block && !belowState->getValue(CoralBlock::DEAD) && count < 3)
            {
                return IDS_TOOLTIPS_GROW;
            }
        }
    }
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* SeaPickle::getBlockState(int data)
{
    int count = data & 7;
    return defaultBlockState()->setValue(COUNT, count)->setValue(FLANGE, (data >> 3) & 1);
}

BlockStateDefinition* SeaPickle::createBlockStateDefinition()
{
    static const Property* properties[] = {COUNT, FLANGE};
    return new BlockStateDefinition(this, properties, 2);
}

bool SeaPickle::isSilkTouchable()
{
    return false;
}

bool SeaPickle::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    int id = level->getBlockId(pos.x, pos.y, pos.z);
    if (id == BlockId::GLASS || id == BlockId::STAINED_GLASS)
        return true;
    if (level->isEmptyBlock(pos) || level->isEmptyWaterBlock(pos))
        return false;
    if (HalfSlabBlock::isHalfSlab(state) && state->getValue(HalfSlabBlock::TYPE) != HalfSlabBlock::TOP)
        return false;
    if (!state->isSolidBlockingBlock())
        return false;
    return true;
}

void SeaPickle::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (state->getValue(FLANGE) != 1 && level->getLiquidBlock(pos)->mMaterial == mMaterial)
    {
        level->setExtraBlock(pos, Blocks::flowing_water->defaultBlockState(), Block::UPDATE_ALL, false);
    }
}

void SeaPickle::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!canSurvive(level, pos, state))
    {
        int count = level->getBlockState(pos)->getValue(COUNT);
        if (count < 4)
            ++count;
        Block::popResource(level, pos,
                           not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::sea_pickle), count,
                                                                       getSpawnResourcesAuxValue(state))));
        level->setBlockAndData(pos.x, pos.y, pos.z, BlockId::AIR, 0, Block::UPDATE_ALL);
    }
}

void SeaPickle::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!level->mIsClientSide)
        level->addToTickNextTick(pos, this, 1);
}

void SeaPickle::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (level->mIsClientSide)
        return;
    checkAlive(level, pos, state);
    Block* block = level->getBlock(pos);
    if (block != this)
        return;

    int count = state->getValue(COUNT);
    bool flange = state->getValue(FLANGE) == 1;
    bool underWater = level->isUnderWater(pos, BlockId::SEA_PICKLE, 0);
    if (!flange)
    {
        if (!underWater)
        {
            const BlockState* exposed = block->defaultBlockState()->setValue(FLANGE, 1)->setValue(COUNT, count);
            level->setBlock(pos, exposed, Block::UPDATE_ALL, false);
        }
    }
    else if (underWater)
    {
        const BlockState* submerged = block->defaultBlockState()->setValue(FLANGE, 0)->setValue(COUNT, count);
        level->setBlock(pos, submerged, Block::UPDATE_ALL, false);
    }
}

bool SeaPickle::canProvideSupport(const BlockState*, const Direction*, EBlockSupportType)
{
    return false;
}
