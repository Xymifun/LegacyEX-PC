#ifndef GAME_CORE_ARMORCATEGORY_HPP
#define GAME_CORE_ARMORCATEGORY_HPP

#include "game/item/ArmorItem.hpp"
#include "game/item/EnchantmentCategory.hpp"

class ArmorCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const { return dynamic_cast<ArmorItem*>(item) != NULL; }
};

#endif
