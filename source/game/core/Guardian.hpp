#ifndef GAME_CORE_GUARDIAN_HPP
#define GAME_CORE_GUARDIAN_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Monster.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/world/block/Predicate.hpp"

class BlockPos;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Entity;
class EntityDataAccessor_Base;
class Goal;
class Level;
class MobGroupData;
class PathNavigation;
class RandomStrollGoal;
class SoundEvent;

class Guardian : public Monster
{
public:
    friend class GuardianAttackGoal;
    friend class GuardianMoveControl;
    class GuardianAttackGoal : public Goal
    {
    public:
        GuardianAttackGoal(Guardian* guardian);
        virtual ~GuardianAttackGoal();
        virtual bool canUse();
        virtual bool canContinueToUse();
        virtual void start();
        virtual void stop();
        virtual void tick();
        virtual std::wstring getName() { return L"GuardianAttack"; }

    private:
        Guardian* mGuardian;
        int mAttackTime;
        bool mElder;
        char mUnused0;
        char mUnused1;
        char mUnused2;
    };

    class GuardianAttackSelector : public Predicate<boost::shared_ptr<Entity> >
    {
    public:
        GuardianAttackSelector(Guardian* guardian);
        virtual ~GuardianAttackSelector();
        virtual bool apply(boost::shared_ptr<Entity> entity) const;

    private:
        Guardian* mGuardian;
    };

    class GuardianMoveControl : public MoveControl
    {
    public:
        GuardianMoveControl(Guardian* guardian);
        virtual ~GuardianMoveControl();
        virtual void tick();

    private:
        Guardian* mGuardian;
    };

    Guardian(Level* level, bool createEntityData);
    virtual ~Guardian();

    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    static Entity* create(Level* level);

    virtual void registerGoals();
    virtual void registerAttributes();
    virtual PathNavigation* createNavigation(Level* level);
    virtual void defineSynchedData();
    virtual unsigned int getAttackDuration();
    void setActiveAttackTarget(int targetId);
    bool hasActiveAttackTarget();
    boost::shared_ptr<LivingEntity> getActiveAttackTarget();
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual int getAmbientSoundInterval();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getFlopSound();
    virtual bool makeStepSound();
    virtual float getEyeHeight();
    virtual float getWalkTargetValue(const BlockPos& pos);
    float getAttackAnimationScale(float partialTicks);
    virtual void aiStep();
    float getTailAnimation(float partialTicks);
    float getSpikesAnimation(float partialTicks);
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool isDarkEnoughToSpawn();
    virtual bool checkSpawnObstruction();
    virtual bool checkSpawnRules();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual int getMaxHeadXRot();
    virtual void travel(float xxa, float yya, float zza);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString);
    virtual unsigned long long GetType();

private:
    static const EntityDataAccessor<int>* DATA_ID_ATTACK_TARGET;
    float mTailAnimation;
    float mTailAnimationOld;
    float mTailAnimationSpeed;

protected:
    float mSpikesAnimation;
    float mSpikesAnimationOld;

private:
    boost::shared_ptr<LivingEntity> mCachedAttackTarget;
    int mAttackTime;
    bool mTouchedGround;

protected:
    RandomStrollGoal* mRandomStrollGoal;
};

#endif
