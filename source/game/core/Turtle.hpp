#ifndef GAME_CORE_TURTLE_HPP
#define GAME_CORE_TURTLE_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Animal.hpp"
#include "game/world/block/EntityDataAccessor.hpp"
#include "game/world/block/Predicate.hpp"

class PathNavigation;
class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DifficultyInstance;
class EntityTooltipDataHolder;
class Level;
class LightningBolt;
class MobGroupData;
class TurtlePathNavigation;
class ItemInstance;
class Entity;

class Turtle : public Animal
{
public:
    static Predicate<boost::shared_ptr<Entity> >* BABY_ON_LAND_SELECTOR;
    Turtle(Level* level);

    static Entity* create(Level* level);
    static void staticCtor();

    void setHomePos(const BlockPos& pos);
    bool hasEgg();
    void setHasEgg(bool hasEgg);
    bool hasGivenUpOnEgg();
    BlockPos getHomePos();
    void setGoingHome(bool goingHome);
    bool closeToNextPos();
    bool isGoingHome();
    void GiveUpOnEgg();
    void setIsLayingEgg(bool layingEgg);
    bool isLayingEgg();
    void setTravelPos(const BlockPos& pos);
    void setTravelling(bool travelling);
    BlockPos getTravelPos();

    virtual void registerGoals();
    virtual void registerAttributes();
    virtual void grewUp();
    virtual void defineSynchedData();
    virtual PathNavigation* createNavigation(Level* level);
    virtual bool generatesBubbles();
    virtual void tick();
    virtual void travel(float xxa, float yya, float zza);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    bool isTravelling();
    virtual bool breathesWater();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString);
    virtual bool checkSpawnRules();
    virtual bool isPushedByWater();
    virtual bool isWaterMob();
    virtual bool isFood(const not_null_ptr<ItemInstance>& itemInstance);
    virtual float getWalkTargetValue(const BlockPos& pos);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    virtual void thunderHit(const LightningBolt* lightning);
    virtual void die(const DamageSource* source);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual const SoundEvent* getSwimSound();
    virtual int getAmbientSoundInterval();
    virtual ResourceLocation* getDefaultLootTable();
    virtual unsigned long long GetType();

private:
    int mLayEggTime;
    static const EntityDataAccessor<BlockPos>* DATA_HOME_POS;
    static const EntityDataAccessor<bool>* DATA_HAS_EGG;
    static const EntityDataAccessor<bool>* DATA_HAS_GIVEN_UP_ON_EGG;
    static const EntityDataAccessor<bool>* DATA_IS_LAYING_EGG;
    static const EntityDataAccessor<BlockPos>* DATA_TRAVEL_POS;
    static const EntityDataAccessor<bool>* DATA_GOING_HOME;
    static const EntityDataAccessor<bool>* DATA_TRAVELLING;
};

#endif
