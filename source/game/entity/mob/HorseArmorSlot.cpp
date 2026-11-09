#include "game/entity/mob/HorseArmorSlot.hpp"

#include "game/entity/mob/AbstractHorse.hpp"
#include "game/ui/HorseInventoryMenu.hpp"

bool HorseArmorSlot::mayPlace(not_null_ptr<ItemInstance> itemInstance)
{
    return menu->horse->isArmor(itemInstance);
}

bool HorseArmorSlot::isActive()
{
    return menu->horse->wearsArmor();
}

int HorseArmorSlot::getMaxStackSize()
{
    return 1;
}
