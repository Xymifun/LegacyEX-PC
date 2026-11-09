#include "game/network/ServerSettingsChangedPacket.hpp"
#include "game/network/ServerboundPreLoginPacket.hpp"
#include "game/network/ServerboundPlayerAbilitiesPacket.hpp"
#include "game/network/ServerboundChatAutoCompletePacket.hpp"
#include "game/network/ServerboundChatPacket.hpp"
#include "game/network/ServerboundSwingPacket.hpp"
#include "game/network/PacketListener.hpp"

#include "game/network/BlockRegionUpdatePacket.hpp"
#include "game/network/ChunkVisibilityAreaPacket.hpp"
#include "game/network/ChunkVisibilityPacket.hpp"
#include "game/network/ClientboundAddEntityPacket.hpp"
#include "game/network/ClientboundAddExperienceOrbPacket.hpp"
#include "game/network/ClientboundAddGlobalEntityPacket.hpp"
#include "game/network/ClientboundAddMobPacket.hpp"
#include "game/network/ClientboundAddPaintingPacket.hpp"
#include "game/network/ClientboundAddPlayerPacket.hpp"
#include "game/network/ClientboundAnimatePacket.hpp"
#include "game/network/ClientboundAwardStatPacket.hpp"
#include "game/network/ClientboundBlockCollectionDestructionPacket.hpp"
#include "game/network/ClientboundBlockDestructionPacket.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/network/ClientboundBlockEventPacket.hpp"
#include "game/network/ClientboundBlockUpdatePacket.hpp"
#include "game/network/ClientboundBossEventPacket.hpp"
#include "game/network/ClientboundChangeDifficultyPacket.hpp"
#include "game/network/ClientboundChatAutoCompletePacket.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ClientboundChunkBlocksUpdatePacket.hpp"
#include "game/network/ClientboundContainerAckPacket.hpp"
#include "game/network/ClientboundContainerClosePacket.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/network/ClientboundContainerSetContentPacket.hpp"
#include "game/network/ClientboundContainerSetDataPacket.hpp"
#include "game/network/ClientboundContainerSetSlotPacket.hpp"
#include "game/network/ClientboundCooldownPacket.hpp"
#include "game/network/ClientboundCustomPayloadPacket.hpp"
#include "game/network/ClientboundDamageIndicatorPacket.hpp"
#include "game/network/ClientboundEntityEventPacket.hpp"
#include "game/network/ClientboundExplodePacket.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ClientboundKeepAlivePacket.hpp"
#include "game/network/ClientboundLevelEventPacket.hpp"
#include "game/network/ClientboundLevelParticlesPacket.hpp"
#include "game/network/ClientboundLoginPacket.hpp"
#include "game/network/ClientboundMGPlayerSettingsUpdatePacket.hpp"
#include "game/network/ClientboundMapItemDataPacket.hpp"
#include "game/network/ClientboundMoveEntityPacket.hpp"
#include "game/network/ClientboundMoveVehiclePacket.hpp"
#include "game/network/ClientboundOpenSignEditorPacket.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/ClientboundPlayerCombatPacket.hpp"
#include "game/network/ClientboundPlayerPositionPacket.hpp"
#include "game/network/ClientboundPlayerSleepPacket.hpp"
#include "game/network/ClientboundPowerupPacket.hpp"
#include "game/network/ClientboundPreLoginPacket.hpp"
#include "game/network/ClientboundRemoveEntitiesPacket.hpp"
#include "game/network/ClientboundRemoveMobEffectPacket.hpp"
#include "game/network/ClientboundResourcePackPacket.hpp"
#include "game/network/ClientboundRespawnPacket.hpp"
#include "game/network/ClientboundRotateHeadPacket.hpp"
#include "game/network/ClientboundSetBorderPacket.hpp"
#include "game/network/ClientboundSetCameraPacket.hpp"
#include "game/network/ClientboundSetCarriedItemPacket.hpp"
#include "game/network/ClientboundSetEntityDataPacket.hpp"
#include "game/network/ClientboundSetEntityLinkPacket.hpp"
#include "game/network/ClientboundSetEntityMotionPacket.hpp"
#include "game/network/ClientboundSetEquippedItemPacket.hpp"
#include "game/network/ClientboundSetExperiencePacket.hpp"
#include "game/network/ClientboundSetHealthPacket.hpp"
#include "game/network/ClientboundSetPassengersPacket.hpp"
#include "game/network/ClientboundSetPlayerTeamPacket.hpp"
#include "game/network/ClientboundSetSpawnPositionPacket.hpp"
#include "game/network/ClientboundSetTimePacket.hpp"
#include "game/network/ClientboundSetTitlesPacket.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/ClientboundTabListPacket.hpp"
#include "game/network/ClientboundTakeItemEntityPacket.hpp"
#include "game/network/ClientboundTeleportEntityPacket.hpp"
#include "game/network/ClientboundUpdateAttributesPacket.hpp"
#include "game/network/ClientboundUpdateMobEffectPacket.hpp"
#include "game/network/CraftItemPacket.hpp"
#include "game/network/DebugOptionsPacket.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/GetInfoPacket.hpp"
#include "game/network/KickPlayerPacket.hpp"
#include "game/network/MapSelectInfoPacket.hpp"
#include "game/network/MoveEntityPacketSmall.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/network/PlayerReadyPacket.hpp"
#include "game/network/ScoreboardPacket.hpp"
#include "game/network/ServerboundAcceptTeleportationPacket.hpp"
#include "game/network/ServerboundClientCommandPacket.hpp"
#include "game/network/ServerboundClientInformationPacket.hpp"
#include "game/network/ServerboundContainerAckPacket.hpp"
#include "game/network/ServerboundContainerButtonClickPacket.hpp"
#include "game/network/ServerboundContainerClickPacket.hpp"
#include "game/network/ServerboundContainerClosePacket.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/network/ServerboundInteractPacket.hpp"
#include "game/network/ServerboundKeepAlivePacket.hpp"
#include "game/network/ServerboundMovePlayerPacket.hpp"
#include "game/network/ServerboundMoveVehiclePacket.hpp"
#include "game/network/ServerboundPaddleBoatPacket.hpp"
#include "game/network/ServerboundPlayerActionPacket.hpp"
#include "game/network/ServerboundPlayerCommandPacket.hpp"
#include "game/network/ServerboundPlayerInputPacket.hpp"
#include "game/network/ServerboundResourcePackPacket.hpp"
#include "game/network/ServerboundSetCarriedItemPacket.hpp"
#include "game/network/ServerboundSetCreativeModeSlotPacket.hpp"
#include "game/network/ServerboundSignUpdatePacket.hpp"
#include "game/network/ServerboundUseItemOnPacket.hpp"
#include "game/network/ServerboundUseItemPacket.hpp"
#include "game/network/ServerboundTeleportToEntityPacket.hpp"
#include "game/network/TextureAndGeometryChangePacket.hpp"
#include "game/network/TextureAndGeometryPacket.hpp"
#include "game/network/TextureChangePacket.hpp"
#include "game/network/TexturePacket.hpp"
#include "game/network/TradeItemPacket.hpp"
#include "game/network/UpdateGameRuleProgressPacket.hpp"
#include "game/network/UpdateProgressPacket.hpp"
#include "game/network/VotePacket.hpp"
#include "game/network/XZPacket.hpp"

