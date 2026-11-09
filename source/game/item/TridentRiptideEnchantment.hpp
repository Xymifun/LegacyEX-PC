#ifndef GAME_ITEM_TRIDENTRIPTIDEENCHANTMENT_HPP
#define GAME_ITEM_TRIDENTRIPTIDEENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class EquipmentSlot;
class Rarity;

class TridentRiptideEnchantment : public Enchantment
{
public:
    TridentRiptideEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~TridentRiptideEnchantment();

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool checkCompatibility(const Enchantment* other) const;
};

#endif
