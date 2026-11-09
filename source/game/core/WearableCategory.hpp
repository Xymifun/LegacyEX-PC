#ifndef GAME_CORE_WEARABLECATEGORY_HPP
#define GAME_CORE_WEARABLECATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class WearableCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
