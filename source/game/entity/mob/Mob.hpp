#ifndef GAME_ENTITY_MOB_MOB_HPP
#define GAME_ENTITY_MOB_MOB_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockPathTypes;
class BodyControl;
class CensoredString;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class EntityTooltipDataHolder;
class EquipmentSlot;
class Level;
class MobGroupData;
class PathNavigation;
class Sensing;
class SoundEvent;
class HumanoidArm;
class ItemEntity;
class Item;
class JumpControl;
class LookControl;
class MoveControl;
class ResourceLocation;

template <class T> class EntityDataAccessor;

class Mob : public LivingEntity
{
    friend class FollowFlockLeaderGoal;

public:
    enum SpawnPlacementType
    {
        SPAWN_PLACEMENT_TYPE_ON_GROUND = 0,
        SPAWN_PLACEMENT_TYPE_1 = 1,
        SPAWN_PLACEMENT_TYPE_IN_WATER = 2,
        SPAWN_PLACEMENT_TYPE_IN_OPEN_WATER = 3,
        SPAWN_PLACEMENT_TYPE_ON_WATER_FLOOR = 4
    };

    Mob(Level* level);
    virtual ~Mob();
    virtual unsigned long long GetType();
    virtual void fjDerivedCtorCalls();
    virtual void defineSynchedData();
    virtual void tick();
    virtual void baseTick();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual bool interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    using Entity::startRiding;
    virtual bool startRiding(boost::shared_ptr<Entity> entity, bool force);
    virtual void dropEquipment(bool hitByPlayer, int lootingLevel);
    virtual void dropAllDeathLoot(bool hitByPlayer, int lootingLevel, const DamageSource* source);
    virtual void dropDeathLoot(bool hitByPlayer, int lootingLevel);
    virtual void setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> getItemSlot(const EquipmentSlot* slot);
    virtual void setSpeed(float speed);
    virtual float tickHeadTurn(float yBodyRot, float headTurnDelta);
    virtual void aiStep();
    virtual void serverAiStep();
    virtual bool isEffectiveAi();
    virtual const HumanoidArm* getMainArm();
    virtual void registerGoals();
    virtual PathNavigation* createNavigation(Level* level);
    virtual float getPathfindingMalus(const BlockPathTypes* type);
    virtual void setPathfindingMalus(const BlockPathTypes* type, float malus);
    virtual BodyControl* createBodyControl();

    static void staticCtor();
    void _init();
    void registerAttributes();
    virtual LookControl* getLookControl();
    virtual MoveControl* getMoveControl();
    virtual JumpControl* getJumpControl();
    virtual PathNavigation* getNavigation();
    virtual Sensing* getSensing();
    virtual boost::shared_ptr<LivingEntity> getTarget();
    virtual void setTarget(boost::shared_ptr<LivingEntity> target);
    virtual bool canAttackType(unsigned long long type);
    virtual void ate();
    virtual int getAmbientSoundInterval();
    virtual void playAmbientSound();
    virtual void resetAmbientSoundTime();
    virtual void setLevel(Level* level);
    virtual void spawnAnim();
    virtual const SoundEvent* getAmbientSound();
    virtual Item* getDeathLoot();
    virtual ResourceLocation* getDefaultLootTable();
    void playHurtSound(const DamageSource* source);
    virtual ResourceLocation* getDefaultGearTable();
    int getExperienceReward(boost::shared_ptr<Player> player);
    virtual void setZza(float zza);
    void handleEntityEvent(unsigned char event, int data);
    void setLeader(bool leader);
    virtual void setYya(float yya);
    virtual void setXxa(float xxa);
    static void addDataWalkers(DataFixerUpper* fixer, unsigned long long entityType);
    bool isLeftHanded();
    virtual void pickUpItem(boost::shared_ptr<ItemEntity> item);
    virtual bool canHoldItem(not_null_ptr<ItemInstance> item);
    void setPersistenceRequired(bool required);
    virtual bool removeWhenFarAway();
    virtual void checkDespawn();
    virtual void customServerAiStep();
    virtual int getMaxHeadXRot();
    virtual int getMaxHeadYRot();
    virtual bool checkSpawnRules();
    static const EquipmentSlot* getEquipmentSlotForItem(not_null_ptr<ItemInstance> item);
    virtual bool checkSpawnObstruction();
    virtual float getSizeScale();
    virtual float getHeadSizeScale();
    virtual int getMaxSpawnClusterSize();
    virtual void populateDefaultEquipmentSlots(const DifficultyInstance& difficulty);
    virtual void populateDefaultEquipmentEnchantments(const DifficultyInstance& difficulty);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString);
    float rotlerp(float from, float to, float max);
    void lookAt(boost::shared_ptr<Entity> entity, float maxHeadXRot, float maxHeadYRot);
    virtual void finalizeSpawnEggSpawn(int type);
    virtual bool canBeControlledByRider();
    virtual void setDropChance(const EquipmentSlot* slot, float chance);
    virtual bool canPickUpLoot();
    virtual void setCanPickUpLoot(bool canPickUp);
    int getMaxFallDistance();
    NonNullList<not_null_ptr<ItemInstance> > getHandSlots();
    NonNullList<not_null_ptr<ItemInstance> > getArmorSlots();
    static Item* getEquipmentForSlot(const EquipmentSlot* slot, int tier);
    virtual bool isPersistenceRequired();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual void underwaterEjectRider();
    virtual void setNoAi(bool noAi);
    virtual void setLeftHanded(bool leftHanded);
    virtual bool isNoAi();
    bool canBeLeashed(const boost::shared_ptr<Player>& player);
    unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    bool shouldRender(double distance);
    static bool isValidSlotForItem(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item);
    bool setSlot(int slot, const not_null_ptr<ItemInstance>& item);
    bool isControlledByLocalInstance();
    int maxFlockSize();
    bool shouldFollow();
    void setFollowsLeader(bool follows);
    bool followsLeader();
    bool isLeader();
    virtual std::wstring getName();

protected:
    int mAmbientSoundTime;
    int mExperienceReward;
    LookControl* mLookControl;
    MoveControl* mMoveControl;
    JumpControl* mJumpControl;
    BodyControl* mBodyControl;
    PathNavigation* mNavigation;
    GoalSelector mGoalSelector;
    GoalSelector mTargetSelector;
    boost::weak_ptr<LivingEntity> mTarget;
    Sensing* mSensing;
    NonNullList<not_null_ptr<ItemInstance> > mHandItems;
    float mHandDropChances[2];
    NonNullList<not_null_ptr<ItemInstance> > mArmorItems;
    float mArmorDropChances[4];
    NonNullList<not_null_ptr<ItemInstance> > mAdditionalItems;
    float* mAdditionalDropChances;
    unsigned int mAdditionalItemCount;
    bool mCanPickUpLoot;
    bool mPersistenceRequired;
    boost::unordered_map<const BlockPathTypes*, float> mPathfindingMalus;
    boost::shared_ptr<CensoredString> mCustomName;
    float mMoveSpeed;
    ResourceLocation* mLootTable;
    long long mLootTableSeed;
    bool mLeader;
    bool mFollowsLeader;
    int mMaxFlockSize;
    int mFlockSize;

private:
    static const EntityDataAccessor<unsigned char>* DATA_MOB_FLAGS_ID;
};

#endif
