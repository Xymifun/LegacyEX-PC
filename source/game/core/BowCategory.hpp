#ifndef GAME_CORE_BOWCATEGORY_HPP
#define GAME_CORE_BOWCATEGORY_HPP

#include "game/item/BowItem.hpp"
#include "game/item/EnchantmentCategory.hpp"

class BowCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const { return dynamic_cast<BowItem*>(item) != NULL; }
};

#endif
