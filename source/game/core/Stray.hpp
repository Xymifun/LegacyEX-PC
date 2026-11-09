#ifndef GAME_CORE_STRAY_HPP
#define GAME_CORE_STRAY_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AbstractSkeleton.hpp"

class DamageSource;
class DataFixerUpper;
class Level;
class Arrow;
class SoundEvent;

class Stray : public AbstractSkeleton
{
public:
    Stray(Level* level);

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual bool checkSpawnRules();
    virtual void registerGoals();
    virtual ResourceLocation* getDefaultLootTable();
    virtual ResourceLocation* getDefaultGearTable();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getStepSound();
    virtual boost::shared_ptr<Arrow> getArrow(float distanceFactor);
    virtual unsigned long long GetType();
};

#endif
