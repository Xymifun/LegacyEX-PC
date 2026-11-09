#ifndef GAME_ENTITY_MOB_ITEMENTITY_HPP
#define GAME_ENTITY_MOB_ITEMENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include "game/entity/mob/Entity.hpp"
#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Level;
class ItemDataWalker;
class ItemInstance;
class Player;

template <class T> class EntityDataAccessor;

class ItemEntity : public Entity
{
    friend class ItemRenderer;

public:
    ItemEntity(Level* level, double x, double y, double z, not_null_ptr<ItemInstance> item);
    ItemEntity(Level* level, double x, double y, double z);
    explicit ItemEntity(Level* level);
    static Entity* create(Level* level);
    void _init();
    void _init(Level* level, double x, double y, double z);
    void setItem(not_null_ptr<ItemInstance> item);
    void setDefaultPickUpDelay();
    static void addDataWalkers(DataFixerUpper* dataFixerUpper);
    static void staticCtor();
    virtual bool makeStepSound();
    virtual void defineSynchedData();
    not_null_ptr<ItemInstance> getItem();
    void setUnderwaterMovement();
    bool merge(boost::shared_ptr<ItemEntity> entity);
    void mergeWithNeighbours();
    virtual void tick();
    void setShortLifeTime();
    virtual bool updateInWaterState();
    virtual void burn(int damage);
    virtual bool hurt(const DamageSource* source, float damage);
    std::wstring getThrower();
    std::wstring getOwner();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void playerTouch(boost::shared_ptr<Player> player);
    virtual std::wstring getName();
    virtual boost::shared_ptr<Entity> changeDimension(int dimension);
    virtual bool isAttackable();
    void setOwner(const std::wstring& owner);
    void setThrower(const std::wstring& thrower);
    void setNoPickUpDelay();
    void setNeverPickUp();
    void setPickUpDelay(int delay);
    bool hasPickUpDelay();
    void setExtendedLifetime();
    void makeFakeItem();
    virtual unsigned long long GetType();
    int getAge() { return mAge; }

private:
    int mAge;
    int mPickupDelay;
    int mHealth;
    std::wstring mThrower;
    std::wstring mOwner;

public:
    float mBobOffs;

private:
    static const EntityDataAccessor<not_null_ptr<ItemInstance> >* DATA_ITEM;
};

#endif
