#ifndef GAME_ITEM_BINDINGCURSEENCHANTMENT_HPP
#define GAME_ITEM_BINDINGCURSEENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class BindingCurseEnchantment : public Enchantment
{
public:
    BindingCurseEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~BindingCurseEnchantment();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool isTreasureOnly();
    virtual bool isCurse();
};

#endif
