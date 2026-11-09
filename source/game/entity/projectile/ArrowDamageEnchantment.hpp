#ifndef GAME_ENTITY_PROJECTILE_ARROWDAMAGEENCHANTMENT_HPP
#define GAME_ENTITY_PROJECTILE_ARROWDAMAGEENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class EquipmentSlot;

class ArrowDamageEnchantment : public Enchantment
{
public:
    ArrowDamageEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual ~ArrowDamageEnchantment();
};

#endif
