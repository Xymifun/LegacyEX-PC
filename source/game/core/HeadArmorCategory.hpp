#ifndef GAME_CORE_HEADARMORCATEGORY_HPP
#define GAME_CORE_HEADARMORCATEGORY_HPP

#include "game/core/EquipmentSlot.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/EnchantmentCategory.hpp"

class HeadArmorCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const
    {
        ArmorItem* armorItem = dynamic_cast<ArmorItem*>(item);
        return armorItem != NULL && armorItem->mSlot == EquipmentSlot::HEAD;
    }
};

#endif
