#ifndef GAME_ITEM_KNOCKBACKENCHANTMENT_HPP
#define GAME_ITEM_KNOCKBACKENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class EquipmentSlot;
class Rarity;

class KnockbackEnchantment : public Enchantment
{
public:
    KnockbackEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual ~KnockbackEnchantment();
};

#endif
