#ifndef GAME_ENTITY_MOB_ABSTRACTHORSE_HPP
#define GAME_ENTITY_MOB_ABSTRACTHORSE_HPP

#include <string>
#include "game/core/EntityType.hpp"
#include <boost/shared_ptr.hpp>
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/OwnableEntity.hpp"
#include "game/entity/mob/PlayerRideableJumping.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/SimpleContainer.hpp"
#include "game/network/net_minecraft_world__ContainerListener.hpp"

class Attribute;
class Block;
class BlockPos;
class CompoundTag;
class Container;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Entity;
class Goal;
class Level;
class MobGroupData;
class SoundType;
class ItemInstance;
class AgableMob;
template <class T> class EntityDataAccessor;


class AbstractHorse : public Animal,
                      public net_minecraft_world::ContainerListener,
                      public PlayerRideableJumping,
                      public OwnableEntity
{
public:
    AbstractHorse(Level* level);
    virtual ~AbstractHorse() {}
    bool getFlag(int flag);
    bool isSaddled();
    void setFlag(int flag, bool value);
    void setEating(bool eating);
    void setStanding(bool standing);
    void stand();
    int getTailCounter() const;
    static void staticCtor();
    virtual void fjDerivedCtorCalls();
    virtual void registerGoals();
    virtual void defineSynchedData();
    virtual bool isTamed();
    virtual std::wstring getOwnerUUID();
    virtual boost::shared_ptr<Entity> getOwner();
    virtual float getFoalScale() const;
    virtual int getInventorySize();
    virtual void createInventory();
    virtual void updateEquipment();
    virtual void containerChanged(Container* container);
    virtual const SoundEvent* getJumpSound();
    virtual const SoundEvent* getAngrySound();
    virtual bool canBeSaddled() const;
    virtual void playGallopSound(const SoundType* sound);
    virtual int getMaxTemper() const;
    virtual int handleEating(const boost::shared_ptr<Player>& player, const not_null_ptr<ItemInstance>& item);
    virtual void followMommy();
    virtual bool canEatGrass() const;
    virtual void makeMad();
    virtual void onPlayerJump(int jump);
    virtual bool canJump();
    virtual double getJumpRidingScale();
    virtual void setJumpRidingScale(float scale);
    virtual void handleStartJump(int jump);
    virtual void handleStopJump();
    virtual bool wearsArmor();
    virtual bool isArmor(const not_null_ptr<ItemInstance>& item);
    void setOwnerUUID(const std::wstring& uuid);
    virtual void updateSize(bool baby);
    bool isJumping() const;
    void setTamed(bool tamed);
    void setIsJumping(bool jumping);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    bool isEating();
    virtual void onLeashDistance(float distance);
    bool isStanding();
    bool isBred();
    void setBred(bool bred);
    void setSaddled(bool saddled);
    int getTemper() const;
    void setTemper(int temper);
    int modifyTemper(int amount);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual bool isPushable(boost::shared_ptr<Entity> entity);
    void openMouth();
    void eating(int animation);
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    boost::shared_ptr<AbstractHorse> getClosestMommy(const boost::shared_ptr<Entity>& entity, double distance);
    double getCustomJump();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getAmbientSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual void registerAttributes();
    virtual int getMaxSpawnClusterSize();
    virtual float getSoundVolume();
    virtual int getAmbientSoundInterval();
    void openInventory(const boost::shared_ptr<Player>& player);
    int CheckAndSetInLove(boost::shared_ptr<Player> player);
    void doPlayerRide(const boost::shared_ptr<Player>& player);
    virtual bool isImmobile();
    virtual bool isFood(const not_null_ptr<ItemInstance>& item);
    void moveTail();
    virtual void die(const DamageSource* source);
    virtual void aiStep();
    virtual void tick();
    bool tameWithName(const boost::shared_ptr<Player>& player);
    virtual void travel(float x, float y, float z);
    virtual void updateModelAnimation();
    static void addDataWalkers(DataFixerUpper* fixer, unsigned long long type);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    bool canParent();
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    double generateRandomMaxHealth();
    double generateRandomJumpStrength();
    double generateRandomSpeed();
    void setOffspringAttributes(const boost::shared_ptr<AgableMob>& other,
                                const boost::shared_ptr<AbstractHorse>& offspring);
    virtual bool canBeControlledByRider();
    float getEatAnim(float partialTick) const;
    float getStandAnim(float partialTick) const;
    float getMouthAnim(float partialTick) const;
    void spawnTamingParticles(bool success);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual void positionRider(const boost::shared_ptr<Entity>& passenger);
    virtual bool onLadder();
    virtual float getEyeHeight();
    virtual bool setSlot(int slot, const not_null_ptr<ItemInstance>& item);
    virtual boost::shared_ptr<Entity> getControllingPassenger();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& name);
    virtual bool removeWhenFarAway();
    virtual unsigned long long GetType();

protected:
    static const EntityDataAccessor<unsigned char>* DATA_FLAGS;
    static const EntityDataAccessor<std::wstring>* DATA_OWNER_UUID;
    static Attribute* JUMP_STRENGTH;

    int mEatingCounter;
    int mMouthCounter;
    int mStandCounter;
    int mTailCounter;
    int mSprintCounter;
    long long mLastAngrySoundTime;
    bool mJumping;
    boost::shared_ptr<SimpleContainer> mInventory;
    int mTemper;
    float mJumpRidingScale;
    bool mAllowStandSliding;
    float mEatAnim;
    float mEatAnimO;
    float mStandAnim;
    float mStandAnimO;
    float mMouthAnim;
    float mMouthAnimO;
    bool mCanGallop;
    int mGallopSoundCounter;
    Goal* mPanicGoal;
    Goal* mFloatGoal;
    Goal* mWaterAvoidingRandomStrollGoal;
};

#endif
