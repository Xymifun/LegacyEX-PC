#ifndef GAME_CORE_POLARBEAR_HPP
#define GAME_CORE_POLARBEAR_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Animal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/mob/MobGroupData.hpp"

class Block;
class BlockPos;
class DamageSource;
class DifficultyInstance;
class Entity;
class ItemInstance;
class Level;
class LivingEntity;
class MobGroupData;
class PathfinderMob;
class SoundEvent;
template <class T> class EntityDataAccessor;

class PolarBear : public Animal
{
public:
    class PolarBearAttackPlayersGoal : public NearestAttackableTargetGoal
    {
    public:
        PolarBearAttackPlayersGoal(PolarBear* polarBear);
        virtual bool canUse();
        virtual double getFollowDistance();
    };

    class PolarBearGroupData : public MobGroupData
    {
    public:
        PolarBearGroupData();
        virtual ~PolarBearGroupData();
        bool mIsBaby;
    };

    class PolarBearHurtByTargetGoal : public HurtByTargetGoal
    {
    public:
        PolarBearHurtByTargetGoal(PolarBear* polarBear);
        virtual void start();
        virtual void alertOther(boost::shared_ptr<PathfinderMob> mob, boost::shared_ptr<LivingEntity> target);
    };

    class PolarBearMeleeAttackGoal : public MeleeAttackGoal
    {
    public:
        PolarBearMeleeAttackGoal(PolarBear* polarBear);
        virtual bool canUse();
        virtual void checkAndPerformAttack(boost::shared_ptr<LivingEntity> target, double distance);
        virtual void stop();
        virtual double getAttackReachSqr(boost::shared_ptr<LivingEntity> target);
    };

    class PolarBearPanicGoal : public PanicGoal
    {
    public:
        PolarBearPanicGoal(PolarBear* polarBear);
        virtual ~PolarBearPanicGoal();
        virtual bool canUse();
    };

    PolarBear(Level* level);
    static Entity* create(Level* level);
    static void staticCtor();
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual bool isFood(const not_null_ptr<ItemInstance>& itemInstance);
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    void playWarningSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual void defineSynchedData();
    bool isStanding();
    virtual void tick();
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    void setStanding(bool standing);
    float getStandingAnimationScale(float partialTick);
    virtual float getWaterSlowDown();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& name);
    virtual bool checkSpawnRules();
    virtual unsigned long long GetType();

private:
    static const EntityDataAccessor<bool>* DATA_STANDING;
    float mStandingScaleO;
    float mStandingScale;
    int mWarningSoundTicks;
};

#endif
