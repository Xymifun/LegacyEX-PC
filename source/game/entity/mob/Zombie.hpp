#ifndef GAME_ENTITY_MOB_ZOMBIE_HPP
#define GAME_ENTITY_MOB_ZOMBIE_HPP

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/item/not_null_ptr.hpp"

class Attribute;
class AttributeModifier;
class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Entity;
class EntityDataAccessor_Base;
class Goal;
class ItemInstance;
class Level;
class MobGroupData;
class SoundEvent;

template <class T> class EntityDataAccessor;

class Zombie : public Monster
{
public:
    class ZombieGroupData : public MobGroupData
    {
    public:
        ZombieGroupData(bool isBaby, bool isJockey, bool isVillager);
        virtual ~ZombieGroupData();

        bool mIsBaby;
        bool mIsJockey;
        bool mIsVillager;
    };

    static Attribute* SPAWN_REINFORCEMENTS_CHANCE;
    static AttributeModifier* SPEED_MODIFIER_BABY;

    Zombie(Level* level);
    Zombie(Level* level, bool initializeGoals);
    virtual ~Zombie();

    static Entity* create(Level* level);
    static void staticCtor();
    void _init();
    virtual void registerGoals();
    virtual void addBehaviourGoals();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    virtual void setAggressive(bool aggressive);
    virtual bool isAggressive();
    virtual bool canBreakDoors();
    virtual void setCanBreakDoors(bool canBreakDoors);
    virtual bool isBaby();
    bool isJockey();
    virtual int getExperienceReward(boost::shared_ptr<Player> player);
    virtual void setBaby(bool baby);
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual void aiStep();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual bool isSunSensitive() const;
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getStepSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual EMobType getMobType();
    virtual ResourceLocation* getDefaultLootTable();
    virtual ResourceLocation* getDefaultGearTable();
    virtual void populateDefaultEquipmentSlots(const DifficultyInstance& difficulty);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void killed(boost::shared_ptr<LivingEntity> entity);
    virtual float getEyeHeight();
    virtual void positionRider(const boost::shared_ptr<Entity>& entity);
    virtual bool breathesWater();
    virtual bool canHoldItem(not_null_ptr<ItemInstance> item);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString);
    bool isConverting();
    void startConverting(int conversionTime);
    void waterCheck();
    virtual void tick();
    virtual void setSize(float width, float height);
    virtual void internalSetSize(float scale);
    virtual not_null_ptr<ItemInstance> getSkull();
    virtual double getRidingHeight();
    virtual void die(const DamageSource* source);
    virtual bool startRiding(boost::shared_ptr<Entity> vehicle, bool force);
    virtual void finishConversion();
    virtual unsigned long long GetType();

protected:
    Goal* mBreakDoorGoal;
    bool mCanBreakDoors;
    float mBaseWidth;
    float mBaseHeight;
    bool mIsJockey;

public:
    int mConversionTime;

protected:
    int mWaterConversionTime;
};

#endif
