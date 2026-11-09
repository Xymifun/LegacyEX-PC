#ifndef GAME_ITEM_DIGGINGENCHANTMENT_HPP
#define GAME_ITEM_DIGGINGENCHANTMENT_HPP
#include "game/item/Enchantment.hpp"
class DiggingEnchantment : public Enchantment
{
public:
    DiggingEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~DiggingEnchantment();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool canEnchant(not_null_ptr<ItemInstance> item);
};
#endif
