#ifndef GAME_ENTITY_MOB_HANGINGENTITY_HPP
#define GAME_ENTITY_MOB_HANGINGENTITY_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/BlockPos.hpp"

class CompoundTag;
class DamageSource;
class Direction;
class LightningBolt;
class Mirror;
class Rotation;
class ItemInstance;
class ItemEntity;

template <class T> class not_null_ptr;

class HangingEntity : public Entity
{
    friend class TrackedEntity;
    friend class ItemFrameRenderer;
    friend class ClientboundAddPaintingPacket;
    friend class PaintingRenderer;
    friend class MapItemSavedData;
    friend class CompassItem;

public:
    class HangingEntitySelector : public Predicate<boost::shared_ptr<Entity> >
    {
    public:
        virtual bool apply(boost::shared_ptr<Entity> entity) const;
        virtual ~HangingEntitySelector();
    };

    HangingEntity(Level* level);
    HangingEntity(Level* level, const BlockPos& pos);
    virtual ~HangingEntity() {}

    void _init(Level* level);
    virtual void defineSynchedData();
    virtual void setDirection(const Direction* direction);
    double offs(int value);
    virtual void recalculateBoundingBox();
    virtual void tick();
    virtual bool survives();
    virtual bool isPickable();
    virtual bool skipAttackInteraction(boost::shared_ptr<Entity> entity);
    virtual const Direction* getDirection();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void move(MoverType type, double x, double y, double z, bool pushedByPiston);
    virtual void push(double x, double y, double z);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual boost::shared_ptr<ItemEntity> spawnAtLocation(not_null_ptr<ItemInstance> itemInstance, float yOffset);
    virtual bool repositionEntityAfterLoad();
    virtual void setPos(double x, double y, double z);
    virtual float rotate(Rotation* rotation);
    virtual float mirror(Mirror* mirror);
    virtual void thunderHit(const LightningBolt* lightningBolt);
    virtual bool shouldRender(double distanceSqr);
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual unsigned long long GetType();

    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
    virtual void dropItem(boost::shared_ptr<Entity> breaker) = 0;
    virtual void playPlacementSound() = 0;
    virtual BlockPos getPos();

protected:
    int mTickCounter;
    BlockPos mPos;
    const Direction* mDirection;

private:
    static Predicate<boost::shared_ptr<Entity> >* HANGING_ENTITY_SELECTOR;
};

extern int gHangingEntityRenderCount;

#endif
