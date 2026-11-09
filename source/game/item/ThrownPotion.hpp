#ifndef GAME_ITEM_THROWNPOTION_HPP
#define GAME_ITEM_THROWNPOTION_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/ThrowableProjectile.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/Predicate.hpp"

class BlockPos;
class CompoundTag;
class DataFixerUpper;
class Direction;
class HitResult;
class ItemInstance;
class Level;
class Potion;
class MobEffectInstance;

class ThrownPotion : public ThrowableProjectile
{
public:
    static const double SPLASH_RANGE;
    static const EntityDataAccessor<not_null_ptr<ItemInstance> >* DATA_ITEM;
    class WATER_SENSITIVE : public Predicate<boost::shared_ptr<Entity> >
    {
    public:
        virtual bool apply(boost::shared_ptr<Entity> entity) const;
        virtual ~WATER_SENSITIVE();
    };

    ThrownPotion(Level* level);
    ThrownPotion(Level* level, boost::shared_ptr<LivingEntity> owner, not_null_ptr<ItemInstance> item);
    ThrownPotion(Level* level, double x, double y, double z, not_null_ptr<ItemInstance> item);

    static Entity* create(Level* level);
    not_null_ptr<ItemInstance> getItem();
    void dowseFire(const BlockPos& pos, const Direction* direction);
    static bool isWaterSensitiveEntity(const boost::shared_ptr<Entity>& entity);
    void applyWater();
    bool isLingering();
    void makeAreaOfEffectCloud(const not_null_ptr<ItemInstance>& item, Potion* potion);
    void applySplash(const HitResult* result, std::vector<MobEffectInstance*>* effects);
    virtual bool onHit(const HitResult* result);
    static void staticCtor();
    void _init();
    void setItem(not_null_ptr<ItemInstance> item);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual float getGravity();
    virtual void defineSynchedData();
    virtual unsigned long long GetType();

private:
    not_null_ptr<ItemInstance> mItem;
};

#endif
