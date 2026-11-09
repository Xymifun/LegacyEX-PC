#include "game/world/block/ClayBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/SeaGrass.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/item/ItemId.hpp"

ClayBlock::ClayBlock() : Block(Material::CLAY)
{
    DerivedInit();
}

bool ClayBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool ClayBlock::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    return SeaGrass::trySpawnSeaGrass(true, level, random, pos, state);
}

void ClayBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    SeaGrass::trySpawnSeaGrass(false, level, random, pos, state);
}

int ClayBlock::getResourceCount(Random*)
{
    return 4;
}

Item* ClayBlock::getResource(const BlockState*, Random*, int)
{
    return Items::clayBall;
}

unsigned int ClayBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
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
