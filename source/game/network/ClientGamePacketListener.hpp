#ifndef GAME_NETWORK_CLIENTGAMEPACKETLISTENER_HPP
#define GAME_NETWORK_CLIENTGAMEPACKETLISTENER_HPP

#include <boost/shared_ptr.hpp>

#include "game/network/PacketListener.hpp"

class BlockRegionUpdatePacket;
class ClientboundAddEntityPacket;
class ClientboundAddExperienceOrbPacket;
class ClientboundAddGlobalEntityPacket;
class ClientboundAddMobPacket;
class ClientboundAddPaintingPacket;
class ClientboundAddPlayerPacket;
class ClientboundAnimatePacket;
class ClientboundBlockDestructionPacket;
class ClientboundBlockEntityDataPacket;
class ClientboundBlockEventPacket;
class ClientboundBlockUpdatePacket;
class ClientboundBossEventPacket;
class ClientboundChangeDifficultyPacket;
class ClientboundChatAutoCompletePacket;
class ClientboundChatPacket;
class ClientboundChunkBlocksUpdatePacket;
class ClientboundContainerAckPacket;
class ClientboundContainerClosePacket;
class ClientboundContainerOpenPacket;
class ClientboundContainerSetContentPacket;
class ClientboundContainerSetDataPacket;
class ClientboundContainerSetSlotPacket;
class ClientboundCustomPayloadPacket;
class ClientboundDamageIndicatorPacket;
class ClientboundEntityEventPacket;
class ClientboundExplodePacket;
class ClientboundGameEventPacket;
class ClientboundKeepAlivePacket;
class ClientboundLevelEventPacket;
class ClientboundLevelParticlesPacket;
class ClientboundLoginPacket;
class ClientboundMapItemDataPacket;
class ClientboundMoveEntityPacket;
class ClientboundOpenSignEditorPacket;
class ClientboundPlayerAbilitiesPacket;
class ClientboundPlayerCombatPacket;
class ClientboundPlayerPositionPacket;
class ClientboundPlayerSleepPacket;
class ClientboundRemoveEntitiesPacket;
class ClientboundRemoveMobEffectPacket;
class ClientboundResourcePackPacket;
class ClientboundRespawnPacket;
class ClientboundRotateHeadPacket;
class ClientboundSetBorderPacket;
class ClientboundSetCameraPacket;
class ClientboundSetCarriedItemPacket;
class ClientboundSetEntityDataPacket;
class ClientboundSetEntityLinkPacket;
class ClientboundSetEntityMotionPacket;
class ClientboundSetEquippedItemPacket;
class ClientboundSetExperiencePacket;
class ClientboundSetHealthPacket;
class ClientboundSetPlayerTeamPacket;
class ClientboundSetSpawnPositionPacket;
class ClientboundSetTimePacket;
class ClientboundSetTitlesPacket;
class ClientboundSoundPacket;
class ClientboundTabListPacket;
class ClientboundTakeItemEntityPacket;
class ClientboundTeleportEntityPacket;
class ClientboundUpdateAttributesPacket;
class ClientboundUpdateMobEffectPacket;
class DisconnectPacket;
class PlayerInfoPacket;

class ClientGamePacketListener : public PacketListener
{
public:
    virtual ~ClientGamePacketListener();

