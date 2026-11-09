#ifndef GAME_ITEM_TRIDENTLOYALTYENCHANTMENT_HPP
#define GAME_ITEM_TRIDENTLOYALTYENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class EquipmentSlot;
class Rarity;

class TridentLoyaltyEnchantment : public Enchantment
{
public:
    TridentLoyaltyEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~TridentLoyaltyEnchantment();

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool checkCompatibility(const Enchantment* other) const;
};

#endif
