#ifndef GAME_NETWORK_CLIENTPACKETLISTENER_HPP
#define GAME_NETWORK_CLIENTPACKETLISTENER_HPP

#include <stdint.h>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/network/ClientGamePacketListener.hpp"
#include "game/core/Socket.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/util/Random.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/minigame/Tutorial.hpp"

class Minecraft;
class Level;
class MultiPlayerLevel;
class Connection;
class SavedDataMemoryStorage;
class BlockPos;
class ClientboundBlockEntityDataPacket;
class ClientboundLoginPacket;
class ClientboundAddEntityPacket;
class ClientboundAddExperienceOrbPacket;
class ClientboundAddGlobalEntityPacket;
class ClientboundAddPaintingPacket;
class ClientboundSetEntityMotionPacket;
class ClientboundSetEntityDataPacket;
class ClientboundAddPlayerPacket;
class ClientboundTeleportEntityPacket;
class ClientboundSetCarriedItemPacket;
class ClientboundMoveEntityPacket;
class ClientboundRotateHeadPacket;
class MoveEntityPacketSmall;
class ClientboundRemoveEntitiesPacket;
class ClientboundPlayerPositionPacket;
class ChunkVisibilityAreaPacket;
class ChunkVisibilityPacket;
class ClientboundChunkBlocksUpdatePacket;
class BlockRegionUpdatePacket;
class ClientboundBlockUpdatePacket;
class DisconnectPacket;
class ClientboundTakeItemEntityPacket;
class ClientboundChatPacket;
class ClientboundAnimatePacket;
class ClientboundPlayerSleepPacket;
class ClientboundPreLoginPacket;
class ClientboundSetTimePacket;
class ClientboundSetSpawnPositionPacket;
class ClientboundSetPassengersPacket;
class ClientboundSetEntityLinkPacket;
class ClientboundEntityEventPacket;
class ClientboundDamageIndicatorPacket;
class ClientboundMGPlayerSettingsUpdatePacket;
class ClientboundSetHealthPacket;
class ClientboundSetExperiencePacket;
class ClientboundRespawnPacket;
class ClientboundExplodePacket;
class ClientboundContainerOpenPacket;
class ClientboundContainerSetSlotPacket;
class ClientboundContainerAckPacket;
class ClientboundContainerSetContentPacket;
class ClientboundOpenSignEditorPacket;
class ClientboundContainerSetDataPacket;
class ClientboundSetEquippedItemPacket;
class ClientboundContainerClosePacket;
class ClientboundBlockEventPacket;
class ClientboundBlockDestructionPacket;
class ClientboundBlockCollectionDestructionPacket;
class ClientboundGameEventPacket;
class ClientboundMapItemDataPacket;
class ClientboundLevelEventPacket;
class ClientboundAwardStatPacket;
class ClientboundUpdateMobEffectPacket;
class ClientboundPlayerCombatPacket;
class ClientboundChangeDifficultyPacket;
class ClientboundSetCameraPacket;
class ClientboundSetBorderPacket;
class ClientboundSetTitlesPacket;
class ClientboundTabListPacket;
class ClientboundRemoveMobEffectPacket;
class PlayerInfoPacket;
class ClientboundKeepAlivePacket;
class ClientboundPlayerAbilitiesPacket;
class ClientboundSoundPacket;
class ClientboundMoveVehiclePacket;
class ClientboundBossEventPacket;
class ClientboundCooldownPacket;
class ClientboundCustomPayloadPacket;
class ServerSettingsChangedPacket;
class TexturePacket;
class TextureAndGeometryPacket;
class UpdateProgressPacket;
class TextureChangePacket;
class TextureAndGeometryChangePacket;
class UpdateGameRuleProgressPacket;
class XZPacket;
class ScoreboardPacket;
class ClientboundSetPlayerTeamPacket;
class ClientboundLevelParticlesPacket;
class ClientboundUpdateAttributesPacket;
class GameModePacket;
class MapSelectInfoPacket;
class PlayerReadyPacket;
class ClientboundPowerupPacket;
class VotePacket;
class MultiplayerLocalPlayer;
class Entity;
class Packet;
class ClientPacketListener : public ClientGamePacketListener
{
public:
    class DeferredEntityLinkPacket
    {
    public:
        DeferredEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet);
        unsigned int mTick;
        boost::shared_ptr<ClientboundSetEntityLinkPacket> mPacket;
    };

    ClientPacketListener(Minecraft* minecraft, Socket* socket, int playerId);
    virtual ~ClientPacketListener();
    static void handleBlockEntityDataInternal(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet, Level* level);
    uint8_t isStarted();
    uint8_t isClosed();
    Socket* getSocket();
    uint8_t getAndSetClosed();
    void tick();
    INetworkPlayer* getNetworkPlayer();
    bool IsPrimaryPlayer();
    void displayPrivilegeChanges(boost::shared_ptr<MultiplayerLocalPlayer> player, uint32_t changes);
    virtual void handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet);
    boost::shared_ptr<Entity> getEntity(int entityId);
    void checkDeferredEntityLinkPackets(int entityId);
    virtual void handleAddEntity(boost::shared_ptr<ClientboundAddEntityPacket> packet);
    virtual void handleAddExperienceOrb(boost::shared_ptr<ClientboundAddExperienceOrbPacket> packet);
    virtual void handleAddGlobalEntity(boost::shared_ptr<ClientboundAddGlobalEntityPacket> packet);
    virtual void handleAddPainting(boost::shared_ptr<ClientboundAddPaintingPacket> packet);
    virtual void handleSetEntityMotion(boost::shared_ptr<ClientboundSetEntityMotionPacket> packet);
    virtual void handleSetEntityData(boost::shared_ptr<ClientboundSetEntityDataPacket> packet);
    void send(boost::shared_ptr<Packet> packet);
