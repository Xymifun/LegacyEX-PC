#ifndef GAME_ITEM_TRIDENTIMPALERENCHANTMENT_HPP
#define GAME_ITEM_TRIDENTIMPALERENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class EquipmentSlot;
class Rarity;

class TridentImpalerEnchantment : public Enchantment
{
public:
    TridentImpalerEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual ~TridentImpalerEnchantment();

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual float getDamageBonus(int level, const boost::shared_ptr<LivingEntity>& target);
};

#endif
