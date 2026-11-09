#ifndef GAME_ENTITY_MOB_ENTITY_HPP
#define GAME_ENTITY_MOB_ENTITY_HPP

#include <set>
#include "game/core/EntityType.hpp"
#include <string>
#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/command/CommandSender.hpp"
#include "game/command/CommandStats.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/entity/mob/EntityCategory.hpp"
#include "game/entity/mob/EntityTickCache.hpp"
#include "game/entity/mob/MoverType.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockPos.hpp"
#include "vendor/_SkinAdjustments.hpp"

class AABB;
class Block;
class BlockState;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Direction;
class EntityDataAccessor_Base;
class EntityPos;
class HitResult;
class EntityTooltipDataHolder;
class EquipmentSlot;
class Explosion;
class Item;
class ItemEntity;
class ItemInstance;
class Level;
class LightningBolt;
class ListTag;
class LivingEntity;
class Material;
class Mirror;
class MinecraftServer;
class Player;
class Random;
class Rotation;
class ServerLevel;
class ServerPlayer;
class SoundEvent;
class SynchedEntityData;
class Team;
class Vec3;

namespace phys
{

class Vec2;

}

template <class T> class EntityDataAccessor;
template <class T> class not_null_ptr;


class Entity;


int indexOf(const std::vector<boost::weak_ptr<Entity> >& entities, const boost::shared_ptr<Entity>& entity);

class Entity : public boost::enable_shared_from_this<Entity>, public CommandSender
{
public:
    enum ESharedFlag
    {
        FLAG_ONFIRE = 0,
        FLAG_SNEAKING = 1,
        FLAG_INVULNERABLE = 2,
        FLAG_SPRINTING = 3,
        FLAG_INVISIBLE = 5,
        FLAG_IDLE = 6,
        FLAG_WEAKENED = 7,
        FLAG_GLOWING = 8,
        FLAG_FALL_FLYING = 9,
        FLAG_SWIMMER = 10,
        FLAG_SWIMMING = 11,
        FLAG_DAMAGE_NEARBY_MOBS = 12
    };

    enum EEntityLocation
    {
        ENTITY_LOCATION_FEET,
        ENTITY_LOCATION_BODY,
        ENTITY_LOCATION_WEAPON_ATTACH_POINT,
        ENTITY_LOCATION_HEAD,
        ENTITY_LOCATION_DROP_ATTACH_POINT,
        ENTITY_LOCATION_EXPLOSION_POINT,
        ENTITY_LOCATION_EYES,
        ENTITY_LOCATION_BREATHING_POINT,
        ENTITY_LOCATION_MOUTH
    };

    Entity(Level* level, bool createEntityData);

