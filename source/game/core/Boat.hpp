#ifndef GAME_CORE_BOAT_HPP
#define GAME_CORE_BOAT_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/world/block/BlockPos.hpp"

class AABB;
class Block;
class CompoundTag;
class EntityTooltipDataHolder;
class Level;
class Item;
class SoundEvent;

class Boat : public Entity
{
public:
    enum Type
    {
        OAK,
        SPRUCE,
        BIRCH,
        JUNGLE,
        ACACIA,
        DARK_OAK
    };

    enum EStatus
    {
        IN_WATER,
        UNDER_WATER,
        UNDER_FLOWING_WATER,
        NOT_UNDERWATER,
        ON_LAND,
        IN_AIR
    };

    Boat(Level* level);
    Boat(Level* level, double x, double y, double z);

    static void staticCtor();
    void _init();
    virtual bool makeStepSound();
    virtual void defineSynchedData();
    virtual AABB* getCollideAgainstBox(boost::shared_ptr<Entity> entity);
    virtual AABB* getCollideBox();
    virtual bool isPushable(boost::shared_ptr<Entity> entity);
    virtual double getRideHeight();
    Type getType();
    Item* getDropItem();
    virtual bool hurt(const DamageSource* source, float damage);
    int getBubbleTime();
    void setBubbleTime(int time);
    virtual void onAboveBubbleCol(bool down);
    virtual void push(boost::shared_ptr<Entity> entity);
    virtual void animateHurt();
    virtual bool isPickable();
    virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport);
    virtual const Direction* getMotionDirection();
    EStatus isUnderwater();
    float getGroundFriction();
    bool checkInWater();
    EStatus getStatus();
    const SoundEvent* getPaddleSound();
    bool getIsSinking();
    void tickLerp();
    void setPaddleState(bool left, bool right);
    bool findBlockHeightOnPlane(const AABB* box, float& height, BlockPos& pos);
    float getWaterLevelAbove();
    void floatBoat();
    void controlBoat();
    bool getPaddleState(int paddle);
    void setIsSinking(bool sinking);
    void tickBubbleColumn();
    virtual void tick();
    float getRowingTime(int paddle, float partialTicks);
    void clampRotation(boost::shared_ptr<Entity> passenger);
    virtual void positionRider(const boost::shared_ptr<Entity>& passenger);
    virtual void onPassengerTurned(boost::shared_ptr<Entity> passenger);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setType(Type type);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    bool testInteract(boost::shared_ptr<Player> player);
    virtual bool interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual unsigned int GetHitTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual void checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos);
    virtual void setDamage(float damage);
    virtual float getDamage();
    virtual void setHurtTime(int time);
    virtual int getHurtTime();
    float getBubbleAngle(float partialTicks);
    virtual void setHurtDir(int direction);
    virtual int getHurtDir();
    virtual bool canAddPassenger(boost::shared_ptr<Entity> entity);
    boost::shared_ptr<Entity> getControllingPassenger();
    void setInput(bool left, bool right, bool up, bool down);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    bool hasPlayerPassenger();
    virtual void tickLeash();
    virtual unsigned long long GetType();
    static Entity* create(Level* level);
    bool isOutOfControl();

private:
    static const EntityDataAccessor<int>* DATA_HURT_TIME;
    static const EntityDataAccessor<int>* DATA_HURT_DIR;
    static const EntityDataAccessor<float>* DATA_DAMAGE;
    static const EntityDataAccessor<int>* DATA_TYPE;
    static const EntityDataAccessor<bool>* DATA_ID_PADDLE[2];
    static const EntityDataAccessor<int>* DATA_BUBBLE_TIME;
    static const EntityDataAccessor<bool>* DATA_SINKING;

    float mPaddlePositions[2];
    float mInvFriction;
    int mOutOfControlTicks;
    float mDeltaRotation;
    int mLerpSteps;
    double mLerpX;
    double mLerpY;
    double mLerpZ;
    double mLerpYRot;
    double mLerpXRot;
    bool mInputLeft;
    bool mInputRight;
    bool mInputUp;
    bool mInputDown;
    double mWaterLevel;
    float mLandFriction;
    EStatus mStatus;
    EStatus mOldStatus;
    double mLastYd;
    bool mIsAboveBubbleColumn;
    bool mBubbleColumnDirectionIsDown;
    float mBubbleMultiplier;
    float mBubbleAngle;
    float mBubbleAngleO;
    double mRockPhase;
};

#endif
