#ifndef GAME_ENTITY_PROJECTILE_ARROWFIREENCHANTMENT_HPP
#define GAME_ENTITY_PROJECTILE_ARROWFIREENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class EquipmentSlot;

class ArrowFireEnchantment : public Enchantment
{
public:
    ArrowFireEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual ~ArrowFireEnchantment();
};

#endif
