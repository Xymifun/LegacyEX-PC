#include "game/network/FakeClientPacketListener.hpp"
#include "game/network/Connection.hpp"
#include "game/network/ClientboundLoginPacket.hpp"
#include "game/network/ClientboundPreLoginPacket.hpp"
#include "game/network/ClientboundPlayerPositionPacket.hpp"
#include "game/network/ServerboundAcceptTeleportationPacket.hpp"
#include "game/core/GameSettings.hpp"
#include "game/world/generation/PlayerUID.hpp"

FakeClientPacketListener::~FakeClientPacketListener()
{
}


void FakeClientPacketListener::handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet)
{
}

void FakeClientPacketListener::handleAddEntity(boost::shared_ptr<ClientboundAddEntityPacket> packet)
{
}

void FakeClientPacketListener::handleAddExperienceOrb(boost::shared_ptr<ClientboundAddExperienceOrbPacket> packet)
{
}

void FakeClientPacketListener::handleAddGlobalEntity(boost::shared_ptr<ClientboundAddGlobalEntityPacket> packet)
{
}

void FakeClientPacketListener::handleAddPainting(boost::shared_ptr<ClientboundAddPaintingPacket> packet)
{
}

void FakeClientPacketListener::handleSetEntityMotion(boost::shared_ptr<ClientboundSetEntityMotionPacket> packet)
{
}

void FakeClientPacketListener::handleSetEntityData(boost::shared_ptr<ClientboundSetEntityDataPacket> packet)
{
}

void FakeClientPacketListener::handleAddPlayer(boost::shared_ptr<ClientboundAddPlayerPacket> packet)
{
}

void FakeClientPacketListener::handleTeleportEntity(boost::shared_ptr<ClientboundTeleportEntityPacket> packet)
{
}

void FakeClientPacketListener::handleSetCarriedItem(boost::shared_ptr<ClientboundSetCarriedItemPacket> packet)
{
}

void FakeClientPacketListener::handleMoveEntity(boost::shared_ptr<ClientboundMoveEntityPacket> packet)
{
}

void FakeClientPacketListener::handleRotateMob(boost::shared_ptr<ClientboundRotateHeadPacket> packet)
{
}

void FakeClientPacketListener::handleMoveEntitySmall(boost::shared_ptr<MoveEntityPacketSmall> packet)
{
}

void FakeClientPacketListener::handleRemoveEntity(boost::shared_ptr<ClientboundRemoveEntitiesPacket> packet)
{
}

void FakeClientPacketListener::handleMovePlayer(boost::shared_ptr<ClientboundPlayerPositionPacket> packet)
{
    ClientPacketListener::getConnection()->send(boost::shared_ptr<ServerboundAcceptTeleportationPacket>(
        new ServerboundAcceptTeleportationPacket(packet->getId())));
    m_hasPosition = true;
    m_x = packet->getX();
    m_y = packet->getY();
    m_z = packet->getZ();
}

void FakeClientPacketListener::handleChunkVisibilityArea(boost::shared_ptr<ChunkVisibilityAreaPacket> packet)
{
}

void FakeClientPacketListener::handleChunkVisibility(boost::shared_ptr<ChunkVisibilityPacket> packet)
{
}

void FakeClientPacketListener::handleChunkBlocksUpdate(boost::shared_ptr<ClientboundChunkBlocksUpdatePacket> packet)
{
}

void FakeClientPacketListener::handleBlockRegionUpdate(boost::shared_ptr<BlockRegionUpdatePacket> packet)
{
}

void FakeClientPacketListener::handleBlockUpdate(boost::shared_ptr<ClientboundBlockUpdatePacket> packet)
{
}

void FakeClientPacketListener::handleDisconnect(boost::shared_ptr<DisconnectPacket> packet)
{
}

void FakeClientPacketListener::onDisconnect(DisconnectPacket::eDisconnectReason reason, void* userData)
{
}

void FakeClientPacketListener::handleTakeItemEntity(boost::shared_ptr<ClientboundTakeItemEntityPacket> packet)
{
}

