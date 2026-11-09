#ifndef GAME_CORE_LLAMASPIT_HPP
#define GAME_CORE_LLAMASPIT_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/projectile/Projectile.hpp"

class AABB;
class CompoundTag;
class HitResult;
class Level;
class Llama;
class Vec3;

class LlamaSpit : public Entity, public Projectile
{
public:
    LlamaSpit(Level* level, const boost::shared_ptr<Llama>& owner);
    LlamaSpit(Level* level);
    LlamaSpit(Level* level, double x, double y, double z, double xd, double yd, double zd);

    static Entity* create(Level* level);
    void _init();
    virtual void shoot(double x, double y, double z, float velocity, float inaccuracy);
    virtual void shootFromRotation(boost::shared_ptr<Entity> entity, float xRot, float yRot, float offset,
                                   float velocity, float inaccuracy);
    boost::shared_ptr<Llama> getOwner();
    virtual bool onHit(const HitResult* result);
    void restoreOwnerFromSave();
    boost::shared_ptr<Entity> getHitEntity(Vec3* from, Vec3* to);
    virtual void tick();
    virtual void lerpMotion(double xd, double yd, double zd);
    virtual void defineSynchedData();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual unsigned long long GetType();
    virtual bool shouldTick();

private:
    boost::weak_ptr<Llama> mLlamaOwner;
    CompoundTag* mOwnerTag;
};

#endif
