#ifndef GAME_ITEM_AREAEFFECTCLOUD_HPP
#define GAME_ITEM_AREAEFFECTCLOUD_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/client/render/ParticleType.hpp"

class CompoundTag;
class Level;
class MobEffectInstance;
class Potion;
class EntityDataAccessor_Base;
template <class T> class EntityDataAccessor;

class AreaEffectCloud : public Entity
{
public:
    AreaEffectCloud(Level* level);
    AreaEffectCloud(Level* level, double x, double y, double z);
    virtual ~AreaEffectCloud();

    static void staticCtor();
    void setRadius(float radius);
    void _init();
    void updateColor();
    float getRadius();
    boost::shared_ptr<LivingEntity> getOwner();
    void applyMobEffects();
    bool isWaiting();
    const ParticleType* getParticle();
    int getColor();
    int getParticleParam1();
    int getParticleParam2();
    void generateParticles();
    void suckInNearbyPlayers();

    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual void tick();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual int getPistonPushReaction();

    void setPotion(Potion* potion);
    void addEffect(MobEffectInstance* effect);
    void setFixedColor(int color);
    void setParticle(const ParticleType* particle);
    void setParticleParam1(int value);
    void setParticleParam2(int value);
    void setParticleSpawnRateOnTick(int value);
    void setWaiting(bool waiting);
    int getDuration();
    void setDuration(int duration);
    void setRadiusOnUse(float radius);
    void setRadiusPerTick(float radius);
    void setWaitTime(int waitTime);
    void setOwner(boost::shared_ptr<LivingEntity> owner);

    float getRadiusOnUse();
    float getRadiusPerTick();
    int getDurationOnUse();
    void setDurationOnUse(int duration);
    int getWaitTime();
    bool use();
    bool canUse();
    Potion* getPotion();
    std::vector<MobEffectInstance*>& getEffects();

    static Entity* create(Level* level);

private:
    static const EntityDataAccessor<float>* DATA_RADIUS;
    static const EntityDataAccessor<int>* DATA_COLOR;
    static const EntityDataAccessor<bool>* DATA_WAITING;
    static const EntityDataAccessor<int>* DATA_PARTICLE;
    static const EntityDataAccessor<int>* DATA_PARTICLE_PARAM_1;
    static const EntityDataAccessor<int>* DATA_PARTICLE_PARAM_2;
    static const EntityDataAccessor<int>* DATA_PARTICLE_RATE;

    Potion* mPotion;
    std::vector<MobEffectInstance*> mEffects;
    boost::unordered_map<int, int> mReapplicationDelayMap;
    int mDuration;
    int mWaitTime;
    int mReapplicationDelay;
    bool mFixedColor;
    int mDurationOnUse;
    float mRadiusOnUse;
    float mRadiusPerTick;
    boost::weak_ptr<LivingEntity> mOwner;
    std::wstring mOwnerUUID;
};

#endif
