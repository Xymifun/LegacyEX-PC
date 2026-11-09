#ifndef GAME_ENTITY_PROJECTILE_THROWABLEPROJECTILE_HPP
#define GAME_ENTITY_PROJECTILE_THROWABLEPROJECTILE_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include "game/entity/mob/Entity.hpp"
#include "game/entity/projectile/Projectile.hpp"

class CompoundTag;
class DataFixerUpper;
class Level;
class LivingEntity;
class Block;
class Vec3;

class ThrowableProjectile : public Entity, public Projectile
{
public:
    ThrowableProjectile(Level* level);
    ThrowableProjectile(Level* level, boost::shared_ptr<LivingEntity> owner);
    ThrowableProjectile(Level* level, double x, double y, double z);
    virtual ~ThrowableProjectile();
    virtual void shootFromRotation(boost::shared_ptr<Entity> entity, float xRot, float yRot, float offset,
                                   float velocity, float inaccuracy);
    virtual void shoot(double x, double y, double z, float velocity, float inaccuracy);
    void fjSetParticleSpawnPos(double x, double y, double z);
    static void addDataWalkers(DataFixerUpper* fixer, unsigned long long version);
    void _throwableInit();
    virtual void defineSynchedData();
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual void lerpMotion(double xd, double yd, double zd);
    virtual void tick();
    virtual float getGravity();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual boost::shared_ptr<LivingEntity> getOwner();
    Vec3* fjGetParticleSpawnPos() const;
    virtual bool shouldTick();

protected:
    using Entity::mLevel;
    int mXTile;
    int mYTile;
    int mZTile;
    Block* mInTile;
    bool mInGround;
    int mShake;
    std::wstring mOwnerName;
    int mLife;
    int mFlightTime;
    Vec3* mParticleSpawnPos;
};

#endif
