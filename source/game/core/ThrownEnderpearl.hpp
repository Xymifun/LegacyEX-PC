#ifndef GAME_CORE_THROWNENDERPEARL_HPP
#define GAME_CORE_THROWNENDERPEARL_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/ThrowableProjectile.hpp"

class DataFixerUpper;
class HitResult;
class Level;

class ThrownEnderpearl : public ThrowableProjectile
{
public:
    ThrownEnderpearl(Level* level);
    ThrownEnderpearl(Level* level, boost::shared_ptr<LivingEntity> owner);
    ThrownEnderpearl(Level* level, double x, double y, double z);
    virtual unsigned long long GetType();
    virtual void tick();
    virtual bool onHit(const HitResult* result);

    static Entity* create(Level* level);
    void PlayTeleportSound();
    static void addDataWalkers(DataFixerUpper* fixer);
    void _init();

private:
    boost::shared_ptr<LivingEntity> mThrower;
};

#endif
