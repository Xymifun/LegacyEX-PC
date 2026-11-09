#ifndef GAME_CORE_FEETARMORCATEGORY_HPP
#define GAME_CORE_FEETARMORCATEGORY_HPP

#include "game/core/EquipmentSlot.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/EnchantmentCategory.hpp"

class FeetArmorCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const
    {
        ArmorItem* armorItem = dynamic_cast<ArmorItem*>(item);
        return armorItem != NULL && armorItem->mSlot == EquipmentSlot::FEET;
    }
};

#endif
