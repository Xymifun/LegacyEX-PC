#ifndef GAME_ITEM_THORNSENCHANTMENT_HPP
#define GAME_ITEM_THORNSENCHANTMENT_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/Enchantment.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Entity;
class EquipmentSlot;
class ItemInstance;
class LivingEntity;
class Random;

class ThornsEnchantment : public Enchantment
{
public:
    ThornsEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots);
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool canEnchant(not_null_ptr<ItemInstance> itemInstance);
    static bool shouldHit(int level, Random* random);
    static int getDamage(int level, Random* random);
    virtual void doPostHurt(boost::shared_ptr<LivingEntity> target, boost::shared_ptr<Entity> attacker, int level);
    virtual ~ThornsEnchantment();
};

#endif
