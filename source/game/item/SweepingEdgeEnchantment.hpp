#ifndef GAME_ITEM_SWEEPINGEDGEENCHANTMENT_HPP
#define GAME_ITEM_SWEEPINGEDGEENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class EquipmentSlot;
class Rarity;

class SweepingEdgeEnchantment : public Enchantment
{
public:
    SweepingEdgeEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);

    static float getSweepingDamageRatio(int level);

    virtual int getMaxLevel();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getDescriptionId();
    virtual ~SweepingEdgeEnchantment();
};

#endif
