#ifndef GAME_CORE_PHANTOM_HPP
#define GAME_CORE_PHANTOM_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Monster.hpp"
#include "game/world/block/Predicate.hpp"

class PathNavigation;
class DamageSource;
class FlyingPathNavigation;
class Level;
class Player;
class SoundEvent;

class Phantom : public Monster
{
public:
    class PlayerSelector : public Predicate<boost::shared_ptr<Player> >
    {
    public:
        PlayerSelector(Phantom* phantom) : mPhantom(phantom) {}
        virtual bool apply(boost::shared_ptr<Player> player) const;
        virtual ~PlayerSelector() {}

    private:
        Phantom* mPhantom;
    };

    Phantom(Level* level);
    static Entity* create(Level* level);
    virtual void tick();
    virtual bool checkSpawnRules();
    virtual void aiStep();
    virtual EMobType getMobType();
    virtual bool canFly();
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual PathNavigation* createNavigation(Level* level);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual unsigned long long GetType();
};

#endif
