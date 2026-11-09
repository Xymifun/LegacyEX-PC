#ifndef GAME_CORE_FISHINGRODCATEGORY_HPP
#define GAME_CORE_FISHINGRODCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"
#include "game/item/FishingRodItem.hpp"

class FishingRodCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const { return dynamic_cast<FishingRodItem*>(item) != NULL; }
};

#endif
