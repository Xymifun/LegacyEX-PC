#ifndef GAME_CORE_WOLF_HPP
#define GAME_CORE_WOLF_HPP
#include <boost/shared_ptr.hpp>
#include "game/core/EntityType.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"
class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DyeColor;
class DataFixerUpper;
class Entity;
class ItemInstance;
class Level;
class SoundEvent;
class ResourceLocation;
class AvoidEntityGoal;
class EntityTooltipDataHolder;
class Llama;
template <class T> class EntityDataAccessor;
class Wolf : public TamableAnimal
{
public:
    class WolfAvoidEntityGoal : public AvoidEntityGoal
    {
    public:
        WolfAvoidEntityGoal(Wolf* wolf, const std::type_info& avoidType, float maxDistance, double walkSpeed,
                            double sprintSpeed);
        bool avoidLlama(const boost::shared_ptr<Llama>& llama);
        virtual bool canUse();
        virtual void start();
        virtual void tick();

    private:
        Wolf* mWolf;
    };
    explicit Wolf(Level* level);
    static Entity* create(Level* level);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerGoals();
    virtual void registerAttributes();
    void setAngry(bool angry);
    virtual void setTarget(boost::shared_ptr<LivingEntity> target);
    virtual void customServerAiStep();
    virtual void defineSynchedData();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    bool isAngry();
    const DyeColor* getCollarColor();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setCollarColor(const DyeColor* color);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual float getSoundVolume();
    virtual ResourceLocation* getDefaultLootTable();
    virtual void aiStep();
    bool isInterested();
    virtual void tick();
    float getBodyRollAngle(float a, float b);
    float getHeadRollAngle(float partialTick);
    virtual float getEyeHeight();
    virtual int getMaxHeadXRot();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual void setTame(bool tame);
    void tame(boost::shared_ptr<Player> player, bool success, bool broadcast);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual void handleEntityEvent(unsigned char event, int data);
    float getTailAngle();
    virtual bool isFood(const not_null_ptr<ItemInstance>& item);
    int GetSynchedHealth();
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    void setIsInterested(bool interested);
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    virtual bool wantsToAttack(boost::shared_ptr<LivingEntity> target, boost::shared_ptr<LivingEntity> owner);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    virtual unsigned long long GetType();
    bool isWet();
    double getWetShade(float partialTick);
    virtual int getMaxSpawnClusterSize();

private:
    static const EntityDataAccessor<float>* DATA_HEALTH_ID;
    static const EntityDataAccessor<bool>* DATA_INTERESTED;
    static const EntityDataAccessor<int>* DATA_COLLAR_COLOR;
    float mInterestedAngle;
    float mInterestedAngleO;
    bool mIsWet;
    bool mIsShaking;
    float mShakeAnim;
    float mShakeAnimO;
};
#endif
