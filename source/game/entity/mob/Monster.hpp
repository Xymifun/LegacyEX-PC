#ifndef GAME_ENTITY_MOB_MONSTER_HPP
#define GAME_ENTITY_MOB_MONSTER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundSource.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/Enemy.hpp"

class BlockPos;
class DamageSource;
class Level;
class Player;
class SoundEvent;
class Entity;

class Monster : public PathfinderMob, public Enemy
{
public:
    Monster(Level* level);
    virtual ~Monster() {}
    virtual unsigned long long GetType();
    virtual void tick();
    virtual const SoundEvent* getSwimSound();
    virtual const SoundEvent* getSwimSplashSound();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual SoundSource::ESoundSource getSoundSource();
    virtual void registerAttributes();
    virtual bool shouldDropExperience();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getFallDamageSound(int distance);
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual void aiStep();
    virtual bool checkSpawnRules();
    virtual float getWalkTargetValue(const BlockPos& pos);
    virtual bool isDarkEnoughToSpawn();
    virtual bool isPreventingPlayerRest(boost::shared_ptr<Player> player);
};

#endif