#ifdef MINECRAFT_PC
    void SendViewDistance();
    static void SendViewDistanceToAll();
#endif
    virtual void handleAddPlayer(boost::shared_ptr<ClientboundAddPlayerPacket> packet);
    virtual void handleTeleportEntity(boost::shared_ptr<ClientboundTeleportEntityPacket> packet);
    virtual void handleSetCarriedItem(boost::shared_ptr<ClientboundSetCarriedItemPacket> packet);
    virtual void handleMoveEntity(boost::shared_ptr<ClientboundMoveEntityPacket> packet);
    virtual void handleRotateMob(boost::shared_ptr<ClientboundRotateHeadPacket> packet);
    virtual void handleMoveEntitySmall(boost::shared_ptr<MoveEntityPacketSmall> packet);
    virtual void handleRemoveEntity(boost::shared_ptr<ClientboundRemoveEntitiesPacket> packet);
    virtual void handleMovePlayer(boost::shared_ptr<ClientboundPlayerPositionPacket> packet);
    virtual void handleChunkVisibilityArea(boost::shared_ptr<ChunkVisibilityAreaPacket> packet);
    virtual void handleChunkVisibility(boost::shared_ptr<ChunkVisibilityPacket> packet);
    virtual void handleChunkBlocksUpdate(boost::shared_ptr<ClientboundChunkBlocksUpdatePacket> packet);
    virtual void handleBlockRegionUpdate(boost::shared_ptr<BlockRegionUpdatePacket> packet);
    virtual void handleBlockUpdate(boost::shared_ptr<ClientboundBlockUpdatePacket> packet);
    virtual void handleDisconnect(boost::shared_ptr<DisconnectPacket> packet);
    static int ExitGameAndSaveReturned(void* context, int result, C4JStorage::EMessageResult messageResult);
    static int HostDisconnectReturned(void* context, int result, C4JStorage::EMessageResult messageResult);
    virtual void onDisconnect(DisconnectPacket::eDisconnectReason reason, void* userData);
    void sendAndDisconnect(boost::shared_ptr<Packet> packet);
    virtual void handleTakeItemEntity(boost::shared_ptr<ClientboundTakeItemEntityPacket> packet);
    std::wstring GetDisplayNameByGamertag(std::wstring gamertag);
    virtual void handleChat(boost::shared_ptr<ClientboundChatPacket> packet);
    uint32_t isLocalPlayer(int entityId);
    virtual void handleAnimate(boost::shared_ptr<ClientboundAnimatePacket> packet);
    virtual void handleEntityActionAtPosition(boost::shared_ptr<ClientboundPlayerSleepPacket> packet);
    virtual void handlePreLogin(boost::shared_ptr<ClientboundPreLoginPacket> packet);
    void close();
    virtual void handleAddMob(boost::shared_ptr<ClientboundAddMobPacket> packet);
    virtual void handleSetTime(boost::shared_ptr<ClientboundSetTimePacket> packet);
    virtual void handleSetSpawn(boost::shared_ptr<ClientboundSetSpawnPositionPacket> packet);
    virtual void handleSetEntityPassengersPacket(boost::shared_ptr<ClientboundSetPassengersPacket> packet);
    virtual void handleEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet);
    virtual void handleEntityEvent(boost::shared_ptr<ClientboundEntityEventPacket> packet);
    void handleDamageIndicator(boost::shared_ptr<ClientboundDamageIndicatorPacket> packet);
    void handleMiniGamePlayerSettingsUpdatePacket(boost::shared_ptr<ClientboundMGPlayerSettingsUpdatePacket> packet);
    virtual void handleSetHealth(boost::shared_ptr<ClientboundSetHealthPacket> packet);
    void handleSetExperience(boost::shared_ptr<ClientboundSetExperiencePacket> packet);
    virtual void handleRespawn(boost::shared_ptr<ClientboundRespawnPacket> packet);
    virtual void handleExplosion(boost::shared_ptr<ClientboundExplodePacket> packet);
    virtual void handleContainerOpen(boost::shared_ptr<ClientboundContainerOpenPacket> packet);
    virtual void handleContainerSetSlot(boost::shared_ptr<ClientboundContainerSetSlotPacket> packet);
    virtual void handleContainerAck(boost::shared_ptr<ClientboundContainerAckPacket> packet);
    virtual void handleContainerContent(boost::shared_ptr<ClientboundContainerSetContentPacket> packet);
    virtual void handleOpenSignEditor(boost::shared_ptr<ClientboundOpenSignEditorPacket> packet);
    virtual void handleBlockEntityData(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet);
    virtual void handleContainerSetData(boost::shared_ptr<ClientboundContainerSetDataPacket> packet);
    virtual void handleSetEquippedItem(boost::shared_ptr<ClientboundSetEquippedItemPacket> packet);
    virtual void handleContainerClose(boost::shared_ptr<ClientboundContainerClosePacket> packet);
    virtual void handleBlockEvent(boost::shared_ptr<ClientboundBlockEventPacket> packet);
    virtual void handleBlockDestruction(boost::shared_ptr<ClientboundBlockDestructionPacket> packet);
    virtual void handleBlockCollectionDestruction(
        boost::shared_ptr<ClientboundBlockCollectionDestructionPacket> packet);
    virtual bool canHandleAsyncPackets();
    virtual void handleGameEvent(boost::shared_ptr<ClientboundGameEventPacket> packet);
    virtual void handleMapItemData(boost::shared_ptr<ClientboundMapItemDataPacket> packet);
    void handleLevelEvent(boost::shared_ptr<ClientboundLevelEventPacket> packet);
    virtual void handleAwardStat(boost::shared_ptr<ClientboundAwardStatPacket> packet);
    virtual void handleUpdateMobEffect(boost::shared_ptr<ClientboundUpdateMobEffectPacket> packet);
    void handlePlayerCombat(boost::shared_ptr<ClientboundPlayerCombatPacket> packet);
    virtual void handleChangeDifficulty(boost::shared_ptr<ClientboundChangeDifficultyPacket> packet);
    virtual void handleSetCamera(boost::shared_ptr<ClientboundSetCameraPacket> packet);
    virtual void handleSetBorder(boost::shared_ptr<ClientboundSetBorderPacket> packet);
    void handleSetTitles(boost::shared_ptr<ClientboundSetTitlesPacket> packet);
    void handleTabListCustomisation(boost::shared_ptr<ClientboundTabListPacket> packet);
    virtual void handleRemoveMobEffect(boost::shared_ptr<ClientboundRemoveMobEffectPacket> packet);
    virtual bool isServerPacketListener();
    virtual void handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet);
    void handleKeepAlive(boost::shared_ptr<ClientboundKeepAlivePacket> packet);
    virtual void handlePlayerAbilities(boost::shared_ptr<ClientboundPlayerAbilitiesPacket> packet);
    virtual void handleSoundEvent(boost::shared_ptr<ClientboundSoundPacket> packet);
    virtual void handleMoveVehicle(boost::shared_ptr<ClientboundMoveVehiclePacket> packet);
    virtual void handleBossUpdate(boost::shared_ptr<ClientboundBossEventPacket> packet);
    void handleItemCooldown(boost::shared_ptr<ClientboundCooldownPacket> packet);
    virtual void handleCustomPayload(boost::shared_ptr<ClientboundCustomPayloadPacket> packet);
    Connection* getConnection();
    virtual void handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet);
    virtual void handleTexture(boost::shared_ptr<TexturePacket> packet);
    virtual void handleTextureAndGeometry(boost::shared_ptr<TextureAndGeometryPacket> packet);
    virtual void handleUpdateProgress(boost::shared_ptr<UpdateProgressPacket> packet);
    virtual void handleTextureChange(boost::shared_ptr<TextureChangePacket> packet);
    virtual void handleTextureAndGeometryChange(boost::shared_ptr<TextureAndGeometryChangePacket> packet);
    void handleUpdateGameRuleProgressPacket(boost::shared_ptr<UpdateGameRuleProgressPacket> packet);
    virtual void handleXZ(boost::shared_ptr<XZPacket> packet);
    virtual void handleScoreboardPacket(boost::shared_ptr<ScoreboardPacket> packet);
    virtual void handleClientboundSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet);
    virtual void handleParticleEvent(boost::shared_ptr<ClientboundLevelParticlesPacket> packet);
    virtual void handleUpdateAttributes(boost::shared_ptr<ClientboundUpdateAttributesPacket> packet);
    void updateTutorialOnGameMode(eTutorial_GameMode gameMode);
    virtual void handleGameMode(boost::shared_ptr<GameModePacket> packet);
    void handleMapSelectInfo(boost::shared_ptr<MapSelectInfoPacket> packet);
    void handlePlayerReady(boost::shared_ptr<PlayerReadyPacket> packet);
    virtual void handlePowerup(boost::shared_ptr<ClientboundPowerupPacket> packet);
    virtual void handleVote(boost::shared_ptr<VotePacket> packet);
    static void staticCtor();
    static OSFastMutex s_deferredLinkMutex;
    static std::vector<DeferredEntityLinkPacket> s_deferredLinks;

private:
    bool m_closed;
    OSFastMutex m_mutex;
    Connection* m_connection;

public:
    std::wstring mRemoteAddress;
    bool m_createdOk;

private:
    Minecraft* m_minecraft;
    MultiPlayerLevel* m_level;
    bool m_started;
    bool m_suppressStartOnPosition;
    int m_maxPlayers;
    Random m_random;
    BlockPos* m_destroyedBlocks;
    unsigned int m_destroyedBlockCount;
    int m_playerId;
    SavedDataMemoryStorage* m_savedDataStorage;
};

#endif
