#ifndef GAME_CORE_WITCH_HPP
#define GAME_CORE_WITCH_HPP
#include <boost/shared_ptr.hpp>
#include "game/core/EntityType.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"
class DamageSource;
class DataFixerUpper;
class Level;
class LivingEntity;
class SoundEvent;
class Witch : public Monster, public RangedAttackMob
{
public:
    explicit Witch(Level* level);
    static Entity* create(Level* level);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerGoals();
    virtual void defineSynchedData();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void setUsingItem(bool usingItem);
    virtual bool isDrinkingPotion();
    virtual void performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor);
    virtual bool isAggressive();
    virtual void setAggressive(bool aggressive);
    virtual void registerAttributes();
    virtual void aiStep();
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual float getDamageAfterMagicAbsorb(const DamageSource* source, float damage);
    virtual ResourceLocation* getDefaultLootTable();
    virtual float getEyeHeight();
    virtual unsigned long long GetType();
    static void DEATH_LOOT();

private:
    static const EntityDataAccessor<bool>* DATA_USING_ITEM;

    int mUsingTime;
};
#endif
