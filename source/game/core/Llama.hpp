#ifndef GAME_CORE_LLAMA_HPP
#define GAME_CORE_LLAMA_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/mob/AbstractChestedHorse.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/mob/MobGroupData.hpp"

class Block;
class BlockPos;
class CompoundTag;
class Container;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class DyeColor;
class Entity;
class EntityTooltipDataHolder;
class Level;
class Player;
class SoundEvent;
class SoundType;
class ItemInstance;
class AgableMob;

class Llama : public AbstractChestedHorse, public RangedAttackMob
{
public:
    class LlamaDefendTargetGoal : public NearestAttackableTargetGoal
    {
    public:
        LlamaDefendTargetGoal(Llama* llama);
        virtual bool canUse();
        virtual double getFollowDistance();
    };

    class LlamaGroupData : public MobGroupData
    {
    public:
        LlamaGroupData(int variant);
        virtual ~LlamaGroupData();
        int mVariant;
    };

    class LlamaHurtByTargetGoal : public HurtByTargetGoal
    {
    public:
        LlamaHurtByTargetGoal(Llama* llama);
        virtual bool canContinueToUse();
    };

    Llama(Level* level);
    virtual ~Llama();
    static Entity* create(Level* level);
    const DyeColor* getSwag();
    virtual void containerChanged(Container* container);
    void spit(const boost::shared_ptr<LivingEntity>& target);
    virtual void performRangedAttack(boost::shared_ptr<LivingEntity> target, float velocity);
    virtual bool isAggressive();
    virtual void setAggressive(bool aggressive);
    static void staticCtor();
    void setStrength(int strength);
    void setRandomStrength();
    int getStrength();
    int getVariant();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setVariant(int variant);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    virtual int getInventorySize();
    virtual void positionRider(const boost::shared_ptr<Entity>& passenger);
    virtual double getRideHeight();
    virtual bool canBeControlledByRider();
    virtual int handleEating(const boost::shared_ptr<Player>& player, const not_null_ptr<ItemInstance>& item);
    virtual bool isImmobile();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& name);
    bool hasSwag();
    virtual const SoundEvent* getAngrySound();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual void playChestEquipsSound();
    virtual void makeMad();
    virtual ResourceLocation* getDefaultLootTable();
    virtual int getInventoryColumns();
    virtual bool wearsArmor();
    virtual bool isArmor(const not_null_ptr<ItemInstance>& item);
    virtual bool canBeSaddled() const;
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual bool HideUseTooltips();
    void setSwag(const DyeColor* color);
    void setSwagByItem(const not_null_ptr<ItemInstance>& item);
    virtual void updateEquipment();
    virtual int getMaxTemper() const;
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    void setDidSpit(bool didSpit);
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    void leaveCaravan();
    void joinCaravan(const boost::shared_ptr<Llama>& llama);
    bool hasCaravanTail();
    bool inCaravan();
    boost::shared_ptr<Llama> getCaravanHead();
    double followLeashSpeed();
    virtual void followMommy();
    virtual bool canEatGrass() const;
    virtual unsigned long long GetType();
    static void addDataWalkers(DataFixerUpper* fixer);

private:
    static const EntityDataAccessor<int>* DATA_STRENGTH;
    static const EntityDataAccessor<int>* DATA_SWAG;
    static const EntityDataAccessor<int>* DATA_VARIANT;

    bool mDidSpit;
    boost::weak_ptr<Llama> mCaravanHead;
    boost::weak_ptr<Llama> mCaravanTail;
};

#endif
