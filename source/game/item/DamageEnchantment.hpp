#ifndef GAME_ITEM_DAMAGEENCHANTMENT_HPP
#define GAME_ITEM_DAMAGEENCHANTMENT_HPP

#include "game/item/Enchantment.hpp"

class DamageEnchantment : public Enchantment
{
public:
    DamageEnchantment(const Rarity* rarity, int type, arrayWithLength<const EquipmentSlot*> slots);

    virtual int getMaxLevel();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual float getDamageBonus(int level, const boost::shared_ptr<LivingEntity>& target);
    virtual bool checkCompatibility(const Enchantment* other) const;
    virtual int getDescriptionId();
    virtual bool canEnchant(not_null_ptr<ItemInstance> itemInstance);
    virtual void doPostAttack(boost::shared_ptr<LivingEntity> attacker, boost::shared_ptr<Entity> target, int level);
    virtual ~DamageEnchantment();

private:
    int mType;
};

#endif
