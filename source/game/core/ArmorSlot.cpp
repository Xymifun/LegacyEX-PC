#include "game/core/ArmorSlot.hpp"

#include "game/entity/mob/Mob.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"

ArmorSlot::ArmorSlot(const EquipmentSlot* equipmentSlot, boost::shared_ptr<Container> container, int index, int x,
                     int y)
    : Slot(container, index, x, y),
      mEquipmentSlot(equipmentSlot)
{
}

int ArmorSlot::getMaxStackSize()
{
    return 1;
}

bool ArmorSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return mEquipmentSlot == Mob::getEquipmentSlotForItem(item);
}

bool ArmorSlot::mayPickup(boost::shared_ptr<Player> player)
{
    not_null_ptr<ItemInstance> item = getItem();
    if (!item->isEmpty() && !player->isCreative() && EnchantmentHelper::hasBindingCurse(item))
    {
        return false;
    }
    return Slot::mayPickup(player);
}

std::wstring ArmorSlot::getNoItemIcon()
{
    return std::wstring(L"");
}
