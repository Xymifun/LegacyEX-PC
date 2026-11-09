#ifndef GAME_ITEM_VANISHINGCURSEENCHANTMENT_HPP
#define GAME_ITEM_VANISHINGCURSEENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class VanishingCurseEnchantment : public Enchantment
{
public:
    VanishingCurseEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~VanishingCurseEnchantment();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool isTreasureOnly();
    virtual bool isCurse();
};

#endif
