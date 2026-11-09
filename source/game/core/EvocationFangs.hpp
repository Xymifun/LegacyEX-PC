#ifndef GAME_CORE_EVOCATIONFANGS_HPP
#define GAME_CORE_EVOCATIONFANGS_HPP

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/mob/Entity.hpp"

class CompoundTag;
class Level;
class LivingEntity;

class EvocationFangs : public Entity
{
public:
    EvocationFangs(Level* level);
    EvocationFangs(Level* level, double x, double y, double z, float yRot, int warmupDelay,
                   const boost::shared_ptr<LivingEntity>& owner);

    void _init();
    void setOwner(const boost::shared_ptr<LivingEntity>& owner);
    boost::shared_ptr<LivingEntity> getOwner();
    void dealDamageTo(const boost::shared_ptr<LivingEntity>& target);
    float getAnimationProgress(float partialTicks) const;

    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual void tick();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void handleEntityEvent(unsigned char event, int data);

    static Entity* create(Level* level);

private:
    int mWarmupDelay;
    bool mSentSpikeEvent;
    int mLifeTicks;
    bool mClientSideAttack;
    boost::weak_ptr<LivingEntity> mOwner;
    std::wstring mOwnerUUID;
};

#endif
