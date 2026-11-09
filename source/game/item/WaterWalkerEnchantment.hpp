#ifndef GAME_ITEM_WATERWALKERENCHANTMENT_HPP
#define GAME_ITEM_WATERWALKERENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class WaterWalkerEnchantment : public Enchantment
{
public:
    WaterWalkerEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~WaterWalkerEnchantment();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool checkCompatibility(const Enchantment* other) const;
};

#endif
