#include "game/network/Packet.hpp"

#include <stdio.h>

#include <sstream>

#include <boost/assert.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/core/DataOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/System.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
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
#include "game/network/ServerSettingsChangedPacket.hpp"
#include "game/network/ServerboundAcceptTeleportationPacket.hpp"
#include "game/network/ServerboundChatAutoCompletePacket.hpp"
#include "game/network/ServerboundChatPacket.hpp"
#include "game/network/ServerboundClientCommandPacket.hpp"
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
#include "game/network/ServerboundPlayerAbilitiesPacket.hpp"
#include "game/network/ServerboundPlayerActionPacket.hpp"
#include "game/network/ServerboundPlayerCommandPacket.hpp"
#include "game/network/ServerboundPlayerInputPacket.hpp"
#include "game/network/ServerboundPreLoginPacket.hpp"
#include "game/network/ServerboundResourcePackPacket.hpp"
#include "game/network/ServerboundSetCarriedItemPacket.hpp"
#include "game/network/ServerboundSetCreativeModeSlotPacket.hpp"
#include "game/network/ServerboundSignUpdatePacket.hpp"
#include "game/network/ServerboundSwingPacket.hpp"
#include "game/network/ServerboundTeleportToEntityPacket.hpp"
#include "game/network/ServerboundUseItemOnPacket.hpp"
#include "game/network/ServerboundUseItemPacket.hpp"
#include "game/network/TextureAndGeometryChangePacket.hpp"
#include "game/network/TextureAndGeometryPacket.hpp"
#include "game/network/TextureChangePacket.hpp"
#include "game/network/TexturePacket.hpp"
#include "game/network/TradeItemPacket.hpp"
#include "game/network/UpdateGameRuleProgressPacket.hpp"
#include "game/network/UpdateProgressPacket.hpp"
#include "game/network/VotePacket.hpp"
#include "game/network/XZPacket.hpp"
#include "platform/wiiu/CProfile.hpp"

int32_t getMaskInt(uint32_t offset, uint32_t width);

typedef boost::shared_ptr<Packet> (*PacketFactory)();

static boost::unordered_map<int, PacketFactory> sPacketFactories;
static boost::unordered_set<int> sClientboundPacketIds;
static boost::unordered_set<int> sServerboundPacketIds;
static boost::unordered_set<int> sSendToAnyClientPacketIds;

Packet::Packet()
{
    creationTime = System::processTimeInMilliSecs();
    shouldDelay = false;
}

void Packet::map(int packetId, bool clientbound, bool serverbound, bool sendToAnyClient, bool canBeDelayed,
                 const std::type_info& type, boost::shared_ptr<Packet> (*create)(), std::wstring name)
{
    sPacketFactories.insert(std::pair<const int, PacketFactory>(packetId, create));

    if (clientbound)
    {
        sClientboundPacketIds.insert(packetId);
    }

    if (serverbound)
    {
        sServerboundPacketIds.insert(packetId);
    }

    if (sendToAnyClient)
    {
        sSendToAnyClientPacketIds.insert(packetId);
    }
}

