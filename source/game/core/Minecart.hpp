#ifndef GAME_CORE_MINECART_HPP
#define GAME_CORE_MINECART_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/world/block/EntityDataAccessor.hpp"

class AABB;
class BlockPos;
class BlockState;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;

class Minecart : public Entity
{
public:
    class Type
    {
    public:
        enum EType
        {
            eType_Rideable,
            eType_Chest,
            eType_Furnace,
            eType_TNT,
            eType_Spawner,
            eType_Hopper,
            eType_CommandBlock
        };

        Type(int id = 0) : mId(id) {}
        std::wstring getName() const;

        static const Type* RIDEABLE;
        static const Type* CHEST;
        static const Type* FURNACE;
        static const Type* TNT;
        static const Type* SPAWNER;
        static const Type* HOPPER;
        static const Type* COMMAND_BLOCK;
        static const Type* VALUES[7];

        static std::wstring NAMES[7];

        int mId;
    };

    Minecart(Level* level);
    Minecart(Level* level, double x, double y, double z);
    virtual ~Minecart();

    static void staticCtor();
    void _init();
    virtual void setPos(double x, double y, double z);
    static boost::shared_ptr<Minecart> createMinecart(Level* level, double x, double y, double z, const Type* type);
    virtual bool makeStepSound();
    virtual void defineSynchedData();
    virtual AABB* getCollideAgainstBox(boost::shared_ptr<Entity> entity);
    virtual AABB* getCollideBox();
    virtual AABB* getBoundingBoxForCulling();
    virtual bool isPushable(boost::shared_ptr<Entity> entity);
    virtual double getRideHeight();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void destroy(const DamageSource* source);
    virtual void animateHurt();
    virtual bool isPickable();
    virtual const Direction* getMotionDirection();
    virtual void onAboveBubbleCol(bool down);
    virtual bool shouldTick();
    virtual void tick();
    virtual double getMaxSpeed();
    virtual void activateMinecart(int x, int y, int z, bool powered);
    virtual void comeOffTrack();
    virtual void moveAlongTrack(const BlockPos& pos, const BlockState* state);
    virtual void applyNaturalSlowdown();
    virtual Vec3* getPosOffs(double x, double y, double z, double distance);
    virtual Vec3* getPos(double x, double y, double z);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    using Entity::push;
    virtual void push(boost::shared_ptr<Entity> entity);
    bool isInterpolating() const;
    double getLerpToYRot() const;
    double getLerpToXRot() const;
    virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport);
    virtual void lerpMotion(double xd, double yd, double zd);
    virtual void setDamage(float damage);
    virtual float getDamage();
    virtual void setHurtTime(int hurtTime);
    virtual int getHurtTime();
    virtual void setHurtDir(int hurtDir);
    virtual int getHurtDir();
    virtual const Type* getType() = 0;
    virtual const BlockState* getDisplayBlockState();
    virtual const BlockState* getDefaultDisplayBlockState();
    virtual int getDisplayOffset();
    virtual int getDefaultDisplayOffset();
    virtual void setDisplayBlockState(const BlockState* state);
    virtual void setDisplayOffset(int offset);
    virtual bool hasCustomDisplay();
    virtual void setCustomDisplay(bool customDisplay);
    virtual unsigned int GetHitTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual boost::shared_ptr<Entity> getControllingPassenger();
    virtual unsigned long long GetType();
    static void addDataWalkers(DataFixerUpper* dataFixerUpper, unsigned long long gameTime);

    static const EntityDataAccessor<float>* DATA_ID_DAMAGE;
    static const EntityDataAccessor<int>* DATA_ID_DISPLAY_BLOCK;
    static const EntityDataAccessor<int>* DATA_ID_DISPLAY_OFFSET;
    static const EntityDataAccessor<int>* DATA_ID_HURT_TIME;
    static const EntityDataAccessor<int>* DATA_ID_HURT_DIR;
    static const EntityDataAccessor<bool>* DATA_ID_CUSTOM_DISPLAY;

protected:
    static const int EXITS[10][2][3];

    bool mFlipped;
    bool mInBubbleColumn;
    void* mReserved;
    bool mPushedMinecart;
    int mLerpSteps;
    double mLerpX;
    double mLerpY;
    double mLerpZ;
    double mLerpYRot;
    double mLerpXRot;
    double mLerpXd;
    double mLerpYd;
    double mLerpZd;
};

#endif
