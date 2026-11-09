#ifndef GAME_CORE_ALLCATEGORY_HPP
#define GAME_CORE_ALLCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class AllCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
