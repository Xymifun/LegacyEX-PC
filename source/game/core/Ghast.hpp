#ifndef GAME_CORE_GHAST_HPP
#define GAME_CORE_GHAST_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/FlyingMob.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/mob/Enemy.hpp"

class CompoundTag;
class DamageSource;
class DataFixerUpper;
class EntityDataAccessorBase;
class Level;
class Random;
class SoundEvent;

class Ghast : public FlyingMob, public Enemy
{
public:
    class GhastLookGoal : public Goal
    {
    public:
        explicit GhastLookGoal(Ghast* ghast);
        virtual ~GhastLookGoal();
        virtual bool canUse();
        virtual void tick();
        virtual std::wstring getName() { return L"GhastLook"; }

    private:
        Ghast* mGhast;
    };

    class GhastMoveControl : public MoveControl
    {
    public:
        explicit GhastMoveControl(Ghast* ghast);
        virtual ~GhastMoveControl();
        bool canReach(double x, double y, double z, double distance);
        virtual void tick();

    private:
        Ghast* mGhast;
        int mCountdown;
    };

    class GhastShootFireballGoal : public Goal
    {
    public:
        explicit GhastShootFireballGoal(Ghast* ghast);
        virtual ~GhastShootFireballGoal();
        virtual bool canUse();
        virtual void start();
        virtual void stop();
        virtual void tick();
        virtual std::wstring getName() { return L"GhastShootFireball"; }

    private:
        Ghast* mGhast;
        int mChargeTime;
    };

    class RandomFloatAroundGoal : public Goal
    {
    public:
        explicit RandomFloatAroundGoal(Ghast* ghast);
        virtual ~RandomFloatAroundGoal();
        virtual bool canUse();
        virtual bool canContinueToUse();
        virtual void start();
        virtual std::wstring getName() { return L"RandomFloatAround"; }

    private:
        Ghast* mGhast;
    };

    explicit Ghast(Level* level);
    static Entity* create(Level* level);
    static void staticCtor();
    void _init();
    virtual void registerGoals();
    virtual bool isCharging();
    virtual void setCharging(bool charging);
    virtual int getExplosionPower();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void defineSynchedData();
    virtual void registerAttributes();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual float getSoundVolume();
    virtual bool checkSpawnRules();
    virtual int getMaxSpawnClusterSize();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual float getEyeHeight();
    virtual void tick();
    virtual unsigned long long GetType();

private:
    int mUnused;
    int mExplosionPower;
};

#endif
