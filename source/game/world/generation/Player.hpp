#ifndef GAME_WORLD_GENERATION_PLAYER_HPP
#define GAME_WORLD_GENERATION_PLAYER_HPP

#include <stdint.h>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/core/StatsUID.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/world/block/Predicate.hpp"
#include "vendor/_SkinAdjustments.hpp"
#include "game/core/Abilities.hpp"
#include "game/core/FoodData.hpp"

class TextureAtlasSprite;
class ItemInstance;
class AbstractContainerMenu;
class AbstractHorse;
class BaseCommandBlock;
class CommandBlockEntity;
class Container;
class FishingHook;
class ModelPart;
class SoundEvent;
class FoodData;
class GameMode;
class Inventory;
class PlayerEnderChestContainer;
class SignBlockEntity;
class ItemEntity;
class Merchant;
class MultiPlayerGameMode;


class Achievement;
class Block;
class BlockPos;
class BlockState;
class CameraController;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Direction;
class EquipmentSlot;
class GameType;
class HumanoidArm;
class InteractionObject;
class Level;
class LockCode;
class PlayerModelPart;
struct PlayerUID;
class Stat;
class Scoreboard;
struct arr_unsigned_char;
struct not_null_ptr_ItemInstance;
struct sp_Container;
struct sp_Entity;
struct sp_LivingEntity;
struct sp_Player;


enum EDefaultSkins
{
    eDefaultSkins_ServerSelected = 0,
    eDefaultSkins_Skin0 = 1
};

class Player : public LivingEntity
{
public:
    friend class MultiPlayerGameMode;
    friend class ServerGamePacketListenerImpl;

    enum EPlayerGamePrivileges
    {
        ePlayerGamePrivilege_CannotMine = 0,
        ePlayerGamePrivilege_CannotBuild = 1,
        ePlayerGamePrivilege_CannotAttackPlayers = 3,
        ePlayerGamePrivilege_Op = 4,
        ePlayerGamePrivilege_CanFly = 5,
        ePlayerGamePrivilege_ExhaustionDisabled = 6,
        ePlayerGamePrivilege_Invisible = 7,
        ePlayerGamePrivilege_Invulnerable = 8,
        ePlayerGamePrivilege_CreativeMode = 9,
        ePlayerGamePrivilege_CannotAttackAnimals = 10,
        ePlayerGamePrivilege_CanUseDoorsAndSwitches = 11,
        ePlayerGamePrivilege_CanUseContainers = 12,
        ePlayerGamePrivilege_CanToggleInvisible = 13,
        ePlayerGamePrivilege_CanToggleFly = 14,
        ePlayerGamePrivilege_CanToggleExhaustion = 15,
        ePlayerGamePrivilege_CanTeleport = 16,
        ePlayerGamePrivilege_All = 0x21,
        ePlayerGamePrivilege_Moderator = 0x22
    };

    static const EntityDataAccessor<float>* DATA_PLAYER_ABSORPTION_ID;
    static const EntityDataAccessor<int>* DATA_SCORE_ID;
    static const EntityDataAccessor<unsigned char>* DATA_PLAYER_MODE_CUSTOMISATION;
    static const EntityDataAccessor<unsigned char>* DATA_MAIN_HAND;
    static const EntityDataAccessor<CompoundTag*>* DATA_SHOULDER_LEFT;
    static const EntityDataAccessor<CompoundTag*>* DATA_SHOULDER_RIGHT;
    static std::vector<unsigned int> sDlcSkinIds;


    class ChatVisiblity
    {
    public:
        ChatVisiblity(int id);
        int getId();
        static ChatVisiblity byId(int id);
        std::wstring getKey();

    private:
        int mId;
    };


    enum TimerAchievements
    {
        eTimerAchievement_Castaway = 0,
        eTimerAchievement_SleepWithTheFishes = 1
    };

    enum BedSleepingProblem
    {
        BED_SLEEPING_OK = 0,
        BED_SLEEPING_NOT_POSSIBLE_HERE = 1,
        BED_SLEEPING_NOT_POSSIBLE_NOW = 2,
        BED_SLEEPING_TOO_FAR_AWAY = 3,
        BED_SLEEPING_OTHER_PROBLEM = 4,
        BED_SLEEPING_NOT_SAFE = 5
    };

    class PreventsMeFromSleeping : public Predicate<boost::shared_ptr<Entity> >
    {
    public:
        PreventsMeFromSleeping(boost::shared_ptr<Player> player);
        bool apply(boost::shared_ptr<Entity> entity) const;

    private:
        boost::shared_ptr<Player> mPlayer;
    };


