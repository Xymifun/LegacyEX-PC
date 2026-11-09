#ifndef GAME_CORE_DOLPHIN_HPP
#define GAME_CORE_DOLPHIN_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AgableWaterAnimal.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/entity/mob/Creature.hpp"

class PathNavigation;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Entity;
class EntityTooltipDataHolder;
class FindUnderwaterTreasureGoal;
class ItemEntity;
class Level;
class MobGroupData;
class SoundEvent;
class WaterBoundPathNavigation;
class Player;
template <class T> class not_null_ptr;

class Dolphin : public AgableWaterAnimal, public Creature
{
public:
    Dolphin(Level* level);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& name);
    virtual int getMaxAirSupply();
    virtual float getEyeHeight();
    virtual int getAmbientSoundInterval();
    virtual int getMaxHeadXRot();
    virtual int getMaxHeadYRot();
    virtual void aiStep();
    virtual void tick();
    virtual bool isPushedByWater();
    bool isInSpawnableOcean();
    virtual bool checkSpawnRules();
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual void positionRider(const boost::shared_ptr<Entity>& passenger);
    boost::shared_ptr<ItemEntity> drop(const not_null_ptr<ItemInstance>& item);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> parent);
    bool isItemABribingFish(not_null_ptr<ItemInstance> item);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual PathNavigation* createNavigation(Level* level);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual bool canHoldItem(not_null_ptr<ItemInstance> item);
    virtual bool canRide(boost::shared_ptr<Entity> entity);
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getSwimSound();
    virtual const SoundEvent* getSwimSplashSound();
    virtual bool generatesBubbles();
    virtual unsigned long long GetType();
    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);

private:
    int mTreasureTime;
    int mBlowholeCooldown;
    FindUnderwaterTreasureGoal* mTreasureGoal;
};

#endif