void Packet::staticCtor()
{
    map(0, true, true, true, false, typeid(ClientboundKeepAlivePacket), ClientboundKeepAlivePacket::create,
        L"KeepAlive");
    map(1, true, true, true, false, typeid(ClientboundLoginPacket), ClientboundLoginPacket::create, L"Login");
    map(2, true, true, true, false, typeid(ClientboundPreLoginPacket), ClientboundPreLoginPacket::create, L"PreLogin");
    map(3, true, true, true, false, typeid(ClientboundChatPacket), ClientboundChatPacket::create, L"Chat");
    map(4, true, false, false, true, typeid(ClientboundSetTimePacket), ClientboundSetTimePacket::create, L"SetTime");
    map(5, true, false, false, true, typeid(ClientboundSetEquippedItemPacket), ClientboundSetEquippedItemPacket::create,
        L"SetEquippedItem");
    map(6, true, false, true, true, typeid(ClientboundSetSpawnPositionPacket),
        ClientboundSetSpawnPositionPacket::create, L"SetSpawnPosition");
    map(7, false, true, false, false, typeid(ServerboundInteractPacket), ServerboundInteractPacket::create,
        L"Interact");
    map(8, true, false, true, true, typeid(ClientboundSetHealthPacket), ClientboundSetHealthPacket::create,
        L"SetHealth");
    map(9, true, true, true, false, typeid(ClientboundRespawnPacket), ClientboundRespawnPacket::create, L"Respawn");
    map(10, true, true, true, false, typeid(ServerboundMovePlayerPacket), ServerboundMovePlayerPacket::create,
        L"MovePlayer");
    map(11, true, true, true, true, typeid(ServerboundMovePlayerPacket::Pos), ServerboundMovePlayerPacket::Pos::create,
        L"MovePlayer");
    map(12, true, true, true, true, typeid(ServerboundMovePlayerPacket::Rot), ServerboundMovePlayerPacket::Rot::create,
        L"MovePlayer");
    map(13, true, true, true, true, typeid(ServerboundMovePlayerPacket::PosRot),
        ServerboundMovePlayerPacket::PosRot::create, L"MovePlayer");
    map(14, false, true, false, false, typeid(ServerboundPlayerActionPacket), ServerboundPlayerActionPacket::create,
        L"PlayerAction");
    map(15, false, true, false, false, typeid(ServerboundUseItemPacket), ServerboundUseItemPacket::create, L"UseItem");
    map(16, true, true, true, false, typeid(ClientboundSetCarriedItemPacket), ClientboundSetCarriedItemPacket::create,
        L"SetCarriedItem");
    map(17, true, false, true, false, typeid(ClientboundPlayerSleepPacket), ClientboundPlayerSleepPacket::create,
        L"PlayerSleep");
    map(18, true, true, true, false, typeid(ClientboundAnimatePacket), ClientboundAnimatePacket::create, L"Animate");
    map(19, false, true, false, false, typeid(ServerboundPlayerCommandPacket), ServerboundPlayerCommandPacket::create,
        L"PlayerCommand");
    map(20, true, false, false, true, typeid(ClientboundAddPlayerPacket), ClientboundAddPlayerPacket::create,
        L"AddPlayer");
    map(22, true, false, true, true, typeid(ClientboundTakeItemEntityPacket), ClientboundTakeItemEntityPacket::create,
        L"TakeItemEntity");
    map(23, true, false, false, true, typeid(ClientboundAddEntityPacket), ClientboundAddEntityPacket::create,
        L"AddEntity");
    map(24, true, false, false, true, typeid(ClientboundAddMobPacket), ClientboundAddMobPacket::create, L"AddMob");
    map(25, true, false, false, false, typeid(ClientboundAddPaintingPacket), ClientboundAddPaintingPacket::create,
        L"AddPainting");
    map(26, true, false, false, false, typeid(ClientboundAddExperienceOrbPacket),
        ClientboundAddExperienceOrbPacket::create, L"AddExperienceOrb");
    map(27, false, true, false, false, typeid(ServerboundPlayerInputPacket), ServerboundPlayerInputPacket::create,
        L"PlayerInput");
    map(28, true, false, true, true, typeid(ClientboundSetEntityMotionPacket), ClientboundSetEntityMotionPacket::create,
        L"SetEntityMotion");
    map(29, true, false, false, true, typeid(ClientboundRemoveEntitiesPacket), ClientboundRemoveEntitiesPacket::create,
        L"RemoveEntities");
    map(30, true, false, false, false, typeid(ClientboundMoveEntityPacket), ClientboundMoveEntityPacket::create,
        L"MoveEntity");
    map(31, true, false, false, true, typeid(ClientboundMoveEntityPacket::Pos),
        ClientboundMoveEntityPacket::Pos::create, L"MoveEntity");
    map(32, true, false, false, true, typeid(ClientboundMoveEntityPacket::Rot),
        ClientboundMoveEntityPacket::Rot::create, L"MoveEntity");
    map(33, true, false, false, true, typeid(ClientboundMoveEntityPacket::PosRot),
        ClientboundMoveEntityPacket::PosRot::create, L"MoveEntity");
    map(34, true, false, false, true, typeid(ClientboundTeleportEntityPacket), ClientboundTeleportEntityPacket::create,
        L"TeleportEntity");
    map(35, true, false, false, false, typeid(ClientboundRotateHeadPacket), ClientboundRotateHeadPacket::create,
        L"RotateHead");
    map(38, true, false, true, true, typeid(ClientboundEntityEventPacket), ClientboundEntityEventPacket::create,
        L"EntityEvent");
    map(39, true, false, true, false, typeid(ClientboundSetEntityLinkPacket), ClientboundSetEntityLinkPacket::create,
        L"SetEntityLink");
    map(40, true, false, true, true, typeid(ClientboundSetEntityDataPacket), ClientboundSetEntityDataPacket::create,
        L"SetEntityData");
    map(41, true, false, true, false, typeid(ClientboundUpdateMobEffectPacket),
        ClientboundUpdateMobEffectPacket::create, L"UpdateMobEffect");
    map(42, true, false, true, false, typeid(ClientboundRemoveMobEffectPacket),
        ClientboundRemoveMobEffectPacket::create, L"RemoveMobEffect");
    map(43, true, false, true, false, typeid(ClientboundSetExperiencePacket), ClientboundSetExperiencePacket::create,
        L"SetExperience");
    map(44, true, false, true, false, typeid(ClientboundUpdateAttributesPacket),
        ClientboundUpdateAttributesPacket::create, L"UpdateAttributes");
    map(50, true, false, true, true, typeid(ChunkVisibilityPacket), ChunkVisibilityPacket::create, L"ChunkVisibility");
    map(51, true, false, true, true, typeid(BlockRegionUpdatePacket), BlockRegionUpdatePacket::create,
        L"BlockRegionUpdate");
    map(52, true, false, true, true, typeid(ClientboundChunkBlocksUpdatePacket),
        ClientboundChunkBlocksUpdatePacket::create, L"ChunkBlocksUpdate");
    map(53, true, false, true, true, typeid(ClientboundBlockUpdatePacket), ClientboundBlockUpdatePacket::create,
        L"BlockUpdate");
    map(54, true, false, true, true, typeid(ClientboundBlockEventPacket), ClientboundBlockEventPacket::create,
        L"BlockEvent");
    map(55, true, false, false, false, typeid(ClientboundBlockDestructionPacket),
        ClientboundBlockDestructionPacket::create, L"BlockDestruction");
    map(60, true, false, true, false, typeid(ClientboundExplodePacket), ClientboundExplodePacket::create, L"Explode");
    map(61, true, false, true, false, typeid(ClientboundLevelEventPacket), ClientboundLevelEventPacket::create,
        L"LevelEvent");
    map(62, true, true, true, false, typeid(ClientboundSoundPacket), ClientboundSoundPacket::create, L"Sound");
    map(63, true, false, true, false, typeid(ClientboundLevelParticlesPacket), ClientboundLevelParticlesPacket::create,
        L"LevelParticles");
    map(70, true, false, false, false, typeid(ClientboundGameEventPacket), ClientboundGameEventPacket::create,
        L"GameEvent");
    map(71, true, false, false, false, typeid(ClientboundAddGlobalEntityPacket),
        ClientboundAddGlobalEntityPacket::create, L"AddGlobalEntity");
    map(100, true, false, true, false, typeid(ClientboundContainerOpenPacket), ClientboundContainerOpenPacket::create,
        L"ContainerOpen");
    map(101, true, true, true, false, typeid(ClientboundContainerClosePacket), ClientboundContainerClosePacket::create,
        L"ContainerClose");
    map(102, false, true, false, false, typeid(ServerboundContainerClickPacket),
        ServerboundContainerClickPacket::create, L"ContainerClick");
    map(103, true, false, true, false, typeid(ClientboundContainerSetSlotPacket),
        ClientboundContainerSetSlotPacket::create, L"ContainerSetSlot");
    map(104, true, false, true, false, typeid(ClientboundContainerSetContentPacket),
        ClientboundContainerSetContentPacket::create, L"ContainerSetContent");
    map(105, true, false, true, false, typeid(ClientboundContainerSetDataPacket),
        ClientboundContainerSetDataPacket::create, L"ContainerSetData");
    map(106, true, true, true, false, typeid(ClientboundContainerAckPacket), ClientboundContainerAckPacket::create,
        L"ContainerAck");
    map(107, true, true, true, false, typeid(ServerboundSetCreativeModeSlotPacket),
        ServerboundSetCreativeModeSlotPacket::create, L"SetCreativeModeSlot");
    map(108, false, true, false, false, typeid(ServerboundContainerButtonClickPacket),
        ServerboundContainerButtonClickPacket::create, L"ContainerButtonClick");
    map(132, true, false, true, false, typeid(ClientboundBlockEntityDataPacket),
        ClientboundBlockEntityDataPacket::create, L"BlockEntityData");
    map(133, true, false, false, false, typeid(ClientboundMoveVehiclePacket), ClientboundMoveVehiclePacket::create,
        L"MoveVehicle");
    map(134, true, false, false, false, typeid(ClientboundSetPassengersPacket), ClientboundSetPassengersPacket::create,
        L"SetPassengers");
    map(135, false, true, false, false, typeid(ServerboundAcceptTeleportationPacket),
        ServerboundAcceptTeleportationPacket::create, L"AcceptTeleportation");
    map(136, false, true, false, false, typeid(ServerboundMoveVehiclePacket), ServerboundMoveVehiclePacket::create,
        L"MoveVehicle");
    map(137, false, true, false, false, typeid(ServerboundUseItemOnPacket), ServerboundUseItemOnPacket::create,
        L"UseItemOn");
    map(138, false, true, false, false, typeid(ServerboundPaddleBoatPacket), ServerboundPaddleBoatPacket::create,
        L"PaddleBoat");
    map(139, true, false, true, false, typeid(ClientboundBossEventPacket), ClientboundBossEventPacket::create,
        L"BossEvent");
    map(150, false, true, false, false, typeid(CraftItemPacket), CraftItemPacket::create, L"CraftItem");
    map(151, false, true, true, false, typeid(TradeItemPacket), TradeItemPacket::create, L"TradeItem");
    map(152, false, true, false, false, typeid(DebugOptionsPacket), DebugOptionsPacket::create, L"DebugOptions");
    map(153, true, true, false, false, typeid(ServerSettingsChangedPacket), ServerSettingsChangedPacket::create,
        L"ServerSettingsChanged");
    map(154, true, true, true, false, typeid(TexturePacket), TexturePacket::create, L"Texture");
    map(155, true, false, true, true, typeid(ChunkVisibilityAreaPacket), ChunkVisibilityAreaPacket::create,
        L"ChunkVisibilityArea");
    map(156, true, false, false, true, typeid(UpdateProgressPacket), UpdateProgressPacket::create, L"UpdateProgress");
    map(157, true, true, true, false, typeid(TextureChangePacket), TextureChangePacket::create, L"TextureChange");
    map(158, true, false, true, false, typeid(UpdateGameRuleProgressPacket), UpdateGameRuleProgressPacket::create,
        L"UpdateGameRuleProgress");
    map(159, false, true, false, false, typeid(KickPlayerPacket), KickPlayerPacket::create, L"KickPlayer");
    map(160, true, true, true, false, typeid(TextureAndGeometryPacket), TextureAndGeometryPacket::create,
        L"TextureAndGeometry");
    map(161, true, true, true, false, typeid(TextureAndGeometryChangePacket), TextureAndGeometryChangePacket::create,
        L"TextureAndGeometryChange");
    map(162, true, false, false, false, typeid(MoveEntityPacketSmall), MoveEntityPacketSmall::create,
        L"MoveEntityPacketSmall");
    map(163, true, false, false, true, typeid(MoveEntityPacketSmall::Pos), MoveEntityPacketSmall::Pos::create,
        L"MoveEntityPacketSmall");
    map(164, true, false, false, true, typeid(MoveEntityPacketSmall::Rot), MoveEntityPacketSmall::Rot::create,
        L"MoveEntityPacketSmall");
    map(165, true, false, false, true, typeid(MoveEntityPacketSmall::PosRot), MoveEntityPacketSmall::PosRot::create,
        L"MoveEntityPacketSmall");
    map(166, true, true, false, false, typeid(XZPacket), XZPacket::create, L"XZ");
    map(167, false, true, false, false, typeid(GameCommandPacket), GameCommandPacket::create, L"GameCommand");
    map(168, true, false, false, false, typeid(MapSelectInfoPacket), MapSelectInfoPacket::create, L"MapSelectInfo");
    map(169, true, true, false, false, typeid(VotePacket), VotePacket::create, L"Vote");
    map(170, true, true, false, false, typeid(PlayerReadyPacket), PlayerReadyPacket::create, L"PlayerReady");
    map(171, true, false, true, false, typeid(ClientboundPowerupPacket), ClientboundPowerupPacket::create, L"Powerup");
    map(200, true, false, true, false, typeid(ClientboundAwardStatPacket), ClientboundAwardStatPacket::create,
        L"AwardStat");
    map(201, true, true, false, false, typeid(PlayerInfoPacket), PlayerInfoPacket::create, L"PlayerInfo");
    map(202, true, true, true, false, typeid(ClientboundPlayerAbilitiesPacket),
        ClientboundPlayerAbilitiesPacket::create, L"PlayerAbilities");
    map(205, false, true, true, false, typeid(ServerboundClientCommandPacket), ServerboundClientCommandPacket::create,
        L"ClientCommand");
    map(206, true, false, true, false, typeid(ScoreboardPacket), ScoreboardPacket::create, L"Scoreboard");
    map(209, true, true, false, false, typeid(ClientboundSetPlayerTeamPacket), ClientboundSetPlayerTeamPacket::create,
        L"SetPlayerTeam");
    map(210, true, true, true, false, typeid(GameModePacket), GameModePacket::create, L"GameMode");
    map(211, true, false, true, false, typeid(ClientboundMapItemDataPacket), ClientboundMapItemDataPacket::create,
        L"MapItemData");
    map(212, true, false, true, false, typeid(ClientboundOpenSignEditorPacket), ClientboundOpenSignEditorPacket::create,
        L"OpenSignEditor");
    map(213, true, false, true, false, typeid(ClientboundPlayerCombatPacket), ClientboundPlayerCombatPacket::create,
        L"PlayerCombat");
    map(214, true, false, true, false, typeid(ClientboundPlayerPositionPacket), ClientboundPlayerPositionPacket::create,
        L"PlayerPosition");
    map(216, true, false, false, false, typeid(ClientboundResourcePackPacket), ClientboundResourcePackPacket::create,
        L"ResourcePack");
    map(217, true, false, true, false, typeid(ClientboundSetBorderPacket), ClientboundSetBorderPacket::create,
        L"SetBorder");
    map(218, true, false, false, false, typeid(ClientboundSetCameraPacket), ClientboundSetCameraPacket::create,
        L"SetCamera");
    map(219, true, false, true, false, typeid(ClientboundSetTitlesPacket), ClientboundSetTitlesPacket::create,
        L"SetTitles");
    map(220, true, false, true, false, typeid(ClientboundTabListPacket), ClientboundTabListPacket::create, L"TabList");
    map(221, false, true, false, false, typeid(ServerboundChatAutoCompletePacket),
        ServerboundChatAutoCompletePacket::create, L"ChatAutoComplete");
    map(222, false, true, false, false, typeid(ServerboundChatPacket), ServerboundChatPacket::create, L"Chat");
    map(223, false, true, false, false, typeid(ServerboundContainerAckPacket), ServerboundContainerAckPacket::create,
        L"ContainerAck");
    map(224, false, true, false, false, typeid(ServerboundContainerClosePacket),
        ServerboundContainerClosePacket::create, L"ContainerClose");
    map(225, false, true, false, false, typeid(ServerboundCustomPayloadPacket), ServerboundCustomPayloadPacket::create,
        L"CustomPayload");
    map(226, false, true, false, false, typeid(ServerboundKeepAlivePacket), ServerboundKeepAlivePacket::create,
        L"KeepAlive");
    map(227, false, true, false, false, typeid(ServerboundPlayerAbilitiesPacket),
        ServerboundPlayerAbilitiesPacket::create, L"PlayerAbilities");
    map(228, false, true, false, false, typeid(ServerboundResourcePackPacket), ServerboundResourcePackPacket::create,
        L"ResourcePack");
    map(229, false, true, false, false, typeid(ServerboundSetCarriedItemPacket),
        ServerboundSetCarriedItemPacket::create, L"SetCarriedItem");
    map(230, false, true, false, false, typeid(ServerboundSignUpdatePacket), ServerboundSignUpdatePacket::create,
        L"SignUpdate");
    map(231, false, true, false, false, typeid(ServerboundTeleportToEntityPacket),
        ServerboundTeleportToEntityPacket::create, L"TeleportToEntity");
    map(232, false, true, false, false, typeid(ServerboundPreLoginPacket), ServerboundPreLoginPacket::create,
        L"PreLogin");
    map(233, false, true, false, false, typeid(ServerboundSwingPacket), ServerboundSwingPacket::create, L"Swing");
    map(234, true, false, true, true, typeid(ClientboundDamageIndicatorPacket),
        ClientboundDamageIndicatorPacket::create, L"DamageIndicator");
    map(235, true, false, false, true, typeid(ClientboundBlockCollectionDestructionPacket),
        ClientboundBlockCollectionDestructionPacket::create, L"BlockCollectionDestruction");
    map(236, true, false, true, false, typeid(ClientboundCooldownPacket), ClientboundCooldownPacket::create,
        L"Cooldown");
    map(239, true, false, true, false, typeid(ClientboundMGPlayerSettingsUpdatePacket),
        ClientboundMGPlayerSettingsUpdatePacket::create, L"MiniGameSettingsUpdate");
    map(250, true, true, true, false, typeid(ClientboundCustomPayloadPacket), ClientboundCustomPayloadPacket::create,
        L"CustomPayload");
    map(253, true, false, false, false, typeid(ClientboundChangeDifficultyPacket),
        ClientboundChangeDifficultyPacket::create, L"ChangeDifficulty");
    map(254, false, true, false, false, typeid(GetInfoPacket), GetInfoPacket::create, L"GetInfo");
    map(255, true, true, true, false, typeid(DisconnectPacket), DisconnectPacket::create, L"Disconnect");
}

