#ifndef GAME_ITEM_PROTECTIONENCHANTMENT_HPP
#define GAME_ITEM_PROTECTIONENCHANTMENT_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/Enchantment.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DamageSource;
class EquipmentSlot;
class LivingEntity;

class ProtectionEnchantment : public Enchantment
{
public:
    class Type
    {
    public:
        static const Type* ALL;
        static const Type* FIRE;
        static const Type* FALL;
        static const Type* EXPLOSION;
        static const Type* PROJECTILE;

        Type(int name, int minCost, int levelCost, int levelCostSpan);

        unsigned int getMinCost() const;
        unsigned int getLevelCost() const;
        unsigned int getName() const;
        unsigned int getLevelCostSpan() const;

    private:
        int mName;
        int mMinCost;
        int mLevelCost;
        int mLevelCostSpan;
    };

    ProtectionEnchantment(const Rarity* rarity, const Type* type, arrayWithLength<const EquipmentSlot*> slots);

    virtual int getMaxLevel();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getDamageProtection(int level, const DamageSource* damageSource);
    virtual bool checkCompatibility(const Enchantment* other) const;
    virtual int getDescriptionId();
    static int getFireAfterDampener(const boost::shared_ptr<LivingEntity>& entity, int damage);
    static double getExplosionKnockbackAfterDampener(const boost::shared_ptr<LivingEntity>& entity, double knockback);
    virtual ~ProtectionEnchantment();

private:
    const Type* mType;
};

#endif
