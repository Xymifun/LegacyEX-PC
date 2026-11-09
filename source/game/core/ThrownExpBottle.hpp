#ifndef GAME_CORE_THROWNEXPBOTTLE_HPP
#define GAME_CORE_THROWNEXPBOTTLE_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/ThrowableProjectile.hpp"

class DataFixerUpper;
class HitResult;
class Level;

class ThrownExpBottle : public ThrowableProjectile
{
public:
    ThrownExpBottle(Level* level);
    ThrownExpBottle(Level* level, boost::shared_ptr<LivingEntity> owner);
    ThrownExpBottle(Level* level, double x, double y, double z);

    static Entity* create(Level* level);
    virtual bool onHit(const HitResult* result);
    static void addDataWalkers(DataFixerUpper* fixer);
    void _init();
    virtual float getGravity();
    virtual unsigned long long GetType();
};

#endif
