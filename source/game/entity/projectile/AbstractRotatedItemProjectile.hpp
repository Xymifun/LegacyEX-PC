#ifndef GAME_ENTITY_PROJECTILE_ABSTRACTROTATEDITEMPROJECTILE_HPP
#define GAME_ENTITY_PROJECTILE_ABSTRACTROTATEDITEMPROJECTILE_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MoverType.hpp"
#include "game/entity/projectile/Projectile.hpp"
#include "game/item/not_null_ptr.hpp"

class ArrowRenderer;
class Block;
class CompoundTag;
template <class T> class EntityDataAccessor;
class HitResult;
class ItemInstance;
class Level;
class LivingEntity;
class Player;
class SoundEvent;
class TntBlock;
class Vec3;

void fjSanitisePosAndVel(Entity* entity);

class AbstractRotatedItemProjectile : public Entity, public Projectile
{
public:
    enum ePickup
    {
        ePickup_Disallowed = 0,
        ePickup_Allowed = 1,
        ePickup_CreativeOnly = 2
    };

    AbstractRotatedItemProjectile(Level* level);
    AbstractRotatedItemProjectile(Level* level, double x, double y, double z);
    AbstractRotatedItemProjectile(Level* level, boost::shared_ptr<LivingEntity> owner);
    virtual ~AbstractRotatedItemProjectile() {}

    static void staticCtor();

    virtual void defineSynchedData();
    virtual void tick();
    virtual void move(MoverType type, double x, double y, double z, bool pushedByPiston);
    virtual void playerTouch(boost::shared_ptr<Player> player);
    virtual bool makeStepSound();
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual bool shouldTick();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport);
    virtual void lerpMotion(double xd, double yd, double zd);
    virtual bool isAttackable();
    virtual float getEyeHeight();
    virtual bool canChangeDimensions();

    virtual bool onHit(const HitResult* result);
    virtual const SoundEvent* getOnHitSound() const;
    virtual void onHitEntity(const HitResult* result) = 0;
    virtual void doPostHurtEffects(const boost::shared_ptr<LivingEntity>& entity);
    virtual boost::shared_ptr<Entity> findHitEntity(Vec3* from, Vec3* to);
    virtual not_null_ptr<ItemInstance> getPickupItem() = 0;
    virtual void checkDespawn();
    virtual float getWaterInertia();
    virtual void shootFromRotation(boost::shared_ptr<Entity> entity, float xRot, float yRot, float offset,
                                   float velocity, float inaccuracy);
    virtual void shoot(double x, double y, double z, float velocity, float inaccuracy);

    void setFlag(int flag, bool value);
    void setCritArrow(bool critical);
    bool isNoPhysics();
    bool isCritArrow();
    void setNoPhysics(bool noPhysics);
    void setPickup(ePickup pickup) { mPickup = pickup; }
    ePickup getPickup() const { return mPickup; }

protected:
    friend class ArrowRenderer;
    friend class TntBlock;

    using Entity::mLevel;

    void _init();

    static const EntityDataAccessor<unsigned char>* DATA_FLAGS;

    int mXTile;
    int mYTile;
    int mZTile;
    Block* mInTile;
    int mInData;
    int mLife;
    bool mInGround;
    int mInGroundTime;
    int mFlightTime;
    bool mClampedToLevel;
    ePickup mPickup;
    int mShake;

public:
    boost::shared_ptr<Entity> mOwner;
};

#endif
