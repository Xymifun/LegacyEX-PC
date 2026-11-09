#ifndef GAME_CORE_DROWNED_HPP
#define GAME_CORE_DROWNED_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Zombie.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"
#include "game/item/not_null_ptr.hpp"

class PathNavigation;
class DataFixerUpper;
class DamageSource;
class DifficultyInstance;
class DrownedPathNavigation;
class Goal;
class ItemInstance;
class Level;

class Drowned : public Zombie, public RangedAttackMob
{
public:
    explicit Drowned(Level* level);
    virtual ~Drowned();

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);

    virtual void performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor);
    virtual bool isAggressive();
    virtual void setAggressive(bool aggressive);
    virtual bool breathesWater();
    virtual bool checkSpawnRules();
    virtual bool checkSpawnObstruction();
    virtual void aiStep();
    virtual void registerAttributes();
    virtual void registerGoals();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getStepSound();
    virtual float getWaterSlowDown();
    virtual void travel(float xxa, float yya, float zza);
    virtual not_null_ptr<ItemInstance> getSkull();
    virtual ResourceLocation* getDefaultLootTable();
    virtual ResourceLocation* getDefaultGearTable();
    virtual void populateDefaultEquipmentSlots(const DifficultyInstance& difficulty);
    virtual PathNavigation* createNavigation(Level* level);
    virtual unsigned long long GetType();

private:
    bool mIsRangedAttacking;
    Goal* mMeleeAttackGoal;
    Goal* mRangedAttackGoal;
};

#endif