boost::shared_ptr<Packet> Packet::getPacket(int packetId)
{
    return sPacketFactories[packetId]();
}

bool Packet::canSendToAnyClient(boost::shared_ptr<Packet> packet)
{
    return sSendToAnyClientPacketIds.count(packet->getPacketId()) != 0;
}

boost::shared_ptr<Packet> Packet::readPacket(DataInputStream* input, bool isServer, unsigned int& packetsRead,
                                             bool& protocolError)
#ifdef MINECRAFT_PC
{
    return readPacket(input, isServer, packetsRead, protocolError, false);
}

boost::shared_ptr<Packet> Packet::readPacket(DataInputStream* input, bool isServer, unsigned int& packetsRead,
                                             bool& protocolError, bool inGame)
#endif
{
    int packetId = input->read();
    if (packetId == -1)
    {
        return boost::shared_ptr<Packet>();
    }

    if (isServer ? sServerboundPacketIds.find(packetId) == sServerboundPacketIds.end()
                 : sClientboundPacketIds.find(packetId) == sClientboundPacketIds.end())
    {
        protocolError = true;
#ifdef MINECRAFT_PC
        if (isServer)
        {
            printf("[pc] net: a player sent invalid data (packet id %d); disconnecting only that player\n", packetId);
            fflush(stdout);
            return boost::shared_ptr<Packet>();
        }
#endif
        gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_ConnectionLost, NULL);
        return boost::shared_ptr<Packet>();
    }