    virtual unsigned long long GetType() = 0;
    virtual ~Entity();
    virtual void fjDerivedCtorCalls();
    virtual void kill();
    virtual void defineSynchedData() = 0;
    virtual void resetPos();
    virtual void remove();
    virtual void setDropContainerContent(bool dropContainerContent);
    virtual void setSize(float width, float height);
    virtual void setPos(double x, double y, double z);
    virtual void tick();
    virtual void baseTick();
    virtual void processDimensionDelay();
    virtual int getPortalWaitTime();
    virtual void setSecondsOnFire(int seconds);
    virtual void clearFire();
    virtual void outOfWorld();
    virtual void move(MoverType type, double x, double y, double z, bool pushedByPiston);
    virtual void updateDeltaAfterMove(Block* block, double x, double y, double z, double& movedX, double& movedY,
                                      double& movedZ);
    virtual void setLocationFromBoundingbox();
    virtual const SoundEvent* getSwimSound();
    virtual const SoundEvent* getSwimSplashSound();
    virtual const SoundEvent* getHeavySplashSound();
    virtual void checkInsideBlocks();
    virtual void onInsideBlock(const BlockState* state);
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual void playUnderwaterSound(const BlockPos& pos, Block* block);
    virtual float playFlySound(float moveDist);
    virtual bool makeFlySound();
    virtual void playSound(const SoundEvent* sound, float volume, float pitch);
    virtual bool isSilent();
    virtual void setSilent(bool silent);
    virtual bool isNoGravity();
    virtual void setNoGravity(bool noGravity);
    virtual bool isMoving();
    virtual void setMoving(bool moving);
    virtual bool makeStepSound();
    virtual void checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos);
    virtual AABB* getCollideBox();
    virtual void burn(int damage);
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual bool isInWater();
    virtual bool headInWater();
    virtual bool isOverWater();
    virtual bool updateInWaterState();
    virtual void doWaterSplashEffect();
    virtual void updateSprintingState();
    virtual void doSprintParticleEffect();
    virtual bool isUnderLiquid(Material* material);
    virtual bool isInLiquid(Material* material, Vec3* pos);
    virtual int getLightColor();
    virtual float getBrightness();
    virtual void setLevel(Level* level);
    virtual void moveTo(double x, double y, double z, float yRot, float xRot);
    virtual void playerTouch(boost::shared_ptr<Player> player);
    virtual void push(boost::shared_ptr<Entity> entity);
    virtual void push(double x, double y, double z);
    virtual void markHurt();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual Vec3* getViewVector(float partialTicks);
    virtual bool isPickable();
    virtual bool isPushable(boost::shared_ptr<Entity> entity);
    virtual void awardKillScore(boost::shared_ptr<Entity> killed, int score, const DamageSource* source);
    virtual bool shouldRender(double x, double y, double z);
    virtual bool shouldRender(double distanceSqr);
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual bool save(CompoundTag* tag);
    virtual void load(CompoundTag* tag);
    virtual bool shouldTick();
    virtual bool repositionEntityAfterLoad();
    virtual void readAdditionalSaveData(CompoundTag* tag) = 0;
    virtual void addAdditonalSaveData(CompoundTag* tag) = 0;
    virtual void readAdditionContructionTag(CompoundTag* tag) {}
    virtual CompoundTag* getAdditionContructionTag();
    virtual boost::shared_ptr<ItemEntity> spawnAtLocation(Item* item, int count);
    virtual boost::shared_ptr<ItemEntity> spawnAtLocation(Item* item, int count, float yOffset);
    virtual boost::shared_ptr<ItemEntity> spawnAtLocation(not_null_ptr<ItemInstance> itemInstance, float yOffset);
    virtual bool isAlive();
    virtual bool isInWall();
    virtual bool interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual AABB* getCollideAgainstBox(boost::shared_ptr<Entity> entity);
    virtual void rideTick();
    virtual void positionRider(const boost::shared_ptr<Entity>& passenger);
    virtual void onPassengerTurned(boost::shared_ptr<Entity> passenger);
    virtual double getRidingHeight();
    virtual double getRideHeight();
    virtual bool showVehicleHealth();
    virtual bool startRiding(boost::shared_ptr<Entity> vehicle);
    virtual bool startRiding(boost::shared_ptr<Entity> vehicle, bool force);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual unsigned int GetHitTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual bool HideUseTooltips();
    virtual bool canRide(boost::shared_ptr<Entity> vehicle);
    virtual void ejectPassengers();
    virtual void stopRiding();
    virtual void stopRidingBeforeTeleport();
    virtual void addPassenger(boost::shared_ptr<Entity> passenger);
    virtual void removePassenger(boost::shared_ptr<Entity> passenger);
    virtual bool canAddPassenger(boost::shared_ptr<Entity> passenger);
    virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport);
    virtual float getPickRadius();
    virtual Vec3* getLookAngle();
    virtual phys::Vec2* getRotationVector();
    virtual Vec3* getForward();
    virtual void handleInsidePortal(const BlockPos& pos);
    virtual int getDimensionChangingDelay();
    virtual void lerpMotion(double xd, double yd, double zd);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual void animateHurt();
    virtual NonNullList<not_null_ptr<ItemInstance> > getHandSlots();
    virtual NonNullList<not_null_ptr<ItemInstance> > getArmorSlots();
    virtual NonNullList<not_null_ptr<ItemInstance> > getAllSlots();
    virtual void setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> itemInstance);
    virtual bool isOnFire();
    virtual bool isPassenger();
    virtual bool isVehicle();
    virtual bool isSneaking();
    virtual void setSneaking(bool sneaking);
    virtual bool isIdle();
    virtual void setIsIdle(bool idle);
    virtual bool isSprinting();
    virtual void setSprinting(bool sprinting);
    virtual bool isSwimming();
    virtual bool isSwimmer();
    virtual void startSwimming();
    virtual void stopSwimming();
    virtual void setSwimmer(bool swimmer);
    virtual void setDamageNearbyMobs(bool damageNearbyMobs);
    virtual bool getDamageNearbyMobs();
    virtual bool isGlowing();
    virtual void setGlowing(bool glowing);
    virtual bool isInvisible();
    virtual bool isInvisibleTo(boost::shared_ptr<Player> player);
    virtual Team* getTeam();
    virtual bool isAlliedTo(boost::shared_ptr<Entity> entity);
    virtual bool isAlliedTo(Team* team);
    virtual void setInvisible(bool invisible);
    virtual bool ShouldRenderShadow();
    virtual bool canFly();
    virtual Vec3* getAttachPos(EEntityLocation location, float partialTicks);
    virtual int getMaxAirSupply();
    virtual void setAirSupply(int airSupply);
    virtual void thunderHit(const LightningBolt* lightningBolt);
    virtual void onAboveBubbleCol(bool down);
    virtual void killed(boost::shared_ptr<LivingEntity> killed);
    virtual bool checkInBlock(double x, double y, double z);
    virtual void spawnEatParticles(const ItemInstance& itemInstance, int count);
    virtual void makeStuckInWeb();
    virtual std::wstring getName();
    virtual bool isInvulnerableTo(const DamageSource* source);
    virtual bool isInvulnerable();
    virtual void setInvulnerable(bool invulnerable);
    virtual std::vector<boost::shared_ptr<Entity> >* getSubEntities();
    virtual bool is(boost::shared_ptr<Entity> entity);
    virtual float getYHeadRot();
    virtual void setYHeadRot(float yHeadRot);
    virtual void setYBodyRot(float yBodyRot);
    virtual bool isAttackable();
    virtual bool skipAttackInteraction(boost::shared_ptr<Entity> entity);
    virtual void copyPosition(boost::shared_ptr<Entity> entity);
    virtual boost::shared_ptr<Entity> changeDimension(int dimension);
    virtual bool canChangeDimensions();
    virtual float getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos);
    virtual bool shouldBlockExplode(Explosion* explosion, Level* level, const BlockPos& pos, float power);
    virtual int getMaxFallDistance();
    virtual BlockPos getPortalEntranceBlock();
    virtual Vec3* getPortalEntranceOffset();
    virtual const Direction* getPortalEntranceForwards();
    virtual bool isIgnoringBlockTriggers();
    virtual bool displayFireAnimation();
    virtual void setUUID(const std::wstring& uuid);
    virtual std::wstring getUUID();
    virtual std::wstring getStringUUID();
    virtual bool isPushedByWater();
    virtual double getViewScale();
    virtual void setViewScale(double viewScale);
    virtual std::wstring getDisplayName();
    virtual std::wstring getNetworkName();
    virtual void setCustomName(const std::wstring& customName, const std::wstring& ownerUUID,
                               bool namedByRestrictedPlayer);
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual void setCustomNameVisible(bool customNameVisible);
    virtual bool isCustomNameVisible();
    virtual void teleportTo(double x, double y, double z);
    virtual bool shouldShowName();
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual const Direction* getDirection();
    virtual const Direction* getMotionDirection();
    virtual bool broadcastToPlayer(boost::shared_ptr<ServerPlayer> player);
    virtual AABB* getBoundingBoxForCulling();
    virtual void setBoundingBox(AABB* boundingBox);
    AABB* getBoundingBox() { return mBoundingBox; }
    virtual float getEyeHeight();
    virtual bool setSlot(int slot, const not_null_ptr<ItemInstance>& itemInstance);
    virtual void sendMessage(ClientboundChatPacket::EChatPacketMessage message, int* intParams,
                             unsigned int numIntParams, std::wstring* stringParams, unsigned int numStringParams);
    virtual bool hasPermission(EGameCommand command);
    virtual BlockPos getCommandSenderBlockPosition();
    virtual Vec3* getCommandSenderWorldPosition();
    virtual Level* getCommandSenderWorld();
    virtual boost::shared_ptr<Entity> getCommandSenderEntity();
    virtual bool isSendMessageEnabled();
    virtual void setCommandStats(CommandStats::CommandStatType statType, int value);
    virtual MinecraftServer* getServer();
    virtual InteractionResult::EInteractionResult interactAt(const boost::shared_ptr<Player>& player, Vec3* pos,
                                                             InteractionHand::EInteractionHand hand);
    virtual bool ignoreExplosion();
    virtual void startSeenByPlayer(boost::shared_ptr<ServerPlayer> player);
    virtual void stopSeenByPlayer(boost::shared_ptr<ServerPlayer> player);
    virtual float rotate(Rotation* rotation);
    virtual float mirror(Mirror* mirror);
    virtual bool onlyOpCanSetNbt();
    virtual bool checkAndResetTeleportedFlag();
    virtual boost::shared_ptr<Entity> getControllingPassenger();
    virtual std::vector<boost::weak_ptr<Entity> >* getPassengers();
    virtual bool hasPassenger(boost::shared_ptr<Entity> passenger);
    virtual bool hasPassenger(unsigned long long type);
    virtual void fillIndirectPassengers(unsigned long long type, std::set<boost::shared_ptr<Entity> >* passengers);
    virtual std::set<boost::shared_ptr<Entity> >* getIndirectPassengers(unsigned long long type);
    virtual std::set<boost::shared_ptr<Entity> >* getIndirectPassengers();
    virtual boost::shared_ptr<Entity> getRootVehicle();
    virtual bool isPassengerOfSameVehicle(boost::shared_ptr<Entity> entity);
    virtual bool hasIndirectPassenger(boost::shared_ptr<Entity> passenger);
    virtual bool hasIndirectPassenger(unsigned long long type);
    virtual bool isControlledByLocalInstance();
    virtual boost::shared_ptr<Entity> getVehicle();
    virtual int getPistonPushReaction();
    virtual SoundSource::ESoundSource getSoundSource();
    virtual bool isCreative();
    virtual bool isDespawnProtected();
    virtual void setDespawnProtected();
    virtual bool couldWander();
    virtual bool canCreateParticles();
    virtual void stopCurrentLerp();
    virtual bool PositionLocked() { return false; }
    virtual void SetPositionLocked(bool positionLocked) {}
    virtual bool TakeGlideCollisionDamage() { return false; }
    virtual void SetGlideCollisionDamage(bool glideCollisionDamage) {}
    virtual double GetLiftForceModifier() { return 1.0; }
    virtual void SetLiftForceModifier(double liftForceModifier) {}
    virtual void onLoadedFromSave() {}
    virtual bool isUsingItemFlag() { return false; }
    virtual void setUsingItemFlag(bool usingItem) {}
    virtual void DoAutoAttackOnTouch(boost::shared_ptr<Entity> entity);
    virtual void startSpinAttack() { mAutoSpinAttackTicks = 0; }
    virtual void stopSpinAttack() {}
    virtual void tickLeash();
    virtual void dropLeash(bool broadcast, bool dropItem);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    virtual bool isLeashed();
    virtual boost::shared_ptr<Entity> getLeashHolder();
    virtual bool isLeashedToPlayer(const boost::shared_ptr<Player>& player);
    virtual void setLeashedTo(boost::shared_ptr<Entity> leashHolder, bool broadcast);
    virtual void restoreLeashFromSave();

    static void staticCtor();
    static bool instanceof(const boost::shared_ptr<Entity>& entity, unsigned long long type);
    bool instanceof(unsigned long long type) { return eTYPE_DERIVED_FROM(type, GetType()); }
    static int GetLightColor(Entity* entity);
    static void addDataWalkers(DataFixerUpper* dataFixerUpper);
    static void fjDebugCheckSmallIdLeaks();
    static void tickExtraWandering();
    static void useSmallIds();
    static void countFlagsForPIX();

    void fjCheckDerivedConstruction();
    void freeSmallId(int smallId);
    int getSmallId();
    void resetSmallId();
    void printSmallIds() const;
    void _init(bool createEntityData, Level* level);
    int getId();
    void setId(int id);
    boost::shared_ptr<SynchedEntityData> getEntityData();
    bool hasCategory(EntityCategory::E_EntityCategory category) const;
    void setRot(float yRot, float xRot);
    void setPos(EntityPos* pos);
    void turn(float yRotDelta, float xRotDelta);
    void interpolateTurn(float yRotDelta, float xRotDelta);
    void DamageNearbyMobs();
    bool isInWaterOrRain();
    bool isInWaterRainOrBubble();
    bool isInLava();
    void lavaHurt();
    bool isFree(AABB* box);
    bool isFree(double x, double y, double z);
    bool isFree(float x, float y, float z, float size);
    bool intersects(double x0, double y0, double z0, double x1, double y1, double z1);
    _SkinAdjustments getSkinAdjustments();
    void setSkinAdjustments(_SkinAdjustments skinAdjustments);
    void playSoundToAllPlayers(const SoundEvent* sound, float volume, float pitch);
    bool isFireImmune();
    bool isInWaterOrBubble();
    Vec3* getEyePosition(float partialTicks);
    void moveRelative(float x, float y, float z, float speed);
    void absMoveTo(double x, double y, double z, float yRot, float xRot);
    void absMoveTo(double x, double y, double z);
    void moveTo(const BlockPos& pos, float yRot, float xRot);
    float distanceTo(boost::shared_ptr<Entity> entity) const;
    double distanceTo(double x, double y, double z) const;
    double distanceToSqr(boost::shared_ptr<Entity> entity) const;
    double distanceToSqr(double x, double y, double z) const;
    double distanceToSqr(const BlockPos& pos) const;
    double distanceSqrToBlockPosCenter(const BlockPos& pos) const;
    double distanceAsIfHitscanned(const BlockPos& pos, const AABB& box) const;
    int getFireTransitionStartTick() const;
    Vec3* calculateViewVector(float xRot, float yRot);
    HitResult* pick(double distance, float partialTicks);
    HitResult* pick(Vec3* from, double distance, float partialTicks, bool liquids, bool ignoreEntities);
    HitResult* pickBlockInDirection(Vec3* direction, double distance, float partialTicks);
    std::wstring getEncodeId();
    ListTag* newDoubleList(unsigned int count, double first, ...);
    ListTag* newFloatList(unsigned int count, float first, float second);
    int getAirSupply();
    bool getSharedFlag(int flag);
    void setSharedFlag(int flag, bool value);
    CompoundTag* saveWithoutId(CompoundTag* tag);
    bool saveAsPassenger(CompoundTag* tag);
    void fjImmediateBroadcastPassengerPacket(const boost::shared_ptr<Entity>& passenger);
    float getSwimAmount(float partialTicks) const;
    float getCurrentSwimAmount() const;
    Vec3* getPos(float partialTicks);
    bool isWeakened();
    void setWeakened(bool weakened);
    void onInsideBubbleColumn(bool down);
    void considerForExtraWandering(bool consider);
    bool isExtraWanderingEnabled();
    int getWanderingQuadrant();
    void restoreFrom(boost::shared_ptr<Entity> entity);
    std::wstring getOwnerUUID();
    bool wasNamedByRestrictedPlayer();
    bool wasNamedOnHost();
    void setOutsideWorldBorder(bool outsideWorldBorder);
    void doEnchantDamageEffects(boost::shared_ptr<LivingEntity> attacker, boost::shared_ptr<Entity> target);
    bool controllingPassengerIsA(unsigned long long type);
    bool isImmersedInWater();
    std::set<std::wstring>* getTags();
    bool addTag(const std::wstring& tag);
    bool removeTag(const std::wstring& tag);
    void createHoverEvent();
    CommandStats* getCommandStats();
    void cloneStatsFrom(boost::shared_ptr<Entity> entity);
    void handleEntityTag(CompoundTag* tag);

    static const std::wstring RIDING_TAG;

    static const EntityDataAccessor<int>* DATA_SHARED_FLAGS_ID;
    static const EntityDataAccessor<int>* DATA_AIR_SUPPLY_ID;
    static const EntityDataAccessor<std::wstring>* DATA_CUSTOM_NAME;
    static const EntityDataAccessor<bool>* DATA_CUSTOM_NAME_VISIBLE;
    static const EntityDataAccessor<bool>* DATA_SILENT;
    static const EntityDataAccessor<bool>* DATA_NO_GRAVITY;
    static const EntityDataAccessor<bool>* DATA_MOVING;
    static const EntityDataAccessor<bool>* DATA_NAMED_BY_RESTRICTED_PLAYER;
    static const EntityDataAccessor<bool>* DATA_CREATED_ON_HOST;
    static const EntityDataAccessor<std::wstring>* DATA_OWNER_UUID;