void PacketListener::onUnhandledPacket(boost::shared_ptr<Packet> packet)
{
}

void PacketListener::onDisconnect(DisconnectPacket::eDisconnectReason reason, void* userData)
{
}

bool PacketListener::canHandleAsyncPackets()
{
    return false;
}

void PacketListener::handleAcceptedLogin(boost::shared_ptr<ClientboundLoginPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleGetInfo(boost::shared_ptr<GetInfoPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddEntity(boost::shared_ptr<ClientboundAddEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddExperienceOrb(boost::shared_ptr<ClientboundAddExperienceOrbPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddGlobalEntity(boost::shared_ptr<ClientboundAddGlobalEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddMob(boost::shared_ptr<ClientboundAddMobPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddPainting(boost::shared_ptr<ClientboundAddPaintingPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddPlayer(boost::shared_ptr<ClientboundAddPlayerPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAnimate(boost::shared_ptr<ClientboundAnimatePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAnimate(boost::shared_ptr<ServerboundSwingPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAwardStat(boost::shared_ptr<ClientboundAwardStatPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockDestruction(boost::shared_ptr<ClientboundBlockDestructionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockCollectionDestruction(
    boost::shared_ptr<ClientboundBlockCollectionDestructionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleOpenSignEditor(boost::shared_ptr<ClientboundOpenSignEditorPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockEntityData(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockEvent(boost::shared_ptr<ClientboundBlockEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockUpdate(boost::shared_ptr<ClientboundBlockUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockRegionUpdate(boost::shared_ptr<BlockRegionUpdatePacket> packet)
{
}

void PacketListener::handleChat(boost::shared_ptr<ClientboundChatPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChat(boost::shared_ptr<ServerboundChatPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChatAutoComplete(boost::shared_ptr<ClientboundChatAutoCompletePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChatAutoComplete(boost::shared_ptr<ServerboundChatAutoCompletePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChunkBlocksUpdate(boost::shared_ptr<ClientboundChunkBlocksUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChunkVisibility(boost::shared_ptr<ChunkVisibilityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChunkVisibilityArea(boost::shared_ptr<ChunkVisibilityAreaPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMapItemData(boost::shared_ptr<ClientboundMapItemDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerAck(boost::shared_ptr<ClientboundContainerAckPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerAck(boost::shared_ptr<ServerboundContainerAckPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerClose(boost::shared_ptr<ClientboundContainerClosePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerClose(boost::shared_ptr<ServerboundContainerClosePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerContent(boost::shared_ptr<ClientboundContainerSetContentPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerOpen(boost::shared_ptr<ClientboundContainerOpenPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerSetData(boost::shared_ptr<ClientboundContainerSetDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerSetSlot(boost::shared_ptr<ClientboundContainerSetSlotPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleCustomPayload(boost::shared_ptr<ClientboundCustomPayloadPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleCustomPayload(boost::shared_ptr<ServerboundCustomPayloadPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleDisconnect(boost::shared_ptr<DisconnectPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleEntityActionAtPosition(boost::shared_ptr<ClientboundPlayerSleepPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleEntityEvent(boost::shared_ptr<ClientboundEntityEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetEntityPassengersPacket(boost::shared_ptr<ClientboundSetPassengersPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleExplosion(boost::shared_ptr<ClientboundExplodePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleGameEvent(boost::shared_ptr<ClientboundGameEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleKeepAlive(boost::shared_ptr<ClientboundKeepAlivePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleKeepAlive(boost::shared_ptr<ServerboundKeepAlivePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleLevelEvent(boost::shared_ptr<ClientboundLevelEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMoveEntity(boost::shared_ptr<ClientboundMoveEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMoveEntitySmall(boost::shared_ptr<MoveEntityPacketSmall> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMovePlayer(boost::shared_ptr<ClientboundPlayerPositionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMovePlayer(boost::shared_ptr<ServerboundMovePlayerPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleParticleEvent(boost::shared_ptr<ClientboundLevelParticlesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerAbilities(boost::shared_ptr<ClientboundPlayerAbilitiesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerAbilities(boost::shared_ptr<ServerboundPlayerAbilitiesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePreLogin(boost::shared_ptr<ClientboundPreLoginPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePreLogin(boost::shared_ptr<ServerboundPreLoginPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleRemoveEntity(boost::shared_ptr<ClientboundRemoveEntitiesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleRemoveMobEffect(boost::shared_ptr<ClientboundRemoveMobEffectPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleRespawn(boost::shared_ptr<ClientboundRespawnPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleRotateMob(boost::shared_ptr<ClientboundRotateHeadPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetCarriedItem(boost::shared_ptr<ClientboundSetCarriedItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetCarriedItem(boost::shared_ptr<ServerboundSetCarriedItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetEntityData(boost::shared_ptr<ClientboundSetEntityDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetEntityMotion(boost::shared_ptr<ClientboundSetEntityMotionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetEquippedItem(boost::shared_ptr<ClientboundSetEquippedItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetExperience(boost::shared_ptr<ClientboundSetExperiencePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetHealth(boost::shared_ptr<ClientboundSetHealthPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetSpawn(boost::shared_ptr<ClientboundSetSpawnPositionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetTime(boost::shared_ptr<ClientboundSetTimePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSoundEvent(boost::shared_ptr<ClientboundSoundPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTakeItemEntity(boost::shared_ptr<ClientboundTakeItemEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTeleportEntity(boost::shared_ptr<ClientboundTeleportEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUpdateAttributes(boost::shared_ptr<ClientboundUpdateAttributesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUpdateMobEffect(boost::shared_ptr<ClientboundUpdateMobEffectPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerCombat(boost::shared_ptr<ClientboundPlayerCombatPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChangeDifficulty(boost::shared_ptr<ClientboundChangeDifficultyPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetCamera(boost::shared_ptr<ClientboundSetCameraPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetBorder(boost::shared_ptr<ClientboundSetBorderPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetTitles(boost::shared_ptr<ClientboundSetTitlesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTabListCustomisation(boost::shared_ptr<ClientboundTabListPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleResourcePack(boost::shared_ptr<ClientboundResourcePackPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBossUpdate(boost::shared_ptr<ClientboundBossEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleItemCooldown(boost::shared_ptr<ClientboundCooldownPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMoveVehicle(boost::shared_ptr<ClientboundMoveVehiclePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMoveVehicle(boost::shared_ptr<ServerboundMoveVehiclePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTexture(boost::shared_ptr<TexturePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTextureAndGeometry(boost::shared_ptr<TextureAndGeometryPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUpdateProgress(boost::shared_ptr<UpdateProgressPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTextureChange(boost::shared_ptr<TextureChangePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTextureAndGeometryChange(boost::shared_ptr<TextureAndGeometryChangePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUpdateGameRuleProgressPacket(boost::shared_ptr<UpdateGameRuleProgressPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleXZ(boost::shared_ptr<XZPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleScoreboardPacket(boost::shared_ptr<ScoreboardPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleGameMode(boost::shared_ptr<GameModePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMapSelectInfo(boost::shared_ptr<MapSelectInfoPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerReady(boost::shared_ptr<PlayerReadyPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePowerup(boost::shared_ptr<ClientboundPowerupPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleDamageIndicator(boost::shared_ptr<ClientboundDamageIndicatorPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMiniGamePlayerSettingsUpdatePacket(
    boost::shared_ptr<ClientboundMGPlayerSettingsUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleClientCommand(boost::shared_ptr<ServerboundClientCommandPacket> packet)
{
}

void PacketListener::handleClientInformation(boost::shared_ptr<ServerboundClientInformationPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerButtonClick(boost::shared_ptr<ServerboundContainerButtonClickPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerClick(boost::shared_ptr<ServerboundContainerClickPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleInteract(boost::shared_ptr<ServerboundInteractPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerAction(boost::shared_ptr<ServerboundPlayerActionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerCommand(boost::shared_ptr<ServerboundPlayerCommandPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerInput(boost::shared_ptr<ServerboundPlayerInputPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetCreativeModeSlot(boost::shared_ptr<ServerboundSetCreativeModeSlotPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSignUpdate(boost::shared_ptr<ServerboundSignUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUseItemOn(boost::shared_ptr<ServerboundUseItemOnPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUseItem(boost::shared_ptr<ServerboundUseItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTeleportToEntityPacket(boost::shared_ptr<ServerboundTeleportToEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleResourcePackResponse(boost::shared_ptr<ServerboundResourcePackPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePaddleBoat(boost::shared_ptr<ServerboundPaddleBoatPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAcceptTeleportPacket(boost::shared_ptr<ServerboundAcceptTeleportationPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleCraftItem(boost::shared_ptr<CraftItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTradeItem(boost::shared_ptr<TradeItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleDebugOptions(boost::shared_ptr<DebugOptionsPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleKickPlayer(boost::shared_ptr<KickPlayerPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleGameCommand(boost::shared_ptr<GameCommandPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleVote(boost::shared_ptr<VotePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleClientboundSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet)
{
    onUnhandledPacket(packet);
}

bool PacketListener::isServerPacketListener()
{
    return false;
}

#undef onUnhandledPacket
