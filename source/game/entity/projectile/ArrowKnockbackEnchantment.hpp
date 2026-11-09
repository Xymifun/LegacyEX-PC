#ifndef GAME_ENTITY_PROJECTILE_ARROWKNOCKBACKENCHANTMENT_HPP
#define GAME_ENTITY_PROJECTILE_ARROWKNOCKBACKENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class EquipmentSlot;

class ArrowKnockbackEnchantment : public Enchantment
{
public:
    ArrowKnockbackEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual ~ArrowKnockbackEnchantment();
};

#endif
