#ifndef GAME_ITEM_UNTOUCHINGENCHANTMENT_HPP
#define GAME_ITEM_UNTOUCHINGENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class UntouchingEnchantment : public Enchantment
{
public:
    UntouchingEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~UntouchingEnchantment();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool checkCompatibility(const Enchantment* other) const;
};

#endif
