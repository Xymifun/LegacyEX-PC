#ifndef GAME_ENTITY_MOB_LIVINGENTITY_HPP
#define GAME_ENTITY_MOB_LIVINGENTITY_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockPos.hpp"

class TextureAtlasSprite;
class AABB;
class Attribute;
class AttributeInstance;
class AttributeModifier;
class BaseAttributeMap;
class Block;
class BlockState;
class CombatTracker;
class CompoundTag;
class DamageSource;
class EntityDataAccessor_Base;
class EquipmentSlot;
class HumanoidArm;
class InsomniaComponent;
class ItemInstance;
class Level;
class LivingEntity;
class MobEffect;
class MobEffectInstance;
class Player;
class Random;
class SoundEvent;
class Team;
class ThermalAreaRuleDefinition;
class Vec3;

template <class T> class EntityDataAccessor;

enum EMobType
{
    MOB_TYPE_UNDEFINED,
    MOB_TYPE_UNDEAD,
    MOB_TYPE_ARTHROPOD,
    MOB_TYPE_ILLAGER,
    MOB_TYPE_WATER
};

class LivingEntity : public Entity
{
public:
    LivingEntity(Level* level);

    virtual unsigned long long GetType();
    virtual ~LivingEntity();

    virtual void fjDerivedCtorCalls();
    virtual void kill();
    virtual void defineSynchedData();
    virtual void tick();
    virtual void baseTick();
    virtual void outOfWorld();
    virtual void checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos);
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual int getLightColor();
    virtual void markHurt();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual Vec3* getViewVector(float partialTicks);
    virtual bool isPickable();
    virtual bool isPushable(boost::shared_ptr<Entity> entity);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual bool isAlive();
    virtual void rideTick();
    virtual void stopRiding();
    virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual void animateHurt();
    virtual void setSprinting(bool sprinting);
    virtual Team* getTeam();
    virtual void spawnEatParticles(const ItemInstance& itemInstance, int count);
    virtual float getYHeadRot();
    virtual void setYHeadRot(float yHeadRot);
    virtual void setYBodyRot(float yBodyRot);
    virtual bool shouldShowName();
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual void setBoundingBox(AABB* boundingBox);
    virtual void stopCurrentLerp();

    virtual void registerAttributes();
    virtual bool isWaterMob();
    virtual bool breathesWater();
    virtual bool breathesAir();
    virtual bool breathesLava();
    virtual bool breathesSolids();
    virtual int GetSuffocateTime();
    virtual float GetInhaleTime();
    virtual bool generatesBubbles();
    virtual void onChangedBlock(const BlockPos& pos);
    virtual bool isBaby();
    virtual void tickDeath();
    virtual bool shouldDropExperience();
    virtual int decreaseAirSupply(int airSupply);
    virtual int getExperienceReward(boost::shared_ptr<Player> player);
    virtual bool isAlwaysExperienceDropper();
    virtual Random* getRandom();
    virtual boost::shared_ptr<LivingEntity> getLastHurtByMob();
    virtual int getLastHurtByMobTimestamp();
    virtual void setLastHurtByMob(boost::shared_ptr<LivingEntity> attacker);
    virtual boost::shared_ptr<LivingEntity> getLastHurtMob();
    virtual int getLastHurtMobTimestamp();
    virtual void setLastHurtMob(boost::shared_ptr<Entity> target);
    virtual int getNoActionTime();
    virtual void tickEffects();
    virtual void updateInvisibilityStatus();
    virtual void removeEffectParticles();
    virtual void removeAllEffects();
    virtual std::vector<MobEffectInstance*>* getActiveEffects();
    virtual bool hasEffect(MobEffect* effect);
    virtual MobEffectInstance* getEffect(MobEffect* effect);
    virtual void addEffect(MobEffectInstance* effectInstance, const boost::shared_ptr<Entity>& source);
    virtual void addEffectNoUpdate(MobEffectInstance* effectInstance);
    virtual bool canBeAffected(MobEffectInstance* effectInstance);
    virtual bool isInvertedHealAndHarm();
    virtual MobEffectInstance* removeEffectNoUpdate(MobEffect* effect);
    virtual void removeEffect(MobEffect* effect);
    virtual void onEffectAdded(MobEffectInstance* effectInstance);
    virtual void onEffectUpdated(MobEffectInstance* effectInstance, bool reapply);
    virtual void onEffectRemoved(MobEffectInstance* effectInstance);
    virtual void heal(float amount);
    virtual float getHealth();
    virtual void setHealth(float health);
    virtual const DamageSource* getLastDamageSource();
    virtual void blockUsingShield(const boost::shared_ptr<LivingEntity>& attacker);
    virtual void playHurtSound(const DamageSource* source);
    virtual bool checkTotemDeathProtection(const DamageSource* source);
    virtual bool isDamageSourceBlocked(const DamageSource* source);
    virtual void breakItem(not_null_ptr<ItemInstance> itemInstance);
    virtual void die(const DamageSource* source);
    virtual void dropEquipment(bool hitByPlayer, int lootingLevel);
    virtual void knockback(boost::shared_ptr<Entity> attacker, float strength, double x, double z);
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getFallDamageSound(int distance);
    virtual void dropAllDeathLoot(bool hitByPlayer, int lootingLevel, const DamageSource* source);
    virtual void dropDeathLoot(bool hitByPlayer, int lootingLevel);
    virtual bool onLadder();
    virtual bool isShootable();
    virtual int getArmorValue();
    virtual void hurtArmor(float damage);
    virtual void hurtCurrentlyUsedShield(float damage);
    virtual float getDamageAfterArmorAbsorb(const DamageSource* source, float damage);
    virtual float getDamageAfterMagicAbsorb(const DamageSource* source, float damage);
    virtual void actuallyHurt(const DamageSource* source, float damage);
    virtual CombatTracker* getCombatTracker();
    virtual boost::shared_ptr<LivingEntity> getKillCredit();
    virtual float getMaxHealth();
    virtual int getArrowCount();
    virtual void setArrowCount(int arrowCount);
    virtual bool swing(InteractionHand::EInteractionHand hand);
    virtual void updateSwingTime();
    virtual AttributeInstance* getAttribute(Attribute* attribute);
    virtual BaseAttributeMap* getAttributes();
    virtual EMobType getMobType();
    virtual not_null_ptr<ItemInstance> getMainHandItem();
    virtual not_null_ptr<ItemInstance> getOffhandItem();
    virtual not_null_ptr<ItemInstance> getItemInHand(InteractionHand::EInteractionHand hand);
    virtual void setItemInHand(InteractionHand::EInteractionHand hand, not_null_ptr<ItemInstance> itemInstance);
    virtual bool hasItemInSlot(const EquipmentSlot* slot);
    virtual NonNullList<not_null_ptr<ItemInstance> > getArmorSlots() = 0;
    virtual void setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> itemInstance) = 0;
    virtual not_null_ptr<ItemInstance> getItemSlot(const EquipmentSlot* slot) = 0;
    virtual TextureAtlasSprite* getItemInHandIcon(not_null_ptr<ItemInstance> itemInstance, int frame);
    virtual float getSoundVolume();
    virtual float getVoicePitch();
    virtual bool isImmobile();
    virtual void findStandUpPosition(boost::shared_ptr<Entity> vehicle);
    virtual float getJumpPower();
    virtual void jumpFromGround();
    virtual void jumpInWater();
    virtual void goDownInWater();
    virtual void jumpInLava();
    virtual float getWaterSlowDown();
    virtual void travel(float xxa, float yya, float zza);
    virtual void updateModelAnimation();
    virtual float getSpeed();
    virtual void setSpeed(float speed);
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual bool isSleeping();
    virtual float tickHeadTurn(float yBodyRot, float headTurnDelta);
    virtual void aiStep();
    virtual void serverAiStep();
    virtual void pushEntities();
    virtual void doPush(boost::shared_ptr<Entity> entity);
    virtual void setJumping(bool jumping);
    virtual void take(boost::shared_ptr<Entity> entity, int count);
    virtual bool canSee(boost::shared_ptr<Entity> entity);
    virtual float getAttackAnim(float partialTicks);
    virtual AABB* getSweptVolume();
    virtual bool isEffectiveAi();
    virtual float getAbsorptionAmount();
    virtual void setAbsorptionAmount(float absorptionAmount);
    virtual void onEnterCombat();
    virtual void onLeaveCombat();
    virtual void updateEffectVisibility();
    virtual const HumanoidArm* getMainArm() = 0;
    virtual bool isUsingItem();
    virtual InteractionHand::EInteractionHand getUsedItemHand();
    virtual void updatingUsingItem();
    virtual void startUsingItem(InteractionHand::EInteractionHand hand);
    virtual void spawnItemUseParticles(not_null_ptr<ItemInstance> itemInstance, int count);
    virtual void completeUsingItem();
    virtual int getTicksUsingItem();
    virtual void releaseUsingItem();
    virtual void releaseUsingItem(int useDuration);
    virtual void stopUsingItem();
    virtual bool isBlocking();
    virtual bool isFallFlying();
    virtual int getFallFlyingTicks();
    virtual bool isAffectedByPotions();
    virtual bool attackable();
    virtual void setRecordPlayingNearby(const BlockPos& pos, bool playing);
    virtual bool IsCreativeFlying();

    static void staticCtor();
    static bool areAllEffectsAmbient(std::vector<MobEffectInstance*>* effects);
    static void fallFlyingTravel(double& xd, double& yd, double& zd, Vec3* lookAngle, float& xRot, float& liftFactor,
                                 double& horizontalSpeed, double lookHorizontal);

    void _init();
    void ResetThermalVariables();
    bool isInterpolating() const;
    double getLerpToYRot() const;
    double getLerpToXRot() const;
    InsomniaComponent* getInsomniaComponent() const;
    bool canBreathe();
    int GetAirRegenPerTick();
    void playEquipSound(not_null_ptr<ItemInstance> itemInstance);
    bool trapdoorUsableAsLadder(const BlockPos& pos, const BlockState* state);
    int getCurrentSwingDuration();
    float getFrictionForTravel();
    bool shouldApplyWaterGravity();
    void activateElytraSpeedBoost(double boost);
    void CheckThermalAreas();
    void takeFallFlyingCollisionDamage();
    void updateEquipmentSlots();
    void updateFallFlying();
    int getUseItemRemainingTicks();
    bool getElytraProximityWarning();
    double getElytraCollisionDistance();
    not_null_ptr<ItemInstance> getUseItem();
    bool randomTeleport(double x, double y, double z);
    void disableFallFlyingState(bool disabled);
    bool isFallFlyingStateDisabled();
    boost::unordered_map<MobEffect*, MobEffectInstance*>* getActiveEffectsMap();
    int getSwingCount();

    static const AttributeModifier* SPEED_MODIFIER_SPRINTING;

    static const EntityDataAccessor<unsigned char>* DATA_LIVING_ENTITY_FLAGS;
    static const EntityDataAccessor<float>* DATA_HEALTH_ID;
    static const EntityDataAccessor<int>* DATA_EFFECT_COLOR_ID;
    static const EntityDataAccessor<bool>* DATA_EFFECT_AMBIENCE_ID;
    static const EntityDataAccessor<int>* DATA_ARROW_COUNT_ID;

    BaseAttributeMap* mAttributes;
    CombatTracker* mCombatTracker;
    boost::unordered_map<MobEffect*, MobEffectInstance*> mActiveEffects;
    NonNullList<not_null_ptr<ItemInstance> > mLastHandItems;
    NonNullList<not_null_ptr<ItemInstance> > mLastArmorItems;
    bool mSwinging;
    InteractionHand::EInteractionHand mSwingingArm;
    int mSwingTime;
    int mSwingCount;
    int mRemoveArrowTime;
    int mHurtTime;
    int mHurtDuration;
    float mHurtDir;
    int mDeathTime;
    float mOAttackAnim;
    float mAttackAnim;
    int mAttackStrengthTicker;
    float mAnimationSpeedOld;
    float mAnimationSpeed;
    float mAnimationPosition;
    int mInvulnerableDuration;
    float mOBob;
    float mBob;
    float mTimeOffs;
    float mRotA;
    float mYBodyRot;
    float mYBodyRotO;
    float mYHeadRot;
    float mYHeadRotO;
    float mFlyingSpeed;
    double mGlideDistance;
    double mThermalLiftSpeed;
    bool mThermalLerping;
    int mThermalLerpTicks;
    double mThermalTargetLiftSpeed;
    bool mThermalLiftRising;
    double mThermalCeiling;
    bool mInThermalArea;
    bool mElytraSpeedBoostActive;
    double mElytraSpeedBoost;
    ThermalAreaRuleDefinition* mThermalArea;
    bool mElytraProximityWarning;
    bool mThermalAreaChanged;
    double mElytraCollisionDistance;
    bool mThermalAreasChecked;
    int mDeathMessageId;
    std::wstring mDeathMessage;
    bool mHurtByPlayerOnPeaceful;
    boost::shared_ptr<Player> mLastHurtByPlayer;
    int mLastHurtByPlayerTime;
    bool mDead;
    int mNoActionTime;
    float mORun;
    float mRun;
    float mAnimStep;
    float mAnimStepO;
    float mRotOffs;
    int mDeathScore;
    float mLastHurt;
    bool mJumping;
    InsomniaComponent* mInsomniaComponent;
    float mXxa;
    float mYya;
    float mZza;
    float mYRota;
    int mLerpSteps;
    double mLerpX;
    double mLerpY;
    double mLerpZ;
    double mLerpYRot;
    double mLerpXRot;
    bool mEffectsDirty;
    boost::weak_ptr<LivingEntity> mLastHurtByMob;
    int mLastHurtByMobTimestamp;
    boost::weak_ptr<LivingEntity> mLastHurtMob;
    int mLastHurtMobTimestamp;
    float mSpeed;
    float mWaterTravelSpeedFactor;
    int mWaterTravelCheckTicks;
    int mNoJumpDelay;
    float mAbsorptionAmount;
    not_null_ptr<ItemInstance> mUseItem;
    int mUseItemRemaining;
    int mFallFlyTicks;
    BlockPos mLastPos;
    DamageSource* mLastDamageSource;
    long long mLastDamageStamp;
    bool mFallFlyingStateDisabled;
    int mLightCacheTicks;
    int mLightSampleMinX;
    int mLightSampleMaxX;
    int mLightSampleMinY;
    int mLightSampleMaxY;
    int mLightSampleMinZ;
    int mLightSampleMaxZ;
    int mLightSamples[32];
    bool mLightCacheValid;
    int mLastLightColor;
    bool mSwimSoundPlayed;
};

#endif