    virtual void handleAddEntity(boost::shared_ptr<ClientboundAddEntityPacket> packet);
    virtual void handleAddExperienceOrb(boost::shared_ptr<ClientboundAddExperienceOrbPacket> packet);
    virtual void handleAddGlobalEntity(boost::shared_ptr<ClientboundAddGlobalEntityPacket> packet);
    virtual void handleAddMob(boost::shared_ptr<ClientboundAddMobPacket> packet);
    virtual void handleAddPainting(boost::shared_ptr<ClientboundAddPaintingPacket> packet);
    virtual void handleAddPlayer(boost::shared_ptr<ClientboundAddPlayerPacket> packet);
    virtual void handleAnimate(boost::shared_ptr<ClientboundAnimatePacket> packet);
    virtual void handleBlockDestruction(boost::shared_ptr<ClientboundBlockDestructionPacket> packet);
    virtual void handleOpenSignEditor(boost::shared_ptr<ClientboundOpenSignEditorPacket> packet);
    virtual void handleBlockEntityData(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet);
    virtual void handleBlockEvent(boost::shared_ptr<ClientboundBlockEventPacket> packet);
    virtual void handleBlockUpdate(boost::shared_ptr<ClientboundBlockUpdatePacket> packet);
    virtual void handleBlockRegionUpdate(boost::shared_ptr<BlockRegionUpdatePacket> packet);
    virtual void handleChat(boost::shared_ptr<ClientboundChatPacket> packet);
    virtual void handleChatAutoComplete(boost::shared_ptr<ClientboundChatAutoCompletePacket> packet);
    virtual void handleChunkBlocksUpdate(boost::shared_ptr<ClientboundChunkBlocksUpdatePacket> packet);
    virtual void handleMapItemData(boost::shared_ptr<ClientboundMapItemDataPacket> packet);
    virtual void handleContainerAck(boost::shared_ptr<ClientboundContainerAckPacket> packet);
    virtual void handleContainerClose(boost::shared_ptr<ClientboundContainerClosePacket> packet);
    virtual void handleContainerContent(boost::shared_ptr<ClientboundContainerSetContentPacket> packet);
    virtual void handleContainerOpen(boost::shared_ptr<ClientboundContainerOpenPacket> packet);
    virtual void handleContainerSetData(boost::shared_ptr<ClientboundContainerSetDataPacket> packet);
    virtual void handleContainerSetSlot(boost::shared_ptr<ClientboundContainerSetSlotPacket> packet);
    virtual void handleCustomPayload(boost::shared_ptr<ClientboundCustomPayloadPacket> packet);
    virtual void handleDisconnect(boost::shared_ptr<DisconnectPacket> packet);
    virtual void handleEntityActionAtPosition(boost::shared_ptr<ClientboundPlayerSleepPacket> packet);
    virtual void handleEntityEvent(boost::shared_ptr<ClientboundEntityEventPacket> packet);
    virtual void handleEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet);
    virtual void handleExplosion(boost::shared_ptr<ClientboundExplodePacket> packet);
    virtual void handleGameEvent(boost::shared_ptr<ClientboundGameEventPacket> packet);
    virtual void handleKeepAlive(boost::shared_ptr<ClientboundKeepAlivePacket> packet);
    virtual void handleLevelEvent(boost::shared_ptr<ClientboundLevelEventPacket> packet);
    virtual void handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet);
    virtual void handleMoveEntity(boost::shared_ptr<ClientboundMoveEntityPacket> packet);
    virtual void handleMovePlayer(boost::shared_ptr<ClientboundPlayerPositionPacket> packet);
    virtual void handleParticleEvent(boost::shared_ptr<ClientboundLevelParticlesPacket> packet);
    virtual void handlePlayerAbilities(boost::shared_ptr<ClientboundPlayerAbilitiesPacket> packet);
    virtual void handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet);
    virtual void handleRemoveEntity(boost::shared_ptr<ClientboundRemoveEntitiesPacket> packet);
    virtual void handleRemoveMobEffect(boost::shared_ptr<ClientboundRemoveMobEffectPacket> packet);
    virtual void handleRespawn(boost::shared_ptr<ClientboundRespawnPacket> packet);
    virtual void handleRotateMob(boost::shared_ptr<ClientboundRotateHeadPacket> packet);
    virtual void handleSetCarriedItem(boost::shared_ptr<ClientboundSetCarriedItemPacket> packet);
    virtual void handleSetEntityData(boost::shared_ptr<ClientboundSetEntityDataPacket> packet);
    virtual void handleSetEntityMotion(boost::shared_ptr<ClientboundSetEntityMotionPacket> packet);
    virtual void handleSetEquippedItem(boost::shared_ptr<ClientboundSetEquippedItemPacket> packet);
    virtual void handleSetExperience(boost::shared_ptr<ClientboundSetExperiencePacket> packet);
    virtual void handleSetHealth(boost::shared_ptr<ClientboundSetHealthPacket> packet);
    virtual void handleSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet);
    virtual void handleSetSpawn(boost::shared_ptr<ClientboundSetSpawnPositionPacket> packet);
    virtual void handleSetTime(boost::shared_ptr<ClientboundSetTimePacket> packet);
    virtual void handleSoundEvent(boost::shared_ptr<ClientboundSoundPacket> packet);
    virtual void handleTakeItemEntity(boost::shared_ptr<ClientboundTakeItemEntityPacket> packet);
    virtual void handleTeleportEntity(boost::shared_ptr<ClientboundTeleportEntityPacket> packet);
    virtual void handleUpdateAttributes(boost::shared_ptr<ClientboundUpdateAttributesPacket> packet);
    virtual void handleUpdateMobEffect(boost::shared_ptr<ClientboundUpdateMobEffectPacket> packet);
    virtual void handlePlayerCombat(boost::shared_ptr<ClientboundPlayerCombatPacket> packet);
    virtual void handleChangeDifficulty(boost::shared_ptr<ClientboundChangeDifficultyPacket> packet);
    virtual void handleSetCamera(boost::shared_ptr<ClientboundSetCameraPacket> packet);
    virtual void handleSetBorder(boost::shared_ptr<ClientboundSetBorderPacket> packet);
    virtual void handleSetTitles(boost::shared_ptr<ClientboundSetTitlesPacket> packet);
    virtual void handleTabListCustomisation(boost::shared_ptr<ClientboundTabListPacket> packet);
    virtual void handleResourcePack(boost::shared_ptr<ClientboundResourcePackPacket> packet);
    virtual void handleBossUpdate(boost::shared_ptr<ClientboundBossEventPacket> packet);
    virtual void handleDamageIndicator(boost::shared_ptr<ClientboundDamageIndicatorPacket> packet);
};

#endif