void FakeClientPacketListener::handleChat(boost::shared_ptr<ClientboundChatPacket> packet)
{
}

void FakeClientPacketListener::handleAnimate(boost::shared_ptr<ClientboundAnimatePacket> packet)
{
}

void FakeClientPacketListener::handleEntityActionAtPosition(boost::shared_ptr<ClientboundPlayerSleepPacket> packet)
{
}

void FakeClientPacketListener::handlePreLogin(boost::shared_ptr<ClientboundPreLoginPacket> packet)
{
    PlayerUID playerUid(0);
    std::wstring name;
    name.assign(L"FakeName");
    send(boost::shared_ptr<ClientboundLoginPacket>(
        new ClientboundLoginPacket(name, 316, playerUid, playerUid, false, packet->mSeed,
                                   GameSettings::GetPlayerSkinId(0), GameSettings::GetPlayerCapeId(0), false, false)));
}

void FakeClientPacketListener::handleAddMob(boost::shared_ptr<ClientboundAddMobPacket> packet)
{
}

void FakeClientPacketListener::handleSetTime(boost::shared_ptr<ClientboundSetTimePacket> packet)
{
}

void FakeClientPacketListener::handleSetSpawn(boost::shared_ptr<ClientboundSetSpawnPositionPacket> packet)
{
}

void FakeClientPacketListener::handleEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet)
{
}

void FakeClientPacketListener::handleEntityEvent(boost::shared_ptr<ClientboundEntityEventPacket> packet)
{
}

void FakeClientPacketListener::handleDamageIndicator(boost::shared_ptr<ClientboundDamageIndicatorPacket> packet)
{
}

void FakeClientPacketListener::handleSetHealth(boost::shared_ptr<ClientboundSetHealthPacket> packet)
{
}

void FakeClientPacketListener::handleSetExperience(boost::shared_ptr<ClientboundSetExperiencePacket> packet)
{
}

void FakeClientPacketListener::handleRespawn(boost::shared_ptr<ClientboundRespawnPacket> packet)
{
}

void FakeClientPacketListener::handleExplosion(boost::shared_ptr<ClientboundExplodePacket> packet)
{
}

void FakeClientPacketListener::handleContainerOpen(boost::shared_ptr<ClientboundContainerOpenPacket> packet)
{
}

void FakeClientPacketListener::handleContainerSetSlot(boost::shared_ptr<ClientboundContainerSetSlotPacket> packet)
{
}

void FakeClientPacketListener::handleContainerAck(boost::shared_ptr<ClientboundContainerAckPacket> packet)
{
}

void FakeClientPacketListener::handleContainerContent(boost::shared_ptr<ClientboundContainerSetContentPacket> packet)
{
}

void FakeClientPacketListener::handleOpenSignEditor(boost::shared_ptr<ClientboundOpenSignEditorPacket> packet)
{
}

void FakeClientPacketListener::handleBlockEntityData(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet)
{
}

void FakeClientPacketListener::handleContainerSetData(boost::shared_ptr<ClientboundContainerSetDataPacket> packet)
{
}

void FakeClientPacketListener::handleSetEquippedItem(boost::shared_ptr<ClientboundSetEquippedItemPacket> packet)
{
}

void FakeClientPacketListener::handleContainerClose(boost::shared_ptr<ClientboundContainerClosePacket> packet)
{
}

void FakeClientPacketListener::handleBlockEvent(boost::shared_ptr<ClientboundBlockEventPacket> packet)
{
}

void FakeClientPacketListener::handleBlockDestruction(boost::shared_ptr<ClientboundBlockDestructionPacket> packet)
{
}

bool FakeClientPacketListener::canHandleAsyncPackets()
{
    return true;
}

void FakeClientPacketListener::handleGameEvent(boost::shared_ptr<ClientboundGameEventPacket> packet)
{
}

void FakeClientPacketListener::handleMapItemData(boost::shared_ptr<ClientboundMapItemDataPacket> packet)
{
}

void FakeClientPacketListener::handleLevelEvent(boost::shared_ptr<ClientboundLevelEventPacket> packet)
{
}