#ifdef MINECRAFT_PC
    static const int SMALL_ID_COUNT = 16384;
#else
    static const int SMALL_ID_COUNT = 2048;
#endif
    static int sEntityCount;
    static double sViewScale;
    static unsigned int sSmallIdsInUse[SMALL_ID_COUNT / 32];
    static unsigned int sSmallIdsWandering[SMALL_ID_COUNT / 32];
    static unsigned int sSmallIdsToRemove[SMALL_ID_COUNT / 32];
    static int sWanderingCount;
    static int sWanderingIds[3];
    static int sWanderingTick;

    int mId;
    bool mBlocksBuilding;
    std::vector<boost::weak_ptr<Entity> > mPassengers;
    EntityTickCache mTickCache;
    std::vector<AABB> mCollisionCache;
    AABB* mWaterCheckBox;
    bool mFireCacheResult;
    int mFireCacheMinX;
    int mFireCacheMaxX;
    int mFireCacheMinY;
    int mFireCacheMaxY;
    int mFireCacheMinZ;
    int mFireCacheMaxZ;
    bool mLavaCacheResult;
    int mLavaCacheMinX;
    int mLavaCacheMaxX;
    int mLavaCacheMinY;
    int mLavaCacheMaxY;
    int mLavaCacheMinZ;
    int mLavaCacheMaxZ;
    bool mInWallCacheResult;
    double mInWallCacheX;
    double mInWallCacheY;
    double mInWallCacheZ;
    float mInWallCacheYRot;
    float mInWallCacheXRot;
    int mBoardingCooldown;
    boost::shared_ptr<Entity> mVehicle;
    bool mForcedLoading;
    Level* mLevel;
    double mXo;
    double mYo;
    double mZo;
    double mX;
    double mY;
    double mZ;
    double mXd;
    double mYd;
    double mZd;
    float mYRot;
    float mXRot;
    float mYRotO;
    float mXRotO;
    AABB* mBoundingBox;
    bool mOnGround;
    bool mHorizontalCollision;
    bool mVerticalCollision;
    bool mCollision;
    bool mHurtMarked;
    int mRemainingFireTicks;
    float mEyeHeightO;
    float mEyeHeight;
    float mSwimAmount;
    float mSwimAmountO;
    bool mReserved0x178;
    bool mWasOnFire;
    int mFireTransitionStartTick;
    double mVehicleXRotDelta;
    double mVehicleYRotDelta;
    int mCategories;
    bool mIsInWeb;
    bool mOutsideWorldBorder;
    bool mBlocksSpawning;
    bool mRemoved;
    float mBbWidth;
    float mBbHeight;
    float mWalkDistO;
    float mWalkDist;
    float mMoveDist;
    float mFallDistance;
    unsigned short mFallTicks;
    unsigned short mFallCount;
    int mNextStep;
    float mNextFlap;
    double mXOld;
    double mYOld;
    double mZOld;
    float mMaxUpStep;
    bool mNoPhysics;
    float mPushthrough;
    Random* mRandom;
    int mTickCount;
    bool mIsInWater;
    bool mHeadInWater;
    int mInvulnerableTime;
    bool mFirstTick;
    bool mFireImmune;
    boost::shared_ptr<SynchedEntityData> mEntityData;
    bool mInChunk;
    int mChunkX;
    int mChunkY;
    int mChunkZ;
    long long mXp;
    long long mYp;
    long long mZp;
    int mXRotp;
    int mYRotp;
    bool mNoCulling;
    bool mHasImpulse;
    int mPortalCooldown;
    bool mIsInsidePortal;
    int mPortalTime;
    int mDimension;
    BlockPos mPortalEntrancePos;
    Vec3* mPortalEntranceOffset;
    const Direction* mPortalEntranceForwards;
    std::wstring mUUID;
    std::wstring mStringUUID;
    CommandStats* mCommandStats;
    NonNullList<not_null_ptr<ItemInstance> > mAllSlots;
    bool mGlowing;
    bool mIgnoreVerticalCollision;
    bool mIgnorePortals;
    std::set<std::wstring> mTags;
    bool mTeleported;
    double* mPistonDeltas;
    int mPistonDeltaCount;
    long long mPistonDeltasGameTime;
    bool mLeashed;
    boost::weak_ptr<Entity> mLeashHolder;
    CompoundTag* mLeashInfoTag;
    unsigned long long mType;
    boost::weak_ptr<Entity> mShouldTickNearbyCache;
    int mShouldTickCounter;
    _SkinAdjustments mSkinAdjustments;
    int mAutoSpinAttackTicks;
};

inline bool Entity::instanceof(const boost::shared_ptr<Entity>& entity, unsigned long long type)
{
    return entity != NULL && entity->instanceof(type);
}

#endif
