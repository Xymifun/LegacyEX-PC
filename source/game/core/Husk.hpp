#ifndef GAME_CORE_HUSK_HPP
#define GAME_CORE_HUSK_HPP

#include "game/entity/mob/Zombie.hpp"

class DamageSource;
class DataFixerUpper;
class Level;
class SoundEvent;

class Husk : public Zombie
{
public:
    explicit Husk(Level* level);
    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual bool checkSpawnRules();
    virtual bool isSunSensitive() const;
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getStepSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual not_null_ptr<ItemInstance> getSkull();
    virtual void finishConversion();
    virtual unsigned long long GetType();
};

#endif
