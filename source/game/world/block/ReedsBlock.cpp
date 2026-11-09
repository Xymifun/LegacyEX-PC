#include "game/world/block/ReedsBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/Direction.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/core/DyeColor.hpp"
#include "game/item/ItemId.hpp"

static AbstractProperty<int>* AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 15));
static const AABB* REEDS_SHAPE = AABB::newPermanent(0.125, 0.0, 0.125, 0.875, 1.0, 0.875);

ReedsBlock::ReedsBlock() : Block(Material::PLANT)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(AGE, 0));
    setTicking(true);
}

bool ReedsBlock::isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState*, bool)
{
    int bottom = pos.y;
    int top = bottom + 1;
    int lower = bottom - 4;
    while (--bottom > lower && bottom > 0 && level->getBlockId(pos.x, bottom, pos.z) == BlockId::REEDS)
    {
        --lower;
    }

    int upper = top + 3;
    while (top < upper && top < level->getMaxBuildHeight() && level->getBlockId(pos.x, top, pos.z) == BlockId::REEDS)
    {
        ++top;
        ++upper;
    }

    ++bottom;
    --top;
    return 2 - (top - bottom) > 0;
}

bool ReedsBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

void ReedsBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState*)
{
    int bottom = pos.y;
    int top = bottom + 1;
    int lower = bottom - 4;
    while (--bottom > lower && bottom > 0 && level->getBlockId(pos.x, bottom, pos.z) == BlockId::REEDS)
    {
        --lower;
    }

    int upper = top + 3;
    while (top < upper && top < level->getMaxBuildHeight() && level->getBlockId(pos.x, top, pos.z) == BlockId::REEDS)
    {
        ++top;
        ++upper;
    }

    ++bottom;
    --top;
    int grow = 2 - (top - bottom);
    if (grow > 0)
    {
        int targetTop = top + grow;
        for (int placeY = top + 1; placeY <= targetTop; ++placeY)
        {
            BlockPos placement(pos.x, placeY, pos.z);
            if (!level->isEmptyBlock(placement))
            {
                return;
            }
            level->setBlockAndData(pos.x, placeY, pos.z, BlockId::REEDS, 0, Block::UPDATE_ALL);
        }
    }
}

const AABB* ReedsBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return REEDS_SHAPE;
}

bool ReedsBlock::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (canSurvive(level, pos, NULL))
    {
        return true;
    }

    spawnResources(level, pos, state, 0);
    level->removeBlock(pos);
    return false;
}

void ReedsBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (level->getBlockState(pos.below())->getBlock() != Blocks::reeds && !checkAlive(level, pos, state))
    {
        return;
    }
    if (level->isEmptyBlock(pos.above()))
    {
        int height = 1;
        while (level->getBlockState(pos.below(height))->getBlock() == this)
        {
            ++height;
        }
        if (height < 3)
        {
            int age = state->getValue(AGE);
            if (age == 15)
            {
                level->setBlockAndUpdate(pos.above(), defaultBlockState());
                level->setBlock(pos, state->setValue(AGE, 0), Block::UPDATE_INVISIBLE, false);
            }
            else
            {
                level->setBlock(pos, state->setValue(AGE, ++age), Block::UPDATE_INVISIBLE, false);
            }
        }
    }
}

bool ReedsBlock::mayPlace(Level* level, const BlockPos& pos)
{
    const Block* below = level->getBlockState(pos.below())->getBlock();
    if (below == this)
    {
        return true;
    }

    const Block* current = level->getBlockState(pos)->getBlock();
    if (current != Blocks::air && current != this)
    {
        return false;
    }

    if (below != Blocks::grass && below != Blocks::dirt && below != Blocks::sand)
    {
        return false;
    }

    BlockPos belowPos = pos.below();
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const Direction* direction = directions.data[i];
        const BlockState* adjacentState = level->getBlockState(belowPos.relative(direction));
        const BlockState* extraState = level->getExtraBlockState(belowPos.relative(direction));
        if (adjacentState->getMaterial() == Material::WATER || extraState->getMaterial() == Material::WATER
            || adjacentState->getBlock() == Blocks::frosted_ice)
        {
            return true;
        }
    }
    return false;
}

void ReedsBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    checkAlive(level, pos, state);
}

bool ReedsBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    return mayPlace(level, pos);
}

const AABB* ReedsBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL_AABB;
}

Item* ReedsBlock::getResource(const BlockState*, Random*, int)
{
    return Items::reeds;
}

bool ReedsBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool ReedsBlock::isCubeShaped(const BlockState*)
{
    return false;
}

not_null_ptr<ItemInstance> ReedsBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::reeds));
}

int ReedsBlock::getColor(LevelSource*, const BlockPos&, int)
{
    return 0xFFFFFF;
}

int ReedsBlock::getRenderLayer()
{
    return 0;
}

const BlockState* ReedsBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(AGE, data);
}

int ReedsBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(AGE);
}

bool ReedsBlock::waterSpreadCausesSpawn()
{
    return true;
}

bool ReedsBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* ReedsBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {AGE};
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape ReedsBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

bool ReedsBlock::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState*)
{
    return level->isEmptyBlock(pos.above());
}

EBlockRenderShape ReedsBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_CROSS;
}

unsigned int ReedsBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0
        && isValidBonemealTarget((Level*)tooltipData.m_pLevel, tooltipData.m_blockPos, tooltipData.m_pBlockState, true))
    {
        return IDS_TOOLTIPS_GROW;
    }
    return Block::GetInteractTooltip(tooltipData);
}
