
#ifndef GAME_SERVER_SERVERPLAYER_HPP
#define GAME_SERVER_SERVERPLAYER_HPP

#include "game/world/generation/Player.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include <stdint.h>
#include <cstddef>
#include <list>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/item/not_null_ptr.hpp"

class AbstractContainerMenu;
class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class GameType;
class InteractionObject;
class Level;
class Merchant;
class MinecraftServer;
class MobEffectInstance;
class ServerGamePacketListenerImpl;
class ServerLevel;
class ServerPlayerGameMode;
class Stat;
class Biome;
class BlockEntity;
class Container;

class AbstractContainerMenu;
class Block;
class BlockPos;
class ChunkPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class GameType;
class InteractionObject;
class Level;
class MinecraftServer;
class MobEffectInstance;
class ServerGamePacketListenerImpl;
class ServerLevel;
class ServerPlayerGameMode;
class Stat;
class ServerPlayer : public Player, public net_minecraft_world_inventory::ContainerListener
{
public:
    boost::shared_ptr<ServerGamePacketListenerImpl> mConnection;
    MinecraftServer* mServer;
    ServerPlayerGameMode* mGameMode;
    uint8_t mPadding1[4];
    double mLastSentX;
    double mLastSentZ;
    std::list<ChunkPos> mLoadedChunks;
    std::vector<int32_t> mEntitiesToRemove;
    boost::unordered_set<ChunkPos, ChunkPosKeyHash, ChunkPosKeyEq> mTrackedChunks;
    int32_t mState;
    Biome* mLastBiome;
    int64_t mChunkRequestTime;
    double* mLevitationSourceY;
    float mLastRecordedHealth;
    int32_t mLastRecordedFood;
    int32_t mLastRecordedAir;
    int32_t mLastRecordedArmor;
    int32_t mLastRecordedExperienceLevel;
    int32_t mLastRecordedTotalExperience;
    float mLastSentHealth;
    int32_t mLastSentFood;
    bool mLastSentSaturationWasZero;
    int32_t mLastSentTotalExperience;
    int32_t mSpawnInvulnerableTime;
    int64_t mLastActionTime;
    int32_t mLastChunkSendingTick;
    boost::shared_ptr<void> mSharedState;
    uint8_t mChangingDimension;
    BlockPos* mFallStartPosition;
    bool mDisconnected;
    int32_t mContainerCounter;
    bool mIsChangingQuantityOnly;
    int32_t latency;
    bool mRespawnState;
    bool mWonGame;
    uint8_t mChangedDimension;
    int32_t mPvpState;
    int32_t mCauseOfDeathId;
    int32_t mPlayerWeaponId;
    int32_t mChunkRadius;

    ServerPlayer(MinecraftServer* server, ServerLevel* level, const std::wstring& name, ServerPlayerGameMode* gameMode);
    ~ServerPlayer();

