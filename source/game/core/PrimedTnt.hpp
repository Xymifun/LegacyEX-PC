#ifndef GAME_CORE_PRIMEDTNT_HPP
#define GAME_CORE_PRIMEDTNT_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/mob/Entity.hpp"

class CompoundTag;
class EntityDataAccessor_Base;
class Level;
class LivingEntity;

template <class T> class EntityDataAccessor;

class PrimedTnt : public Entity
{
public:
    PrimedTnt(Level* level);
    PrimedTnt(Level* level, double x, double y, double z, boost::shared_ptr<LivingEntity> owner);
    virtual ~PrimedTnt();

    static Entity* create(Level* level);
    static void staticCtor();
    void _init();
    void setFuse(int fuse);
    virtual void defineSynchedData();
    virtual bool makeStepSound();
    virtual bool isPickable();
    void explode();
    virtual void tick();
    int getLife();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual boost::shared_ptr<LivingEntity> getOwner();
    virtual float getEyeHeight();
    int getFuse();
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual unsigned long long GetType();

private:
    static const EntityDataAccessor<int>* DATA_FUSE_ID;

    boost::weak_ptr<LivingEntity> mOwner;
    int mLife;
};

#endif
