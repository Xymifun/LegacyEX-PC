#ifndef GAME_CORE_LEGSARMORCATEGORY_HPP
#define GAME_CORE_LEGSARMORCATEGORY_HPP

#include "game/core/EquipmentSlot.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/EnchantmentCategory.hpp"

class LegsArmorCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const
    {
        ArmorItem* armorItem = dynamic_cast<ArmorItem*>(item);
        return armorItem != NULL && armorItem->mSlot == EquipmentSlot::LEGS;
    }
};

#endif
