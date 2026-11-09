#ifndef GAME_ENTITY_PROJECTILE_LARGEFIREBALL_HPP
#define GAME_ENTITY_PROJECTILE_LARGEFIREBALL_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/Fireball.hpp"

class CompoundTag;
class DataFixerUpper;
class Level;

class LargeFireball : public Fireball
{
public:
    LargeFireball(Level* level);
    LargeFireball(Level* level, double x, double y, double z, double xd, double yd, double zd);
    LargeFireball(Level* level, boost::shared_ptr<LivingEntity> owner, double xd, double yd, double zd);

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void onHit(const HitResult* result);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual unsigned long long GetType();

    int mExplosionPower;
};

#endif
