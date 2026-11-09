#ifndef GAME_ENTITY_MOB_WATERANIMAL_HPP
#define GAME_ENTITY_MOB_WATERANIMAL_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/Creature.hpp"

class PathNavigation;
class Level;
class Player;
class SoundEvent;
class WaterBoundPathNavigation;

class WaterAnimal : public PathfinderMob, public Creature
{
public:
    WaterAnimal(Level* level);
    virtual ~WaterAnimal() {}

    virtual bool isWaterMob();
    virtual bool checkSpawnRules();
    virtual bool checkSpawnObstruction();
    virtual int getAmbientSoundInterval();
    virtual bool removeWhenFarAway();
    virtual int getExperienceReward(boost::shared_ptr<Player> player);
    virtual PathNavigation* createNavigation(Level* level);
    virtual const SoundEvent* getFlopSound() const;
    virtual void aiStep();
    virtual void baseTick();
    virtual bool isPushedByWater();
    virtual bool breathesAir();

protected:
    friend class SwimIdleGoal;

    float mSwimAmplitude;
    float mSwimFrequency;
    float mWaterValueC;
};

#endif
