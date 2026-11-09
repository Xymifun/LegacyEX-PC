#include "game/core/ShulkerBoxSlot.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"

ShulkerBoxSlot::ShulkerBoxSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

bool ShulkerBoxSlot::mayPlace(not_null_ptr<ItemInstance> itemInstance)
{
    return dynamic_cast<ShulkerBoxBlock*>(Block::byItem(itemInstance->getItem())) == NULL;
}
