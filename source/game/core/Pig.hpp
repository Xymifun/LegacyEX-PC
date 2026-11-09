#ifndef GAME_CORE_PIG_HPP
#define GAME_CORE_PIG_HPP

#include "game/world/biome/arrayWithLength.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Animal.hpp"

class Block;
class BlockPos;
class CompoundTag;
class DataFixerUpper;
class DamageSource;
class EntityDataAccessor_Base;
class EntityTooltipDataHolder;
class Level;
class LightningBolt;
class SoundEvent;
class Item;
class ItemInstance;

template <class T> class EntityDataAccessor;

class Pig : public Animal
{
public:
    Pig(Level* level);
    static Entity* create(Level* level);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);

    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual void thunderHit(const LightningBolt* lightning);
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual boost::shared_ptr<Entity> getControllingPassenger();
    virtual void registerAttributes();
    virtual void die(const DamageSource* source);
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void travel(float xxa, float yya, float zza);
    virtual void registerGoals();
    virtual const SoundEvent* getAmbientSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool removeWhenFarAway();
    virtual bool canBeControlledByRider();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual bool isFood(const not_null_ptr<ItemInstance>& itemInstance);

    bool hasSaddle();
    void setSaddle(bool saddled);
    bool boost();
    bool testboost();

private:
    static const EntityDataAccessor<bool>* DATA_SADDLE;
    static const EntityDataAccessor<int>* DATA_BOOST_TIME;
    static arrayWithLength<Item*> FOOD_ITEMS;
    bool mBoosting;
    int mBoostTime;
    int mBoostTotalTime;
    long long mBoostStartTime;
};

#endif
