#ifndef GAME_ENTITY_PROJECTILE_DRAGONFIREBALL_HPP
#define GAME_ENTITY_PROJECTILE_DRAGONFIREBALL_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/Fireball.hpp"

class DamageSource;
class DataFixerUpper;
class HitResult;
class Level;
class LivingEntity;
class ParticleType;

class DragonFireball : public Fireball
{
public:
    static const double SPLASH_RANGE;

    DragonFireball(Level* level);
    DragonFireball(Level* level, boost::shared_ptr<LivingEntity> owner, double xd, double yd, double zd);
    DragonFireball(Level* level, double x, double y, double z, double xd, double yd, double zd);

    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void onHit(const HitResult* result);
    virtual bool isPickable();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual const ParticleType* getTrailParticleType();
    virtual bool shouldBurn();
    virtual unsigned long long GetType();
    static Entity* create(Level* level);
};

#endif
