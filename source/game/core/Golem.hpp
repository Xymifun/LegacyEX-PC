#ifndef GAME_CORE_GOLEM_HPP
#define GAME_CORE_GOLEM_HPP

#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/Creature.hpp"

class DamageSource;
class Level;
class SoundEvent;

class Golem : public PathfinderMob, public Creature
{
public:
    Golem(Level* level);
    virtual ~Golem() {}
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual int getAmbientSoundInterval();
    virtual bool removeWhenFarAway();
};

#endif
