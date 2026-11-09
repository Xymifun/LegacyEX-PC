#ifndef GAME_ITEM_DIGDURABILITYENCHANTMENT_HPP
#define GAME_ITEM_DIGDURABILITYENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class Random;

class DigDurabilityEnchantment : public Enchantment
{
public:
    DigDurabilityEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~DigDurabilityEnchantment();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool canEnchant(not_null_ptr<ItemInstance> item);
    static bool shouldIgnoreDurabilityDrop(not_null_ptr<ItemInstance> item, int level, Random* random);
};

#endif
