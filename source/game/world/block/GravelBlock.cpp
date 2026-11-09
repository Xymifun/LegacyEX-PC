#include "game/world/block/GravelBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/core/SeaGrass.hpp"
#include "game/item/ItemId.hpp"

GravelBlock::GravelBlock() : FallingBlock()
{
    DerivedInit();
}

bool GravelBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool GravelBlock::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    return SeaGrass::trySpawnSeaGrass(true, level, random, pos, state);
}

void GravelBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    SeaGrass::trySpawnSeaGrass(false, level, random, pos, state);
}

Item* GravelBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    if (lootBonus > 3)
    {
        lootBonus = 3;
    }

    if (random->nextInt(10 - lootBonus * 3) == 0)
    {
        return Items::flint;
    }

    return Block::getResource(state, random, lootBonus);
}

const MaterialColor* GravelBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::STONE;
}

unsigned int GravelBlock::getDustColor(const BlockState*)
{
    return 0xff807c7bu;
}

unsigned int GravelBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE)
    {
        BlockPos above = tooltipData.m_blockPos.above();
        if (((Level*)tooltipData.m_pLevel)->isUnderWater(above, 270, 0) && tooltipData.m_eInteractionResult != 2
            && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        {
            return IDS_TOOLTIPS_GROW;
        }
    }

    return Block::GetInteractTooltip(tooltipData);
}