    void sendMessage(ClientboundChatPacket::EChatPacketMessage message, int32_t* intParams, uint32_t numIntParams,
                     std::wstring* stringParams, uint32_t numStringParams);
    bool hasPermission(EGameCommand command);
    BlockPos getCommandSenderBlockPosition();
    virtual int openFireworks(int32_t x, int32_t y, int32_t z);
    virtual void slotChanged(AbstractContainerMenu* menu, int32_t slot, not_null_ptr<ItemInstance> item);
    virtual void refreshContainer(AbstractContainerMenu* menu, std::vector<not_null_ptr<ItemInstance> >* items);
    virtual void setContainerData(AbstractContainerMenu* menu, int32_t dataIndex, int32_t value);
    virtual void setAllContainerData(AbstractContainerMenu* menu, boost::shared_ptr<Container> container);
    void flagEntitiesToBeRemoved(uint32_t* removedFlags, bool* flagsInitialised);
    static int getFlagIndexForChunk(const ChunkPos& chunkPos, int dimension);
    void disconnect();
    ServerLevel* getLevel();
    void doTickA();
    void broadcast(boost::shared_ptr<BlockEntity> blockEntity, bool delay);
    void doChunkSendingTick(bool dontDelay, bool historyOnly);
#ifdef MINECRAFT_PC
    int ChunkSendsPerTick();
    void DoChunkSendingTicks();
#endif
    void doTickB();
    void doTick(bool tickFrame, bool sendChunks, bool ignorePortals);
    void initMenu();
    boost::shared_ptr<ServerPlayer> SpectatePlayerChange(bool previous);
    void updateClientsCameras();
    void notifyPlayerRespawn(boost::shared_ptr<ServerPlayer> oldPlayer, boost::shared_ptr<ServerPlayer> newPlayer);
    void flushEntitiesToRemove();
    void markAttributesDirty();
    void refreshContainer(AbstractContainerMenu* menu);
    void resetSentInfo();
    static int32_t getFlagIndexForChunk(int32_t x, int32_t z, int32_t dimension);
    void resetChunkFlagsOutsideArea(int32_t minX, int32_t minZ, int32_t maxX, int32_t maxZ, int32_t dimension);
    void readAdditionalSaveData(CompoundTag* tag);
    static void addWalkers(DataFixerUpper* dataFixer);
    void addAdditonalSaveData(CompoundTag* tag);
    void giveExperienceLevels(int32_t levels);
    void onEnchantmentPerformed(int32_t cost);
    void onEnterCombat();
    void onLeaveCombat();
    void ResetSpawnInvulnerableTime();
    void tick();
    void die(const DamageSource* source);
    bool hurt(const DamageSource* source, float damage);
    bool isPvpAllowed();
    uint32_t canHarmPlayer(boost::shared_ptr<Player> wstring);
    bool canHarmPlayer(std::wstring wstring);
    virtual void addEffect(MobEffectInstance* effectInstance, const boost::shared_ptr<Entity>& source);
    virtual boost::shared_ptr<Entity> changeDimension(int dimension);
    bool broadcastToPlayer(boost::shared_ptr<ServerPlayer> player);
    void take(boost::shared_ptr<Entity> entity, int32_t count);
    uint32_t startSleepInBed(const BlockPos& position, bool force);
    void stopSleepInBed(bool resetTimer, bool updateLevel, bool setSpawn);
    virtual bool startRiding(boost::shared_ptr<Entity> vehicle);
    void stopRiding();
    void stopRidingBeforeTeleport();
    uint8_t isChangingDimension();
    bool hasChangedDimensionState() const { return mChangedDimension != 0; }
    virtual bool isInvulnerableTo(const DamageSource* source);
    void checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos);
    void doCheckFallDamage(double yMovement, bool onGround);
    void nextContainerCounter();
    void openTextEdit(boost::shared_ptr<SignBlockEntity> sign);
    virtual int startBlockInteraction(InteractionObject& interaction);
    virtual int openContainer(boost::shared_ptr<Container> container);
    virtual int openTrading(boost::shared_ptr<Merchant> merchant, const std::wstring& name, int entityId);
    virtual int openHorseInventory(boost::shared_ptr<AbstractHorse> horse, boost::shared_ptr<Container> container);
    virtual void openItemInstanceGui(not_null_ptr<ItemInstance> item, InteractionHand::EInteractionHand hand);
    void doCloseContainer();
    virtual void closeContainer();
    void broadcastCarriedItem();
    void setPlayerInput(float xxa, float zza, bool jumping, bool sneaking);
    void awardStat(Stat* stat, arrayWithLength<unsigned char> parameters);
    void resetStat(Stat* stat);
    uint8_t hasDisconnected();
    void displayClientMessage(int32_t message, bool force);
    void completeUsingItem();
    virtual void restoreFrom(boost::shared_ptr<Player> oldPlayer, bool keepEverything);
    void onEffectAdded(MobEffectInstance* effect);
    void onEffectUpdated(MobEffectInstance* effect, bool attributesChanged);
    void onEffectRemoved(MobEffectInstance* effect);
    void teleportTo(double x, double y, double z);
    virtual void teleportTo(double x, double y, double z, float yRot, float xRot);
    void crit(boost::shared_ptr<Entity> entity);
    void magicCrit(boost::shared_ptr<Entity> entity);
    void onUpdateAbilities();
    void setGameMode(const GameType* gameType);
    bool isSpectator();
    bool isCreative();
    void resetLastActionTime();
    void sendRemoveEntity(boost::shared_ptr<Entity> entity);
    void cancelRemoveEntity(boost::shared_ptr<Entity> entity);
    void updateInvisibilityStatus();
    void processDimensionDelay();
    void attack(boost::shared_ptr<Entity> entity);
    virtual bool swing(InteractionHand::EInteractionHand hand);
    void hasChangedDimension();
    void startFallFlying();
    void stopFallFlying();
    uint32_t getPlayerViewDistanceModifier();
    void handleCollectItem(not_null_ptr<ItemInstance> item);
    void SetCamera(boost::shared_ptr<Entity> entity);
    const GameType* GetGameType();
    GameMode* GetGameMode();
    void OnEquipArmor(not_null_ptr<ItemInstance> item);
    bool checkTotemDeathProtection(const DamageSource* source);
    uint64_t GetType();
    void getStats();
    void getLastActionTime();
};

#if UINTPTR_MAX == 0xffffffffu && !defined(RECON_NATIVE)
#endif

#endif
