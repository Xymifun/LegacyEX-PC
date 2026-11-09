#ifndef GAME_CORE_ENDERMAN_HPP
#define GAME_CORE_ENDERMAN_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Monster.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"

class AttributeModifier;
class Block;
class BlockPos;
class BlockState;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Entity;
class EntityDataAccessor_Base;
class Level;
class Player;
class SoundEvent;

template <class T> class EntityDataAccessor;

class EnderMan : public Monster
{
public:
    class EndermanLeaveBlockGoal : public Goal
    {
    public:
        EndermanLeaveBlockGoal(EnderMan* enderMan);
        virtual ~EndermanLeaveBlockGoal();
        virtual bool canUse();
        bool canPlaceBlock(Level* level, const BlockPos& pos, Block* block, const BlockState* oldState,
                           const BlockState* newState);
        virtual void tick();
        virtual std::wstring getName();

    private:
        EnderMan* mEnderMan;
    };

    class EndermanLookForPlayerGoal : public NearestAttackableTargetGoal
    {
    public:
        EndermanLookForPlayerGoal(EnderMan* enderMan);
        virtual bool canUse();
        virtual void start();
        virtual void stop();
        virtual bool canContinueToUse();
        virtual void tick();

    private:
        boost::shared_ptr<Player> mPendingTarget;
        int mAggroTime;
        int mTeleportTime;
        EnderMan* mEnderMan;
    };

    class EndermanTakeBlockGoal : public Goal
    {
    public:
        EndermanTakeBlockGoal(EnderMan* enderMan);
        virtual ~EndermanTakeBlockGoal();
        virtual bool canUse();
        virtual void tick();
        virtual std::wstring getName();

    private:
        EnderMan* mEnderMan;
    };

    EnderMan(Level* level);
    static void staticCtor();
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual void setTarget(boost::shared_ptr<LivingEntity> target);
    virtual void defineSynchedData();
    void playCreepySound();
    bool isCreepy();
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    static void addDataWalkers(DataFixerUpper* fixer);
    const BlockState* getCarryingBlock();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setCarryingBlock(const BlockState* state);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    bool isLookingAtMe(boost::shared_ptr<Player> player);
    virtual float getEyeHeight();
    virtual void aiStep();
    bool teleport(double x, double y, double z);
    bool teleport();
    virtual void customServerAiStep();
    bool teleportTowards(boost::shared_ptr<Entity> entity);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void dropEquipment(bool wasRecentlyHit, int lootingModifier);
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool hurt(const DamageSource* source, float damage);
    static Entity* create(Level* level);
    static AttributeModifier* SPEED_MODIFIER_ATTACKING;
    virtual unsigned long long GetType();

private:
    int mLastCreepySound;
    int mTargetChangeTime;
};

#endif
