#ifndef GAME_ENTITY_PROJECTILE_ARROW_HPP
#define GAME_ENTITY_PROJECTILE_ARROW_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/AbstractRotatedItemProjectile.hpp"

class CompoundTag;
class DataFixerUpper;
class HitResult;
class Level;
class LivingEntity;

class Arrow : public AbstractRotatedItemProjectile
{
public:
    Arrow(Level* level);
    Arrow(Level* level, boost::shared_ptr<LivingEntity> owner);
    Arrow(Level* level, double x, double y, double z);
    virtual ~Arrow() {}

    static void addDataWalkers(DataFixerUpper* fixer, unsigned long long version);
    static void addDataWalkers(DataFixerUpper* fixer);
    static void addDataWalkers(DataFixerUpper* fixer, const std::wstring& type);

    virtual void onHitEntity(const HitResult* result);
    void _init();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    void setKnockback(int knockback);
    void setEnchantPower(int power);
    void setEnchantmentEffectsFromEntity(const boost::shared_ptr<LivingEntity>& entity, float damage);
    int getKnockback() const;
    int getEnchantPower() const;

protected:
    int mEnchantPower;
    int mKnockback;
};

#endif
