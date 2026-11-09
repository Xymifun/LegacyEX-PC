#ifndef GAME_CORE_ARMORSLOT_HPP
#define GAME_CORE_ARMORSLOT_HPP

#include <string>

#include "game/core/EquipmentSlot.hpp"
#include "game/core/Slot.hpp"

class ArmorSlot : public Slot
{
public:
    ArmorSlot(const EquipmentSlot* equipmentSlot, boost::shared_ptr<Container> container, int index, int x, int y);

    virtual int getMaxStackSize();
    virtual bool mayPlace(not_null_ptr<ItemInstance> item);
    virtual bool mayPickup(boost::shared_ptr<Player> player);
    virtual std::wstring getNoItemIcon();

private:
    const EquipmentSlot* mEquipmentSlot;
};

#endif
