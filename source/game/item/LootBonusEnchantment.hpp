#ifndef GAME_ITEM_LOOTBONUSENCHANTMENT_HPP
#define GAME_ITEM_LOOTBONUSENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class EnchantmentCategory;
class EquipmentSlot;
class Rarity;

class LootBonusEnchantment : public Enchantment
{
public:
    LootBonusEnchantment(const Rarity* rarity, const EnchantmentCategory* category,
                         arrayWithLength<const EquipmentSlot*> slots);
    virtual ~LootBonusEnchantment();

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool checkCompatibility(const Enchantment* other) const;
};

#endif
