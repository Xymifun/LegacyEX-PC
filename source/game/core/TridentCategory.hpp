#ifndef GAME_CORE_TRIDENTCATEGORY_HPP
#define GAME_CORE_TRIDENTCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"
#include "game/item/TridentItem.hpp"

class TridentCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const { return dynamic_cast<TridentItem*>(item) != NULL; }
};

#endif
