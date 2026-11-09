#ifndef GAME_ENTITY_MOB_SKELETON_HPP
#define GAME_ENTITY_MOB_SKELETON_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AbstractSkeleton.hpp"

class DamageSource;
class DataFixerUpper;
class Level;
class SoundEvent;
class Arrow;

class Skeleton : public AbstractSkeleton
{
public:
    Skeleton(Level* level);

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerGoals();
    virtual ResourceLocation* getDefaultLootTable();
    virtual ResourceLocation* getDefaultGearTable();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getStepSound();
    virtual void die(const DamageSource* source);
    virtual boost::shared_ptr<Arrow> getArrow(float distanceFactor);
    virtual unsigned long long GetType();
};

#endif
