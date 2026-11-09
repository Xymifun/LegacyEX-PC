#include "game/world/block/MushroomBlock.hpp"
#include "game/item/ItemId.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/HugeMushroomFeature.hpp"

static const AABB* MUSHROOM_SHAPE = AABB::newPermanent(0.3f, 0.0, 0.3f, 0.7f, 0.4f, 0.7f);

MushroomBlock::MushroomBlock() : Bush()
{
    DerivedInit();
    setTicking(true);
}

bool MushroomBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool MushroomBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

bool MushroomBlock::growMushroom(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    level->removeBlock(pos);
    HugeMushroomFeature* feature = NULL;
    if (this == Blocks::brown_mushroom)
        feature = new HugeMushroomFeature(Blocks::brown_mushroom_block);
    else if (this == Blocks::red_mushroom)
        feature = new HugeMushroomFeature(Blocks::red_mushroom_block);

    if (feature != NULL && feature->place(level, *random, pos))
    {
        delete feature;
        return true;
    }

    level->setBlock(pos, state, Block::UPDATE_ALL, false);
    delete feature;
    return false;
}

void MushroomBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    growMushroom(level, pos, state, random);
}

const AABB* MushroomBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return MUSHROOM_SHAPE;
}

void MushroomBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    Bush::tick(level, pos, state, random);
    BlockPos currentPos = pos;
    if (random->nextInt(25) == 0)
    {
        int count = 5;
        Iterator<MutableBlockPos*>* positions = BlockPos::mutablesBetween(currentPos.offset(-4, -1, -4),
                                                                          currentPos.offset(4, 1, 4));
        while (positions->hasNext())
        {
            BlockPos current = *positions->next();
            if (level->getBlockState(current)->getBlock() == this)
            {
                --count;
                if (count < 1)
                {
                    delete positions;
                    return;
                }
            }
        }
        delete positions;

        BlockPos target = currentPos.offset(random->nextInt(3) - 1, random->nextInt(2) - random->nextInt(2),
                                            random->nextInt(3) - 1);
        for (int attempt = 0; attempt < 4; ++attempt)
        {
            if (level->isEmptyBlock(target) && canSurvive(level, target, defaultBlockState()))
                currentPos = target;
            target = currentPos.offset(random->nextInt(3) - 1, random->nextInt(2) - random->nextInt(2),
                                       random->nextInt(3) - 1);
        }

        if (level->isEmptyBlock(target) && canSurvive(level, target, defaultBlockState()))
            level->setBlock(target, defaultBlockState(), Block::UPDATE_CLIENTS, false);
    }
}

bool MushroomBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return Bush::mayPlace(level, pos) && canSurvive(level, pos, defaultBlockState());
}

bool MushroomBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    return state->isSolid();
}

bool MushroomBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (static_cast<unsigned int>(pos.y) >= 256)
        return false;

    const BlockState* belowState = level->getBlockState(pos.below());
    if (belowState->getBlock() == Blocks::mycelium)
        return true;

    if (belowState->getBlock() == Blocks::dirt
        && belowState->getValue(DirtBlock::VARIANT) == DirtBlock::Variant::PODZOL)
        return true;

    return level->getDaytimeRawBrightness(pos) < 13 && mayPlaceOn(level, pos.below(), belowState);
}

unsigned int MushroomBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        return IDS_TOOLTIPS_GROW;
    return Block::GetInteractTooltip(tooltipData);
}

bool MushroomBlock::canContainLiquid(const BlockState*) const
{
    return false;
}
