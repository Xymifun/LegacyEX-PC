#include "game/world/block/AnvilBlockItem.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/AnvilBlock.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

static const int ANVIL_BLOCK_NAME_IDS[] = {(int)0x99A9E601, (int)0x872533DB, (int)0x872533DB};

AnvilBlockItem::AnvilBlockItem(Block* block) : MultiBlockItem(block, block, AnvilBlockNameResolver, NULL)
{
}

int AnvilBlockItem::AnvilBlockNameResolver(int damage)
{
    return ANVIL_BLOCK_NAME_IDS[Blocks::anvil->getBlockState(damage)->getValue(AnvilBlock::DAMAGE)];
}

int AnvilBlockItem::AnvilBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    int auxValue = itemInstance->getAuxValue();
    return AnvilBlockNameResolver(auxValue);
}

int AnvilBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

int AnvilBlockItem::getDescriptionId(int auxValue)
{
    return AnvilBlockNameResolver(auxValue >> 2);
}
