#ifndef GAME_ITEM_TRIDENTCHANNELINGENCHANTMENT_HPP
#define GAME_ITEM_TRIDENTCHANNELINGENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class EquipmentSlot;

class TridentChannelingEnchantment : public Enchantment
{
public:
    TridentChannelingEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool checkCompatibility(const Enchantment* other) const;
    virtual ~TridentChannelingEnchantment();
};

#endif