#ifdef MINECRAFT_PC
    if (isServer && inGame && (packetId == 1 || packetId == 232))
    {
        protocolError = true;
        printf("[pc] net: a player in the game sent a login packet (id %d), so their data is out of step; "
               "disconnecting only that player\n", packetId);
        fflush(stdout);
        return boost::shared_ptr<Packet>();
    }
#endif

    boost::shared_ptr<Packet> packet;
    packet = getPacket(packetId);
    packet->read(input);
    ++packetsRead;

    return packet;
}

void Packet::writePacket(boost::shared_ptr<Packet> packet, DataOutputStream* output, bool isServer,
                         unsigned int& packetsWritten)
{
    output->write(packet->getPacketId());
    packet->write(output);
    ++packetsWritten;
}

void Packet::writeUtf(const std::wstring& text, DataOutputStream* output)
{
    output->writeShort((short)text.size());
    output->writeChars(text);
}

std::wstring Packet::readUtf(DataInputStream* input, int maxLength)
{
    short length = input->readShort();
    if (length > maxLength)
    {
        std::wstringstream message;
        message << L"Received string length longer than maximum allowed (" << length << " > " << maxLength << ")";
    }

    std::wstring text = L"";
    for (int i = 0; i < length; ++i)
    {
        text.insert(text.end(), input->readChar());
    }

    return text;
}

void Packet::handle(PacketListener* listener)
{
}

int Packet::getEstimatedSize()
{
    return 8;
}

bool Packet::canBeInvalidated()
{
    return false;
}

bool Packet::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    return false;
}

bool Packet::isAync()
{
    return false;
}

bool Packet::tryReplaceDuplicatePacket(
    std::deque<boost::shared_ptr<Packet>, std::allocator<boost::shared_ptr<Packet> > >* queuedPackets)
{
    return false;
}

uint32_t packInt(int32_t value, uint32_t offset, uint32_t width)
{
    uint32_t mask = static_cast<uint32_t>(getMaskInt(offset, width));
    return (static_cast<uint32_t>(value) << (offset & 0x3f)) & mask;
}

int32_t unpackSigned(uint32_t value, uint32_t offset, uint32_t width)
{
    return static_cast<int32_t>(value << ((32 - width) - offset)) >> (32 - width);
}

uint32_t unpackUnsigned(uint32_t value, uint32_t offset, uint32_t width)
{
    uint32_t mask = static_cast<uint32_t>(getMaskInt(offset, width));
    return (value >> offset) & mask;
}
