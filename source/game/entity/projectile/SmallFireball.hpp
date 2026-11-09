#ifndef GAME_ENTITY_PROJECTILE_SMALLFIREBALL_HPP
#define GAME_ENTITY_PROJECTILE_SMALLFIREBALL_HPP

#include "game/entity/projectile/Fireball.hpp"

class DataFixerUpper;

class SmallFireball : public Fireball
{
public:
    SmallFireball(Level* level);
    SmallFireball(Level* level, boost::shared_ptr<LivingEntity> owner, double xd, double yd, double zd);
    SmallFireball(Level* level, double x, double y, double z, double xd, double yd, double zd);

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void onHit(const HitResult* result);
    virtual bool isPickable();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual unsigned long long GetType();
};

#endif
