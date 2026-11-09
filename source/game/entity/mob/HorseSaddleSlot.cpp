#include "game/entity/mob/HorseSaddleSlot.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/HorseInventoryMenu.hpp"
#include "game/entity/mob/AbstractHorse.hpp"

bool HorseSaddleSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return item->getItem() == Items::saddle && !hasItem() && mMenu->horse->canBeSaddled();
}

bool HorseSaddleSlot::isActive()
{
    return mMenu->horse->canBeSaddled();
}
