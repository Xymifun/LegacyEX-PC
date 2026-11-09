#ifndef GAME_CORE_DIGGERCATEGORY_HPP
#define GAME_CORE_DIGGERCATEGORY_HPP

#include "game/item/DiggerItem.hpp"
#include "game/item/EnchantmentCategory.hpp"

class DiggerCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const { return dynamic_cast<DiggerItem*>(item) != NULL; }
};

#endif
