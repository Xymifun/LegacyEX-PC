#ifndef GAME_CORE_SNOWBALL_HPP
#define GAME_CORE_SNOWBALL_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/ThrowableProjectile.hpp"

class DataFixerUpper;
class HitResult;
class Level;
class LivingEntity;

class Snowball : public ThrowableProjectile
{
public:
    Snowball(Level* level, boost::shared_ptr<LivingEntity> owner);
    Snowball(Level* level);
    Snowball(Level* level, double x, double y, double z);

    static Entity* create(Level* level);
    void _init();
    virtual bool onHit(const HitResult* result);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual unsigned long long GetType();
};

#endif