    boost::shared_ptr<Inventory> mInventory;
    boost::shared_ptr<PlayerEnderChestContainer> mEnderChestInventory;
    AbstractContainerMenu* mInventoryMenu;
    AbstractContainerMenu* mContainerMenu;
    FoodData mFoodData;
    int32_t mJumpTriggerTime;
    float mPrevCameraYaw;
    float mCameraYaw;
    int32_t mTakeXpDelay;
    std::wstring mSkinLocation;
    std::wstring mCapeLocation;
    int32_t mField0x66c;
    double mPrevChasingPosX;
    double mPrevChasingPosY;
    double mPrevChasingPosZ;
    double mChasingPosX;
    double mChasingPosY;
    double mChasingPosZ;
    std::wstring mDisplayName;
    bool mSleeping;
    BlockPos* mBedPosition;
    int32_t mSleepCounter;
    int32_t mDeathFadeTimer;
    float mBedOffsetX;
    float mBedOffsetY;
    float mBedOffsetZ;
    int32_t mField0x6dc;
    BlockPos* mRespawnPosition;
    uint8_t mRespawnForced;
    BlockPos* mStartRidingPos;
    int32_t mWalkDistanceCm;
    int32_t mSwimDistanceCm;
    int32_t mDiveDistanceCm;
    int32_t mFallDistanceCm;
    int32_t mClimbDistanceCm;
    int32_t mMinecartDistanceCm;
    int32_t mBoatDistanceCm;
    int32_t mPigDistanceCm;
    Abilities mAbilities;
    int32_t mExperienceLevel;
    int32_t mTotalExperience;
    float mExperienceProgress;
    boost::shared_ptr<FishingHook> mFishing;
    int32_t mEnchantmentSeed;
    float mDefaultFlySpeed;
    ItemCooldowns* mField0x738;
    int32_t mLastLevelUpTime;
    std::wstring mName;
    uint8_t mReducedDebugInfo;
    not_null_ptr<ItemInstance> mField0x764;
    NonNullList<not_null_ptr<ItemInstance> > mField0x76c;
    CameraController* mCameraController;
    bool mOwnsCameraController;
    uint8_t mField0x781;
    bool mWasFallFlying;
    PlayerUID mXuid;
    PlayerUID mOnlineXuid;
    StatsUID mSuid;
    int32_t mWinnerParticleTicks;
    bool mShowOnMaps;
    bool mIsGuest;
    NonNullList<not_null_ptr<ItemInstance> > mField0x7c8;
    uint32_t mField0x7d8;
    int32_t mCustomSkin;
    int32_t mCustomCape;
    int32_t mPlayerIndex;
#ifdef MINECRAFT_PC
    bool mHiddenHost = false;
#endif
    unsigned int mDebugOptions;
    bool mPlayedFallingSound;
    int32_t mLastPoints;
    int32_t mLastCheckpointId;
    uint8_t mEndZoneEffectShown;
    uint32_t mGamePrivileges;
    std::vector<ModelPart*>* mAdditionalModelParts;
    bool mAdditionalModelPartsChecked;
    bool mDLCSkinChecked;
    int32_t mPendingCameraId;
    boost::shared_ptr<Entity> mCamera;
    bool mPositionLocked;
    bool mField0x815;
    double mField0x818;
    double mLastLayerY;
    int32_t mField0x828;
    uint8_t mSpectatorSkullEquipped;
    int32_t mPowerupTicks[3];
    uint8_t mRingSoundAlternate;
    float mFlap;
    float mFlapSpeed;
    float mOFlapSpeed;
    float mOFlap;
    float mField0x850;
    float mField0x854;
    float mField0x858;
    bool mLastAteKelp;
    bool mTimerAchievementAwarded[2];
    int32_t mTimerAchievementTicks[2];


    Player(Level* level, const std::wstring& name);
    ~Player();


