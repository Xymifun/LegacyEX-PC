#include "game/world/block/LeafBlockItem.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/world/block/LeafBlock.hpp"

LeafBlockItem::LeafBlockItem(LeafBlock* block) : BlockItem(block), mLeafBlock(block)
{
    setMaxDamage(0);
    setStackedByData(true);
}

int LeafBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue | 4;
}

int LeafBlockItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return mLeafBlock->getDescriptionId(itemInstance->getAuxValue());
}
