#ifndef GAME_CORE_THROWNEGG_HPP
#define GAME_CORE_THROWNEGG_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/ThrowableProjectile.hpp"

class DataFixerUpper;
class HitResult;
class Level;
class LivingEntity;

class ThrownEgg : public ThrowableProjectile
{
public:
    ThrownEgg(Level* level);
    ThrownEgg(Level* level, boost::shared_ptr<LivingEntity> owner);
    ThrownEgg(Level* level, double x, double y, double z);

    static Entity* create(Level* level);
    virtual bool onHit(const HitResult* result);
    static void addDataWalkers(DataFixerUpper* fixer);
    void _init();
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual unsigned long long GetType();
};

#endif
