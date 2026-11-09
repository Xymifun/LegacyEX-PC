#ifndef GAME_CORE_PUFFERFISH_HPP
#define GAME_CORE_PUFFERFISH_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Fish.hpp"

class DamageSource;
class DataFixerUpper;
class Entity;
class Level;
class SoundEvent;
class PufferfishRenderer;

class Pufferfish : public Fish
{
public:
    friend class PufferfishRenderer;
    enum PuffedState
    {
        PUFFED_STATE_SMALL = 0,
        PUFFED_STATE_MEDIUM = 1,
        PUFFED_STATE_LARGE = 2
    };

    Pufferfish(Level* level);

    static Entity* create(Level* level);
    void setNextPuffedState(PuffedState state, int delay);
    void damageNearbyMobs();
    bool validatePuffedTypes(boost::shared_ptr<Entity> entity);
    void checkForDangerousMobs();
    virtual void baseTick();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual const SoundEvent* getFlopSound() const;
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual unsigned long long GetType();
    static void addDataWalkers(DataFixerUpper* fixer);

private:
    PuffedState mPuffState;
    PuffedState mNextPuffState;
    int mPuffTransitionTime;
    int mPoisonDuration;
    int mDamage;
    float mSmallPuffRadius;
    float mLargePuffRadius;
    float mStingRange;
    int64_t mLastStingSoundTime;
    int64_t mStingSoundDelay;
};

#endif
