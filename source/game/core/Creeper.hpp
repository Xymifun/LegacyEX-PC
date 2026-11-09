#ifndef GAME_CORE_CREEPER_HPP
#define GAME_CORE_CREEPER_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Monster.hpp"

class CompoundTag;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class LightningBolt;
class Entity;

class Creeper : public Monster
{
public:
    Creeper(Level* level);
    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual void tick();
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual void thunderHit(const LightningBolt* lightning);
    virtual int getMaxFallDistance();
    virtual void registerAttributes();
    virtual void die(const DamageSource* source);
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual void registerGoals();
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual bool isPowered();

    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    static Entity* create(Level* level);

    void _init();
    bool isIgnited();
    void ignite();
    int getSwellDir();
    void setSwellDir(int swellDir);
    void spawnLingeringCloud();
    void explodeCreeper();
    bool canDropMobsSkull();
    void increaseDroppedSkulls();
    float getSwelling(float partialTicks);

private:
    int mOldSwell;
    int mSwell;
    int mMaxSwell;
    int mExplosionRadius;
    int mDroppedSkulls;
};

#endif
