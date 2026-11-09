#ifndef GAME_ENTITY_MOB_FIREWORKSROCKETENTITY_HPP
#define GAME_ENTITY_MOB_FIREWORKSROCKETENTITY_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class DataFixerUpper;
class ItemInstance;
class Level;
class LivingEntity;
class Player;
class Block;

class FireworksRocketEntity : public Entity
{
public:
    FireworksRocketEntity(Level* level, const boost::shared_ptr<Player>& player,
                          const not_null_ptr<ItemInstance>& item);
    FireworksRocketEntity(Level* level, double x, double y, double z, not_null_ptr<ItemInstance> item);
    FireworksRocketEntity(Level* level, const not_null_ptr<ItemInstance>& item,
                          const boost::shared_ptr<LivingEntity>& entity);
    explicit FireworksRocketEntity(Level* level);
    FireworksRocketEntity(Level* level, double x, double y, double z, double xd, double yd, double zd,
                          not_null_ptr<ItemInstance> item, bool hasDirectionalData,
                          const boost::shared_ptr<Player>& owner);

    void _init(double x, double y, double z, const not_null_ptr<ItemInstance>& item);
    static void staticCtor();
    bool isAttachedToEntity();
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual bool shouldRender(double x, double y, double z);
    virtual void lerpMotion(double xd, double yd, double zd);
    double HandleDirectionalAcceleration(int direction, double velocity);
    void dealExplosionDamage();
    virtual void tick();
    void setExplosionColourToPipColour(CompoundTag* tag);
    virtual void handleEntityEvent(unsigned char event, int data);
    static void addDataWalkers(DataFixerUpper* dataFixerUpper);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool isAttackable();
    boost::shared_ptr<LivingEntity> getOwner() const;
    virtual void updateDeltaAfterMove(Block* block, double x, double y, double z, double& movedX, double& movedY,
                                      double& movedZ);
    virtual void defineSynchedData();
    static Entity* create(Level* level);
    virtual unsigned long long GetType();
    virtual bool shouldTick();

    static const EntityDataAccessor<not_null_ptr<ItemInstance> >* DATA_ITEM;
    static const EntityDataAccessor<int>* DATA_ATTACHED_TO_ENTITY;

private:
    int mLife;
    int mLifetime;
    boost::weak_ptr<LivingEntity> mAttachedToEntity;
    bool mHasDirectionalData;
    int mDirectionalX;
    int mDirectionalY;
    int mDirectionalZ;
    bool mDirectionalInitialized;
    boost::shared_ptr<Player> mOwner;
};

#endif