void FakeClientPacketListener::handleAwardStat(boost::shared_ptr<ClientboundAwardStatPacket> packet)
{
}

void FakeClientPacketListener::handleUpdateMobEffect(boost::shared_ptr<ClientboundUpdateMobEffectPacket> packet)
{
}

void FakeClientPacketListener::handlePlayerCombat(boost::shared_ptr<ClientboundPlayerCombatPacket> packet)
{
}

void FakeClientPacketListener::handleChangeDifficulty(boost::shared_ptr<ClientboundChangeDifficultyPacket> packet)
{
}

void FakeClientPacketListener::handleSetCamera(boost::shared_ptr<ClientboundSetCameraPacket> packet)
{
}

void FakeClientPacketListener::handleSetBorder(boost::shared_ptr<ClientboundSetBorderPacket> packet)
{
}

void FakeClientPacketListener::handleSetTitles(boost::shared_ptr<ClientboundSetTitlesPacket> packet)
{
}

void FakeClientPacketListener::handleTabListCustomisation(boost::shared_ptr<ClientboundTabListPacket> packet)
{
}

void FakeClientPacketListener::handleRemoveMobEffect(boost::shared_ptr<ClientboundRemoveMobEffectPacket> packet)
{
}

bool FakeClientPacketListener::isServerPacketListener()
{
    return false;
}

void FakeClientPacketListener::handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet)
{
}

void FakeClientPacketListener::handleKeepAlive(boost::shared_ptr<ClientboundKeepAlivePacket> packet)
{
}

void FakeClientPacketListener::handlePlayerAbilities(boost::shared_ptr<ClientboundPlayerAbilitiesPacket> packet)
{
}

void FakeClientPacketListener::handleSoundEvent(boost::shared_ptr<ClientboundSoundPacket> packet)
{
}

void FakeClientPacketListener::handleCustomPayload(boost::shared_ptr<ClientboundCustomPayloadPacket> packet)
{
}

Connection* FakeClientPacketListener::getConnection()
{
    return ClientPacketListener::getConnection();
}

void FakeClientPacketListener::handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet)
{
}

void FakeClientPacketListener::handleTexture(boost::shared_ptr<TexturePacket> packet)
{
}

void FakeClientPacketListener::handleTextureAndGeometry(boost::shared_ptr<TextureAndGeometryPacket> packet)
{
}

void FakeClientPacketListener::handleUpdateProgress(boost::shared_ptr<UpdateProgressPacket> packet)
{
}

void FakeClientPacketListener::handleTextureChange(boost::shared_ptr<TextureChangePacket> packet)
{
}

void FakeClientPacketListener::handleTextureAndGeometryChange(boost::shared_ptr<TextureAndGeometryChangePacket> packet)
{
}

void FakeClientPacketListener::handleUpdateGameRuleProgressPacket(
    boost::shared_ptr<UpdateGameRuleProgressPacket> packet)
{
}

void FakeClientPacketListener::handleXZ(boost::shared_ptr<XZPacket> packet)
{
}

void FakeClientPacketListener::handleScoreboardPacket(boost::shared_ptr<ScoreboardPacket> packet)
{
}

void FakeClientPacketListener::handleClientboundSetPlayerTeamPacket(
    boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet)
{
}

void FakeClientPacketListener::handleParticleEvent(boost::shared_ptr<ClientboundLevelParticlesPacket> packet)
{
}

void FakeClientPacketListener::handleUpdateAttributes(boost::shared_ptr<ClientboundUpdateAttributesPacket> packet)
{
}

void FakeClientPacketListener::handleGameMode(boost::shared_ptr<GameModePacket> packet)
{
}

void FakeClientPacketListener::handleMapSelectInfo(boost::shared_ptr<MapSelectInfoPacket> packet)
{
}

void FakeClientPacketListener::handlePlayerReady(boost::shared_ptr<PlayerReadyPacket> packet)
{
}

void FakeClientPacketListener::handlePowerup(boost::shared_ptr<ClientboundPowerupPacket> packet)
{
}
