#include "game/item/EnchantmentCostSlot.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

EnchantmentCostSlot::EnchantmentCostSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

bool EnchantmentCostSlot::mayPlace(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->getItem() == Items::dye)
    {
        return DyeColor::byItemData(itemInstance->getAuxValue()) == DyeColor::BLUE;
    }
    return false;
}
