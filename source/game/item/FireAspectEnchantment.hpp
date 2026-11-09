#ifndef GAME_ITEM_FIREASPECTENCHANTMENT_HPP
#define GAME_ITEM_FIREASPECTENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class EquipmentSlot;
class Rarity;

class FireAspectEnchantment : public Enchantment
{
public:
    FireAspectEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);

    virtual int getMaxLevel();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual ~FireAspectEnchantment();
};

#endif
