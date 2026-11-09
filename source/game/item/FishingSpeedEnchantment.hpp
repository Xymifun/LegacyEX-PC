#ifndef GAME_ITEM_FISHINGSPEEDENCHANTMENT_HPP
#define GAME_ITEM_FISHINGSPEEDENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class FishingSpeedEnchantment : public Enchantment
{
public:
    FishingSpeedEnchantment(const Rarity* rarity, const EnchantmentCategory* category,
                            arrayWithLength<const EquipmentSlot*> slots);
    virtual ~FishingSpeedEnchantment();
    virtual int getMaxLevel();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
};

#endif
