#ifndef GAME_CORE_WITHERBOSS_HPP
#define GAME_CORE_WITHERBOSS_HPP

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/Goal.hpp"
#include "game/entity/mob/BossMob.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"
#include "game/world/block/Predicate.hpp"

class AABB;
class Block;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Level;
class LivingEntity;
class MobEffectInstance;
class MobGroupData;
class PathNavigation;
class ServerBossEvent;
class ServerPlayer;
class SoundEvent;
class Vec3;

class WitherBoss : public Monster, public RangedAttackMob, public BossMob
{
public:
    class WitherDoNothingGoal : public Goal
    {
    public:
        WitherDoNothingGoal(WitherBoss* wither);
        virtual ~WitherDoNothingGoal();

        virtual bool canUse();
        virtual std::wstring getName() { return L"WitherDoNothing"; }

    private:
        WitherBoss* mWither;
    };


    WitherBoss(Level* level);
    virtual ~WitherBoss();

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    static void staticCtor();
    static bool canDestroy(Block* block);

    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual void causeFallDamage(float fallDistance, float damageModifier);
    virtual int getLightColor();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual bool isPickable();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionContructionTag(CompoundTag* tag);
    virtual CompoundTag* getAdditionContructionTag();
    virtual bool canRide(boost::shared_ptr<Entity> entity);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual bool canFly();
    virtual void makeStuckInWeb();
    virtual bool canChangeDimensions();
    virtual std::wstring getDisplayName();
    virtual void setCustomName(const std::wstring& name, const std::wstring& displayName, bool visible);
    virtual void startSeenByPlayer(boost::shared_ptr<ServerPlayer> player);
    virtual void stopSeenByPlayer(boost::shared_ptr<ServerPlayer> player);
    virtual void registerAttributes();
    virtual void addEffect(MobEffectInstance* effectInstance, const boost::shared_ptr<Entity>& source);
    virtual void heal(float amount);
    virtual float getHealth();
    virtual void die(const DamageSource* source);
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void dropDeathLoot(bool hitByPlayer, int lootingLevel);
    virtual int getArmorValue();
    virtual float getMaxHealth() { return LivingEntity::getMaxHealth(); }
    virtual EMobType getMobType();
    virtual void aiStep();
    virtual void registerGoals();
    virtual PathNavigation* createNavigation(Level* level);
    virtual boost::shared_ptr<LivingEntity> getTarget();
    virtual const SoundEvent* getAmbientSound();
    virtual void checkDespawn();
    virtual void customServerAiStep();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString);

    virtual int getDimension();
    virtual void makeInvulnerable();
    virtual void performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor);
    virtual void performRangedAttack(int head, boost::shared_ptr<LivingEntity> target);
    virtual void performRangedAttack(int head, double x, double y, double z, bool dangerous);
    virtual bool isAggressive();
    virtual void setAggressive(bool aggressive);
    virtual double getHeadX(int head);
    virtual double getHeadY(int head);
    virtual double getHeadZ(int head);
    virtual float rotlerp(float from, float to, float maxDelta);

    void setAerialAttack(bool aerialAttack);
    bool getAerialAttack() const;
    int getInvulnerableTicks();
    void setInvulnerableTicks(int ticks);
    int getAlternativeTarget(int head);
    void setAlternativeTarget(int head, int target);
    bool isPowered();
    void generateParticles();
    bool IsChargeAttacking();
    bool canShoot();
    std::map<boost::shared_ptr<ServerPlayer>, int>& getPlayerParty();
    void changePhase();
    float getHeadYRot(int head);
    float getHeadXRot(int head);
    float getOverlayAlpha();
    float getSwellAmount(float partialTicks) const;
    bool wantsToMove();
    void setWantsToMove(bool wantsToMove);
    void setIsPathing(bool isPathing);
    void setShotDelay(int shotDelay);
    bool IsSpawning() const;
    void destroyBlocks(Level* level, const AABB* area, int radius);

private:
    static const EntityDataAccessor<int>* DATA_TARGET_A;
    static const EntityDataAccessor<int>* DATA_TARGET_B;
    static const EntityDataAccessor<int>* DATA_TARGET_C;
    static const EntityDataAccessor<int>* DATA_TARGETS[3];
    static const EntityDataAccessor<int>* DATA_ID_INV;
    static const EntityDataAccessor<bool>* DATA_AERIAL_ATTACK;
    static const EntityDataAccessor<bool>* DATA_CHARGE_ATTACK;
    static Predicate<boost::shared_ptr<Entity> >* LIVING_ENTITY_SELECTOR;

    int mInitialShieldHealth;
    float mXRotHeads[2];
    float mYRotHeads[2];
    float mXRotOHeads[2];
    float mYRotOHeads[2];
    DamageSource* mDeathSource;
    int mPhase;
    int mShotDelay;
    int mPauseTicks;
    int mDyingFrames;
    int mDyingFlashInterval;
    float mOverlayAlpha;
    float mSwellAmount;
    float mOldSwellAmount;
    int mChargeDelay;
    bool mCharging;
    int mHeadShotCooldown;
    int mFireRate;
    int mShotTicks;
    float mTargetRange;
    int mSpawningFrames;
    bool mDiving;
    int mChargeTimer;
    Vec3* mChargeDirection;
    bool mWantsToMove;
    int mChargeTicks;
    int mTicksSinceShot;
    int mShieldHealth;
    int mShotCount;
    int mChargeInterval;
    bool mSkipNextCharge;
    int mMaxHealth;
    int mLastHealthInterval;
    float mSpinSpeed;
    int mNextHeadUpdate[2];
    int mIdleHeadUpdates[2];
    int mDestroyBlocksTick;
    int mPhaseHealth;
    int mHealthInterval;
    bool mIsPathing;
    ServerBossEvent* mBossEvent;
};

#endif
