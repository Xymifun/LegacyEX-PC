#ifndef GAME_CORE_BLAZE_HPP
#define GAME_CORE_BLAZE_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Monster.hpp"
#include "game/entity/ai/Goal.hpp"

class DataFixerUpper;
class DamageSource;
class Level;
class Entity;

class Blaze : public Monster
{
public:
    class BlazeAttackGoal : public Goal
    {
    public:
        BlazeAttackGoal(Blaze* blaze);
        virtual ~BlazeAttackGoal() {}
        virtual bool canUse();
        virtual void start();
        virtual void stop();
        virtual void tick();
        virtual std::wstring getName() { return L"BlazeAttack"; }

    private:
        Blaze* mBlaze;
        int mAttackStep;
        int mAttackTime;
    };

    Blaze(Level* level);
    virtual ~Blaze() {}
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual int getLightColor();
    virtual float getBrightness();
    virtual void aiStep();
    virtual void customServerAiStep();
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual ResourceLocation* getDefaultLootTable();
    bool isCharged();
    virtual bool isOnFire();
    virtual void dropDeathLoot(bool hitByPlayer, int lootingLevel);
    void setCharged(bool charged);
    virtual bool isDarkEnoughToSpawn();
    double getFollowDistance();
    virtual unsigned long long GetType();
    static Entity* create(Level* level);

private:
    float mHeightOffset;
    int mHeightOffsetUpdateTime;
    int mUnused;
};

#endif
