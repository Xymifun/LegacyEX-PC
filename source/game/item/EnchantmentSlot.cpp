#include "game/item/EnchantmentSlot.hpp"

bool EnchantmentSlot::mayPlace(not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}

bool EnchantmentSlot::mayCombine(not_null_ptr<ItemInstance> itemInstance)
{
    return false;
}

int EnchantmentSlot::getMaxStackSize()
{
    return 1;
}
