#include "game/world/block/DyedBlockItem.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/ItemInstance.hpp"

DyedBlockItem::~DyedBlockItem()
{
}

int DyedBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

int DyedBlockItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return DyeColor::byBlockData(itemInstance->getAuxValue())->getName();
}
