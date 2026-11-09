#ifndef GAME_CORE_OCELOT_HPP
#define GAME_CORE_OCELOT_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/TamableAnimal.hpp"
#include "game/core/InteractionHand.hpp"

class AgableMob;
class AvoidEntityGoal;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Entity;
class EntityTooltipDataHolder;
class ItemInstance;
class Level;
class MobGroupData;
class Player;
class SoundEvent;
class TemptGoal;
template <class T> class EntityDataAccessor;

class Ocelot : public TamableAnimal
{
public:
    Ocelot(Level* level);
    virtual ~Ocelot();
    static Entity* create(Level* level);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    void setSittingOnBlock(bool sitting);
    bool isSittingOnBlock();
    void SetIsTempted(bool tempted);
    virtual void registerGoals();
    virtual void defineSynchedData();
    virtual void customServerAiStep();
    virtual bool removeWhenFarAway();
    virtual void registerAttributes();
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual float getSoundVolume();
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual bool isFood(const not_null_ptr<ItemInstance>& item);
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    virtual int getCatType();
    virtual void setCatType(int catType);
    virtual bool checkSpawnRules();
    virtual bool checkSpawnObstruction();
    virtual std::wstring getName();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString);
    virtual void positionRider(const boost::shared_ptr<Entity>& passenger);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual void reassessTameGoals();
    virtual unsigned long long GetType();

private:
    static const EntityDataAccessor<int>* DATA_CAT_TYPE;
    static const EntityDataAccessor<bool>* DATA_IS_TEMPTED;
    AvoidEntityGoal* mAvoidPlayersGoal;
    TemptGoal* mTemptGoal;
};

#endif
