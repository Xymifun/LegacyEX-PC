#ifndef GAME_ITEM_MENDINGENCHANTMENT_HPP
#define GAME_ITEM_MENDINGENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class MendingEnchantment : public Enchantment
{
public:
    MendingEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~MendingEnchantment();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual bool isTreasureOnly();
    virtual int getMaxLevel();
};

#endif
