#ifndef GAME_CORE_DELAYEDRELEASE_HPP
#define GAME_CORE_DELAYEDRELEASE_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"

class CompoundTag;
class DamageSource;
class Level;

class DelayedRelease : public Entity
{
public:
    DelayedRelease(Level* level, boost::shared_ptr<Entity> entity, int delay);
    virtual bool makeStepSound();
    virtual void tick();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void defineSynchedData();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual unsigned long long GetType();

private:
    boost::shared_ptr<Entity> mEntity;
    int mDelay;
};

#endif
