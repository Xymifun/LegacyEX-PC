#ifndef GAME_CORE_MAGMACUBE_HPP
#define GAME_CORE_MAGMACUBE_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Slime.hpp"

class DataFixerUpper;
class DamageSource;
class Level;
class SoundEvent;

class MagmaCube : public Slime
{
public:
    MagmaCube(Level* level);
    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerAttributes();
    virtual void defineSynchedData();
    virtual bool checkSpawnRules();
    virtual bool checkSpawnObstruction();
    virtual void setSize(int size, bool reset);
    virtual int getLightColor();
    virtual float getBrightness();
    virtual const ParticleType* getParticleType();
    virtual boost::shared_ptr<Slime> createChild();
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool isOnFire();
    virtual bool breathesLava();
    virtual int getJumpDelay();
    virtual void decreaseSquish();
    virtual void jumpFromGround();
    virtual void jumpInLava();
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual bool isDealsDamage();
    virtual int getAttackDamage();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getSquishSound();
    virtual const SoundEvent* getJumpSound();
    virtual unsigned long long GetType();
};

#endif
