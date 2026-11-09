#ifndef GAME_CORE_WEAPONCATEGORY_HPP
#define GAME_CORE_WEAPONCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"
#include "game/item/SwordItem.hpp"

class WeaponCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const { return dynamic_cast<SwordItem*>(item) != NULL; }
};

#endif
