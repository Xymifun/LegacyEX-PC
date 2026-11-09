#ifndef GAME_ENTITY_PROJECTILE_FISHINGHOOK_HPP
#define GAME_ENTITY_PROJECTILE_FISHINGHOOK_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"

class BlockPos;
class CompoundTag;
class EntityDataAccessor_Base;
class Level;
class Player;

template <class T> class EntityDataAccessor;

class FishingHook : public Entity
{
public:
    FishingHook(Level* level, boost::shared_ptr<Player> owner);
    FishingHook(Level* level, boost::shared_ptr<Player> owner, double x, double y, double z);

    void bringInHookedEntity();
    int retrieve();
    void _init();
    void init(const boost::shared_ptr<Player>& owner);
    void shoot();
    void setLureSpeed(int lureSpeed);
    void setLuck(int luck);
    static void staticCtor();
    virtual void defineSynchedData();
    bool canBeHooked(const boost::shared_ptr<Entity>& entity);
    virtual bool makeStepSound();
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual void handleEntityEvent(unsigned char event, int data);
    bool shouldStopFishing();
    void updateRotation();
    void setHookedEntity();
    void checkCollision();
    void catchingFish(const BlockPos& pos);
    virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport);
    virtual void tick();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    boost::shared_ptr<Player> getOwner();
    virtual void remove();
    virtual unsigned long long GetType();
    virtual bool shouldTick();

private:
    bool mInGround;
    int mTicksInGround;
    boost::shared_ptr<Player> mOwner;
    int mTicksInAir;
    int mTicksCatchable;
    int mTicksCaughtDelay;
    int mTicksCatchableDelay;
    float mFishAngle;
    int mCurrentState;
    int mLuck;
    int mLureSpeed;
    boost::shared_ptr<Entity> mHookedIn;

    static const EntityDataAccessor<int>* DATA_HOOKED_ENTITY;
};

#endif
