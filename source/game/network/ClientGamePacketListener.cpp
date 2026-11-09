#include "game/network/ClientGamePacketListener.hpp"

#include "game/network/BlockRegionUpdatePacket.hpp"
#include "game/network/ClientboundAddEntityPacket.hpp"
#include "game/network/ClientboundAddExperienceOrbPacket.hpp"
#include "game/network/ClientboundAddGlobalEntityPacket.hpp"
#include "game/network/ClientboundAddMobPacket.hpp"
#include "game/network/ClientboundAddPaintingPacket.hpp"
#include "game/network/ClientboundAddPlayerPacket.hpp"
#include "game/network/ClientboundAnimatePacket.hpp"
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
#include "game/network/ClientboundCustomPayloadPacket.hpp"
#include "game/network/ClientboundDamageIndicatorPacket.hpp"
#include "game/network/ClientboundEntityEventPacket.hpp"
#include "game/network/ClientboundExplodePacket.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ClientboundKeepAlivePacket.hpp"
#include "game/network/ClientboundLevelEventPacket.hpp"
#include "game/network/ClientboundLevelParticlesPacket.hpp"
#include "game/network/ClientboundLoginPacket.hpp"
#include "game/network/ClientboundMapItemDataPacket.hpp"
#include "game/network/ClientboundMoveEntityPacket.hpp"
#include "game/network/ClientboundOpenSignEditorPacket.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/ClientboundPlayerCombatPacket.hpp"
#include "game/network/ClientboundPlayerPositionPacket.hpp"
#include "game/network/ClientboundPlayerSleepPacket.hpp"
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
#include "game/network/DisconnectPacket.hpp"
#include "game/network/PlayerInfoPacket.hpp"

ClientGamePacketListener::~ClientGamePacketListener()
{
}

void ClientGamePacketListener::handleAddEntity(boost::shared_ptr<ClientboundAddEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleAddExperienceOrb(boost::shared_ptr<ClientboundAddExperienceOrbPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleAddGlobalEntity(boost::shared_ptr<ClientboundAddGlobalEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleAddMob(boost::shared_ptr<ClientboundAddMobPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleAddPainting(boost::shared_ptr<ClientboundAddPaintingPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleAddPlayer(boost::shared_ptr<ClientboundAddPlayerPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleAnimate(boost::shared_ptr<ClientboundAnimatePacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleBlockDestruction(boost::shared_ptr<ClientboundBlockDestructionPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleOpenSignEditor(boost::shared_ptr<ClientboundOpenSignEditorPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleBlockEntityData(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleBlockEvent(boost::shared_ptr<ClientboundBlockEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleBlockUpdate(boost::shared_ptr<ClientboundBlockUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleBlockRegionUpdate(boost::shared_ptr<BlockRegionUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleChat(boost::shared_ptr<ClientboundChatPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleChatAutoComplete(boost::shared_ptr<ClientboundChatAutoCompletePacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleChunkBlocksUpdate(boost::shared_ptr<ClientboundChunkBlocksUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleMapItemData(boost::shared_ptr<ClientboundMapItemDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleContainerAck(boost::shared_ptr<ClientboundContainerAckPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleContainerClose(boost::shared_ptr<ClientboundContainerClosePacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleContainerContent(boost::shared_ptr<ClientboundContainerSetContentPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleContainerOpen(boost::shared_ptr<ClientboundContainerOpenPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleContainerSetData(boost::shared_ptr<ClientboundContainerSetDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleContainerSetSlot(boost::shared_ptr<ClientboundContainerSetSlotPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleCustomPayload(boost::shared_ptr<ClientboundCustomPayloadPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleDisconnect(boost::shared_ptr<DisconnectPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleEntityActionAtPosition(boost::shared_ptr<ClientboundPlayerSleepPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleEntityEvent(boost::shared_ptr<ClientboundEntityEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleExplosion(boost::shared_ptr<ClientboundExplodePacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleGameEvent(boost::shared_ptr<ClientboundGameEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleKeepAlive(boost::shared_ptr<ClientboundKeepAlivePacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleLevelEvent(boost::shared_ptr<ClientboundLevelEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleMoveEntity(boost::shared_ptr<ClientboundMoveEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleMovePlayer(boost::shared_ptr<ClientboundPlayerPositionPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleParticleEvent(boost::shared_ptr<ClientboundLevelParticlesPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handlePlayerAbilities(boost::shared_ptr<ClientboundPlayerAbilitiesPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleRemoveEntity(boost::shared_ptr<ClientboundRemoveEntitiesPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleRemoveMobEffect(boost::shared_ptr<ClientboundRemoveMobEffectPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleRespawn(boost::shared_ptr<ClientboundRespawnPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleRotateMob(boost::shared_ptr<ClientboundRotateHeadPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetCarriedItem(boost::shared_ptr<ClientboundSetCarriedItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetEntityData(boost::shared_ptr<ClientboundSetEntityDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetEntityMotion(boost::shared_ptr<ClientboundSetEntityMotionPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetEquippedItem(boost::shared_ptr<ClientboundSetEquippedItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetExperience(boost::shared_ptr<ClientboundSetExperiencePacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetHealth(boost::shared_ptr<ClientboundSetHealthPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetSpawn(boost::shared_ptr<ClientboundSetSpawnPositionPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetTime(boost::shared_ptr<ClientboundSetTimePacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSoundEvent(boost::shared_ptr<ClientboundSoundPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleTakeItemEntity(boost::shared_ptr<ClientboundTakeItemEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleTeleportEntity(boost::shared_ptr<ClientboundTeleportEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleUpdateAttributes(boost::shared_ptr<ClientboundUpdateAttributesPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleUpdateMobEffect(boost::shared_ptr<ClientboundUpdateMobEffectPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handlePlayerCombat(boost::shared_ptr<ClientboundPlayerCombatPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleChangeDifficulty(boost::shared_ptr<ClientboundChangeDifficultyPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetCamera(boost::shared_ptr<ClientboundSetCameraPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetBorder(boost::shared_ptr<ClientboundSetBorderPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleSetTitles(boost::shared_ptr<ClientboundSetTitlesPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleTabListCustomisation(boost::shared_ptr<ClientboundTabListPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleResourcePack(boost::shared_ptr<ClientboundResourcePackPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleBossUpdate(boost::shared_ptr<ClientboundBossEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void ClientGamePacketListener::handleDamageIndicator(boost::shared_ptr<ClientboundDamageIndicatorPacket> packet)
{
    onUnhandledPacket(packet);
}
