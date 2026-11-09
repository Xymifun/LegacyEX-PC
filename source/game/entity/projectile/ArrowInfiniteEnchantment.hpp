#ifndef GAME_ENTITY_PROJECTILE_ARROWINFINITEENCHANTMENT_HPP
#define GAME_ENTITY_PROJECTILE_ARROWINFINITEENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class EquipmentSlot;

class ArrowInfiniteEnchantment : public Enchantment
{
public:
    ArrowInfiniteEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool checkCompatibility(const Enchantment* other) const;
    virtual ~ArrowInfiniteEnchantment();
};

#endif
