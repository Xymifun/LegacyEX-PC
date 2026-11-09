#ifndef GAME_ITEM_OXYGENENCHANTMENT_HPP
#define GAME_ITEM_OXYGENENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class OxygenEnchantment : public Enchantment
{
public:
    OxygenEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~OxygenEnchantment();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
};

#endif
