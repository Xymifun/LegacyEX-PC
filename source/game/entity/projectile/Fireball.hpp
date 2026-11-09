#ifndef GAME_ENTITY_PROJECTILE_FIREBALL_HPP
#define GAME_ENTITY_PROJECTILE_FIREBALL_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"

class CompoundTag;
class DataFixerUpper;
class DamageSource;
class HitResult;
class Level;
class LivingEntity;
class ParticleType;
class TrackedEntity;

class Fireball : public Entity
{
    friend class TrackedEntity;

public:
    Fireball(Level* level);
    Fireball(Level* level, double x, double y, double z, double xd, double yd, double zd);
    Fireball(Level* level, boost::shared_ptr<LivingEntity> owner, double xd, double yd, double zd);
    virtual ~Fireball() {}

    virtual bool shouldTick();
    static void addDataWalkers(DataFixerUpper* fixer, unsigned long long type);
    void _init();
    virtual void defineSynchedData();
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual void tick();
    virtual float getInertia();
    virtual void onHit(const HitResult* result) = 0;
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool isPickable();
    virtual float getPickRadius();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual float getBrightness();
    virtual int getLightColor();
    virtual const ParticleType* getTrailParticleType();
    virtual bool shouldBurn();
    virtual std::vector<unsigned long long>* getEntityTypesToIgnoreCollisionsWith();
    virtual unsigned long long GetType();

protected:
    boost::shared_ptr<LivingEntity> mOwner;
    int mLife;
    int mUnknown;

public:
    double mPowerX;
    double mPowerY;
    double mPowerZ;
};

#endif
