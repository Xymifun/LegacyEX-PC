#ifndef GAME_CORE_PARROT_HPP
#define GAME_CORE_PARROT_HPP

#include "game/world/biome/arrayWithLength.hpp"

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/FlyingAnimal.hpp"
#include "game/entity/mob/ShoulderRidingEntity.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/client/audio/SoundSource.hpp"

class PathNavigation;
class AgableMob;
class Animal;
class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DifficultyInstance;
class Entity;
class EntityTooltipDataHolder;
class FlyingPathNavigation;
class Item;
class ItemInstance;
class Level;
class MobGroupData;
class Random;
class SoundEvent;
class Vec3i;

class Parrot : public ShoulderRidingEntity, public FlyingAnimal
{
public:
    static const EntityDataAccessor<int>* DATA_VARIANT;
    static Item* POISONOUS_FOOD;
    static arrayWithLength<Item*> TAME_FOOD;
    static Predicate<boost::shared_ptr<Entity> >* CAN_MIMIC;
    class NotParrotPredicate : public Predicate<boost::shared_ptr<Entity> >
    {
    public:
        virtual bool apply(boost::shared_ptr<Entity> entity) const;
        virtual ~NotParrotPredicate() {}
    };

    Parrot(Level* level);
    virtual ~Parrot();
    static Entity* create(Level* level);
    virtual bool isFlying();
    static const SoundEvent* getImitatedSound(unsigned long long type);
    static float getPitch(Random* random);
    static bool imitateNearbyMobs(Level* level, Entity* entity);
    static const SoundEvent* getAmbient(Random* random);
    static void playAmbientSound(Level* level, boost::shared_ptr<Entity> entity);
    void _init();
    static void staticCtor();
    void setVariant(int variant);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& entityData);
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual PathNavigation* createNavigation(Level* level);
    virtual float getEyeHeight();
    void calculateFlapping();
    virtual void aiStep();
    void setRecordPlayingNearby(const BlockPos& pos, bool playing);
    bool isPartyParrot();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual bool isFood(const not_null_ptr<ItemInstance>& itemInstance);
    virtual bool checkSpawnRules();
    virtual void causeFallDamage(float distance, float damageMultiplier);
    bool isTameFood(const not_null_ptr<ItemInstance>& itemInstance);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual void checkFallDamage(double distance, bool onGround, Block* block, const BlockPos& pos);
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> parent);
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual float playFlySound(float volume);
    virtual bool makeFlySound();
    virtual float getVoicePitch();
    virtual SoundSource::ESoundSource getSoundSource();
    virtual bool isPushable(boost::shared_ptr<Entity> entity);
    virtual void doPush(boost::shared_ptr<Entity> entity);
    virtual bool hurt(const DamageSource* source, float damage);
    int getVariant();
    virtual void defineSynchedData();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual ResourceLocation* getDefaultLootTable();
    virtual unsigned long long GetType();

    float mFlap;
    float mFlapSpeed;
    float mOFlapSpeed;
    float mOFlap;

private:
    float mFlapping;
    bool mRecordPlayingNearby;
    BlockPos* mRecordPosition;
    FlyingPathNavigation* mFlyingNavigation;
    int mReserved;
};

#endif