    std::wstring getDisplayName();
    bool isSendMessageEnabled();
    void SetPowerupTicks(PowerupItems::eGlide_Timed_Powerup_ID powerup, int32_t ticks);
    bool isStarving();
    void tickArmor();
    static void staticCtor();
    void _init();
    void registerAttributes();
    void defineSynchedData();
    bool shouldRenderAtSqrDistance(double distance);
    bool shouldTick();
    bool checkBedExists();
    virtual void updateFrameTick();
    StatsUID getSUID();
    void AwardTimerAchievement(TimerAchievements achievement);
    void CheckTimerAchievements();
    static unsigned int getPlayerGamePrivilege(unsigned int privileges, EPlayerGamePrivileges privilege);
    unsigned int getPlayerGamePrivilege(EPlayerGamePrivileges privilege);
    uint32_t hasInvulnerablePrivilege();
    void tickCamera();
    void moveCloak();
    void resetAttackStrengthTicker();
    void updatePlayerSize();
    void PlayerRingSoundEffect(int32_t ring);
    void tick();
    int getPortalWaitTime();
    int GetSuffocateTime();
    float GetInhaleTime();
    bool generatesBubbles();
    const SoundEvent* getSwimSound();
    const SoundEvent* getSwimSplashSound();
    const SoundEvent* getHeavySplashSound();
    int getDimensionChangingDelay();
    void playSound(const SoundEvent* sound, float volume, float pitch);
    SoundSource::ESoundSource getSoundSource();
    void handleEntityEvent(uint8_t event, int32_t data);
    bool isImmobile();
    void onChangedBlock(const BlockPos& position);
    virtual void closeContainer();
    void prepareCustomTextures();
    void checkRidingStatistiscs(double x, double y, double z);
    void rideTick();
    void SetNormalSize();
    void resetPos();
    void serverAiStep();
    void playShoulderEntityAmbientSound(CompoundTag* tag);
    CompoundTag* getShoulderEntityLeft();
    CompoundTag* getShoulderEntityRight();
    void respawnEntityOnShoulder(CompoundTag* tag);
    void setShoulderEntityLeft(CompoundTag* tag);
    void setShoulderEntityRight(CompoundTag* tag);
    void removeEntitiesOnShoulder();
    void calculateFlapping();
    void aiStep();
    virtual void touch(boost::shared_ptr<Entity> entity);
    virtual uint32_t getScore();
    virtual void setScore(int32_t score);
    virtual void increaseScore(int32_t amount);
    boost::shared_ptr<ItemEntity> drop(not_null_ptr<ItemInstance> item, bool randomThrow, bool retainOwnership);
    void destroyVanishingCursedItems();
    void die(const DamageSource* source);
    const SoundEvent* getHurtSound(const DamageSource* source);
    const SoundEvent* getDeathSound();
    boost::shared_ptr<ItemEntity> drop(not_null_ptr<ItemInstance> item, bool retainOwnership);
    virtual boost::shared_ptr<ItemEntity> drop(bool dropAll);
    virtual not_null_ptr<ItemInstance> reallyDrop(boost::shared_ptr<ItemEntity> item);
    float getDestroySpeed(const BlockState* state);
    bool canDestroy(const BlockState* state);
    static void addWalkers(DataFixerUpper* fixer);
    void readAdditionalSaveData(CompoundTag* tag);
    void addAdditonalSaveData(CompoundTag* tag);
    virtual int GetThirdPersonView(bool firstPerson);
    virtual void SetThirdPersonView(int32_t view);
    uint32_t isAllowedToFly();
    bool hurt(const DamageSource* source, float damage);
    void blockUsingShield(const boost::shared_ptr<LivingEntity>& attacker);
    virtual uint32_t CanChangeThirdPersonView();
    uint32_t hasInvisiblePrivilege();
    uint32_t isAllowedToAttackPlayers();
    virtual CameraController* GetCameraController();
    void hurtArmor(float damage);
    virtual void SetCameraController(CameraController* controller, bool owned);
    void hurtCurrentlyUsedShield(float damage);
    uint32_t isAllowedToIgnoreExhaustion();
    void causeFoodExhaustion(float exhaustion);
    void actuallyHurt(const DamageSource* source, float damage);
    virtual uint32_t canHarmPlayer(boost::shared_ptr<Player> player);
    virtual bool canHarmPlayer(std::wstring playerName);
    virtual float getArmorCoverPercentage();
    virtual void openTextEdit(boost::shared_ptr<SignBlockEntity> sign);
    virtual void openMinecartCommandBlock(boost::shared_ptr<BaseCommandBlock> commandBlock);
    virtual void openCommandBlock(boost::shared_ptr<CommandBlockEntity> block);
    virtual int openTrading(boost::shared_ptr<Merchant> merchant, const std::wstring& name, int entityId);
    virtual int openContainer(boost::shared_ptr<Container> container);
    virtual int openHorseInventory(boost::shared_ptr<AbstractHorse> horse, boost::shared_ptr<Container> container);
    double getRidingHeight();
    void stopRiding();
    virtual int startBlockInteraction(InteractionObject& interaction);
    virtual void openItemInstanceGui(not_null_ptr<ItemInstance> item, InteractionHand::EInteractionHand hand);
    virtual uint32_t interactOn(boost::shared_ptr<Entity> entity, InteractionHand::EInteractionHand hand);
    virtual void attack(boost::shared_ptr<Entity> entity);
    virtual void disableShield(bool force);
    void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    void remove();
    bool isInWall();
    virtual void crit(boost::shared_ptr<Entity> entity);
    virtual void magicCrit(boost::shared_ptr<Entity> entity);
    bool bedInRange(const BlockPos& position, const Direction* direction);
    void setBedOffset(const Direction* direction);
    virtual void respawn();
    virtual bool isLocalPlayer();
    static BlockPos* checkBedValidRespawnPosition(Level* level, const BlockPos& position, bool forced);
    float getSleepRotation();
    bool isSleeping();
    bool isSleepingLongEnough();
    int getSleepTimer();
    int getDeathFadeTimer();
    virtual std::wstring getGameProfile();
    virtual uint32_t startSleepInBed(const BlockPos& position, bool force);
    virtual void stopSleepInBed(bool resetTimer, bool updateLevel, bool setSpawn);
    virtual void displayClientMessage(int32_t message, bool force);
    virtual BlockPos* getRespawnPosition();
    virtual uint8_t isRespawnForced();
    virtual void setRespawnPosition(BlockPos* position, bool forced);
    void jumpFromGround();
    void fjCheckSuperSonic(double oldX, double oldY, double oldZ, double newX, double newY, double newZ);
    void travel(float x, float y, float z);
    float getSpeed();
    void ResetTimerAchievement(TimerAchievements achievement);
    virtual uint32_t hasAchievement(Achievement* achievement);
    void causeFallDamage(float distance, float multiplier);
    void doWaterSplashEffect();
    const SoundEvent* getFallDamageSound(int32_t damage);
    void killed(boost::shared_ptr<LivingEntity> entity);
    void makeStuckInWeb();
    virtual TextureAtlasSprite* getItemInHandIcon(not_null_ptr<ItemInstance> item, int frame);
    int32_t getXpNeededForNextLevel();
    virtual void awardStat(Stat* stat, arrayWithLength<unsigned char> params);
    virtual void resetStat(Stat* stat);
    virtual void checkMovementStatistiscs(double x, double y, double z);
    virtual void increaseXp(int32_t amount);
    FoodData* getFoodData();
    bool canEat(bool alwaysEdible);
    bool isHurt();
    virtual uint32_t getEnchantmentSeed();
    virtual void onEnchantmentPerformed(int32_t cost);
    int32_t getExperienceReward(boost::shared_ptr<Player> killer);
    bool isAlwaysExperienceDropper();
    bool shouldShowName();
    bool makeStepSound();
    virtual void giveExperienceLevels(int32_t levels);
    virtual bool mayBuild();
    std::wstring getName();
    std::wstring getNetworkName();
    boost::shared_ptr<PlayerEnderChestContainer> getEnderChestInventory();
    not_null_ptr<ItemInstance> getItemSlot(const EquipmentSlot* slot);
    void setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item);
    uint32_t addItem(const not_null_ptr<ItemInstance>& item);
    NonNullList<not_null_ptr<ItemInstance> > getHandSlots();
    NonNullList<not_null_ptr<ItemInstance> > getArmorSlots();
    uint32_t setEntityOnShoulder(CompoundTag* tag);
    bool isInvisibleTo(boost::shared_ptr<Player> player);
    bool isPushedByWater();
    virtual bool mayUseItemAt(const BlockPos& position, const Direction* direction, not_null_ptr<ItemInstance> item);
    Team* getTeam();
    uint8_t SpectateAsOtherEntity();
    bool SpectateAsFloatingHead();
    float getEyeHeight();
    void setAbsorptionAmount(float amount);
    float getAbsorptionAmount();
    bool canUnlock(LockCode* lock);
    bool isModelPartShown(const PlayerModelPart* part);
    bool setSlot(int32_t slot, const not_null_ptr<ItemInstance>& item);
    uint8_t isReducedDebugInfo();
    const HumanoidArm* getMainArm();
    void setMainArm(const HumanoidArm* arm);
    double getCurrentItemAttackStrengthDelay();
    float getAttackStrengthScale(float partialTicks);
    ItemCooldowns* getCooldowns();
    void push(boost::shared_ptr<Entity> entity);
    void pushEntities();
    float getLuck();
    uint8_t canUseCommandBlocks();
    static uint32_t GetModelTypeFromAnimBitmask(uint32_t value);
    static uint32_t GetModelTypeFromTextureId(int32_t value);
    uint32_t getModelName();
    static uint32_t hash_fnct(boost::shared_ptr<Player> player);
    static bool eq_test(boost::shared_ptr<Player> first, boost::shared_ptr<Player> second);
    virtual void onUpdateAbilities();
    void setPlayerDefaultSkin(EDefaultSkins skin);
    static _SkinAdjustments getSkinAdjustmentsById(uint32_t id);
    void SetAdditionalModelParts(std::vector<ModelPart*>* parts);
    virtual void setGameMode(const GameType* gameType);
    virtual bool isSpectator() = 0;
    static std::wstring getCapePathFromId(uint32_t id);
    virtual Scoreboard* GetScoreboard();
    static uint32_t getCapeIdFromPath(const std::wstring& path);
    void setXuid(PlayerUID xuid);
    PlayerUID getXuid() { return mXuid; }
    static void setPlayerGamePrivilege(uint32_t& privileges, EPlayerGamePrivileges privilege, uint32_t enabled);
    void setPlayerGamePrivilege(EPlayerGamePrivileges privilege, uint32_t enabled);
    uint32_t isAllowedToUse(Block* block);
    uint32_t isAllowedToUse(not_null_ptr<ItemInstance> item);
    bool isAllowedToInteract(boost::shared_ptr<Entity> entity);
    uint32_t isAllowedToMine();
    bool isAllowedToAttackAnimals();
    uint32_t isAllowedToHurtEntity(boost::shared_ptr<Entity> entity);
    bool isModerator();
    bool IsCreativeFlying();
    static void enableAllPlayerPrivileges(uint32_t& privileges, bool enabled);
    void enableAllPlayerPrivileges(bool enabled);
    bool canCreateParticles();
    std::vector<ModelPart*>* GetAdditionalModelParts();
    void UpdateSize();
    void SetSmallSize();
    void SetNoCollisionSize();
    void setName(const std::wstring& name);
    virtual void onCrafted(not_null_ptr<ItemInstance> item);
    boost::shared_ptr<Entity> GetCamera();
    void Follow(bool follow);
    uint8_t SpectateAsSkull();
    bool SpectateAsInvisible();
    uint64_t GetSpectateEntity();
    Vec3* getAttachPos(Entity::EEntityLocation location, float partialTicks);
    virtual uint8_t getTexture();
    virtual void setCustomSkin(uint32_t skin);
    uint32_t CheckPowerup(PowerupItems::eGlide_Timed_Powerup_ID powerup);
    uint8_t CanRecordStats(Stat* stat, arrayWithLength<unsigned char> parameters);
    double getUnderwaterLightLevel() const;
    void setUnderwaterLightLevel(float level);
    double getUnderwaterVisionScale() const;
    void setUnderwaterVisionScale(float scale);
    void startSpinAttack();
    void stopSpinAttack();
    void AdvanceTimerAchievement(TimerAchievements achievement, int32_t ticks);
    void SetLastAteKelp(bool ate);
    void sendMessage(ClientboundChatPacket::EChatPacketMessage message, int32_t* intParams, uint32_t numIntParams,
                     std::wstring* stringParams, uint32_t numStringParams);
    uint64_t GetType();
    virtual uint64_t setCustomCape(uint32_t cape);
    virtual void handleCollectItem(not_null_ptr<ItemInstance> item);
    virtual void SetCamera(boost::shared_ptr<Entity> entity);
    virtual void SpectatePlayerNext();
    void setInvisible(bool invisible);
    virtual void SpectatePlayerPrev();
    virtual void StopSpectatingPlayer();
    virtual uint8_t IsSpectatingOtherPlayer();
    virtual const GameType* GetGameType() = 0;
    virtual GameMode* GetGameMode();
    bool ShouldRenderShadow();
    bool PositionLocked();
    void SetPositionLocked(bool locked);
    bool TakeGlideCollisionDamage();
    void SetGlideCollisionDamage(bool enabled);
    double GetLiftForceModifier();
    void SetLiftForceModifier(double modifier);
    virtual int32_t AutoEquip(not_null_ptr<ItemInstance>& item, not_null_ptr<ItemInstance>& replaced,
                              bool& replacedExisting);
    bool isPickable() { return !isSpectator(); }
    virtual void OnEquipArmor(not_null_ptr<ItemInstance> item);
    virtual void OnTakeFromAnvil(not_null_ptr<ItemInstance> item);
    void ChangePlayerSkin();
    void GetQuickCraftItem(int playerIndex, int slotIndex);
    void sweepAttack();
    static PlayerUID createPlayerUUID(const std::wstring& name);
    void setReducedDebugInfo(bool reducedDebugInfo);
    bool isAllowedToTeleport();
    double GetCollisionSize();
    void AdvanceAllTimerAchievements(int amount);
};


#endif
