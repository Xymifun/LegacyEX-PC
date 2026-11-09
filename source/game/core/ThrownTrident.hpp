#ifndef GAME_CORE_THROWNTRIDENT_HPP
#define GAME_CORE_THROWNTRIDENT_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/AbstractRotatedItemProjectile.hpp"
#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class DataFixerUpper;
class HitResult;
class ItemInstance;
class Level;
class LivingEntity;
class Player;
class SoundEvent;
class Vec3;

template <class T> class EntityDataAccessor;

class ThrownTrident : public AbstractRotatedItemProjectile
{
public:
    ThrownTrident(Level* level, boost::shared_ptr<LivingEntity> owner, not_null_ptr<ItemInstance> item);
    ThrownTrident(Level* level);
    ThrownTrident(Level* level, double x, double y, double z);
    ThrownTrident(Level* level, not_null_ptr<ItemInstance> item, double x, double y, double z);

    static Entity* create(Level* level);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);

    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual void tick();
    virtual void outOfWorld();
    virtual void playerTouch(boost::shared_ptr<Player> player);
    virtual bool shouldRender(double x, double y, double z);
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual const SoundEvent* getOnHitSound() const;
    virtual void onHitEntity(const HitResult* result);
    virtual void doPostHurtEffects(const boost::shared_ptr<LivingEntity>& entity);
    virtual boost::shared_ptr<Entity> findHitEntity(Vec3* from, Vec3* to);
    virtual not_null_ptr<ItemInstance> getPickupItem();
    virtual void checkDespawn();
    virtual float getWaterInertia();

    void _init();
    unsigned char getLoyaltyLevel();
    unsigned char isEnchanted();

private:
    friend class ThrownTridentRenderer;

    not_null_ptr<ItemInstance> mTridentItem;
    bool mDealtDamage;
    int mOwnerId;
    int mReturnTimer;

    static const EntityDataAccessor<unsigned char>* ID_LOYALTY;
    static const EntityDataAccessor<bool>* ID_FOIL;
};

#endif
