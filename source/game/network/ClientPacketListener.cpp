#include "game/network/ClientPacketListener.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModHooks.hpp"
#endif
#ifdef MINECRAFT_PC
#include "platform/pc/PcNativeMod.h"
#endif
#include "game/core/StringConversion.hpp"
#include "game/core/StringIds.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"
#include "game/network/Connection.hpp"
#include "game/core/Socket.hpp"
#include "game/data/SavedDataMemoryStorage.hpp"
#include "game/network/ClientboundRespawnPacket.hpp"
#include "game/network/ClientboundSetTimePacket.hpp"
#include "game/network/ClientboundSetCarriedItemPacket.hpp"
#include "game/network/ClientboundRemoveEntitiesPacket.hpp"
#include "game/network/ClientboundRotateHeadPacket.hpp"
#include "game/network/ClientboundBlockEventPacket.hpp"
#include "game/network/ClientboundSetEquippedItemPacket.hpp"
#include "game/network/ClientboundAddExperienceOrbPacket.hpp"
#include "game/network/ClientboundPowerupPacket.hpp"
#include "game/network/ClientboundBlockCollectionDestructionPacket.hpp"
#include "game/network/ClientboundAddEntityPacket.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/core/Boat.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/core/EyeOfEnderSignal.hpp"
#include "game/core/ThrownExpBottle.hpp"
#include "game/entity/mob/LeashFenceKnotEntity.hpp"
#include "game/core/ArmorStand.hpp"
#include "game/core/EvocationFangs.hpp"
#include "game/entity/projectile/SpectralArrow.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/core/Snowball.hpp"
#include "game/core/ThrownEgg.hpp"
#include "game/entity/projectile/LargeFireball.hpp"
#include "game/entity/projectile/SmallFireball.hpp"
#include "game/core/ThrownEnderpearl.hpp"
#include "game/core/WitherSkull.hpp"
#include "game/core/ShulkerBullet.hpp"
#include "game/core/LlamaSpit.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Item.hpp"
#include "game/network/ClientboundBlockDestructionPacket.hpp"
#include "game/network/ClientboundContainerClosePacket.hpp"
#include "game/network/ClientboundSetBorderPacket.hpp"
#include "game/network/ClientboundChangeDifficultyPacket.hpp"
#include "game/network/ServerSettingsChangedPacket.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/world/Dimension.hpp"
#include "game/core/User.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/DLCPack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/core/TrialMode.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/item/Items.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/network/ClientboundDamageIndicatorPacket.hpp"
#include "game/network/ClientboundSetExperiencePacket.hpp"
#include "game/network/ClientboundCooldownPacket.hpp"
#include "game/network/ClientboundSetEntityMotionPacket.hpp"
#include "game/network/ClientboundMoveEntityPacket.hpp"
#include "game/network/ClientboundBlockUpdatePacket.hpp"
#include "game/network/ClientboundLevelEventPacket.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/ClientboundSetCameraPacket.hpp"
#include "game/network/ClientboundSetHealthPacket.hpp"
#include "game/network/ClientboundContainerSetDataPacket.hpp"
#include "game/network/BlockRegionUpdatePacket.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/core/Gui.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Stats.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/util/StringUtils.hpp"
#include "game/network/ClientboundAwardStatPacket.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/Voteable.hpp"
#include "game/core/Voteables.hpp"
#include "game/network/VotePacket.hpp"
#include "game/network/UpdateProgressPacket.hpp"
#include "game/network/PlayerReadyPacket.hpp"
#include "game/network/ChunkVisibilityPacket.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/CauldronBlockEntity.hpp"
#include "game/world/block/ConduitBlockEntity.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/core/Mth.hpp"
#include "game/world/LevelSettings.hpp"
#include "vendor/_ConnectionProgressParams.hpp"
#include "game/world/LevelType.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/core/ClientSideMerchant.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/GameModeTransitionParams.hpp"
#include "game/core/MOJANG_DATA.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/MapRenderer.hpp"
#include "game/core/Socket.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRules.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/generation/LevelGenerators.hpp"
#include "game/world/generation/RemotePlayer.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/core/Guardian.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/entity/projectile/FishingHook.hpp"
#include "game/entity/projectile/DragonFireball.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/item/ItemId.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/client/render/ItemPickupParticle.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/client/audio/FollowPlayerSoundInstance.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/world/chunk/ClientChunkCache.hpp"
#include "game/client/audio/GuardianAttackSoundInstance.hpp"
#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/ui/LocalMenuContainer.hpp"
#include "game/world/block/LocalBlockInteraction.hpp"
#include "game/entity/mob/HorseChest.hpp"
#ifdef MINECRAFT_PC
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "platform/pc/PcSettings.hpp"
#endif
#include "game/network/ChunkVisibilityAreaPacket.hpp"
#include "game/network/ClientboundAddGlobalEntityPacket.hpp"
#include "game/network/ClientboundAddMobPacket.hpp"
#include "game/network/ClientboundAddPaintingPacket.hpp"
#include "game/network/ClientboundAddPlayerPacket.hpp"
#include "game/network/ClientboundAnimatePacket.hpp"
#include "game/network/ClientboundBossEventPacket.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ClientboundChunkBlocksUpdatePacket.hpp"
#include "game/network/ClientboundContainerAckPacket.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/network/ClientboundContainerSetContentPacket.hpp"
#include "game/network/ClientboundContainerSetSlotPacket.hpp"
#include "game/network/ClientboundCustomPayloadPacket.hpp"
#include "game/network/ClientboundEntityEventPacket.hpp"
#include "game/network/ClientboundExplodePacket.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ClientboundKeepAlivePacket.hpp"
#include "game/network/ClientboundLevelParticlesPacket.hpp"
#include "game/network/ClientboundLoginPacket.hpp"
#include "game/network/ClientboundMapItemDataPacket.hpp"
#include "game/network/ClientboundMGPlayerSettingsUpdatePacket.hpp"
#include "game/network/ClientboundMoveVehiclePacket.hpp"
#include "game/network/ClientboundOpenSignEditorPacket.hpp"
#include "game/network/ClientboundPlayerCombatPacket.hpp"
#include "game/network/ClientboundPlayerPositionPacket.hpp"
#include "game/network/ClientboundPlayerSleepPacket.hpp"
#include "game/network/ClientboundPreLoginPacket.hpp"
#include "game/network/ClientboundRemoveMobEffectPacket.hpp"
#include "game/network/ClientboundSetEntityDataPacket.hpp"
#include "game/network/ClientboundSetEntityLinkPacket.hpp"
#include "game/network/ClientboundSetPassengersPacket.hpp"
#include "game/network/ClientboundSetPlayerTeamPacket.hpp"
#include "game/network/ClientboundSetSpawnPositionPacket.hpp"
#include "game/network/ClientboundSetTitlesPacket.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/ClientboundTabListPacket.hpp"
#include "game/network/ClientboundTakeItemEntityPacket.hpp"
#include "game/network/ClientboundTeleportEntityPacket.hpp"
#include "game/network/ClientboundUpdateAttributesPacket.hpp"
#include "game/network/ClientboundUpdateMobEffectPacket.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/network/ScoreboardPacket.hpp"
#include "game/network/UpdateGameRuleProgressPacket.hpp"
#include "game/network/XZPacket.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/network/ServerboundMoveVehiclePacket.hpp"
#include "game/network/ServerboundContainerClosePacket.hpp"
#include "game/network/TexturePacket.hpp"
#include "game/network/TextureChangePacket.hpp"
#include "game/network/TextureAndGeometryPacket.hpp"
#include "game/network/TextureAndGeometryChangePacket.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/MapSelectInfoPacket.hpp"
#include "game/network/MoveEntityPacketSmall.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/LevelData.hpp"
#include "game/core/Slime.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/SimpleContainer.hpp"
#include "game/item/Container.hpp"
#include "game/network/ServerboundAcceptTeleportationPacket.hpp"
#include "game/network/ServerboundMovePlayerPacket.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/data/McRegionLevelStorage.hpp"
#include "game/data/ConsoleSaveFileReadOnly.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/AreaConstraint.hpp"
#include "game/core/RegionFileCache.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/UIScene_InGameSettingsListMenu.hpp"
#include "game/core/DLCSkinFile.hpp"
#include "game/core/DLCManager.hpp"
#include "game/ui/UIScene_GameModeScoreboardMenu.hpp"
#include "game/world/generation/MiniGamePlayerSettings.hpp"
#include "game/core/Painting.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/core/Explosion.hpp"
#include "game/network/ServerboundContainerAckPacket.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/ui/UIScene_TradingMenu.hpp"
#include "game/ui/MerchantMenu.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/RangedAttribute.hpp"
#include "game/core/System.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/core/Team.hpp"
#include "game/core/File.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include <cfloat>
#include <cmath>

namespace
{
}

namespace
{
void replaceFirst(std::wstring& value, const std::wstring& key, const std::wstring& replacement)
{
    const std::wstring::size_type pos = value.find(key);
    if (pos != std::wstring::npos)
        value.replace(pos, key.size(), replacement);
}

}

OSFastMutex ClientPacketListener::s_deferredLinkMutex;
std::vector<ClientPacketListener::DeferredEntityLinkPacket> ClientPacketListener::s_deferredLinks;

ClientPacketListener::ClientPacketListener(Minecraft* minecraft, Socket* socket, int playerId)
    : m_closed(false),
      m_connection(0),
      mRemoteAddress(L""),
      m_createdOk(false),
      m_minecraft(minecraft),
      m_level(0),
      m_started(false),
      m_suppressStartOnPosition(false),
      m_maxPlayers(20),
      m_random(),
      m_destroyedBlocks(0),
      m_destroyedBlockCount(0),
      m_playerId(playerId),
      m_savedDataStorage(0)
{
    InitializeCriticalSection(&m_mutex);
    m_savedDataStorage = new SavedDataMemoryStorage();
    if (playerId < 0)
        m_playerId = gProfile->GetPrimaryPad();
    if (socket == 0)
        socket = new Socket();
    m_createdOk = socket->mOpen;
    if (m_createdOk)
        m_connection = new Connection(socket, std::wstring(L"Client"), this);
    else
        m_connection = 0;
}

ClientPacketListener::~ClientPacketListener()
{
    delete m_connection;
    m_connection = 0;
    delete m_savedDataStorage;
    m_savedDataStorage = 0;
    DeleteCriticalSection(&m_mutex);
}

void ClientPacketListener::handleBlockEntityDataInternal(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet,
                                                         Level* level)
{
    if (level == NULL || packet == NULL)
    {
        return;
    }

    BlockPos pos = packet->getPos();
    if (!level->hasChunkAt(pos))
    {
        return;
    }

    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity == NULL)
    {
        return;
    }

    const int type = packet->getType();
    CompoundTag* tag = packet->getTag();

#ifdef MINECRAFT_PC
    if (type == PC_NATIVE_BLOCK_ENTITY_PACKET)
    {
        if (PcModHooksIsModType(blockEntity->GetType()))
            blockEntity->load(tag);
        return;
    }
#endif
    if (type == 0xc)
    {
        if (eTYPE_DERIVED_FROM(BlockEntityType::CAULDRON, blockEntity->GetType()))
        {
            boost::shared_ptr<CauldronBlockEntity> cauldron = boost::static_pointer_cast<CauldronBlockEntity>(
                blockEntity);
            cauldron->loadFromPacket(level, pos, tag);
        }
        return;
    }

    if (type == 0xd)
    {
        if (eTYPE_DERIVED_FROM(BlockEntityType::CONDUIT, blockEntity->GetType()))
        {
            boost::shared_ptr<ConduitBlockEntity> conduit = boost::static_pointer_cast<ConduitBlockEntity>(blockEntity);
            conduit->loadFromPacket(level, pos, tag);
        }
        return;
    }

    bool loadTag = false;
    switch (type)
    {
    case 0:
        break;
    case 1:
        loadTag = eTYPE_DERIVED_FROM(BlockEntityType::MOB_SPAWNER, blockEntity->GetType());
        break;
    case 2:
        loadTag = eTYPE_DERIVED_FROM(BlockEntityType::COMMAND, blockEntity->GetType());
        break;
    case 3:
        loadTag = eTYPE_DERIVED_FROM(BlockEntityType::BEACON, blockEntity->GetType());
        break;
    case 4:
        loadTag = eTYPE_DERIVED_FROM(BlockEntityType::SKULL, blockEntity->GetType());
        break;
    case 5:
        loadTag = eTYPE_DERIVED_FROM(BlockEntityType::FLOWER_POT, blockEntity->GetType());
        break;
    case 6:
        loadTag = eTYPE_DERIVED_FROM(BlockEntityType::BANNER, blockEntity->GetType());
        break;
    case 8:
        loadTag = eTYPE_DERIVED_FROM(BlockEntityType::THE_END_GATEWAY_ENTITY, blockEntity->GetType());
        break;
    case 9:
        loadTag = eTYPE_DERIVED_FROM(BlockEntityType::SIGN, blockEntity->GetType());
        break;
    case 10:
        loadTag = eTYPE_DERIVED_FROM(BlockEntityType::SHULKER_BOX, blockEntity->GetType());
        break;
    case 11:
        loadTag = eTYPE_DERIVED_FROM(BlockEntityType::BED, blockEntity->GetType());
        break;
    default:
        break;
    }

    if (loadTag)
    {
        blockEntity->load(tag);
    }
}

uint8_t ClientPacketListener::isStarted()
{
    return m_started ? 1 : 0;
}
uint8_t ClientPacketListener::isClosed()
{
    return m_closed ? 1 : 0;
}
Socket* ClientPacketListener::getSocket()
{
    return m_connection->m_socket;
}
uint8_t ClientPacketListener::getAndSetClosed()
{
    EnterCriticalSection(&m_mutex);
    const uint8_t wasClosed = m_closed ? 1 : 0;
    m_closed = true;
    LeaveCriticalSection(&m_mutex);
    return wasClosed;
}

void ClientPacketListener::tick()
{
    if (!m_closed)
        m_connection->tick();
    m_connection->flush();
}

INetworkPlayer* ClientPacketListener::getNetworkPlayer()
{
    if (m_connection != 0)
    {
        Socket* socket = getSocket();
        if (socket != 0)
            return socket->getPlayer();
    }
    return 0;
}

bool ClientPacketListener::IsPrimaryPlayer()
{
    return m_playerId == gProfile->GetPrimaryPad();
}

void ClientPacketListener::displayPrivilegeChanges(boost::shared_ptr<MultiplayerLocalPlayer> player, uint32_t changes)
{
    const uint32_t userIndex = player->mUserIndex;
    const uint32_t oldPrivileges = player->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All);
    for (uint32_t bit = 0; bit < 0x20; ++bit)
    {
        const uint32_t oldValue = Player::getPlayerGamePrivilege(oldPrivileges,
                                                                 static_cast<Player::EPlayerGamePrivileges>(bit));
        const uint32_t newValue = Player::getPlayerGamePrivilege(changes,
                                                                 static_cast<Player::EPlayerGamePrivileges>(bit));
        if (oldValue == newValue)
            continue;
        const bool granted = oldValue == 0;
        std::wstring message;
        if (gApp.GetGameHostOption(eGameHostOption_TrustPlayers) == 0)
        {
            switch (bit)
            {
            case 0:
                message = L10N::GetString(granted ? IDS_PRIV_MINE_TOGGLE_OFF : IDS_PRIV_MINE_TOGGLE_ON);
                break;
            case 1:
                message = L10N::GetString(granted ? IDS_PRIV_BUILD_TOGGLE_OFF : IDS_PRIV_BUILD_TOGGLE_ON);
                break;
            case 2:
                message = L10N::GetString(granted ? IDS_PRIV_ATTACK_MOB_TOGGLE_OFF : IDS_PRIV_ATTACK_MOB_TOGGLE_ON);
                break;
            case 3:
                message = L10N::GetString(granted ? IDS_PRIV_ATTACK_PLAYER_TOGGLE_OFF
                                                  : IDS_PRIV_ATTACK_PLAYER_TOGGLE_ON);
                break;
            case 10:
                message = L10N::GetString(granted ? IDS_PRIV_ATTACK_ANIMAL_TOGGLE_OFF
                                                  : IDS_PRIV_ATTACK_ANIMAL_TOGGLE_ON);
                break;
            case 11:
                message = L10N::GetString(granted ? IDS_PRIV_USE_DOORS_TOGGLE_OFF : IDS_PRIV_USE_DOORS_TOGGLE_ON);
                break;
            case 12:
                message = L10N::GetString(granted ? IDS_PRIV_USE_CONTAINERS_TOGGLE_OFF
                                                  : IDS_PRIV_USE_CONTAINERS_TOGGLE_ON);
                break;
            default:
                break;
            }
        }
        if (bit == 4)
            message = L10N::GetString(granted ? IDS_PRIV_MODERATOR_TOGGLE_OFF : IDS_PRIV_MODERATOR_TOGGLE_ON);
        if (gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0)
        {
            switch (bit)
            {
            case 5:
                message = L10N::GetString(granted ? IDS_PRIV_FLY_TOGGLE_OFF : IDS_PRIV_FLY_TOGGLE_ON);
                break;
            case 6:
                message = L10N::GetString(granted ? IDS_PRIV_EXHAUSTION_TOGGLE_OFF : IDS_PRIV_EXHAUSTION_TOGGLE_ON);
                break;
            case 7:
                message = L10N::GetString(granted ? IDS_PRIV_INVISIBLE_TOGGLE_OFF : IDS_PRIV_INVISIBLE_TOGGLE_ON);
                break;
            case 8:
                message = L10N::GetString(granted ? IDS_PRIV_INVULNERABLE_TOGGLE_OFF : IDS_PRIV_INVULNERABLE_TOGGLE_ON);
                break;
            case 13:
                message = L10N::GetString(granted ? IDS_PRIV_CAN_INVISIBLE_TOGGLE_OFF
                                                  : IDS_PRIV_CAN_INVISIBLE_TOGGLE_ON);
                break;
            case 14:
                message = L10N::GetString(granted ? IDS_PRIV_CAN_FLY_TOGGLE_OFF : IDS_PRIV_CAN_FLY_TOGGLE_ON);
                break;
            case 15:
                message = L10N::GetString(granted ? IDS_PRIV_CAN_EXHAUSTION_TOGGLE_OFF
                                                  : IDS_PRIV_CAN_EXHAUSTION_TOGGLE_ON);
                break;
            case 16:
                message = L10N::GetString(granted ? IDS_PRIV_CAN_TELEPORT_TOGGLE_OFF : IDS_PRIV_CAN_TELEPORT_TOGGLE_ON);
                break;
            default:
                break;
            }
        }
        if (!message.empty())
            m_minecraft->mGui->addMessage(message, userIndex, false, false, false, false);
    }
}

#ifdef MINECRAFT_PC
void ClientPacketListener::SendViewDistance()
{
    if (m_closed)
        return;
    ByteArrayOutputStream bytes;
    DataOutputStream output(&bytes);
    output.writeInt(PcSettingsRenderDistance());
    send(boost::shared_ptr<Packet>(
        new ServerboundCustomPayloadPacket(ServerGamePacketListenerImpl::VIEW_DISTANCE_PACKET, bytes.toByteArray())));
}

void ClientPacketListener::SendViewDistanceToAll()
{
    Minecraft* mc = Minecraft::GetInstance();
    for (int i = 0; i < 4; ++i)
    {
        if (mc->mLocalPlayers[i] == NULL)
            continue;
        ClientPacketListener* conn = mc->getConnection(i);
        if (conn != NULL)
            conn->SendViewDistance();
    }
}
#endif

void ClientPacketListener::handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet)
{
    if (m_closed)
        return;

    PlayerUID suid(0);
    gProfile->GetXUID(m_playerId, &suid, true);
    MOJANG_DATA* mojangData = 0;
    if (!CGameNetworkManager::sInstance.IsLocalGame())
        mojangData = gApp.GetMojangDataForXuid(suid);

    if (!CGameNetworkManager::sInstance.IsHost())
        Minecraft::GetInstance()->mProgressRenderer->progressStagePercentage(75);

    INetworkPlayer* networkPlayer = m_connection->m_socket->getPlayer();
    int userIndex = -1;
    if (IsPrimaryPlayer())
    {
        userIndex = m_playerId;
        telemetryManagerInstance->SetMultiplayerInstanceId(packet->mChunkDistance);
    }
    else if (!networkPlayer->IsGuest() && networkPlayer->IsLocal())
    {
        for (int i = 0; i < 4; ++i)
        {
            if (CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(i) == networkPlayer)
                userIndex = i;
        }
    }

    if (userIndex != -1)
    {
        if (mojangData == 0)
            gApp.SetBanListCheck(userIndex, false);
        else
        {
            if (mojangData->skinPath[0] != 0)
            {
                std::wstring skinPath = mojangData->skinPath;
                if (gApp.IsFileInMemoryTextures(skinPath))
                    gApp.AddMemoryTextureFile(skinPath, 0, 0);
            }
            if (mojangData->capePath[0] != 0)
            {
                std::wstring capePath = mojangData->capePath;
                if (gApp.IsFileInMemoryTextures(capePath))
                    gApp.AddMemoryTextureFile(capePath, 0, 0);
            }
            gApp.SetBanListCheck(userIndex, false);
        }
    }

    if (IsPrimaryPlayer())
    {
        if (GameType::byId(packet->mGameType) == GameType::LOBBY)
            m_minecraft->mGameMode = new MultiPlayerGameMode(gProfile->GetPrimaryPad(), this, packet->mDifficulty,
                                                             false, true);
        else if (gApp.GetTutorialMode(boost::shared_ptr<Player>()))
            m_minecraft->mGameMode = new MultiPlayerGameMode(gProfile->GetPrimaryPad(), this, packet->mDifficulty, true,
                                                             false);
        else if (!gProfile->IsFullVersion())
            m_minecraft->mGameMode = new TrialMode(gProfile->GetPrimaryPad(), this, packet->mDifficulty);
        else
        {
            MemSect(13);
            m_minecraft->mGameMode = new MultiPlayerGameMode(gProfile->GetPrimaryPad(), this, packet->mDifficulty,
                                                             false, false);
            MemSect(0);
        }
        m_minecraft->GetMiniGame()->SetDisabledEditActions(false);

        if (m_minecraft->getLevel(packet->mGameMode) == 0)
        {
            if (Minecraft::InMiniGame(eMiniGameId_Any, true) && !CGameNetworkManager::sInstance.IsHost())
            {
                std::vector<LevelGenerationOptions*>* generators = gApp.getLevelGenerators();
                for (std::vector<LevelGenerationOptions*>::iterator it = generators->begin(); it != generators->end();
                     ++it)
                {
                    LevelGenerationOptions* options = *it;
                    if (options->isGameModeLobby())
                    {
                        File lobbySave(options->getBaseSaveRoot() + options->getBaseSavePath());
                        options->setBaseSaveData(lobbySave);
                        gApp.setLevelGenerationOptions(options);
                    }
                }
                ConsoleSaveFileReadOnly* saveFile = new ConsoleSaveFileReadOnly(L"", 0, 0, false, kLocalSavePlatform);
                boost::shared_ptr<McRegionLevelStorage> storage(new McRegionLevelStorage(
                    saveFile, File(L"."), L"world", true, Minecraft::GetInstance()->getFixerUpper()));
                m_level = new MultiPlayerLevel(
                    storage, this,
                    new LevelSettings(packet->mSeed, GameType::byId(packet->mGameType), false, false, packet->mIsFlat,
                                      packet->mLevelType, packet->mXZSize, packet->mHellScale, packet->mMaxTick,
                                      packet->mIsFlatWorld, packet->mPlayerFlags, packet->mSpawnProtection,
                                      packet->mSimulationDistance),
                    packet->mGameMode, Difficulty::byId(packet->mPlayerFlag), packet->mUnknownValue3);
                gApp.setLevelGenerationOptions(0);
            }
            else
            {
                m_level = new MultiPlayerLevel(
                    this,
                    new LevelSettings(packet->mSeed, GameType::byId(packet->mGameType), false, false, packet->mIsFlat,
                                      packet->mLevelType, packet->mXZSize, packet->mHellScale, packet->mMaxTick,
                                      packet->mIsFlatWorld, packet->mPlayerFlags, packet->mSpawnProtection,
                                      packet->mSimulationDistance),
                    packet->mGameMode, Difficulty::byId(packet->mPlayerFlag), packet->mUnknownValue3);
            }

            MultiPlayerLevel* otherLevel = m_minecraft->getLevel(packet->mGameMode == 0 ? -1 : 0);
            if (otherLevel == 0)
            {
                if (packet->mGameMode == 0 || packet->mGameMode == -1)
                    otherLevel = m_minecraft->getLevel(1);
                else
                    otherLevel = m_minecraft->getLevel(-1);
            }
            if (otherLevel == 0)
                m_level->setSavedDataStorage(m_savedDataStorage);
            else
                m_level->setSavedDataStorage(otherLevel->getSavedDataStorage());
            m_level->mIsClientSide = true;
            m_minecraft->setLevel(m_level, -1, boost::shared_ptr<Player>(), true, false);
        }

        m_minecraft->mPlayer->mPlayerIndex = packet->mReducedDebugInfo;
        m_minecraft->mPlayer->setCustomSkin(GameSettings::GetPlayerSkinId(m_playerId));
        m_minecraft->mPlayer->setCustomCape(GameSettings::GetPlayerCapeId(m_playerId));
        m_minecraft->createPrimaryLocalPlayer(gProfile->GetPrimaryPad());
        m_minecraft->mPlayer->mDimension = packet->mGameMode;
        m_minecraft->mPlayer->setId(packet->mEntityId);
        gApp.UpdatePlayerInfo(getSocket()->mPlayerId, packet->mReducedDebugInfo, packet->mCurrentTick,
                              packet->mGameType);
        m_minecraft->mPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All, packet->mCurrentTick);
        uint32_t privileges = 0;
        Player::enableAllPlayerPrivileges(privileges, true);
        if (networkPlayer->IsHost())
            Player::setPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_Moderator, 1);
        displayPrivilegeChanges(m_minecraft->mPlayer, privileges);
        GameSettings::SetGameSettingsDebugMask(gProfile->GetPrimaryPad(),
                                               GameSettings::GetGameSettingsDebugMask(-1, true));
    }
    else
    {
        m_level = m_minecraft->getLevel(packet->mGameMode);
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer;
        if (m_level == 0)
        {
            MultiPlayerLevel* otherLevel = m_minecraft->getLevel(packet->mGameMode == 0 ? -1 : 0);
            if (otherLevel == 0)
            {
                if (packet->mGameMode == 0 || packet->mGameMode == -1)
                    otherLevel = m_minecraft->getLevel(1);
                else
                    otherLevel = m_minecraft->getLevel(-1);
            }
            m_level = new MultiPlayerLevel(
                this,
                new LevelSettings(packet->mSeed, GameType::byId(packet->mGameType), false, false, packet->mIsFlat,
                                  packet->mLevelType, packet->mXZSize, packet->mHellScale, packet->mMaxTick,
                                  packet->mIsFlatWorld, packet->mPlayerFlags, packet->mSpawnProtection,
                                  packet->mSimulationDistance),
                packet->mGameMode, Difficulty::byId(packet->mPlayerFlag), packet->mUnknownValue3);
            m_level->setSavedDataStorage(otherLevel->getSavedDataStorage());
            m_level->mIsClientSide = true;
            std::wstring name = networkPlayer->GetOnlineName();
            localPlayer = m_minecraft->createExtraLocalPlayer(m_playerId, name, m_playerId, packet->mGameMode, this,
                                                              m_level, packet->mGameType, packet->mDifficulty);
            boost::shared_ptr<MultiplayerLocalPlayer> currentPlayer = m_minecraft->mPlayer;
            m_minecraft->mPlayer = m_minecraft->mLocalPlayers[m_playerId];
            m_minecraft->setLevel(m_level, -1, boost::shared_ptr<Player>(), true, false);
            m_minecraft->mPlayer = currentPlayer;
        }
        else
        {
            std::wstring name = networkPlayer->GetOnlineName();
            localPlayer = m_minecraft->createExtraLocalPlayer(m_playerId, name, m_playerId, packet->mGameMode, this, 0,
                                                              packet->mGameType, packet->mDifficulty);
        }

        localPlayer->mDimension = packet->mGameMode;
        localPlayer->setId(packet->mEntityId);
        localPlayer->mPlayerIndex = packet->mReducedDebugInfo;
        localPlayer->setCustomSkin(GameSettings::GetPlayerSkinId(m_playerId));
        localPlayer->setCustomCape(GameSettings::GetPlayerCapeId(m_playerId));
        gApp.UpdatePlayerInfo(getSocket()->mPlayerId, packet->mReducedDebugInfo, packet->mCurrentTick,
                              packet->mGameType);
        localPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All, packet->mCurrentTick);
        uint32_t privileges = 0;
        Player::enableAllPlayerPrivileges(privileges, true);
        displayPrivilegeChanges(m_minecraft->mLocalPlayers[m_playerId], privileges);
    }
    m_maxPlayers = packet->mMaxPlayers;
    boost::shared_ptr<MultiplayerLocalPlayer> currentPlayer = m_minecraft->mPlayer;
    m_minecraft->mPlayer = m_minecraft->mLocalPlayers[m_playerId];
    m_minecraft->mGameModes[m_playerId]->setLocalMode(GameType::byId(packet->mGameType));
    currentPlayer->UpdateSize();
    m_minecraft->mPlayer = currentPlayer;
    m_minecraft->mPlayer->updateRichPresence();
    if (userIndex != -1)
        gConsoleUIController.UpdateSelectedItemPos(userIndex);

    gApp.SetFirstMiniGameType(static_cast<EMiniGameId>(packet->mDifficulty));
    m_minecraft->GetClientMasterGameMode()->Setup(static_cast<EMiniGameId>(packet->mDifficulty));
    telemetryManagerInstance->RecordLevelStart(
        m_playerId, eSen_FriendOrMatch_Friend, eSen_CompeteOrCoop_Both,
        Minecraft::GetInstance()->getLevel(packet->mGameMode)->getDifficulty()->getId(), gApp.GetLocalPlayerCount(),
        CGameNetworkManager::sInstance.GetOnlinePlayerCount());
    telemetryManagerInstance->RecordMiniGameHostOptions(m_playerId, static_cast<EMiniGameId>(packet->mDifficulty));
#ifdef MINECRAFT_PC
    SendViewDistance();
#endif
}
boost::shared_ptr<Entity> ClientPacketListener::getEntity(int entityId)
{
    for (int i = 0; i < 4; ++i)
    {
        boost::shared_ptr<Entity> localPlayer = m_minecraft->mLocalPlayers[i];
        if (localPlayer != 0 && localPlayer->getId() == entityId)
        {
            boost::shared_ptr<MultiplayerLocalPlayer>& current = m_minecraft->mLocalPlayers[m_playerId];
            if (current == 0 || current->getId() != entityId)
                return boost::shared_ptr<Entity>();
            return current;
        }
    }
    if (m_level == 0)
        return boost::shared_ptr<Entity>();
    return m_level->getEntity(entityId);
}
void ClientPacketListener::checkDeferredEntityLinkPackets(int entityId)
{
    EnterCriticalSection(&s_deferredLinkMutex);
    if (!s_deferredLinks.empty())
    {
        for (unsigned int i = 0; i < s_deferredLinks.size(); i++)
        {
            DeferredEntityLinkPacket* deferred = &s_deferredLinks[i];
            bool remove = false;
            int deltaTime = GetTickCount() - deferred->mTick;
            if (deltaTime < 1000)
            {
                if (deferred->mPacket->getDestId() == entityId)
                {
                    handleEntityLinkPacket(deferred->mPacket);
                    remove = true;
                }
            }
            else
            {
                remove = true;
            }
            if (remove)
            {
                s_deferredLinks.erase(s_deferredLinks.begin() + i);
                i--;
            }
        }
    }
    LeaveCriticalSection(&s_deferredLinkMutex);
}

void ClientPacketListener::handleAddEntity(boost::shared_ptr<ClientboundAddEntityPacket> packet)
{
    double x = packet->getX() / 32.0;
    double y = packet->getY() / 32.0;
    double z = packet->getZ() / 32.0;
    boost::shared_ptr<Entity> entity;
    bool resetRotation = true;

    switch (packet->getType())
    {
    case ClientboundAddEntityPacket::MINECART:
        entity = Minecart::createMinecart(m_level, x, y, z, Minecart::Type::VALUES[packet->getData()]);
        break;
    case ClientboundAddEntityPacket::FISHING_HOOK:
    {
        boost::shared_ptr<Entity> ownerEntity = getEntity(packet->getData());
        if (ownerEntity == NULL)
        {
            for (int i = 0; i < 4; ++i)
            {
                if (m_minecraft->mLocalPlayers[i] != NULL
                    && m_minecraft->mLocalPlayers[i]->getId() == packet->getData())
                {
                    ownerEntity = m_minecraft->mLocalPlayers[i];
                    break;
                }
            }
        }
        if (ownerEntity != NULL && Entity::instanceof(ownerEntity, EntityType::PLAYER))
        {
            boost::shared_ptr<Player> owner = boost::static_pointer_cast<Player>(ownerEntity);
            boost::shared_ptr<FishingHook> hook(new FishingHook(m_level, owner, x, y, z));
            entity = hook;
            owner->mFishing = hook;
        }
        packet->setData(0);
        break;
    }
    case ClientboundAddEntityPacket::ARROW:
        entity = boost::shared_ptr<Entity>(new TippableArrow(m_level, x, y, z));
        break;
    case ClientboundAddEntityPacket::SPECTRAL_ARROW:
        entity = boost::shared_ptr<Entity>(new SpectralArrow(m_level, x, y, z));
        break;
    case ClientboundAddEntityPacket::TRIDENT:
        entity = boost::shared_ptr<Entity>(new ThrownTrident(m_level, x, y, z));
        break;

    case ClientboundAddEntityPacket::SNOWBALL:
        entity = boost::shared_ptr<Entity>(new Snowball(m_level, x, y, z));
        break;
    case ClientboundAddEntityPacket::LLAMA_SPIT:
        entity = boost::shared_ptr<Entity>(
            new LlamaSpit(m_level, x, y, z, packet->getXa(), packet->getYa(), packet->getZa()));
        break;
    case ClientboundAddEntityPacket::ITEM_FRAME:
        entity = boost::shared_ptr<ItemFrame>(
            new ItemFrame(m_level, BlockPos(x, y, z), Direction::from2DDataValue(packet->getData())));
        packet->setData(0);
        resetRotation = false;
        break;
    case ClientboundAddEntityPacket::THROWN_ENDERPEARL:
        entity = boost::shared_ptr<Entity>(new ThrownEnderpearl(m_level, x, y, z));
        break;
    case ClientboundAddEntityPacket::EYE_OF_ENDER:
        entity = boost::shared_ptr<Entity>(new EyeOfEnderSignal(m_level, x, y, z));
        break;
    case ClientboundAddEntityPacket::FIREBALL:
        entity = boost::shared_ptr<Entity>(
            new LargeFireball(m_level, x, y, z, packet->getXa(), packet->getYa(), packet->getZa()));
        packet->setData(0);
        break;
    case ClientboundAddEntityPacket::SMALL_FIREBALL:
        entity = boost::shared_ptr<Entity>(
            new SmallFireball(m_level, x, y, z, packet->getXa(), packet->getYa(), packet->getZa()));
        packet->setData(0);
        break;
    case ClientboundAddEntityPacket::DRAGON_FIREBALL:
        entity = boost::shared_ptr<Entity>(
            new DragonFireball(m_level, x, y, z, packet->getXa(), packet->getYa(), packet->getZa()));
        packet->setData(0);
        break;
    case ClientboundAddEntityPacket::EGG:
        entity = boost::shared_ptr<Entity>(new ThrownEgg(m_level, x, y, z));
        break;
    case ClientboundAddEntityPacket::EVOCATION_FANGS:
        entity = boost::shared_ptr<Entity>(
            new EvocationFangs(m_level, x, y, z, 0.0f, 0, boost::shared_ptr<LivingEntity>()));
        break;
    case ClientboundAddEntityPacket::POTION:
    {
        not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        if (miniGame->HasFeatureSplashLevitatePotions() && packet->getData() == ItemId::SPLASH_POTION)
            item = miniGame->getEquipmentForSlot(EquipmentSlot::eIndex_MainHand);
        else
            item = not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(packet->getData())));
        entity = boost::shared_ptr<Entity>(new ThrownPotion(m_level, x, y, z, item));
        packet->setData(0);
        break;
    }
    case ClientboundAddEntityPacket::EXPERIENCE_BOTTLE:
        entity = boost::shared_ptr<Entity>(new ThrownExpBottle(m_level, x, y, z));
        packet->setData(0);
        break;
    case ClientboundAddEntityPacket::BOAT:
        entity = boost::shared_ptr<Entity>(new Boat(m_level, x, y, z));
        break;
    case ClientboundAddEntityPacket::PRIMED_TNT:
        entity = boost::shared_ptr<Entity>(new PrimedTnt(m_level, x, y, z, boost::shared_ptr<LivingEntity>()));
        break;
    case ClientboundAddEntityPacket::ARMOR_STAND:
        entity = boost::shared_ptr<Entity>(new ArmorStand(m_level, x, y, z));
        break;
    case ClientboundAddEntityPacket::ENDER_CRYSTAL:
        entity = boost::shared_ptr<Entity>(new EnderCrystal(m_level, x, y, z));
        break;
    case ClientboundAddEntityPacket::ITEM:
        entity = boost::shared_ptr<Entity>(new ItemEntity(m_level, x, y, z));
        break;
    case ClientboundAddEntityPacket::FALLING:
        entity = boost::shared_ptr<Entity>(
            new FallingBlockEntity(m_level, x, y, z, Block::stateByIdWithData(packet->getData())));
        packet->setData(0);
        break;
    case ClientboundAddEntityPacket::WITHER_SKULL:
        entity = boost::shared_ptr<Entity>(
            new WitherSkull(m_level, x, y, z, packet->getXa(), packet->getYa(), packet->getZa()));
        packet->setData(0);
        break;
    case ClientboundAddEntityPacket::SHULKER_BULLET:
        entity = boost::shared_ptr<Entity>(
            new ShulkerBullet(m_level, x, y, z, packet->getXa(), packet->getYa(), packet->getZa()));
        packet->setData(0);
        break;
    case ClientboundAddEntityPacket::FIREWORKS:
    {
        bool directional = Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks();
        boost::shared_ptr<Entity> ownerEntity = m_level->getEntity(packet->getData());
        boost::shared_ptr<Player> owner;
        not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
        if (ownerEntity != NULL && Entity::instanceof(ownerEntity, EntityType::PLAYER))
        {
            owner = boost::static_pointer_cast<Player>(ownerEntity);
            if (directional)
                item = owner->getItemInHand(InteractionHand::eInteractionHand_MainHand);
        }
        entity = boost::shared_ptr<Entity>(new FireworksRocketEntity(m_level, x, y, z, packet->getXa(), packet->getYa(),
                                                                     packet->getZa(), item, true, owner));
        break;
    }
    case ClientboundAddEntityPacket::LEASH_KNOT:
        entity = boost::shared_ptr<Entity>(new LeashFenceKnotEntity(m_level, BlockPos((int)x, (int)y, (int)z)));
        packet->setData(0);
        break;
    case ClientboundAddEntityPacket::AREA_EFFECT_CLOUD:
        entity = boost::shared_ptr<Entity>(new AreaEffectCloud(m_level, x, y, z));
        break;
#ifdef MINECRAFT_PC
    case PC_MOD_ENTITY_PACKET:
        entity = EntityIO::newEntity(packet->getData(), m_level);
        break;
#endif
    }

    if (entity == NULL)
        return;

    entity->mXp = packet->getX();
    entity->mYp = packet->getY();
    entity->mZp = packet->getZ();
    float xRot = packet->getxRot() * 360 / 256.0f;
    float yRot = packet->getyRot() * 360 / 256.0f;
    entity->mXRotp = packet->getxRot();
    entity->mYRotp = packet->getyRot();
    if (resetRotation)
    {
        entity->mYRot = 0.0f;
        entity->mXRot = 0.0f;
    }

    std::vector<boost::shared_ptr<Entity> >* subEntities = entity->getSubEntities();
    if (subEntities != NULL)
    {
        int offset = packet->getId() - entity->getId();
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = subEntities->begin(); it != subEntities->end();
             ++it)
            (*it)->setId((*it)->getId() + offset);
    }

    if (packet->getType() == ClientboundAddEntityPacket::LEASH_KNOT
        || packet->getType() == ClientboundAddEntityPacket::BOAT)
        entity->absMoveTo(entity->mX, entity->mY, entity->mZ, yRot, xRot);
    else if (packet->getType() != ClientboundAddEntityPacket::ITEM_FRAME)
        entity->absMoveTo(x, y, z, yRot, xRot);

    entity->setId(packet->getId());
    entity->setUUID(packet->getUUID());
    if (m_level != NULL)
        m_level->putEntity(packet->getId(), entity);

    if (packet->getData() >= 0)
    {
        if (m_level != NULL
            && (packet->getType() == ClientboundAddEntityPacket::ARROW
                || packet->getType() == ClientboundAddEntityPacket::SPECTRAL_ARROW
                || packet->getType() == ClientboundAddEntityPacket::TRIDENT
                || packet->getType() == ClientboundAddEntityPacket::AREA_EFFECT_CLOUD))
        {
            boost::shared_ptr<Entity> owner = m_level->getEntity(packet->getData() - 1);
            if (owner == NULL)
            {
                for (int i = 0; i < 4; ++i)
                {
                    if (m_minecraft->mLocalPlayers[i] != NULL
                        && m_minecraft->mLocalPlayers[i]->getId() == packet->getData())
                    {
                        owner = m_minecraft->mLocalPlayers[i];
                        break;
                    }
                }
            }
            if (owner != NULL && Entity::instanceof(owner, EntityType::LIVING_ENTITY))
            {
                if (Entity::instanceof(entity, EntityType::ARROW))
                    boost::static_pointer_cast<Arrow>(entity)->mOwner = boost::static_pointer_cast<LivingEntity>(owner);
                else if (eTYPE_DERIVED_FROM(EntityType::THROWN_TRIDENT, entity->GetType()))
                    boost::static_pointer_cast<ThrownTrident>(entity)->mOwner =
                        boost::static_pointer_cast<LivingEntity>(owner);
                else if (eTYPE_DERIVED_FROM(EntityType::AREA_EFFECT_CLOUD, entity->GetType()))
                    boost::static_pointer_cast<AreaEffectCloud>(entity)->setOwner(
                        boost::static_pointer_cast<LivingEntity>(owner));
            }
        }
        entity->lerpMotion(packet->getXa(), packet->getYa(), packet->getZa());
    }

    checkDeferredEntityLinkPackets(entity->getId());
}

void ClientPacketListener::handleAddExperienceOrb(boost::shared_ptr<ClientboundAddExperienceOrbPacket> packet)
{
    int x = packet->getX();
    int y = packet->getY();
    int z = packet->getZ();
    boost::shared_ptr<Entity> entity(new ExperienceOrb(m_level, x / 32.0, y / 32.0, z / 32.0, packet->getValue()));
    entity->mXp = x;
    entity->mYp = y;
    entity->mZp = z;
    entity->mYRot = 0.0f;
    entity->mXRot = 0.0f;
    entity->setId(packet->getId());
    m_level->putEntity(packet->getId(), entity);
}

void ClientPacketListener::handleAddGlobalEntity(boost::shared_ptr<ClientboundAddGlobalEntityPacket> packet)
{
    double x = packet->getX() / 32.0;
    double y = packet->getY() / 32.0;
    double z = packet->getZ() / 32.0;
    boost::shared_ptr<Entity> entity;
    if (packet->getType() == 1)
        entity = boost::shared_ptr<Entity>(new LightningBolt(m_level, x, y, z, false));
    if (entity != NULL)
    {
        entity->mXp = packet->getX();
        entity->mYp = packet->getY();
        entity->mZp = packet->getZ();
        entity->mYRot = 0.0f;
        entity->mXRot = 0.0f;
        entity->setId(packet->getId());
        m_level->addGlobalEntity(entity);
    }
}

void ClientPacketListener::handleAddPainting(boost::shared_ptr<ClientboundAddPaintingPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<Painting> painting(
        new Painting(level, packet->getPos(), packet->getDirection(), packet->getMotive()));
    painting->setId(packet->getId());
    level->putEntity(packet->getId(), painting);
}

void ClientPacketListener::handleSetEntityMotion(boost::shared_ptr<ClientboundSetEntityMotionPacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->getId());
    if (entity == 0)
        return;
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
        && boost::static_pointer_cast<Player>(entity)->isFallFlying())
        return;
    entity->lerpMotion(static_cast<double>(packet->getXa()) / 8000.0, static_cast<double>(packet->getYa()) / 8000.0,
                       static_cast<double>(packet->getZa()) / 8000.0);
}

void ClientPacketListener::handleSetEntityData(boost::shared_ptr<ClientboundSetEntityDataPacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->getId());
    if (entity != NULL)
    {
        if (!eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
            && static_cast<int>(entity->GetType()) != packet->getEntityType())
            return;
    }
    if (entity != NULL && packet->getUnpackedData() != NULL)
        entity->getEntityData()->assignValues(packet->getUnpackedData());
}

void ClientPacketListener::send(boost::shared_ptr<Packet> packet)
{
    if (!m_closed)
        m_connection->send(packet);
}

void ClientPacketListener::handleAddPlayer(boost::shared_ptr<ClientboundAddPlayerPacket> packet)
{
    for (int i = 0; i < 4; ++i)
    {
        PlayerUID onlineXuid(0);
        PlayerUID offlineXuid(0);
        gProfile->GetXUID(i, &onlineXuid, true);
        gProfile->GetXUID(i, &offlineXuid, false);
        if (onlineXuid != PlayerUID(0) && gProfile->AreXUIDSEqual(onlineXuid, packet->getPlayerId()))
            return;
        if (offlineXuid != PlayerUID(0) && gProfile->AreXUIDSEqual(offlineXuid, packet->getPlayerId()))
            return;
    }

    double x = packet->getX() / 32.0;
    double y = packet->getY() / 32.0;
    double z = packet->getZ() / 32.0;
    float yRot = packet->getyRot() * 360 / 256.0f;
    float xRot = packet->getxRot() * 360 / 256.0f;
    boost::shared_ptr<RemotePlayer> player(new RemotePlayer(m_minecraft->mLevel, packet->getName()));
    player->mXo = player->mXOld = packet->getX();
    player->mYo = player->mYOld = packet->getY();
    player->mZo = player->mZOld = packet->getZ();
    player->mXp = packet->getX();
    player->mYp = packet->getY();
    player->mZp = packet->getZ();
    player->mXRotp = packet->getxRot();
    player->mYRotp = packet->getyRot();
    player->mYHeadRot = packet->myHeadRot * 360 / 256.0f;
    player->setXuid(packet->getPlayerId());

    INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerByXuid(player->getXuid());
    if (networkPlayer != NULL)
        player->mDisplayName = networkPlayer->GetDisplayName();

    player->absMoveTo(x, y, z, yRot, xRot);
    player->mPlayerIndex = packet->mPlayerIndex;
    player->setCustomSkin(packet->mSkinId);
    player->setCustomCape(packet->mCapeId);
    player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All, packet->mPlayerPrivilege);

    if (!player->mSkinLocation.empty() && player->mSkinLocation.substr(0, 3).compare(L"def") != 0
        && !gApp.IsFileInMemoryTextures(player->mSkinLocation))
    {
        if (m_minecraft->addPendingClientTextureRequest(player->mSkinLocation))
            send(boost::shared_ptr<TextureAndGeometryPacket>(
                new TextureAndGeometryPacket(player->mSkinLocation, NULL, 0)));
    }
    else if (!player->mSkinLocation.empty() && gApp.IsFileInMemoryTextures(player->mSkinLocation))
    {
        gApp.AddMemoryTextureFile(player->mSkinLocation, NULL, 0);
    }

    if (!player->mCapeLocation.empty() && player->mCapeLocation.substr(0, 3).compare(L"def") != 0
        && !gApp.IsFileInMemoryTextures(player->mCapeLocation))
    {
        if (m_minecraft->addPendingClientTextureRequest(player->mCapeLocation))
            send(boost::shared_ptr<TexturePacket>(new TexturePacket(player->mCapeLocation, NULL, 0)));
    }
    else if (!player->mCapeLocation.empty() && gApp.IsFileInMemoryTextures(player->mCapeLocation))
    {
        gApp.AddMemoryTextureFile(player->mCapeLocation, NULL, 0);
    }

    if (m_level != NULL)
        m_level->putEntity(packet->getEntityId(), player);

    std::vector<DataItem*>* unpackedData = packet->getUnpackedData();
    if (unpackedData != NULL)
        player->getEntityData()->assignValues(unpackedData);

    player->SetGameType(packet->mGameType);
}

void ClientPacketListener::handleTeleportEntity(boost::shared_ptr<ClientboundTeleportEntityPacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->getId());
    if (entity == NULL)
        return;

    entity->mXp = packet->getX();
    entity->mYp = packet->getY();
    entity->mZp = packet->getZ();
    double x = entity->mXp / 32.0;
    double y = entity->mYp / 32.0 + 0.015625;
    double z = entity->mZp / 32.0;
    if (!entity->isControlledByLocalInstance())
    {
        float yRot = Mth::wrapDegrees(packet->getyRot() * 360 / 256.0f);
        float xRot = Mth::wrapDegrees(packet->getxRot() * 360 / 256.0f);
        entity->mYRotp = packet->getyRot();
        entity->mXRotp = packet->getxRot();
        if (std::fabs(entity->mX - x) >= 0.03125 || std::fabs(entity->mY - y) >= 0.015625
            || std::fabs(entity->mZ - z) >= 0.03125)
            entity->lerpTo(x, y, z, yRot, xRot, 3, true);
        else
            entity->lerpTo(entity->mX, entity->mY, entity->mZ, yRot, xRot, 0, true);
        entity->mOnGround = packet->isOnGround();
    }
}

void ClientPacketListener::handleSetCarriedItem(boost::shared_ptr<ClientboundSetCarriedItemPacket> packet)
{
    if (packet->getSlot() >= 0 && packet->getSlot() < Inventory::getSelectionSize())
        Minecraft::GetInstance()->mLocalPlayers[m_playerId].get()->mInventory->m_selected = packet->getSlot();
}

void ClientPacketListener::handleMoveEntity(boost::shared_ptr<ClientboundMoveEntityPacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->getEntityId());
    if (entity == NULL)
        return;
    entity->mXp += packet->getXa();
    entity->mYp += packet->getYa();
    entity->mZp += packet->getZa();
    double x = entity->mXp / 32.0;
    double y = entity->mYp / 32.0 + 0.015625;
    double z = entity->mZp / 32.0;
    if (packet->hasRotation())
    {
        entity->mYRotp = packet->getyRot();
        entity->mXRotp = packet->getxRot();
    }

    float yRot = entity->mYRot;
    float xRot = entity->mXRot;
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);
        if (living->isInterpolating())
        {
            yRot = static_cast<float>(living->getLerpToYRot());
            xRot = static_cast<float>(living->getLerpToXRot());
        }
    }
    if (eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType()))
    {
        boost::shared_ptr<Minecart> minecart = boost::static_pointer_cast<Minecart>(entity);
        if (minecart->isInterpolating())
        {
            yRot = static_cast<float>(minecart->getLerpToYRot());
            xRot = static_cast<float>(minecart->getLerpToXRot());
        }
    }
    if (packet->hasRotation())
        yRot = packet->getyRot() * 360 / 256.0f;
    if (packet->hasRotation())
        xRot = packet->getxRot() * 360 / 256.0f;

    entity->lerpTo(x, y, z, Mth::wrapDegrees(yRot), Mth::wrapDegrees(xRot), 3, false);
    entity->mOnGround = packet->isOnGround();
}

void ClientPacketListener::handleRotateMob(boost::shared_ptr<ClientboundRotateHeadPacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->getEntityId());
    if (entity != 0)
        entity->setYHeadRot(packet->getYHeadRot() * 360 / 256.0f);
}

void ClientPacketListener::handleMoveEntitySmall(boost::shared_ptr<MoveEntityPacketSmall> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->mEntityId);
    if (entity == NULL)
        return;
    entity->mXp += packet->mXa;
    entity->mYp += packet->mYa;
    entity->mZp += packet->mZa;
    bool hasXRot = packet->mxRot != -1;
    double x = entity->mXp / 32.0;
    double y = entity->mYp / 32.0 + 0.015625;
    double z = entity->mZp / 32.0;

    float yRot = entity->mYRot;
    float xRot = entity->mXRot;
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);
        if (living->isInterpolating())
        {
            yRot = static_cast<float>(living->getLerpToYRot());
            xRot = static_cast<float>(living->getLerpToXRot());
        }
    }
    if (eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType()))
    {
        boost::shared_ptr<Minecart> minecart = boost::static_pointer_cast<Minecart>(entity);
        if (minecart->isInterpolating())
        {
            yRot = static_cast<float>(minecart->getLerpToYRot());
            xRot = static_cast<float>(minecart->getLerpToXRot());
        }
    }
    if (packet->mHasRot)
        yRot = packet->myRot * 360 / 256.0f;
    if (packet->mHasRot)
        xRot = packet->mxRot * 360 / 256.0f;
    yRot = Mth::wrapDegrees(yRot);
    xRot = Mth::wrapDegrees(xRot);
    if (packet->mHasRot)
    {
        entity->mYRotp = packet->myRot;
        if (hasXRot)
            entity->mXRotp = packet->mxRot;
    }

    entity->lerpTo(x, y, z, yRot, xRot, 3, false);
    entity->mOnGround = packet->isOnGround();
}

void ClientPacketListener::handleRemoveEntity(boost::shared_ptr<ClientboundRemoveEntitiesPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    for (unsigned int i = 0; i < packet->entityIds.length; ++i)
        level->removeEntity(packet->entityIds.data[i]);
}

void ClientPacketListener::handleMovePlayer(boost::shared_ptr<ClientboundPlayerPositionPacket> packet)
{
    boost::shared_ptr<Player> player = m_minecraft->mLocalPlayers[m_playerId];
    double x = packet->getX();
    double y = packet->getY();
    double z = packet->getZ();
    float yRot = packet->getYRot();
    float xRot = packet->getXRot();
    bool hasRot = packet->hasRot();
    if (packet->getRelativeArguments().find(&ClientboundPlayerPositionPacket::RelativeArgument::X)
        != packet->getRelativeArguments().end())
        x += player->mX;
    else
    {
        player->mXd = 0.0;
        player->mXOld = x;
    }
    if (packet->getRelativeArguments().find(&ClientboundPlayerPositionPacket::RelativeArgument::Y)
        != packet->getRelativeArguments().end())
        y += player->mY;
    else
    {
        player->mYd = 0.0;
        player->mYOld = y;
    }
    if (packet->getRelativeArguments().find(&ClientboundPlayerPositionPacket::RelativeArgument::Z)
        != packet->getRelativeArguments().end())
        z += player->mZ;
    else
    {
        player->mZd = 0.0;
        player->mZOld = z;
    }
    if (packet->getRelativeArguments().find(&ClientboundPlayerPositionPacket::RelativeArgument::X_ROT)
        != packet->getRelativeArguments().end())
        xRot += player->mXRot;
    if (packet->getRelativeArguments().find(&ClientboundPlayerPositionPacket::RelativeArgument::Y_ROT)
        != packet->getRelativeArguments().end())
        yRot += player->mYRot;
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(player);
    if (localPlayer != 0)
        localPlayer->fjResetBounceHeight();
    if (hasRot)
    {
        player->absMoveTo(x, y, z, yRot, xRot);
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            player->mYBodyRot = yRot;
    }
    else
        player->absMoveTo(x, y, z);
    m_connection->send(boost::shared_ptr<Packet>(new ServerboundAcceptTeleportationPacket(packet->getId())));
    m_connection->send(boost::shared_ptr<Packet>(new ServerboundMovePlayerPacket::PosRot(
        player->mX, player->mBoundingBox->mMinY, player->mZ, player->mYRot, player->mXRot, player->mOnGround, false)));
    if (!m_started)
    {
        if (!CGameNetworkManager::sInstance.IsHost())
            Minecraft::GetInstance()->mProgressRenderer->progressStagePercentage(100);
        player->mXo = player->mX;
        player->mYo = player->mY;
        player->mZo = player->mZ;
        player->mXOld = player->mX;
        player->mYOld = player->mY;
        player->mZOld = player->mZ;
        if (!m_suppressStartOnPosition)
            m_started = true;
        m_minecraft->setScreen(0);
        if (!m_suppressStartOnPosition && gApp.GetGameStarted())
            gConsoleUIController.CloseUIScenes(m_playerId, false);
        m_suppressStartOnPosition = false;
    }
}

void ClientPacketListener::handleChunkVisibilityArea(boost::shared_ptr<ChunkVisibilityAreaPacket> packet)
{
    if (m_level != 0)
    {
        if (!packet->outside)
        {
            for (int z = packet->minZ; z <= packet->maxZ; ++z)
                for (int x = packet->minX; x <= packet->maxX; ++x)
                    m_level->setChunkVisible(x, z, packet->visible, packet->loaded);
        }
        else
        {
            const int radius = m_level->mDimension->getXZSize() / 2;
            for (int x = -radius; x <= radius; ++x)
            {
                for (int z = -radius; z <= radius; ++z)
                {
                    if (x < packet->minX || x > packet->maxX || z < packet->minZ || z > packet->maxZ)
                    {
                        if (m_level->hasChunk(x, z, false))
                            m_level->setChunkVisible(x, z, packet->visible, packet->loaded);
                    }
                }
            }
        }
    }
}

void ClientPacketListener::handleChunkVisibility(boost::shared_ptr<ChunkVisibilityPacket> packet)
{
    if (m_level != 0 && packet != 0)
    {
        MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
        if (level != 0)
            level->setChunkVisible(packet->x, packet->z, packet->visible, packet->loaded);
    }
}

void ClientPacketListener::handleChunkBlocksUpdate(boost::shared_ptr<ClientboundChunkBlocksUpdatePacket> packet)
{
    MultiPlayerLevel* level = m_minecraft->mLevels[packet->dimension];
    if (level == 0)
        return;
    PIXBeginNamedEvent(0.0f, "Handle chunk blocks update");
    LevelChunk* chunk = level->getChunk(packet->x, packet->z);
    const int baseX = packet->x * 16;
    const int baseZ = packet->z * 16;
    int minLocalX = 0x7fffffff;
    int minY = 0x7fffffff;
    int minLocalZ = 0x7fffffff;
    int maxLocalX = 0;
    int maxY = 0;
    int maxLocalZ = 0;
    for (int i = 0; i < packet->count; ++i)
    {
        const int packed = packet->positions[i];
        const int localX = (packed >> 12) & 15;
        const int localZ = (packed >> 8) & 15;
        const int y = packed & 255;
        const unsigned int element = packet->elements[i];
        const int blockId = static_cast<int>((element & 0xffff) >> 4);
        const int data = static_cast<int>(element & 0xf);
        const unsigned short extra = static_cast<unsigned short>(element >> 16);
        Block* incoming = Block::byId(blockId);
        minLocalX = std::min(minLocalX, localX);
        minY = std::min(minY, y);
        minLocalZ = std::min(minLocalZ, localZ);
        maxLocalX = std::max(maxLocalX, localX);
        maxY = std::max(maxY, y);
        maxLocalZ = std::max(maxLocalZ, localZ);
        chunk->setBlockAndData(localX, y, localZ, blockId, data, true, false);
        chunk->setExtraElement(localX, y, localZ, extra);
        const BlockPos worldPos(baseX + localX, y, baseZ + localZ);
        level->checkLight(worldPos, false, false);
        level->clearResetRegion(worldPos.x, worldPos.y, worldPos.z, worldPos.x, worldPos.y, worldPos.z);
        Block* actual = chunk->getBlock(BlockPos(localX, y, localZ));
        const bool skipDirty = (actual == Blocks::flowing_water && incoming == Blocks::water)
                               || (actual == Blocks::water && incoming == Blocks::flowing_water)
                               || (actual == Blocks::flowing_lava && incoming == Blocks::lava)
                               || (actual == Blocks::lava && incoming == Blocks::lava)
                               || (actual == Blocks::lava && incoming == Blocks::flowing_lava);
        if (!skipDirty)
            level->setBlocksDirty(BlockPos(baseX + localX, y, baseZ + localZ),
                                  BlockPos(baseX + localX, y, baseZ + localZ));
    }
    level->removeUnusedBlockEntitiesInRegion(baseX + minLocalX, minY, baseZ + minLocalZ, baseX + maxLocalX + 1,
                                             maxY + 1, baseZ + maxLocalZ + 1);
    PIXEndNamedEvent();
}

void ClientPacketListener::handleBlockRegionUpdate(boost::shared_ptr<BlockRegionUpdatePacket> packet)
{
    MultiPlayerLevel* level = m_minecraft->mLevels[packet->mDimension];
    if (level != 0)
    {
        if (packet->mHasBlockChanges)
        {
            std::vector<LevelChunk::BlockChange>* changes = packet->GetChanges();
            packet->SetChanges(NULL);
            level->ReloadChunk(packet->mX >> 4, packet->mZ >> 4, changes, true);
            return;
        }

        PIXBeginNamedEvent(0.0f, "Handle block region update");
        level->StopReloadChunk(packet->mX >> 4, packet->mZ >> 4);

        int maxY = packet->mY + packet->mHeight;
        if (packet->mFullChunk && m_level != NULL)
        {
            maxY = 256;
            const int chunkX = packet->mX >> 4;
            const int chunkZ = packet->mZ >> 4;
            if (m_level->getChunk(chunkX, chunkZ)->isEmpty())
                m_level->setChunkVisible(chunkX, chunkZ, true, false);
        }

        PIXBeginNamedEvent(0.0f, "Clear rest region");
        level->clearResetRegion(packet->mX, packet->mY, packet->mZ, packet->mX + packet->mWidth - 1, maxY - 1,
                                packet->mZ + packet->mDepth - 1);
        PIXEndNamedEvent();

        PIXBeginNamedEvent(0.0f, "setBlocksAndData");
        if (packet->mFullChunk)
            level->setFromRawData(packet->mX / 16, packet->mZ / 16, packet->mData);
        else
            level->setBlocksAndData(packet->mX, packet->mY, packet->mZ, packet->mWidth, packet->mHeight, packet->mDepth,
                                    packet->mData, false);
        PIXEndNamedEvent();

        PIXBeginNamedEvent(0.0f, "removeUnusedBlockEntitiesInRegion");
        level->removeUnusedBlockEntitiesInRegion(packet->mX, packet->mY, packet->mZ, packet->mX + packet->mWidth, maxY,
                                                 packet->mZ + packet->mDepth);
        PIXEndNamedEvent();

        if (packet->mFullChunk)
        {
            PIXBeginNamedEvent(0.0f, "dataReceivedForChunk");
            level->dataReceivedForChunk(packet->mX >> 4, packet->mZ >> 4);
            PIXEndNamedEvent();
        }
        PIXEndNamedEvent();
        Minecraft::GetInstance()->GetClientMasterGameMode()->RecievedBRUP();
    }
}

void ClientPacketListener::handleBlockUpdate(boost::shared_ptr<ClientboundBlockUpdatePacket> packet)
{
    MultiPlayerLevel* level = m_minecraft->mLevels[packet->getLevelIdx()];
    if (level != 0)
    {
        PIXBeginNamedEvent(0.0f, "Handle block update");
        PIXBeginNamedEvent(0.0f, "Setting data\n");
        level->doSetBlockAndData(packet->getPos(), packet->getBlock(), packet->getExtraBlock());
        PIXEndNamedEvent();
        level->removeUnusedBlockEntitiesInRegion(packet->getPos().x, packet->getPos().y, packet->getPos().z,
                                                 packet->getPos().x + 1, packet->getPos().y + 1,
                                                 packet->getPos().z + 1);
        PIXEndNamedEvent();
    }
}

void ClientPacketListener::handleDisconnect(boost::shared_ptr<DisconnectPacket> packet)
{
    if (!getAndSetClosed())
    {
        m_connection->close(DisconnectPacket::eDisconnectReason_Kicked);
        Minecraft* minecraft = Minecraft::GetInstance();
        minecraft->mDisconnectReasons[m_playerId] = packet->reason;
        minecraft->mConnectionFailed[m_playerId] = true;
        gApp.SetDisconnectReason(packet->reason);
        gApp.SetAction(m_playerId, eXuiAction_ExitWorld, (void*)1);
    }
}

int ClientPacketListener::ExitGameAndSaveReturned(void* context, int result, C4JStorage::EMessageResult messageResult)
{
    gMinecraftServer->saveOnExit = messageResult == C4JStorage::EMessageResult_Decline;
    MinecraftServer::s_saveOnExitAnswered = true;
    gApp.SetAction(result, eXuiAction_ExitWorld, NULL);
    return 0;
}

int ClientPacketListener::HostDisconnectReturned(void* context, int result, C4JStorage::EMessageResult messageResult)
{
    TexturePack* selected = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    if (selected != TexturePackRepository::defaultTexturePack)
    {
        if (!static_cast<DLCTexturePack*>(Minecraft::GetInstance()->mTexturePackRepository->getSelected())
                 ->getDLCInfoParentPack()
                 ->hasPurchasedPack())
        {
            gMinecraftServer->saveOnExit = false;
            MinecraftServer::s_saveOnExitAnswered = true;
            gApp.SetAction(result, eXuiAction_ExitWorld, NULL);
        }
    }

    bool saveExists = false;
    gStorage->DoesSaveExist(&saveExists);
    if (!saveExists)
    {
        gMinecraftServer->saveOnExit = true;
        MinecraftServer::s_saveOnExitAnswered = true;
        gApp.SetAction(result, eXuiAction_ExitWorld, NULL);
    }
    else
    {
        int parameters[2] = {static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK)};
        gConsoleUIController.RequestErrorMessage(IDS_TITLE_SAVE_GAME, static_cast<int>(IDS_CONFIRM_SAVE_GAME),
                                                 parameters, 2, gProfile->GetPrimaryPad(), ExitGameAndSaveReturned,
                                                 NULL, NULL, true);
    }
    return 0;
}

void ClientPacketListener::onDisconnect(DisconnectPacket::eDisconnectReason reason, void* userData)
{
    if (getAndSetClosed())
        return;

    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mConnectionFailed[m_playerId] = true;
    minecraft->mDisconnectReasons[m_playerId] = reason;

    if (CGameNetworkManager::sInstance.IsHost()
        && (reason == DisconnectPacket::eDisconnectReason_TimedOut
            || reason == DisconnectPacket::eDisconnectReason_SendQueueOverflow)
        && IsPrimaryPlayer() && !MinecraftServer::s_saveOnExitAnswered)
    {
        int parameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
        gConsoleUIController.RequestErrorMessage(IDS_EXITING_GAME, IDS_GENERIC_ERROR, parameters, 1,
                                                 gProfile->GetPrimaryPad(), HostDisconnectReturned, 0, 0, true);
        return;
    }

    gApp.SetAction(m_playerId, eXuiAction_ExitWorld, (void*)1);
}

void ClientPacketListener::sendAndDisconnect(boost::shared_ptr<Packet> packet)
{
    if (!getAndSetClosed())
    {
        m_connection->send(packet);
        m_connection->sendAndQuit();
    }
}

void ClientPacketListener::handleTakeItemEntity(boost::shared_ptr<ClientboundTakeItemEntityPacket> packet)
{
    if (m_level == NULL)
    {
        return;
    }

    boost::shared_ptr<Entity> from = getEntity(packet->getItemId());
    boost::shared_ptr<LivingEntity> to = boost::dynamic_pointer_cast<LivingEntity>(getEntity(packet->getPlayerId()));

    bool isLocalPlayer = false;
    for (int i = 0; i < 4; ++i)
    {
        if (m_minecraft->mLocalPlayers[i] != NULL && m_minecraft->mLocalPlayers[i]->getId() == packet->getPlayerId())
        {
            isLocalPlayer = true;
            to = m_minecraft->mLocalPlayers[i];
            break;
        }
    }

    if (to == NULL)
    {
        m_level->removeEntity(packet->getItemId());
        return;
    }

    if (from != NULL)
    {
        if (isLocalPlayer)
        {
            boost::shared_ptr<LocalPlayer> player = boost::dynamic_pointer_cast<LocalPlayer>(to);
            if (m_minecraft->mGameModes[player->mUserIndex] != NULL)
            {
                if (eTYPE_DERIVED_FROM(EntityType::EXPERIENCE_ORB, from->GetType()))
                {
                    m_level->playLocalSound(from->mX, from->mY, from->mZ, SoundEvents::EXPERIENCE_ORB_PICKUP,
                                            SoundSource::eSoundSource_Players, 0.1f,
                                            ((m_random.nextFloat() - m_random.nextFloat()) * 0.7f + 1.0f) * 2.0f, false,
                                            16.0f);
                }
                else
                {
                    m_level->playLocalSound(
                        from->mX, from->mY, from->mZ, SoundEvents::ITEM_PICKUP, SoundSource::eSoundSource_Players, 0.2f,
                        ((m_random.nextFloat() - m_random.nextFloat()) * 0.7f + 1.0f) * 2.0f, false, 16.0f);
                }
                if (eTYPE_DERIVED_FROM(EntityType::ITEM_ENTITY, from->GetType()))
                {
                    boost::static_pointer_cast<ItemEntity>(from)->getItem()->setCount(packet->getAmount());
                }
                m_minecraft->mParticleEngine->add(new ItemPickupParticle(m_level, from, to, -0.5f));
                m_level->removeEntity(packet->getItemId());
            }
            else
            {
                m_level->removeEntity(packet->getItemId());
            }
        }
        else
        {
            m_level->playLocalSound(from->mX, from->mY, from->mZ, SoundEvents::ITEM_PICKUP,
                                    SoundSource::eSoundSource_Players, 0.2f,
                                    ((m_random.nextFloat() - m_random.nextFloat()) * 0.7f + 1.0f) * 2.0f, false, 16.0f);
            m_minecraft->mParticleEngine->add(new ItemPickupParticle(m_level, from, to, -0.5f));
            m_level->removeEntity(packet->getItemId());
        }
    }
}

std::wstring ClientPacketListener::GetDisplayNameByGamertag(std::wstring gamertag)
{
    return CGameNetworkManager::sInstance.GetDisplayNameByGamertag(gamertag);
}
void ClientPacketListener::handleChat(boost::shared_ptr<ClientboundChatPacket> packet)
{
    bool replacePlayer = false;
    bool replaceSource = false;
    bool replaceItem = false;
    bool showMessage = true;
    bool singleMessage = false;
    int singleMessageDuration = 0;
    boost::unordered_map<std::wstring, std::wstring> replacements;
    std::wstring playerName(L"");
    std::wstring sourceName(L"");
    std::wstring text;

    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && IsPrimaryPlayer())
    {
        m_minecraft->GetClientMasterGameMode()->OnChatPacket(packet);
    }

    if (packet->strings.size() > 0)
        playerName = GetDisplayNameByGamertag(packet->strings[0]);
    if (packet->strings.size() > 1)
        sourceName = GetDisplayNameByGamertag(packet->strings[1]);

    switch (packet->message)
    {
    case ClientboundChatPacket::eChatPacketMessage_BedOccupied:
        text = L10N::GetString(IDS_TILE_BED_OCCUPIED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_BedNoSleep:
        text = L10N::GetString(IDS_TILE_BED_NO_SLEEP);
        break;
    case ClientboundChatPacket::eChatPacketMessage_BedNotValid:
        text = L10N::GetString(IDS_TILE_BED_NOT_VALID);
        break;
    case ClientboundChatPacket::eChatPacketMessage_BedNotSafe:
        text = L10N::GetString(IDS_TILE_BED_NOTSAFE);
        break;
    case ClientboundChatPacket::eChatPacketMessage_BedTooFarAway:
        text = L10N::GetString(IDS_TILE_BED_TOO_FAR_AWAY);
        break;
    case ClientboundChatPacket::eChatPacketMessage_BedPlayerSleep:
        text = L10N::GetString(IDS_TILE_BED_PLAYERSLEEP);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case ClientboundChatPacket::eChatPacketMessage_BedMeSleep:
        text = L10N::GetString(IDS_TILE_BED_MESLEEP);
        break;
    case ClientboundChatPacket::eChatPacketMessage_PlayerJoined:
        text = L10N::GetString(IDS_PLAYER_JOINED);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case ClientboundChatPacket::eChatPacketMessage_PlayerLeft:
        text = L10N::GetString(IDS_PLAYER_LEFT);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case ClientboundChatPacket::eChatPacketMessage_PlayerKicked:
        text = L10N::GetString(IDS_PLAYER_KICKED);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case ClientboundChatPacket::eChatPacketMessage_PlayerVotedToKick:
        text = L10N::GetString(IDS_PLAYER_VOTED_TO_KICK);
        replacements[L"{*PLAYER*}"] = playerName;
        replacements[L"{*PLAYER2*}"] = sourceName;
        replacements[L"{*COUNT*}"] = _toString<int>(packet->ints[0]);
        replacements[L"{*MAX_COUNT*}"] = _toString<int>(packet->ints[1]);
        break;
    case ClientboundChatPacket::eChatPacketMessage_InteractionNotAllowed:
        showMessage = false;
        gApp.SetGlobalXuiAction(eXuiAction_LavaNearSpawn);
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathLightningBolt:
    case ClientboundChatPacket::eChatPacketMessage_DeathLightningBoltAlternate:
        text = L10N::GetString(IDS_DEATH_LIGHTNINGBOLT);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathInFire:
        text = L10N::GetString(IDS_DEATH_INFIRE);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathOnFire:
        text = L10N::GetString(IDS_DEATH_ONFIRE);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathLava:
        text = L10N::GetString(IDS_DEATH_LAVA);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathInWall:
        text = L10N::GetString(IDS_DEATH_INWALL);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathDrown:
        text = L10N::GetString(IDS_DEATH_DROWN);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathStarve:
        text = L10N::GetString(IDS_DEATH_STARVE);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathCactus:
        text = L10N::GetString(IDS_DEATH_CACTUS);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFall:
        text = L10N::GetString(IDS_DEATH_FALL);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFlyIntoWall:
        text = L10N::GetString(IDS_DEATH_FLY_INTO_WALL);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathOutOfWorld:
        text = L10N::GetString(IDS_DEATH_OUTOFWORLD);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathCustom:
        if (packet->ints[0] != 0)
        {
            text = L10N::GetString(packet->ints[0]);
        }
        else if (packet->strings[1].compare(L"") != 0)
        {
            const wchar_t* rulesText = gApp.GetGameRulesString(packet->strings[1]);
            if (rulesText != NULL)
                text = std::wstring(rulesText);
        }
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathGeneric:
        text = L10N::GetString(IDS_DEATH_GENERIC);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathExplosion:
        text = L10N::GetString(IDS_DEATH_EXPLOSION);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathMagic:
        text = L10N::GetString(IDS_DEATH_MAGIC);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathAnvil:
        text = L10N::GetString(IDS_DEATH_FALLING_ANVIL);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFallingBlock:
        text = L10N::GetString(IDS_DEATH_FALLING_TILE);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathDragonBreath:
        text = L10N::GetString(IDS_DEATH_DRAGON_BREATH);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathMob:
        text = L10N::GetString(IDS_DEATH_MOB);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathPlayer:
        text = L10N::GetString(IDS_DEATH_PLAYER);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathArrow:
        text = L10N::GetString(IDS_DEATH_ARROW);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFireball:
        text = L10N::GetString(IDS_DEATH_FIREBALL);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathThrown:
        text = L10N::GetString(IDS_DEATH_THROWN);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathIndirectMagic:
        text = L10N::GetString(IDS_DEATH_INDIRECT_MAGIC);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathThorns:
        text = L10N::GetString(IDS_DEATH_THORNS);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathHotFloor:
        text = L10N::GetString(IDS_DEATH_HOT_FLOOR);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathExplosionAlternate:
        text = L10N::GetString(IDS_DEATH_EXPLOSION);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathHotFloorEscaping:
        text = L10N::GetString(IDS_DEATH_HOT_FLOOR_PLAYER);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathTrident:
        text = L10N::GetString(IDS_DEATH_TRIDENT);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFellLadder:
        text = L10N::GetString(IDS_DEATH_FELL_ACCIDENT_LADDER);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFellVines:
        text = L10N::GetString(IDS_DEATH_FELL_ACCIDENT_VINES);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFellWater:
        text = L10N::GetString(IDS_DEATH_FELL_ACCIDENT_WATER);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFellHigh:
        text = L10N::GetString(IDS_DEATH_FELL_ACCIDENT_GENERIC);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFellDoomed:
        text = L10N::GetString(IDS_DEATH_FELL_KILLER);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFellDoomedBy:
        text = L10N::GetString(IDS_DEATH_FELL_ASSIST);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFellDoomedByItem:
        text = L10N::GetString(IDS_DEATH_FELL_ASSIST_ITEM);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFellFinished:
        text = L10N::GetString(IDS_DEATH_FELL_FINISH);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFellFinishedItem:
        text = L10N::GetString(IDS_DEATH_FELL_FINISH_ITEM);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathInFireFighting:
        text = L10N::GetString(IDS_DEATH_INFIRE_PLAYER);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathOnFireFighting:
        text = L10N::GetString(IDS_DEATH_ONFIRE_PLAYER);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathLavaEscaping:
        text = L10N::GetString(IDS_DEATH_LAVA_PLAYER);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathDrownEscaping:
        text = L10N::GetString(IDS_DEATH_DROWN_PLAYER);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathCactusEscaping:
        text = L10N::GetString(IDS_DEATH_CACTUS_PLAYER);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy:
        text = L10N::GetString(IDS_DEATH_EXPLOSION_PLAYER);
        replacePlayer = true;
        replaceSource = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathWither:
        text = L10N::GetString(IDS_DEATH_WITHER);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathMobItem:
        text = L10N::GetString(IDS_DEATH_PLAYER_ITEM);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathArrowItem:
        text = L10N::GetString(IDS_DEATH_ARROW_ITEM);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathFireballItem:
        text = L10N::GetString(IDS_DEATH_FIREBALL_ITEM);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathThrownItem:
        text = L10N::GetString(IDS_DEATH_THROWN_ITEM);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathTridentItem:
        text = L10N::GetString(IDS_DEATH_TRIDENT_ITEM);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_DeathIndirectMagicItem:
        text = L10N::GetString(IDS_DEATH_INDIRECT_MAGIC_ITEM);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_PlayerEnteredEnd:
        text = L10N::GetString(IDS_PLAYER_ENTERED_END);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case ClientboundChatPacket::eChatPacketMessage_PlayerLeftEnd:
        text = L10N::GetString(IDS_PLAYER_LEFT_END);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxEnemies:
        text = L10N::GetString(IDS_MAX_ENEMIES_SPAWNED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxVillagers:
        text = L10N::GetString(IDS_MAX_VILLAGERS_SPAWNED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxAnimals:
        text = L10N::GetString(IDS_MAX_PIGS_SHEEP_COWS_CATS_SPAWNED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxChickens:
        text = L10N::GetString(IDS_MAX_CHICKENS_SPAWNED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxSquid:
        text = L10N::GetString(IDS_MAX_SQUID_SPAWNED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxMooshrooms:
        text = L10N::GetString(IDS_MAX_MOOSHROOMS_SPAWNED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxWolves:
        text = L10N::GetString(IDS_MAX_WOLVES_SPAWNED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxBats:
        text = L10N::GetString(IDS_MAX_BATS_SPAWNED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxBredAnimals:
        text = L10N::GetString(IDS_MAX_PIGS_SHEEP_COWS_CATS_BRED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxBredChickens:
        text = L10N::GetString(IDS_MAX_CHICKENS_BRED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxBredMooshrooms:
        text = L10N::GetString(IDS_MAX_MUSHROOMCOWS_BRED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxBredWolves:
        text = L10N::GetString(IDS_MAX_WOLVES_BRED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_CantShearMooshroom:
        text = L10N::GetString(IDS_CANT_SHEAR_MOOSHROOM);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxHangingEntities:
        text = L10N::GetString(IDS_MAX_HANGINGENTITIES);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxArmorStands:
        text = L10N::GetString(IDS_MAX_ARMOURSTANDS);
        break;
    case ClientboundChatPacket::eChatPacketMessage_CantSpawnInPeaceful:
        text = L10N::GetString(IDS_CANT_SPAWN_IN_PEACEFUL);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxBoats:
        text = L10N::GetString(IDS_MAX_BOATS);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxMinecarts:
        text = L10N::GetString(IDS_MAX_MINECARTS);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxBanners:
        text = L10N::GetString(IDS_MAX_BANNERS);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxEnderCrystals:
        text = L10N::GetString(IDS_MAX_ENDER_CRYSTALS);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxBeacons:
        text = L10N::GetString(IDS_MAX_BEACONS);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxDolphins:
        text = L10N::GetString(IDS_MAX_DOLPHIN_SPAWNED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxPhantoms:
        text = L10N::GetString(IDS_MAX_PHANTOM_SPAWNED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_MaxFishAndSquid:
        text = L10N::GetString(IDS_MAX_WATER_ANIMALS_SPAWNED);
        break;
    case ClientboundChatPacket::eChatPacketMessage_TeleportSuccess:
        text = L10N::GetString(IDS_COMMAND_TELEPORT_SUCCESS);
        replacePlayer = true;
        if (packet->ints[0] == (int)EntityType::SERVER_PLAYER)
        {
            replacements[L"{*DESTINATION*}"] = sourceName;
        }
        else
        {
            int auxValue = -1;
            if (packet->ints.size() > 1)
                auxValue = packet->ints[1];
            replacements[L"{*DESTINATION*}"] = gApp.getEntityName(
                static_cast<uint64_t>(static_cast<int64_t>(packet->ints[0])), auxValue);
        }
        break;
    case ClientboundChatPacket::eChatPacketMessage_TeleportedYouToPosition:
        text = L10N::GetString(IDS_COMMAND_TELEPORT_ME);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_TeleportedToYou:
        text = L10N::GetString(IDS_COMMAND_TELEPORT_TO_ME);
        replacePlayer = true;
        break;
    case ClientboundChatPacket::eChatPacketMessage_TimeSuccess:
    {
        const int time = packet->ints[0];
        if (time == 1000)
        {
            text = L10N::GetString(IDS_COMMAND_TIME_SET_DAY);
        }
        else if (time == 14000)
        {
            text = L10N::GetString(IDS_COMMAND_TIME_SET_NIGHT);
        }
        else
        {
            text = L10N::GetString(IDS_COMMAND_TIME_SET);
            text.replace(text.find(L"%d"), 2, _toString<int>(time));
        }
        break;
    }
    case 100:
        text = L10N::GetString(IDS_COMMAND_DIFFICULTY_SUCCESS);
        replacements[L"{*DIFFICULTY*}"] = L10N::GetString(Difficulty::byId(packet->ints[0])->getKey());
        break;
    case 101:
        text = L10N::GetString(IDS_COMMAND_DEFAULTGAMEMODE_SUCCESS);
        replacements[L"{*MODE*}"] = L10N::GetString(GameType::byId(packet->ints[0])->getLabel());
        break;
    case 102:
        text = L10N::GetString(IDS_COMMAND_SETWORLDSPAWN_SUCCESS);
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[0]));
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[1]));
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[2]));
        break;
    case 103:
        text = L10N::GetString(IDS_COMMAND_SETWORLDSPAWN_FAILED);
        break;
    case 104:
        text = L10N::GetString(IDS_COMMAND_SPAWNPOINT_SUCCESS);
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[0]));
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[1]));
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[2]));
        replacePlayer = true;
        break;
    case 105:
        text = L10N::GetString(IDS_COMMAND_SPAWNPOINT_FAILED);
        replacePlayer = true;
        break;
    case 106:
        text = L10N::GetString(IDS_COMMAND_WEATHER_CLEAR);
        break;
    case 107:
        text = L10N::GetString(IDS_COMMAND_WEATHER_RAIN);
        break;
    case 108:
        text = L10N::GetString(IDS_COMMAND_WEATHER_THUNDER);
        break;
    case 109:
        text = L10N::GetString(IDS_COMMAND_GAMEMODE_SELF_SUCCESS);
        replacements[L"{*MODE*}"] = L10N::GetString(GameType::byId(packet->ints[0])->getLabel());
        break;
    case 110:
        text = L10N::GetString(IDS_COMMAND_GAMEMODE_OTHER_SUCCESS);
        replacements[L"{*MODE*}"] = L10N::GetString(GameType::byId(packet->ints[0])->getLabel());
        replacePlayer = true;
        break;
    case 111:
        text = L10N::GetString(IDS_COMMAND_GAMEMODE_CHANGED);
        break;
    case 112:
        text = L10N::GetString(0xd2bd3683);
        replacePlayer = true;
        break;
    case 113:
        singleMessage = true;
        text = L10N::GetString(IDS_MG_SELECTING_MODE);
        break;
    case 114:
        text = L10N::GetString(IDS_MG_WAITING_FOR_LOAD);
        singleMessage = true;
        break;
    case 115:
        text = L10N::GetString(IDS_MG_ROUND_CHANGING);
        singleMessage = true;
        break;
    case 116:
        text = L10N::GetString(IDS_MG_ROUND_STARTING);
        singleMessage = true;
        break;
    case 117:
        text = L10N::GetString(IDS_MG_ROUND_START);
        singleMessage = true;
        break;
    case 118:
        text = L10N::GetString(IDS_MG_ROUND_ENDING);
        singleMessage = true;
        break;
    case 119:
        text = L10N::GetString(IDS_MG_ROUND_END);
        singleMessage = true;
        break;
    case 120:
        text = L10N::GetString(IDS_MG_SPECTATING);
        singleMessage = true;
        break;
    case 121:
        if (packet->ints[0] == 1)
        {
            text = L10N::GetString(0x989f7641);
        }
        else
        {
            text = L10N::GetString(IDS_MG_LIVES_REMAINING);
            text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[0]));
        }
        singleMessage = true;
        break;
    case 122:
        singleMessage = true;
        text = L10N::GetString(0x2e19c8b8);
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[0]));
        break;
    case 123:
        singleMessageDuration = 60;
        text = L10N::GetString(IDS_MG03_CHECKPOINT);
        singleMessage = true;
        break;
    case 124:
        text = L10N::GetString(IDS_MG_INVULNERABILITY_OFF);
        singleMessage = true;
        break;
    case 125:
        singleMessage = true;
        text = L10N::GetString(IDS_MG_TIMER_INITIALISING);
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[0]));
        break;
    case 126:
        singleMessage = true;
        text = L10N::GetString(IDS_MG_TIMER_WAITING_TO_START);
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[0]));
        break;
    case 127:
    {
        const int value = packet->ints[0];
        text = L10N::GetString(IDS_MG_TIMER_WAITING_TO_END);
        text.replace(text.find(L"%d"), 2, _toString<int>(value));
        singleMessage = true;
        break;
    }
    case 128:
        text = L10N::GetString(0xaf2e63a6);
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[0]));
        singleMessage = true;
        break;
    case 129:
        text = L10N::GetString(IDS_MG_TIMER_INVULNERABILITY);
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[0]));
        singleMessage = true;
        break;
    case 132:
        singleMessage = true;
        text = L10N::GetString(0xa19c12db);
        replacements[L"{*QUANTITY*}"] = _toString<int>(packet->ints[1]);
        replacements[L"{*OBJECT*}"] = L10N::GetString(packet->ints[2]);
        replacements[L"{*EVENT*}"] = L10N::GetString(packet->ints[3]);
        break;
    case 133:
        singleMessage = true;
        text = L10N::GetString(0x431eeece);
        replacements[L"{*QUANTITY*}"] = _toString<int>(packet->ints[1]);
        replacements[L"{*OBJECT*}"] = L10N::GetString(packet->ints[2]);
        replacements[L"{*EVENT*}"] = L10N::GetString(packet->ints[3]);
        break;
    case 130:
        singleMessage = true;
        text = L10N::GetString(0x2fb55d20);
        replacements[L"{*QUANTITY*}"] = _toString<int>(packet->ints[1]);
        replacements[L"{*OBJECT*}"] = L10N::GetString(packet->ints[2]);
        replacements[L"{*EVENT*}"] = L10N::GetString(packet->ints[3]);
        break;
    case 131:
        singleMessage = true;
        text = L10N::GetString(0xb8af706d);
        replacements[L"{*QUANTITY*}"] = _toString<int>(packet->ints[1]);
        replacements[L"{*OBJECT*}"] = L10N::GetString(packet->ints[2]);
        replacements[L"{*EVENT*}"] = L10N::GetString(packet->ints[3]);
        break;
    case 134:
        singleMessage = true;
        text = L10N::GetString(0x1b75ffcc);
        replacements[L"{*QUANTITY*}"] = _toString<int>(packet->ints[0]);
        replacements[L"{*OBJECT*}"] = L10N::GetString(packet->ints[2]);
        replacements[L"{*EVENT*}"] = L10N::GetString(packet->ints[3]);
        break;
    case 135:
        singleMessage = true;
        text = L10N::GetString(IDS_MG_REQUIRED_FOR_PLAY);
        replacements[L"{*QUANTITY*}"] = _toString<int>(packet->ints[0]);
        break;
    case 136:
        singleMessage = true;
        text = L10N::GetString(IDS_MG_WAITING_FOR_READY);
        replacements[L"{*QUANTITY*}"] = _toString<int>(packet->ints[0]);
        break;
    case 137:
        text = L10N::GetString(0xa292a825);
        break;
    case 138:
        text = L10N::GetString(0x3041d7ac);
        break;
    case 139:
        text = L10N::GetString(0x90c46bbb);
        break;
    case 140:
        text = L10N::GetString(IDS_MG_TEAMS_DISABLED);
        break;
    case 141:
        text = L10N::GetString(0x40bc050c);
        replacements[L"{*SIZE*}"] = _toString<int>(packet->ints[0]);
        break;
    case 142:
        text = L10N::GetString(0x09988b49);
        break;
    case 143:
        text = L10N::GetString(0xc08f5c90);
        replacements[L"{*PLAYER*}"] = packet->strings[0];
        replacements[L"{*TEAM*}"] = packet->strings[1];
        break;
    case 144:
        text = L10N::GetString(IDS_MG_INVITE_EXPIRED);
        replacements[L"{*PLAYER*}"] = packet->strings[0];
        break;
    case 145:
        text = L10N::GetString(IDS_MG_TEAM_JOINED_1);
        replacements[L"{*PLAYER*}"] = packet->strings[0];
        replacements[L"{*TEAM*}"] = packet->strings[1];
        break;
    case 146:
        text = L10N::GetString(IDS_MG_TEAM_JOINED_2);
        replacements[L"{*TEAM*}"] = packet->strings[0];
        break;
    case 147:
        text = L10N::GetString(IDS_MG_LEAVING_TEAM_2);
        replacements[L"{*PLAYER*}"] = packet->strings[0];
        replacements[L"{*TEAM*}"] = packet->strings[1];
        break;
    case 148:
        text = L10N::GetString(IDS_MG_LEAVING_TEAM_1);
        replacements[L"{*TEAM*}"] = packet->strings[0];
        break;
    case 149:
        text = L10N::GetString(IDS_MG_LEFT_TEAM_1);
        replacements[L"{*TEAM*}"] = packet->strings[0];
        break;
    case 150:
        break;
    case 151:
        text = L10N::GetString(0x056d9f12);
        replacements[L"{*NUMBER*}"] = _toString<int>(packet->ints[0]);
        break;
    case 152:
        text = L10N::GetString(IDS_MG_FIGHT);
        break;
    case 153:
        text = L10N::GetString(0xd296c94c);
        break;
    case 154:
        singleMessage = true;
        text = L10N::GetString(IDS_SPECTATE_PLAYER);
        replacePlayer = true;
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[0]));
        text.replace(text.find(L"%d"), 2, _toString<int>(packet->ints[1]));
        break;
    case 155:
        text = L10N::GetString(IDS_SPECTATE_NO_PLAYER);
        break;
    default:
        text = playerName;
        break;
    }

    if (gApp.IsConsoleChatCensored() && !gApp.IsUUIDLocal(sourceName) && replaceItem)
    {
        replaceItem = false;
        switch (static_cast<int>(packet->message))
        {
        case 49:
        case 51:
            text = L10N::GetString(IDS_DEATH_FELL_FINISH);
            break;
        case 60:
            text = L10N::GetString(IDS_DEATH_PLAYER);
            break;
        case 61:
            text = L10N::GetString(IDS_DEATH_ARROW);
            break;
        case 62:
            text = L10N::GetString(IDS_DEATH_FIREBALL);
            break;
        case 63:
            text = L10N::GetString(IDS_DEATH_THROWN);
            break;
        case 67:
            text = L10N::GetString(IDS_DEATH_INDIRECT_MAGIC);
            break;
        default:
            break;
        }
    }

    if (replacePlayer)
        replacements[L"{*PLAYER*}"] = playerName;

    if (replaceSource)
    {
        if (packet->ints[0] == (int)EntityType::SERVER_PLAYER)
            replacements[L"{*SOURCE*}"] = sourceName;
        else
            replacements[L"{*SOURCE*}"] = packet->getMobCustomName(m_level);
    }

    if (replaceItem)
        replacements[L"{*ITEM*}"] = packet->strings[2];

    const bool deathMessage = packet->message >= ClientboundChatPacket::eChatPacketMessage_DeathInFire
                              && packet->message <= ClientboundChatPacket::eChatPacketMessage_DeathIndirectMagicItem;
    const bool aboutThisPlayer = playerName.compare(gProfile->GetMiiName(m_playerId)) == 0;
    if (showMessage)
    {
        makeReplacements(replacements, text);
        if (singleMessage && GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplayGameMessages) != 0)
            m_minecraft->mGui->setSingleMessage(text, m_playerId, singleMessageDuration);
        else
            m_minecraft->mGui->addMessage(text, m_playerId, deathMessage, aboutThisPlayer, false, false);
    }
}

uint32_t ClientPacketListener::isLocalPlayer(int entityId)
{
    for (int i = 0; i < 4; ++i)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = m_minecraft->mLocalPlayers[i];
        if (localPlayer != NULL && localPlayer->getId() == entityId)
            return 1;
    }
    return 0;
}
void ClientPacketListener::handleAnimate(boost::shared_ptr<ClientboundAnimatePacket> packet)
{
    if (m_level == 0)
        return;
    boost::shared_ptr<Entity> entity = m_level->getEntity(packet->getId());
    if (entity == 0)
        return;
    const bool local = isLocalPlayer(packet->getId()) != 0;
    if (packet->getAction() == 0 && !local)
    {
        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
            boost::static_pointer_cast<LivingEntity>(entity)->swing(InteractionHand::eInteractionHand_MainHand);
    }
    else if (packet->getAction() == 3 && !local)
    {
        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
            boost::static_pointer_cast<LivingEntity>(entity)->swing(InteractionHand::eInteractionHand_OffHand);
    }
    else if (packet->getAction() == 1 && !local)
    {
        entity->animateHurt();
    }
    else if (packet->getAction() == 2)
    {
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
            boost::static_pointer_cast<Player>(entity)->stopSleepInBed(false, false, false);
    }
    else if (packet->getAction() == 4)
    {
        m_minecraft->mParticleEngine->createTrackingEmitter(entity, PARTICLE_CRIT);
    }
    else if (packet->getAction() == 5)
    {
        m_minecraft->mParticleEngine->createTrackingEmitter(entity, PARTICLE_MAGIC_CRIT);
    }
}

void ClientPacketListener::handleEntityActionAtPosition(boost::shared_ptr<ClientboundPlayerSleepPacket> packet)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(getEntity(packet->getPlayerId()));
    if (player != 0)
    {
        player->startSleepInBed(packet->getPos(), false);
        if (player == m_minecraft->mLocalPlayers[m_playerId])
            telemetryManagerInstance->RecordEnemyKilledOrOvercome(
                m_playerId, static_cast<int>(player->mX), static_cast<int>(player->mY), static_cast<int>(player->mZ), 0,
                0, 0, static_cast<ETelemetryChallenges>(19));
    }
}

void ClientPacketListener::handlePreLogin(boost::shared_ptr<ClientboundPreLoginPacket> packet)
{
    int friendCheckResult = 1;
    int playCheckResult = 1;
    int secondaryMessageVariant = 1;
    int contentCheckResult = 1;
    int restrictionResult = 0;

    CGameNetworkManager::sInstance.SetIsInPublicJoinableGame(packet->mHardcore);
    const bool publicJoinable = CGameNetworkManager::sInstance.IsInPublicJoinableGame();
    if (!CGameNetworkManager::sInstance.IsHost())
    {
        gApp.SetExtraGameData(eGameExtraData_All, 0);
        gApp.SetGameHostOption(eGameHostOption_All, packet->mHostPlayer);
        CGameNetworkManager::sInstance.ClearInviteInfo();
        if (gApp.GetTMSGlobalFileListRead() == 0)
        {
            const int action = gApp.GetTMSAction(gProfile->GetPrimaryPad());
            if (action == 0)
                gApp.SetTMSAction(gProfile->GetPrimaryPad(), static_cast<eTMSAction>(3));
        }
    }

    CGameNetworkManager::sInstance.CheckFriendsCanPlay(m_playerId, friendCheckResult, packet, publicJoinable,
                                                       playCheckResult, secondaryMessageVariant, contentCheckResult,
                                                       restrictionResult);

    if (playCheckResult != 0 && friendCheckResult != 0 && contentCheckResult != 0 && restrictionResult == 0)
    {
        if (IsPrimaryPlayer())
        {
            Minecraft* minecraft = Minecraft::GetInstance();
            minecraft->mTexturePackRepository->selectTexturePackById(packet->mGameType, false);
        }
        if (!CGameNetworkManager::sInstance.IsHost())
            m_minecraft->mProgressRenderer->progressStagePercentage(25);

        PlayerUID onlineUid(0);
        PlayerUID offlineUid(0);
        if (gProfile->IsSignedInLive(m_playerId))
            gProfile->GetXUID(m_playerId, &onlineUid, true);
        gProfile->GetXUID(m_playerId, &offlineUid, false);

        const bool friendsOnly = CGameNetworkManager::sInstance.IsFriendsOnlyUGC(m_playerId);
        boost::shared_ptr<Packet> login(new ClientboundLoginPacket(
            m_minecraft->mUser->name, 0x13c, offlineUid, onlineUid, friendsOnly, packet->mSeed,
            GameSettings::GetPlayerSkinId(m_playerId), GameSettings::GetPlayerCapeId(m_playerId),
            gProfile->IsGuest(m_playerId), false));
        send(login);

        if (!CGameNetworkManager::sInstance.IsHost())
            m_minecraft->mProgressRenderer->progressStagePercentage(50);
        m_minecraft->mGui->clearSingleMessages(m_playerId);
        return;
    }

    if (!IsPrimaryPlayer())
    {
        int parameter = IDS_CONFIRM_OK;
        int title = secondaryMessageVariant ? STRING_JOIN_CONTENT_PRIVILEGE_TOO_RESTRICTIVE
                                            : static_cast<int>(STRING_JOIN_FRIENDS_OF_HOST_ONLY);
        gConsoleUIController.RequestErrorMessage(IDS_CANTJOIN_TITLE, title, &parameter, 1, m_playerId, 0, 0, 0, true);
        gApp.SetAction(m_playerId, eXuiAction_RemoveLocalPlayerNoStats, 0);
        return;
    }

    DisconnectPacket::eDisconnectReason reason = DisconnectPacket::eDisconnectReason_ConnectionClosed;
    if (secondaryMessageVariant != 0)
    {
        if (contentCheckResult == 0)
            reason = DisconnectPacket::eDisconnectReason_NoFriendsInGame;
        else if (friendCheckResult != 0)
            reason = static_cast<DisconnectPacket::eDisconnectReason>(restrictionResult != 0 ? 0x16 : 0x18);
        else
            reason = DisconnectPacket::eDisconnectReason_ContentRestrictedSingleLocal;
    }
    else
        reason = DisconnectPacket::eDisconnectReason_FriendsOfHostOnly;
    gApp.SetDisconnectReason(reason);
    gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_ExitWorld, reinterpret_cast<void*>(1));
}

void ClientPacketListener::close()
{
    if (!getAndSetClosed())
    {
        m_connection->flush();
        m_connection->close(DisconnectPacket::eDisconnectReason_ConnectionClosed);
    }
}

void ClientPacketListener::handleAddMob(boost::shared_ptr<ClientboundAddMobPacket> packet)
{
    double x = packet->getX() / 32.0;
    double y = packet->getY() / 32.0;
    double z = packet->getZ() / 32.0;
    float yRot = (packet->getyRot() * 360) / 256.0f;
    float xRot = (packet->getxRot() * 360) / 256.0f;
    boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(
        EntityIO::newEntity(packet->getType(), m_level));
    if (living != 0)
    {
        living->mXp = packet->getX();
        living->mYp = packet->getY();
        living->mZp = packet->getZ();
        living->mYBodyRot = living->mYHeadRot = (packet->getyHeadRot() * 360) / 256.0f;
        living->mYRotp = packet->getyRot();
        living->mXRotp = packet->getxRot();
        std::vector<boost::shared_ptr<Entity> >* subEntities = living->getSubEntities();
        std::vector<int> subIds = packet->getSubIds();
        if (subEntities != 0)
        {
            for (unsigned int i = 0; i < subIds.size(); ++i)
                subEntities->at(i)->setId(subIds.at(i));
        }
        living->setId(packet->getId());
        living->setUUID(packet->getUUID());
        living->absMoveTo(x, y, z, yRot, xRot);
        living->mXd = static_cast<float>(packet->getXd()) / 8000.0f;
        living->mYd = static_cast<float>(packet->getYd()) / 8000.0f;
        living->mZd = static_cast<float>(packet->getZd()) / 8000.0f;
        if (m_level != 0)
            m_level->putEntity(packet->getId(), living);
        std::vector<DataItem*>* unpacked = packet->getUnpackedData();
        if (unpacked != 0)
            living->getEntityData()->assignValues(unpacked);
        if (eTYPE_DERIVED_FROM(EntityType::SLIME, living->GetType()))
        {
            boost::shared_ptr<Slime> slime = boost::static_pointer_cast<Slime>(living);
            slime->setSize(slime->getSize(), true);
        }
        CompoundTag* tag = packet->getAdditionalContructionDataTag();
        if (tag != 0)
            living->readAdditionContructionTag(tag);
    }
}

void ClientPacketListener::handleSetTime(boost::shared_ptr<ClientboundSetTimePacket> packet)
{
    m_minecraft->mLevel->setGameTime(packet->getGameTime());
    m_minecraft->mLevel->setDayTime(packet->getDayTime());
}

void ClientPacketListener::handleSetSpawn(boost::shared_ptr<ClientboundSetSpawnPositionPacket> packet)
{
    m_minecraft->mLocalPlayers[m_playerId]->setRespawnPosition(new BlockPos(packet->getPos()), true);
    m_minecraft->mLevel->getLevelData()->setSpawn(packet->getPos());
}

void ClientPacketListener::handleSetEntityPassengersPacket(boost::shared_ptr<ClientboundSetPassengersPacket> packet)
{
    if (m_level != 0)
    {
        boost::shared_ptr<Entity> vehicle = m_level->getEntity(packet->getVehicle());
        if (vehicle != 0)
        {
            vehicle->hasIndirectPassenger(m_minecraft->mLocalPlayers[m_playerId]);
            vehicle->ejectPassengers();
            arrayWithLength<int> passengers = packet->getPassengers();
            for (unsigned int i = 0; i < passengers.length; ++i)
            {
                boost::shared_ptr<Entity> passenger = m_level->getEntity(passengers.data[i]);
                if (passenger != 0)
                    passenger->startRiding(vehicle, true);
            }
        }
    }
}

void ClientPacketListener::handleEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<Entity> source = level->getEntity(packet->getSourceId());
    boost::shared_ptr<Entity> dest = level->getEntity(packet->getDestId());
    if (dest == 0 && packet->getDestId() >= 0)
    {
        EnterCriticalSection(&s_deferredLinkMutex);
        s_deferredLinks.push_back(DeferredEntityLinkPacket(packet));
        LeaveCriticalSection(&s_deferredLinkMutex);
        return;
    }
    if (source == 0)
        return;
    if (eTYPE_DERIVED_FROM(EntityType::MOB, source->GetType()))
    {
        boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(source);
        if (dest != 0)
            mob->setLeashedTo(dest, false);
        else
            mob->dropLeash(false, false);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::BOAT, source->GetType()))
    {
        boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(source);
        if (dest != 0)
            boat->setLeashedTo(dest, false);
        else
            boat->dropLeash(false, false);
    }
}

void ClientPacketListener::handleEntityEvent(boost::shared_ptr<ClientboundEntityEventPacket> packet)
{
    if (packet == 0)
        return;
    boost::shared_ptr<Entity> entity = getEntity(packet->getEntityId());
    if (entity == 0)
        return;
    const unsigned char eventId = packet->getEventId();
    if (eventId == 0x15)
    {
        boost::shared_ptr<Guardian> guardian = boost::static_pointer_cast<Guardian>(entity);
        m_minecraft->mSoundEngine->ConsoleSoundEngine::play(new GuardianAttackSoundInstance(guardian));
        return;
    }
    if (eventId != 0x23)
    {
        entity->handleEntityEvent(eventId, packet->getData());
        return;
    }
    m_minecraft->mParticleEngine->createTrackingEmitter(entity, PARTICLE_TOTEM, 30);
    if (m_level != 0)
    {
        m_level->playLocalSound(entity->mX, entity->mY, entity->mZ, SoundEvents::TOTEM_USE, entity->getSoundSource(),
                                1.0f, 1.0f, false, 16.0f);
    }
    if (entity == m_minecraft->mLocalPlayers[m_playerId])
    {
        not_null_ptr<ItemInstance> totemItem(new ItemInstance(Items::totemOfUndying));
        m_minecraft->mGameRenderer->displayItemActivation(totemItem, m_playerId);
    }
}

void ClientPacketListener::handleDamageIndicator(boost::shared_ptr<ClientboundDamageIndicatorPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player != 0)
        player->updateDamageIndicator(static_cast<float>(packet->getX()), static_cast<float>(packet->getZ()),
                                      packet->isAllDirections() != 0);
}

void ClientPacketListener::handleMiniGamePlayerSettingsUpdatePacket(
    boost::shared_ptr<ClientboundMGPlayerSettingsUpdatePacket> packet)
{
    MiniGamePlayerSettings settings = packet->getSettings();
    bool centralSpawnDisabled = settings.mCentralSpawnDisabled;
    settings.mCentralSpawnDisabled = false;
    Minecraft::GetInstance()->GetMiniGame();
    MiniGameDef::SetPlayerSettings(settings);
    GameSettings::SetPlayerSkin(m_playerId, GameSettings::GetPlayerSkinId(m_playerId), false);
    GameSettings::SetPlayerCape(m_playerId, GameSettings::GetPlayerCapeId(m_playerId), false);

    bool notify;
    if (!CGameNetworkManager::sInstance.IsHost() || gProfile->GetPrimaryPad() != m_playerId)
        notify = Minecraft::GetInstance()->GetMiniGame()->GetId() == eMiniGameId_Lobby || centralSpawnDisabled;
    else
        notify = centralSpawnDisabled;

    if (notify && !gConsoleUIController.IsSceneInStack(m_playerId, EUIScene_FullscreenProgress)
        && !gConsoleUIController.IsSceneInStack(m_playerId, EUIScene_ConnectingProgress))
    {
        if (!centralSpawnDisabled)
        {
            std::wstring message = L10N::GetString(IDS_MG_SETTINGS_UPDATED_MSG);
            Minecraft::GetInstance()->mGui->addMessage(message, m_playerId, false, false, false, false);
        }
        else
        {
            std::wstring message = L10N::GetString(0xD5232B99);
            Minecraft::GetInstance()->mGui->addMessage(message, m_playerId, false, false, false, false);
        }

        if (gConsoleUIController.IsSceneInStack(m_playerId, EUIScene_InGameSettingsListMenu))
        {
            UIScene_InGameSettingsListMenu* scene = static_cast<UIScene_InGameSettingsListMenu*>(
                gConsoleUIController.GetTopScene(m_playerId, EUILayer_Scene, EUIGroup_PlayerDefault));
            scene->UpdateSettings(centralSpawnDisabled);
        }
        else if (gConsoleUIController.IsSceneInStack(m_playerId, EUIScene_GameModeScoreboardMenu))
        {
            UIScene_GameModeScoreboardMenu* scene = static_cast<UIScene_GameModeScoreboardMenu*>(
                gConsoleUIController.GetTopScene(m_playerId, EUILayer_Scene, EUIGroup_PlayerDefault));
            scene->CheckReloadMovie();
        }
    }

    if (CGameNetworkManager::sInstance.IsHost())
        return;

    for (int i = 0; i < 4; ++i)
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft->mGameModes[i] == NULL || !minecraft->mGameModes[i]->isSpectator())
            continue;
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        if (!minecraft->GetMiniGame()->IsCustomGame())
            continue;

        std::wstring separator = L" ";
        if (Minecraft::InMiniGame(eMiniGameId_Battle, true))
        {
            std::wstring lives = L10N::GetString(0xE7989CA8) + separator;
            minecraft->mGui->addMessage(lives + _toString<int>(miniGame->GetLivesPerRound()), i, false, false, false,
                                        true);
            std::wstring itemSet = L10N::GetString(0x13ADA435) + separator;
            minecraft->mGui->addMessage(itemSet + miniGame->GetLootSetText(), i, false, false, false, true);
            std::wstring rounds = L10N::GetString(0xD4F52013) + separator;
            minecraft->mGui->addMessage(rounds + _toString<int>(miniGame->GetRoundCount()), i, false, false, false,
                                        true);
        }
        if (Minecraft::InMiniGame(eMiniGameId_Tumble, true))
        {
            std::wstring lives = L10N::GetString(0xE7989CA8) + separator;
            minecraft->mGui->addMessage(lives + _toString<int>(miniGame->GetLivesPerRound()), i, false, false, false,
                                        true);
            std::wstring itemSet = L10N::GetString(0x13ADA435) + separator;
            minecraft->mGui->addMessage(itemSet + miniGame->GetLootSetText(), i, false, false, false, true);
            minecraft->mGui->addMessage(L10N::GetString(0x3E09F1B2) + separator
                                            + _toString<unsigned int>(miniGame->GetLayerCount()),
                                        i, false, false, false, true);
        }
    }
}

void ClientPacketListener::handleSetHealth(boost::shared_ptr<ClientboundSetHealthPacket> packet)
{
    m_minecraft->mLocalPlayers[m_playerId]->hurtTo(packet->getHealth(), packet->damageSource);
    m_minecraft->mLocalPlayers[m_playerId]->getFoodData()->setFoodLevel(packet->getFood());
    m_minecraft->mLocalPlayers[m_playerId]->getFoodData()->setSaturation(packet->getSaturation());
    if (packet->getFood() < static_cast<int>(FoodConstants::HealLevel()) - 1)
    {
        if (m_minecraft->mGameModes[m_playerId] != 0 && !m_minecraft->mGameModes[m_playerId]->hasInfiniteItems())
        {
            Tutorial* tutorial = m_minecraft->mGameModes[m_playerId]->getTutorial();
            if (tutorial != 0)
                tutorial->changeTutorialState(eTutorial_State_FoodBar, 0);
        }
    }
}

void ClientPacketListener::handleSetExperience(boost::shared_ptr<ClientboundSetExperiencePacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player != 0)
        player->setExperienceValues(packet->getExperienceProgress(), packet->getTotalExperience(),
                                    packet->getExperienceLevel());
}

void ClientPacketListener::handleRespawn(boost::shared_ptr<ClientboundRespawnPacket> packet)
{
    if (packet->getDimension() != m_minecraft->mLocalPlayers[m_playerId]->mDimension
        || packet->m_mapSeed != m_minecraft->mLocalPlayers[m_playerId]->mLevel->getSeed())
    {
        const int oldDimension = m_minecraft->mLocalPlayers[m_playerId]->mDimension;
        m_started = false;

        bool netherTransition = false;
        if (m_minecraft->mLocalPlayers[m_playerId]->isAlive())
        {
            if (oldDimension == -1)
                netherTransition = packet->getDimension() == 0;
            else if (oldDimension == 0)
                netherTransition = packet->getDimension() == -1;
        }
        m_suppressStartOnPosition = netherTransition;
        m_level->removeClientPacketListener(this, false);

        MultiPlayerLevel* level = m_minecraft->getLevel(packet->getDimension());
        if (level == NULL)
        {
            LevelSettings* settings = new LevelSettings(
                packet->m_mapSeed, packet->getPlayerGameType(), false,
                m_minecraft->mLevel->getLevelData()->isHardcore(), packet->m_newSeaLevel, packet->getLevelType(),
                packet->m_xzSize, packet->m_hellScale, packet->m_cloudHeight, packet->m_modernEnd, packet->m_biomeScale,
                packet->m_biomeCentreXChunk, packet->m_biomeCentreZChunk);
            level = new MultiPlayerLevel(this, settings, packet->getDimension(), packet->getDifficulty(),
                                         packet->m_seaLevel);
            level->setSavedDataStorage(m_level->mSavedDataStorage);
            level->mIsClientSide = true;
        }
        else
        {
            level->mConnections.push_back(this);
        }

        m_level->removeEntity(m_minecraft->mLocalPlayers[m_playerId]);
        m_level = level;

        boost::shared_ptr<MultiplayerLocalPlayer> activePlayer = m_minecraft->mPlayer;
        m_minecraft->mPlayer = m_minecraft->mLocalPlayers[m_playerId];
        m_minecraft->setLevel(level, -1, boost::shared_ptr<Player>(), true, false);
        m_minecraft->mPlayer = activePlayer;

        telemetryManagerInstance->RecordLevelExit(m_playerId, eSen_LevelExitStatus_Restarted);
        m_minecraft->mLocalPlayers[m_playerId]->mDimension = packet->getDimension();

        if (m_minecraft->mGameModes[m_playerId] != NULL)
        {
            Tutorial* tutorial = m_minecraft->mGameModes[m_playerId]->getTutorial();
            if (tutorial != NULL)
                tutorial->showTutorialPopup(false);
        }
        m_minecraft->mLocalPlayers[m_playerId]->updateRichPresence();

        _ConnectionProgressParams* params = new _ConnectionProgressParams();
        params->m_playerId = m_playerId;
        if (packet->getDimension() == -1)
            params->m_stringId = IDS_PROGRESS_ENTERING_NETHER;
        else if (oldDimension == -1)
            params->m_stringId = IDS_PROGRESS_LEAVING_NETHER;
        else if (packet->getDimension() == 1)
            params->m_stringId = IDS_PROGRESS_ENTERING_END;
        else if (oldDimension == 1)
            params->m_stringId = IDS_PROGRESS_LEAVING_END;
        params->m_setFailTimer = false;
        params->m_showTooltips = false;
        gConsoleUIController.CloseUIScenes(m_playerId, false);
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_ConnectingProgress, params, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        gApp.SetAction(m_playerId, eXuiAction_CloseScenesOnConnect, NULL);
    }

    const int activePlayerIdx = m_minecraft->getLocalPlayerIdx();
    m_minecraft->setLocalPlayerIdx(m_playerId);
    m_minecraft->respawnPlayer(m_minecraft->mLocalPlayers[m_playerId]->mUserIndex, packet->getDimension(),
                               packet->m_newEntityId, packet->shouldKeepOldEntityData());
    m_minecraft->mGameModes[m_playerId]->setLocalMode(packet->getPlayerGameType());
    if (packet->hasPositionAndRotation())
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = boost::static_pointer_cast<MultiplayerLocalPlayer>(
            m_minecraft->mLocalPlayers[m_playerId]);
        player->fjResetBounceHeight();
        player->absMoveTo(packet->m_x, packet->m_y, packet->m_z, packet->m_yRot, packet->m_xRot);
    }
    m_minecraft->setLocalPlayerIdx(activePlayerIdx);
}

void ClientPacketListener::handleExplosion(boost::shared_ptr<ClientboundExplodePacket> packet)
{
    if (!packet->knockbackOnly)
    {
        PIXBeginNamedEvent(0.0f, "Handling explosion");
        Explosion explosion(m_minecraft->mLevel, boost::shared_ptr<Entity>(), packet->getX(), packet->getY(),
                            packet->getZ(), packet->getPower(), packet->getToBlow(), -1.0f);
        PIXBeginNamedEvent(0.0f, "Finalizing");
        MultiPlayerLevel* level = m_minecraft->mLevel;
        level->mAnimateTickReady = false;
        explosion.finalizeExplosion(true, packet->getToBlow());
        level->mAnimateTickReady = true;
        PIXEndNamedEvent();
        PIXEndNamedEvent();
    }
    m_minecraft->mLocalPlayers[m_playerId]->mXd += packet->getKnockbackX();
    m_minecraft->mLocalPlayers[m_playerId]->mYd += packet->getKnockbackY();
    m_minecraft->mLocalPlayers[m_playerId]->mZd += packet->getKnockbackZ();
}

void ClientPacketListener::handleContainerOpen(boost::shared_ptr<ClientboundContainerOpenPacket> packet)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = m_minecraft->mLocalPlayers[m_playerId];
    bool failed = false;

    if (packet->getType() == ClientboundContainerOpenPacket::CONTAINER)
    {
        boost::shared_ptr<SimpleContainer> container(new SimpleContainer(packet->getTitle(), packet->getSize()));
        if (player->openContainer(container))
            player->mContainerMenu->m_containerId = packet->getContainerId();
        else
            failed = true;
    }
    else if (packet->getType() == ClientboundContainerOpenPacket::TRADER_NPC)
    {
        boost::shared_ptr<Villager> villager = boost::dynamic_pointer_cast<Villager>(getEntity(packet->getEntityId()));
        boost::shared_ptr<Player> trader = player;
        boost::shared_ptr<ClientSideMerchant> merchant(
            new ClientSideMerchant(trader, packet->hasCustomName() ? packet->getTitle() : villager->getDisplayName()));
        merchant->setVillager(villager);
        const std::wstring name = packet->hasCustomName() ? packet->getTitle() : villager->getDisplayName();
        if (player->openTrading(merchant, name, packet->getEntityId()))
            player->mContainerMenu->m_containerId = packet->getContainerId();
        else
            failed = true;
    }
    else if (packet->getType() == ClientboundContainerOpenPacket::HORSE)
    {
        boost::shared_ptr<Entity> entity = getEntity(packet->getEntityId());
        if (eTYPE_DERIVED_FROM(EntityType::ABSTRACT_HORSE, entity->GetType()))
        {
            boost::shared_ptr<AbstractHorse> horse = boost::static_pointer_cast<AbstractHorse>(entity);
            boost::shared_ptr<Container> chest(new HorseChest(0, packet->getTitle(), false, packet->getSize()));
            if (player->openHorseInventory(horse, chest))
                player->mContainerMenu->m_containerId = packet->getContainerId();
            else
                failed = true;
        }
    }
#ifdef MINECRAFT_PC
    else if (PcModHooksIsContainer(packet->getType()))
    {
        if (PcModHooksOpenContainer(packet->getType(), player.get(), packet->getContainerId(), packet->getSize(),
                                    packet->getTitle(), packet->getEntityId()))
            return;
        failed = true;
    }
#endif
    else if (packet->getType() != ClientboundContainerOpenPacket::FIREWORKS)
    {
        if (!packet->isContainer())
        {
            LocalBlockInteraction interaction(packet->getType(), packet->getTitle());
            if (player->startBlockInteraction(interaction))
                player->mContainerMenu->m_containerId = packet->getContainerId();
            else
                failed = true;
        }
        else
        {
            boost::shared_ptr<SimpleContainer> container(
                new LocalMenuContainer(packet->getType(), packet->getTitle(), packet->getSize()));
            if (player->openContainer(container))
                player->mContainerMenu->m_containerId = packet->getContainerId();
            else
                failed = true;
        }
    }
    else
    {
        if (player->openFireworks(static_cast<int>(player->mX), static_cast<int>(player->mY),
                                  static_cast<int>(player->mZ)))
        {
            player->mContainerMenu->m_containerId = packet->getContainerId();
            return;
        }
        failed = true;
    }

    if (!failed)
        return;
    if (player->mContainerMenu == player->mInventoryMenu)
        send(boost::shared_ptr<Packet>(new ServerboundContainerClosePacket(packet->getContainerId())));
    else
        gConsoleUIController.CloseUIScenes(m_playerId, false);
}

void ClientPacketListener::handleContainerSetSlot(boost::shared_ptr<ClientboundContainerSetSlotPacket> packet)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = m_minecraft->mLocalPlayers[m_playerId];
    not_null_ptr<ItemInstance> item = packet->getItem();
    int slot = packet->getSlot();
    if (packet->getContainerId() == -1)
    {
        player->mInventory->setCarried(item);
    }
    else if (packet->getContainerId() == -2)
    {
        player->mInventory->setItem(slot, item);
    }
    else if (packet->getContainerId() == 0 && packet->getSlot() >= 36 && packet->getSlot() < 45)
    {
        if (!item->isEmpty())
        {
            not_null_ptr<ItemInstance> existing = player->mInventoryMenu->getSlot(slot)->getItem();
            if (existing->isEmpty() || existing->getCount() < item->getCount())
            {
                item->setPopTime(5);
            }
        }
        player->mInventoryMenu->setItem(slot, item);
    }
    else if (packet->getContainerId() == player->mContainerMenu->m_containerId)
    {
        player->mContainerMenu->setItem(slot, item);
    }
}

void ClientPacketListener::handleContainerAck(boost::shared_ptr<ClientboundContainerAckPacket> packet)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = m_minecraft->mLocalPlayers[m_playerId];
    AbstractContainerMenu* menu = NULL;
    if (packet->getContainerId() == 0)
    {
        menu = player->mInventoryMenu;
    }
    else if (packet->getContainerId() == player->mContainerMenu->m_containerId)
    {
        menu = player->mContainerMenu;
    }
    if (menu != NULL && !packet->isAccepted())
    {
        send(boost::shared_ptr<ServerboundContainerAckPacket>(
            new ServerboundContainerAckPacket(packet->getContainerId(), packet->getUid(), true)));
    }
}

void ClientPacketListener::handleContainerContent(boost::shared_ptr<ClientboundContainerSetContentPacket> packet)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = m_minecraft->mLocalPlayers[m_playerId];
    if (packet->getContainerId() == 0)
    {
        player->mInventoryMenu->setAll(packet->getItems());
    }
    else if (packet->getContainerId() == player->mContainerMenu->m_containerId)
    {
        player->mContainerMenu->setAll(packet->getItems());
    }
}

void ClientPacketListener::handleOpenSignEditor(boost::shared_ptr<ClientboundOpenSignEditorPacket> packet)
{
    if (m_level == 0)
        return;
    boost::shared_ptr<BlockEntity> blockEntity = m_level->getBlockEntity(packet->getPos());
    if (blockEntity == 0 || blockEntity->GetType() != BlockEntityType::SIGN)
    {
        boost::shared_ptr<SignBlockEntity> newSign(new SignBlockEntity());
        newSign->setLevel(m_level);
        newSign->setPosition(packet->getPos());
        blockEntity = newSign;
    }
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    boost::shared_ptr<SignBlockEntity> sign = boost::dynamic_pointer_cast<SignBlockEntity>(blockEntity);
    player->openTextEdit(sign);
}

void ClientPacketListener::handleBlockEntityData(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet)
{
    int levelIndex;
    switch (packet->getDimension())
    {
    case -1:
        levelIndex = 1;
        break;
    case 0:
        levelIndex = 0;
        break;
    case 1:
        levelIndex = 2;
        break;
    default:
        return;
    }
    MultiPlayerLevel* level = m_minecraft->mLevels[levelIndex];
    if (level != 0 && level->MustQueueBlockEntityDataPackets())
    {
        level->QueueBlockEntityDataPacket(packet);
        return;
    }
    handleBlockEntityDataInternal(packet, level);
}

void ClientPacketListener::handleContainerSetData(boost::shared_ptr<ClientboundContainerSetDataPacket> packet)
{
    if (m_minecraft->mLocalPlayers[m_playerId]->mContainerMenu != NULL
        && m_minecraft->mLocalPlayers[m_playerId]->mContainerMenu->m_containerId == packet->getContainerId())
    {
        m_minecraft->mLocalPlayers[m_playerId]->mContainerMenu->setData(packet->getId(), packet->getValue());
    }
}

void ClientPacketListener::handleSetEquippedItem(boost::shared_ptr<ClientboundSetEquippedItemPacket> packet)
{
    if (packet == 0)
        return;
    boost::shared_ptr<Entity> entity = getEntity(packet->getEntity());
    if (entity != 0)
        entity->setItemSlot(packet->getSlot(), packet->getItem());
}

void ClientPacketListener::handleContainerClose(boost::shared_ptr<ClientboundContainerClosePacket> packet)
{
    m_minecraft->mLocalPlayers[m_playerId]->clientSideCloseContainer();
}

void ClientPacketListener::handleBlockEvent(boost::shared_ptr<ClientboundBlockEventPacket> packet)
{
    PIXBeginNamedEvent(0.0f, "Handle block event\n");
    if (packet->getBlock() == m_minecraft->mLevel->getBlockState(packet->getPos())->getBlock())
        m_minecraft->mLevel->blockEvent(packet->getPos(), packet->getBlock(), packet->getB0(), packet->getB1());
    PIXEndNamedEvent();
}

void ClientPacketListener::handleBlockDestruction(boost::shared_ptr<ClientboundBlockDestructionPacket> packet)
{
    m_minecraft->mLevel->destroyBlockProgress(packet->getId(), packet->getPos(), packet->getProgress());
}

void ClientPacketListener::handleBlockCollectionDestruction(
    boost::shared_ptr<ClientboundBlockCollectionDestructionPacket> packet)
{
    if (m_level == 0)
        return;
    if (packet->progress >= 0 && packet->positions.length != 0)
    {
        delete[] m_destroyedBlocks;
        m_destroyedBlockCount = packet->positions.length;
        m_destroyedBlocks = new BlockPos[m_destroyedBlockCount];
        Random random(0xABB);
        for (unsigned int i = 0; i < packet->positions.length; ++i)
        {
            m_destroyedBlocks[i] = packet->positions[i];
            m_level->destroyBlockProgress(packet->sourceId - static_cast<int>(i), m_destroyedBlocks[i],
                                          packet->progress + random.nextInt(4) - 1);
        }
        return;
    }
    Random random(0xABB);
    for (unsigned int i = 0; i < packet->positions.length; ++i)
        m_level->destroyBlockProgress(packet->sourceId - static_cast<int>(i), m_destroyedBlocks[i],
                                      packet->progress + random.nextInt(4) - 1);
}

bool ClientPacketListener::canHandleAsyncPackets()
{
    return m_minecraft != 0 && m_minecraft->mLevel != 0 && m_minecraft->mLocalPlayers[m_playerId] != 0 && m_level != 0;
}
void ClientPacketListener::handleGameEvent(boost::shared_ptr<ClientboundGameEventPacket> packet)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = m_minecraft->mPlayer;
    const int event = packet->getEvent();
    const float param = packet->getParam();
    const int ticks = static_cast<int>(floorf(param + 0.5f));

    if (event == ClientboundGameEventPacket::START_RAINING)
    {
        if (m_level != 0)
        {
            m_level->getLevelData()->setRaining(true);
            m_level->setRainLevel(1.0f);
            if (ticks != 0)
                m_level->tickWeather(true);
        }
    }
    else if (event == ClientboundGameEventPacket::STOP_RAINING)
    {
        if (m_level != 0)
        {
            m_level->getLevelData()->setRaining(false);
            m_level->setRainLevel(0.0f);
            if (ticks != 0)
                m_level->tickWeather(true);
        }
    }
    else if (event == ClientboundGameEventPacket::THUNDER_LEVEL_CHANGE)
    {
        if (m_level != 0)
        {
            m_level->getLevelData()->setThundering(ticks != 0);
            m_level->setThunderLevel(static_cast<float>(ticks));
            if (ticks != 0)
                m_level->tickWeather(true);
        }
    }
    else if (event == ClientboundGameEventPacket::CHANGE_GAME_MODE)
    {
        const GameType* gameType = GameType::byId(ticks);
        boost::shared_ptr<AbstractClientPlayer> target;
        if (packet->getPlayerIndex() >= 0 && packet->getPlayerIndex() < 8)
            target = boost::dynamic_pointer_cast<AbstractClientPlayer>(
                m_minecraft->GetPlayerByPlayerIndex(packet->getPlayerIndex()));
        else
            target = m_minecraft->mLocalPlayers[m_playerId];
        if (target != 0)
        {
            target->GetGameType();
            const MiniGameDef* miniGame = m_minecraft->GetMiniGame();
            if (Minecraft::InMiniGame(eMiniGameId_Any, true))
                miniGame->SupressGameModeMessages();
            target->SetGameType(gameType);
        }
    }
    else if (event == ClientboundGameEventPacket::WIN_GAME)
    {
        if (!gApp.GetGameStarted())
        {
            Minecraft* minecraft = Minecraft::GetInstance();
            for (int i = 0; i < 4; ++i)
            {
                if (minecraft->mLocalPlayers[i] != 0)
                    gApp.SetAction(i, eXuiAction_Respawn, 0);
            }
        }
        else if (!gConsoleUIController.IsSceneInStack(gProfile->GetPrimaryPad(), EUIScene_EndPoem))
        {
            gConsoleUIController.SetWinUserIndex(static_cast<int>(packet->getParam()) & 0xff);
            if (m_minecraft->mGameModes[gProfile->GetPrimaryPad()] != 0)
            {
                Tutorial* tutorial = m_minecraft->mGameModes[gProfile->GetPrimaryPad()]->getTutorial();
                if (tutorial != 0)
                    tutorial->showTutorialPopup(false);
            }
            gConsoleUIController.CloseAllPlayersScenes(false);
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_EndPoem, 0, EUILayer_Scene,
                                                 EUIGroup_Fullscreen);
        }
    }
    else if (event == ClientboundGameEventPacket::START_SAVING)
    {
        if (!CGameNetworkManager::sInstance.IsHost() && IsPrimaryPlayer())
        {
            gApp.SetGameStarted(false);
            gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_RemoteSave, 0);
        }
    }
    else if (event == ClientboundGameEventPacket::STOP_SAVING)
    {
        if (!CGameNetworkManager::sInstance.IsHost() && IsPrimaryPlayer())
            gApp.SetGameStarted(true);
    }
    else if (event == ClientboundGameEventPacket::ARROW_HIT_PLAYER)
    {
        if (m_level != 0)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = m_minecraft->mLocalPlayers[m_playerId];
            m_level->playLocalSound(localPlayer->mX, localPlayer->mY + localPlayer->getEyeHeight(), localPlayer->mZ,
                                    SoundEvents::ARROW_HIT, SoundSource::eSoundSource_Players, 0.18f, 0.45f, false,
                                    16.0f);
        }
    }
    else if (event == ClientboundGameEventPacket::RAIN_LEVEL_CHANGE)
    {
        if (m_level != 0)
            m_level->setRainLevel(param);
    }
    else if (event == ClientboundGameEventPacket::THUNDER_LEVEL_CHANGE)
    {
        if (m_level != 0)
            m_level->setThunderLevel(param);
    }
    else if (event == ClientboundGameEventPacket::GUARDIAN_ELDER_EFFECT)
    {
        if (m_level != 0)
        {
            m_level->addParticle(PARTICLE_MOB_APPEARANCE, player->mX, player->mY, player->mZ, 0.0, 0.0, 0.0,
                                 arrayWithLength<int>());
            m_level->playLocalSound(player->mX, player->mY, player->mZ, SoundEvents::ELDER_GUARDIAN_CURSE,
                                    SoundSource::eSoundSource_Hostile, 1.0f, 1.0f, false, 16.0f);
        }
    }
    else if (event == ClientboundGameEventPacket::NO_RESPAWN_BLOCK_AVAILABLE)
    {
        m_minecraft->mLocalPlayers[m_playerId]->displayClientMessage(IDS_TILE_BED_NOT_VALID, true);
    }
}

void ClientPacketListener::handleMapItemData(boost::shared_ptr<ClientboundMapItemDataPacket> packet)
{
    MapRenderer* mapRenderer = m_minecraft->mGameRenderer->getMapRenderer();
    boost::shared_ptr<MapItemSavedData> mapData = MapItem::getSavedDataIfExists(packet->getMapId(),
                                                                                m_minecraft->mLevel);
    if (mapData == NULL)
    {
        std::wstring id = std::wstring(L"map_") + _toString<int>(packet->getMapId());
        mapData = boost::shared_ptr<MapItemSavedData>(new MapItemSavedData(id));
        if (mapRenderer->getMapInstanceIfExists(id) != NULL)
        {
            boost::shared_ptr<MapItemSavedData> rendererData = mapRenderer->getData(
                mapRenderer->getMapInstanceIfExists(id));
            if (rendererData != NULL)
                mapData = rendererData;
        }
        m_minecraft->mLevel->setSavedData(id, mapData);
    }
    packet->applyToMap(mapData.get());
    mapRenderer->update(mapData);
}

void ClientPacketListener::handleLevelEvent(boost::shared_ptr<ClientboundLevelEventPacket> packet)
{
    if (packet->getType() == LevelEvent::SOUND_DRAGON_DEATH)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (m_minecraft->mLocalPlayers[i] != 0 && m_minecraft->mLocalPlayers[i]->mLevel != 0
                && m_minecraft->mLocalPlayers[i]->mLevel->mDimension->getType() == DimensionType::THE_END)
            {
                m_minecraft->mLocalPlayers[i]->awardStat(GenericStats::completeTheEnd(), GenericStats::param_noArgs());
            }
        }
    }
    if (packet->isGlobalEvent())
        m_minecraft->mLevel->globalLevelEvent(packet->getType(), packet->getPos(), packet->getData());
    else
        m_minecraft->mLevel->levelEvent(packet->getType(), packet->getPos(), packet->getData());
}

void ClientPacketListener::handleAwardStat(boost::shared_ptr<ClientboundAwardStatPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    Stat* stat = Stats::get(packet->statId);
    if (player != 0 && stat != 0)
        player->awardStatFromServer(stat, packet->getParamData());
}

void ClientPacketListener::handleUpdateMobEffect(boost::shared_ptr<ClientboundUpdateMobEffectPacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(static_cast<int>(packet->getEntityId()));
    if (entity == 0 || (entity->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
        return;
    if (MobEffect::byId(packet->getEffectId()) == 0)
        return;
    MobEffectInstance* instance = new MobEffectInstance(MobEffect::byId(packet->getEffectId()),
                                                        packet->getEffectDurationTicks(), packet->getEffectAmplifier(),
                                                        false, packet->isEffectVisible());
    instance->setNoCounter(packet->isSuperLongDuration());
    boost::static_pointer_cast<LivingEntity>(entity)->addEffect(instance, boost::shared_ptr<Entity>());
}

void ClientPacketListener::handlePlayerCombat(boost::shared_ptr<ClientboundPlayerCombatPacket> packet)
{
}

void ClientPacketListener::handleChangeDifficulty(boost::shared_ptr<ClientboundChangeDifficultyPacket> packet)
{
    m_minecraft->mLevel->getLevelData()->setDifficulty(packet->getDifficulty());
    m_minecraft->mLevel->getLevelData()->setDifficultyLocked(packet->isLocked());
}

void ClientPacketListener::handleSetCamera(boost::shared_ptr<ClientboundSetCameraPacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->getCameraId());
    if (entity != 0)
        m_minecraft->setCameraEntity(entity);
}

void ClientPacketListener::handleSetBorder(boost::shared_ptr<ClientboundSetBorderPacket> packet)
{
    if (m_level != NULL)
        packet->applyChanges(m_level->getWorldBorder());
}

void ClientPacketListener::handleSetTitles(boost::shared_ptr<ClientboundSetTitlesPacket> packet)
{
}

void ClientPacketListener::handleTabListCustomisation(boost::shared_ptr<ClientboundTabListPacket> packet)
{
}

void ClientPacketListener::handleRemoveMobEffect(boost::shared_ptr<ClientboundRemoveMobEffectPacket> packet)
{
    boost::shared_ptr<Entity> entity = packet->getEntity(m_level);
    if (entity == 0 || (entity->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
        return;
    boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);
    living->removeEffectNoUpdate(packet->getEffect());
}

bool ClientPacketListener::isServerPacketListener()
{
    return false;
}
void ClientPacketListener::handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet)
{
    uint32_t privileges = gApp.GetPlayerPrivileges(static_cast<uint8_t>(packet->mPlayerId));

    INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerBySmallId(
        static_cast<uint8_t>(packet->mPlayerId));
    if (networkPlayer != 0 && networkPlayer->IsHost())
    {
        Player::enableAllPlayerPrivileges(privileges, true);
        Player::setPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_Moderator, 1);
    }

    gApp.UpdatePlayerInfo(static_cast<uint8_t>(packet->mPlayerId), packet->mPlayerIndex, packet->mPrivileges,
                          packet->mGameType);

    boost::shared_ptr<Entity> entity = getEntity(packet->mEntityId);
    if (entity != 0 && eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All, packet->mPrivileges);
    }

    if (networkPlayer != 0 && networkPlayer->IsLocal())
    {
        for (int i = 0; i < 4; ++i)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> player = m_minecraft->mLocalPlayers[i];
            if (player != 0 && player->mPacketListener != 0
                && player->mPacketListener->getNetworkPlayer() == networkPlayer)
            {
                player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All, packet->mPrivileges);
                displayPrivilegeChanges(player, privileges);
            }
        }
    }
}

void ClientPacketListener::handleKeepAlive(boost::shared_ptr<ClientboundKeepAlivePacket> packet)
{
}

void ClientPacketListener::handlePlayerAbilities(boost::shared_ptr<ClientboundPlayerAbilitiesPacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->getPlayerId());
    boost::shared_ptr<Player> player;
    if (entity != NULL && eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        player = boost::static_pointer_cast<Player>(entity);
    if (player != NULL)
    {
        player->mAbilities.mFlying = packet->isFlying();
        player->mAbilities.mInstabuild = packet->canInstabuild();
        player->mAbilities.mInvulnerable = packet->isInvulnerable();
        player->mAbilities.mMayfly = packet->canFly();
        player->mAbilities.setFlyingSpeed(packet->getFlyingSpeed());
        player->mAbilities.setWalkingSpeed(packet->getWalkingSpeed());
        player->mAbilities.mMayBuild = packet->canBuild();
    }
}

void ClientPacketListener::handleSoundEvent(boost::shared_ptr<ClientboundSoundPacket> packet)
{
    int soundInstance = packet->getSoundInstance();
    if (soundInstance == ClientboundSoundPacket::eSoundInstances_None)
    {
        if (packet->getPlayUI())
        {
            m_minecraft->mSoundEngine->playUI(packet->getSound(), static_cast<float>(packet->getVolume()),
                                              static_cast<float>(packet->getPitch()));
            return;
        }

        m_minecraft->mLevel->playSound(m_minecraft->mLocalPlayers[m_playerId], packet->getX(), packet->getY(),
                                       packet->getZ(), packet->getSound(), packet->getSource(),
                                       static_cast<float>(packet->getVolume()), static_cast<float>(packet->getPitch()),
                                       16.0f);
    }
    else if (soundInstance == ClientboundSoundPacket::eSoundInstances_FollowLocalPlayer)
    {
        boost::shared_ptr<Player> player = m_minecraft->mLocalPlayers[m_playerId];
        m_minecraft->mSoundEngine->ConsoleSoundEngine::play(
            new FollowPlayerSoundInstance(player, packet->getSound(), static_cast<float>(packet->getVolume()),
                                          static_cast<float>(packet->getPitch()), packet->getLooping()));
    }
    else if (soundInstance == ClientboundSoundPacket::eSoundInstances_FollowEntity)
    {
        boost::shared_ptr<Entity> entity = getEntity(packet->getEntityId());
        if (entity != NULL && eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            m_minecraft->mSoundEngine->ConsoleSoundEngine::play(new FollowPlayerSoundInstance(
                boost::static_pointer_cast<Player>(entity), packet->getSound(), static_cast<float>(packet->getVolume()),
                static_cast<float>(packet->getPitch()), packet->getLooping()));
        }
    }
}

void ClientPacketListener::handleMoveVehicle(boost::shared_ptr<ClientboundMoveVehiclePacket> packet)
{
    boost::shared_ptr<Entity> vehicle = m_minecraft->mLocalPlayers[m_playerId]->getRootVehicle();
    if (vehicle != m_minecraft->mLocalPlayers[m_playerId] && vehicle->isControlledByLocalInstance())
    {
        vehicle->absMoveTo(packet->getX(), packet->getY(), packet->getZ(), packet->getYRot(), packet->getXRot());
        m_connection->send(boost::shared_ptr<Packet>(new ServerboundMoveVehiclePacket(vehicle)));
    }
}

void ClientPacketListener::handleBossUpdate(boost::shared_ptr<ClientboundBossEventPacket> packet)
{
    if (m_minecraft != 0 && m_minecraft->mGui != 0 && packet != 0)
        m_minecraft->mGui->getBossOverlay(m_playerId)->update(packet);
}

void ClientPacketListener::handleItemCooldown(boost::shared_ptr<ClientboundCooldownPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player != 0 && player->getCooldowns() != 0)
    {
        if (packet->getDuration() == 0)
            player->getCooldowns()->removeCooldown(packet->getItem());
        else
            player->getCooldowns()->addCooldown(packet->getItem(), packet->getDuration());
    }
}

void ClientPacketListener::handleCustomPayload(boost::shared_ptr<ClientboundCustomPayloadPacket> packet)
{
#ifdef MINECRAFT_PC
    if (packet->getIdentifier() == PC_MOD_MESSAGE_CHANNEL)
    {
        arrayWithLength<unsigned char> modData = packet->getData();
        PcModHooksMessage(0, modData.data, (int)modData.length);
        return;
    }
#endif
    ByteArrayInputStream byteStream(packet->getData());
    DataInputStream input(&byteStream);
    if (packet->getIdentifier() == L"MC|TrList")
    {
        const int containerId = input.readInt();
        if (gConsoleUIController.IsSceneInStack(m_playerId, EUIScene_TradingMenu))
        {
            if (containerId == m_minecraft->mLocalPlayers[m_playerId]->mContainerMenu->m_containerId)
            {
                UIScene_TradingMenu* tradingMenu = static_cast<UIScene_TradingMenu*>(
                    gConsoleUIController.GetTopScene(m_playerId, EUILayer_Scene, EUIGroup_PlayerDefault));
                boost::shared_ptr<Merchant> merchant = tradingMenu->getMerchant();
                merchant->overrideOffers(MerchantRecipeList::createFromStream(&input));
            }
        }
    }
    else if (packet->getIdentifier() == L"MC|BOpen")
    {
        const int hand = input.readInt();
        not_null_ptr<ItemInstance> book = hand == 1 ? m_minecraft->mLocalPlayers[m_playerId]->getOffhandItem()
                                                    : m_minecraft->mLocalPlayers[m_playerId]->getMainHandItem();
        book->getItem();
    }
    byteStream.reset();
}

Connection* ClientPacketListener::getConnection()
{
    return m_connection;
}
void ClientPacketListener::handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet)
{
    if (packet->mSetting == 0)
        gApp.SetGameHostOption(eGameHostOption_Gamertags, packet->mValue);
    else if (packet->mSetting == 1)
        gApp.SetGameHostOption(eGameHostOption_All, packet->mValue);
}

void ClientPacketListener::handleTexture(boost::shared_ptr<TexturePacket> packet)
{
    if (packet == 0)
        return;
    if (packet->length == 0)
    {
        unsigned char* data = 0;
        unsigned int length = 0;
        gApp.GetMemFileDetails(packet->textureName, &data, &length);
        if (length != 0)
            send(boost::shared_ptr<Packet>(new TexturePacket(packet->textureName, data, length)));
    }
    else
    {
        gApp.AddMemoryTextureFile(packet->textureName, packet->data, packet->length);
        Minecraft::GetInstance()->handleClientTextureReceived(packet->textureName);
    }
}

void ClientPacketListener::handleTextureAndGeometry(boost::shared_ptr<TextureAndGeometryPacket> packet)
{
    if (packet->length != 0)
    {
        gApp.AddMemoryTextureFile(packet->textureName, packet->data, packet->length);
        if (packet->additionalBoxCount != 0)
            gApp.SetAdditionalSkinBoxes(packet->geometryId, packet->additionalBoxes, packet->additionalBoxCount,
                                        packet->skinAdjustments);
        gApp.SetSkinAdjustments(packet->geometryId, packet->skinAdjustments);
        Minecraft::GetInstance()->handleClientTextureReceived(packet->textureName);
        return;
    }

    unsigned char* data = NULL;
    unsigned int size = 0;
    gApp.GetMemFileDetails(packet->textureName, &data, &size);
    DLCSkinFile* skinFile = static_cast<DLCSkinFile*>(gApp.mDLCManager.getSkinFile(packet->textureName));
    if (size == 0)
        return;
    if (skinFile != NULL)
    {
        send(boost::shared_ptr<TextureAndGeometryPacket>(
            new TextureAndGeometryPacket(packet->textureName, data, size, skinFile)));
        return;
    }
    send(boost::shared_ptr<TextureAndGeometryPacket>(
        new TextureAndGeometryPacket(packet->textureName, data, size, gApp.GetAdditionalSkinBoxes(packet->geometryId),
                                     gApp.GetSkinAdjustments(packet->geometryId))));
}

void ClientPacketListener::handleUpdateProgress(boost::shared_ptr<UpdateProgressPacket> packet)
{
    if (!CGameNetworkManager::sInstance.IsHost())
        Minecraft::GetInstance()->mProgressRenderer->progressStagePercentage(packet->mProgress);
}

void ClientPacketListener::handleTextureChange(boost::shared_ptr<TextureChangePacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->mEntityId);
    if (entity == NULL || !eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        return;
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    bool isLocalPlayer = false;
    for (int i = 0; i < 4; ++i)
    {
        if (m_minecraft->mLocalPlayers[i] != NULL && m_minecraft->mLocalPlayers[i]->getId() == packet->mEntityId)
        {
            isLocalPlayer = true;
            break;
        }
    }
    if (isLocalPlayer)
        return;

    switch (packet->mChangeType)
    {
    case TextureChangePacket::eTextureChangeType_Skin:
        player->setCustomSkin(CMinecraftApp::getSkinIdFromPath(packet->mTextureName));
        break;
    case TextureChangePacket::eTextureChangeType_Cape:
        player->setCustomCape(Player::getCapeIdFromPath(packet->mTextureName));
        break;
    }

    if (!packet->mTextureName.empty() && packet->mTextureName.substr(0, 3).compare(L"def") != 0
        && !gApp.IsFileInMemoryTextures(packet->mTextureName))
    {
        if (m_minecraft->addPendingClientTextureRequest(packet->mTextureName))
            send(boost::shared_ptr<TexturePacket>(new TexturePacket(packet->mTextureName, NULL, 0)));
    }
    else if (!packet->mTextureName.empty() && gApp.IsFileInMemoryTextures(packet->mTextureName))
    {
        gApp.AddMemoryTextureFile(packet->mTextureName, NULL, 0);
    }
}

void ClientPacketListener::handleTextureAndGeometryChange(boost::shared_ptr<TextureAndGeometryChangePacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->entityId);
    if (entity == NULL)
        return;
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    if (entity == NULL)
        return;
    bool isLocalPlayer = false;
    for (int i = 0; i < 4; ++i)
    {
        if (m_minecraft->mLocalPlayers[i] != NULL && m_minecraft->mLocalPlayers[i]->getId() == packet->entityId)
        {
            isLocalPlayer = true;
            break;
        }
    }
    if (isLocalPlayer)
        return;

    player->setCustomSkin(CMinecraftApp::getSkinIdFromPath(packet->geometryName));

    if (!packet->geometryName.empty() && packet->geometryName.substr(0, 3).compare(L"def") != 0
        && !gApp.IsFileInMemoryTextures(packet->geometryName))
    {
        if (m_minecraft->addPendingClientTextureRequest(packet->geometryName))
            send(boost::shared_ptr<TextureAndGeometryPacket>(
                new TextureAndGeometryPacket(packet->geometryName, NULL, 0)));
    }
    else if (!packet->geometryName.empty() && gApp.IsFileInMemoryTextures(packet->geometryName))
    {
        gApp.AddMemoryTextureFile(packet->geometryName, NULL, 0);
    }
}

void ClientPacketListener::handleUpdateGameRuleProgressPacket(boost::shared_ptr<UpdateGameRuleProgressPacket> packet)
{
    const wchar_t* rulesText = gApp.GetGameRulesString(packet->mDescription);
    if (rulesText != 0)
    {
        std::wstring description(rulesText);
        description = GameRuleDefinition::generateDescriptionString(packet->mRuleType, description,
                                                                    packet->mPayload.data, packet->mPayload.length);
        if (m_minecraft->mGameModes[m_playerId] != 0)
        {
            Tutorial* tutorial = m_minecraft->mGameModes[m_playerId]->getTutorial();
            if (tutorial != 0)
                tutorial->setMessage(description, packet->mItemId, packet->mAuxValue);
        }
    }
    if (packet->mDataTag > 0 && packet->mDataTag <= 32)
        GameSettings::SetSpecialTutorialCompletionFlag(m_playerId, packet->mDataTag - 1);
    delete[] packet->mPayload.data;
}

void ClientPacketListener::handleXZ(boost::shared_ptr<XZPacket> packet)
{
    if (packet->mMode == XZPacket::STRONGHOLD)
    {
        m_minecraft->mLevels[0]->getLevelData()->setXStronghold(packet->mX);
        m_minecraft->mLevels[0]->getLevelData()->setZStronghold(packet->mZ);
        m_minecraft->mLevels[0]->getLevelData()->setHasStronghold();
    }
}

void ClientPacketListener::handleScoreboardPacket(boost::shared_ptr<ScoreboardPacket> packet)
{
    if (m_level != 0)
    {
        ClientScoreboard* scoreboard = dynamic_cast<ClientScoreboard*>(m_level->GetScoreboard());
        switch (packet->GetMessage())
        {
        case ScoreboardPacket::ADD_PLAYER_STATS:
        {
            ScoreboardPacket::AddPlayerStatsData* data = static_cast<ScoreboardPacket::AddPlayerStatsData*>(
                packet->getData());
            if (scoreboard->mCurrentRound < data->mRound)
                scoreboard->mCurrentRound = data->mRound;
            scoreboard->SyncPlayerStats(*data->mStats, data->mStats, data->mRound);
            break;
        }
        case ScoreboardPacket::UPDATE_PLAYER_STATS:
        {
            ScoreboardPacket::UpdatePlayerStatsData* data = static_cast<ScoreboardPacket::UpdatePlayerStatsData*>(
                packet->getData());
            if (scoreboard->mCurrentRound < data->mRound)
                scoreboard->mCurrentRound = data->mRound;
            scoreboard->UpdatePlayerStats(*data->mStats, data->mStats, data->mRound, data->mLife, data->mChanged);
            Minecraft* minecraft = Minecraft::GetInstance();
            if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
            {
                const int checkpoint = scoreboard->GetLastCheckpointID(*data->mStats, -1);
                minecraft->GetClientMasterGameMode()->SetLastCheckpointID(*data->mStats, checkpoint, true);
                if (checkpoint == 0 && scoreboard->GetLastBankedCheckpointID(*data->mStats, -1) != 0)
                    minecraft->GetClientMasterGameMode()->SetLocalRoundTimeStartOverride(
                        System::processTimeInMilliSecs());
            }
            break;
        }
        case ScoreboardPacket::REMOVE_PLAYER_STATS:
        {
            ScoreboardPacket::RemovePlayerStatsData* data = static_cast<ScoreboardPacket::RemovePlayerStatsData*>(
                packet->getData());
            scoreboard->RemovePlayerStats(data->mUID);
            break;
        }
        case ScoreboardPacket::CLEAR_PLAYER_STATS:
            scoreboard->ResetPlayerScores();
            break;
        }
    }
}

void ClientPacketListener::handleClientboundSetPlayerTeamPacket(
    boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet)
{
    if (m_level == NULL)
        return;
    Scoreboard* scoreboard = m_level->GetScoreboard();
    PlayerTeam* team;
    if (packet->getMethod() == 0)
        team = scoreboard->addPlayerTeam(packet->getName());
    else
        team = scoreboard->getPlayerTeam(packet->getName());
    if (packet->getMethod() == 0 || packet->getMethod() == 2)
    {
        team->unpackOptions(packet->getOptions());
        team->setNameTagVisibility(static_cast<Team::Visibility>(packet->getNameTagVisibility()));
    }
    if (packet->getMethod() == 0 || packet->getMethod() == 3)
    {
        for (unsigned int i = 0; i < packet->getPlayers()->size(); i++)
            scoreboard->addPlayerToTeam((*packet->getPlayers())[i], team);
    }
    if (packet->getMethod() == 4)
    {
        for (unsigned int i = 0; i < packet->getPlayers()->size(); i++)
            scoreboard->removePlayerFromTeam((*packet->getPlayers())[i], team);
    }
    if (packet->getMethod() == 1)
        scoreboard->removePlayerTeam(team);
}

void ClientPacketListener::handleParticleEvent(boost::shared_ptr<ClientboundLevelParticlesPacket> packet)
{
    if (m_level == 0)
        return;
    double xa = packet->getXAux();
    double ya = packet->getYAux();
    double za = packet->getZAux();
    if (packet->getCount() == 0)
    {
        if (!packet->getHasAux())
        {
            xa = packet->getMaxSpeed() * packet->getXDist();
            ya = packet->getMaxSpeed() * packet->getYDist();
            za = packet->getMaxSpeed() * packet->getZDist();
        }
        m_level->addParticle(packet->getType(), packet->isOverrideLimiter(), packet->getX(), packet->getY(),
                             packet->getZ(), xa, ya, za, packet->getParams());
    }
    else
    {
        for (int i = 0; i < packet->getCount(); ++i)
        {
            const double xVariance = m_random.nextGaussian() * packet->getXDist();
            const double yVariance = m_random.nextGaussian() * packet->getYDist();
            const double zVariance = m_random.nextGaussian() * packet->getZDist();
            if (!packet->getHasAux())
            {
                xa = m_random.nextGaussian() * packet->getMaxSpeed();
                ya = m_random.nextGaussian() * packet->getMaxSpeed();
                za = m_random.nextGaussian() * packet->getMaxSpeed();
            }
            m_level->addParticle(packet->getType(), packet->isOverrideLimiter(), packet->getX() + xVariance,
                                 packet->getY() + yVariance, packet->getZ() + zVariance, xa, ya, za,
                                 packet->getParams());
        }
    }
}

void ClientPacketListener::handleUpdateAttributes(boost::shared_ptr<ClientboundUpdateAttributesPacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->getEntityId());
    if (entity == 0)
        return;
    if (!eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
        && static_cast<int>(entity->GetType()) != packet->getEntityType())
        return;
    BaseAttributeMap* attributes = boost::dynamic_pointer_cast<LivingEntity>(entity)->getAttributes();
    boost::unordered_set<ClientboundUpdateAttributesPacket::AttributeSnapshot*> snapshots = packet->getValues();
    for (boost::unordered_set<ClientboundUpdateAttributesPacket::AttributeSnapshot*>::iterator it = snapshots.begin();
         it != snapshots.end(); ++it)
    {
        ClientboundUpdateAttributesPacket::AttributeSnapshot* snapshot = *it;
        AttributeInstance* instance = attributes->getInstance(snapshot->getId());
        if (instance == 0)
            instance = attributes->registerAttribute(new RangedAttribute(0, snapshot->getId(), 0.0, DBL_MIN, DBL_MAX));
        instance->setBaseValue(snapshot->getBase());
        instance->removeModifiers();
        boost::unordered_set<AttributeModifier*>* modifiers = snapshot->getModifiers();
        for (boost::unordered_set<AttributeModifier*>::iterator it2 = modifiers->begin(); it2 != modifiers->end();
             ++it2)
        {
            AttributeModifier* modifier = *it2;
            instance->addModifier(
                new AttributeModifier(modifier->getId(), modifier->getAmount(), modifier->getOperation()));
        }
    }
}

void ClientPacketListener::updateTutorialOnGameMode(eTutorial_GameMode gameMode)
{
    if (m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    MultiPlayerGameMode* mode = m_minecraft->mGameModes[m_playerId];
    if (mode != 0 && mode->getTutorial() != 0)
        mode->getTutorial()->onGameModeChange(gameMode);
}
void ClientPacketListener::handleGameMode(boost::shared_ptr<GameModePacket> packet)
{
    switch (packet->GetMessage())
    {
    case GameModePacket::eMessage_SetCamera:
    {
        GameModePacket::SetCameraData data;
        data.camera = -1;
        data.target = -1;
        packet->ReadPacket(data);
        boost::shared_ptr<Player> player = Minecraft::GetInstance()->GetPlayerByPlayerIndex(data.camera);
        boost::shared_ptr<Entity> target = Minecraft::GetInstance()->mLevels[0]->getEntity(data.target);
        if (player != NULL)
        {
            if (target == NULL)
            {
                player->SetCamera(boost::shared_ptr<Entity>());
                player->mPendingCameraId = data.target;
            }
            else
            {
                player->SetCamera(target);
            }
        }
        break;
    }
    case GameModePacket::eMessage_TransitionToRound:
        if (IsPrimaryPlayer())
        {
            GameModePacket::TransitionToRoundData data;
            packet->ReadPacket(data);
            GameModeTransitionParams* params = new GameModeTransitionParams(data.round, false, true, data.transition,
                                                                            data.level, data.x, data.y, data.seed);
            ClientMasterGameMode* master = Minecraft::GetInstance()->GetClientMasterGameMode();
            master->queueTransition(params);
            master->SetCurrentTheme(data.seed, data.mapSize);
            gConsoleUIController.HidePressStart();
            if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
            {
                const MiniGameDef* miniGame = CommonMasterGameMode::GetMiniGame();
                if (Minecraft::GetInstance()->GetGhostController() == NULL)
                {
                    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
                    GhostController* ghostController = new GhostController(
                        gApp.getLevelGenerationOptions()->getUniqueId(), options->getWorldNameKey(),
                        !miniGame->ArePointsActive());
                    Minecraft::GetInstance()->SetGhostController(ghostController);
                }
                else
                {
                    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
                    Minecraft::GetInstance()->GetGhostController()->reset(
                        gApp.getLevelGenerationOptions()->getUniqueId(), options->getWorldNameKey(),
                        !miniGame->ArePointsActive());
                }
            }
        }
        break;
    case GameModePacket::eMessage_Transition:
        if (IsPrimaryPlayer())
        {
            GameModePacket::TransitionData data;
            packet->ReadPacket(data);
            GameModeTransitionParams* params = new GameModeTransitionParams(0, true, false, data.transition, data.level,
                                                                            data.x, data.y, 0x14);
            Minecraft::GetInstance()->GetClientMasterGameMode()->queueTransition(params);
            telemetryManagerInstance->RecordTransitionToLobby();
        }
        break;
    case GameModePacket::eMessage_WaitForChunkLoading:
        if (IsPrimaryPlayer())
        {
            GameModePacket::IntData data;
            packet->ReadPacket(data);
            int brupsToWaitOn = CGameNetworkManager::sInstance.IsHost() ? 0 : data.value;
            ClientMasterGameMode* master = m_minecraft->GetClientMasterGameMode();
            master->SetBRUPSToWaitOn(brupsToWaitOn);
            master->SetServerIsWaitingOnChunkLoading();
        }
        break;
    case GameModePacket::eMessage_RoundStart:
        if (IsPrimaryPlayer())
        {
            m_minecraft->GetClientMasterGameMode()->ClearServerIsWaitingOnChunkLoading();
            GameModePacket::RoundStartGameModeData data;
            packet->ReadPacket(data);
            telemetryManagerInstance->RecordGameModeRoundStart(data.GetRoundId(), data.GetSubRoundId());
        }
        telemetryManagerInstance->RecordLevelExit(m_playerId, eSen_LevelExitStatus_Restarted);
        telemetryManagerInstance->RecordLevelStart(m_playerId, eSen_FriendOrMatch_Friend, eSen_CompeteOrCoop_Both,
                                                   Minecraft::GetInstance()->getLevel(0)->getDifficulty()->getId(),
                                                   gApp.GetLocalPlayerCount(),
                                                   CGameNetworkManager::sInstance.GetOnlinePlayerCount());
        break;
    case GameModePacket::eMessage_SyncTeamInvite:
    {
        GameModePacket::SyncTeamInviteData data;
        packet->ReadPacket(data);
        m_minecraft->mLocalPlayers[m_playerId]->GetLocalGameMode()->SyncTeamInvite(data.invite);
        break;
    }
    case GameModePacket::eMessage_RemoveTeamInvite:
    {
        GameModePacket::TeamIDData data;
        packet->ReadPacket(data);
        m_minecraft->mLocalPlayers[m_playerId]->GetLocalGameMode()->RemoveTeamInvite(data.teamId);
        break;
    }
    case GameModePacket::eMessage_AddAreaBounds:
    {
        GameModePacket::AddAreaBoundsData data;
        packet->ReadPacket(data);
        AreaConstraint* constraint = new AreaConstraint(-1, data.minX, data.minY, data.minZ, data.maxX, data.maxY,
                                                        data.maxZ, true, true, true, 0);
        m_minecraft->mGameModes[m_playerId]->getGameModeConstraints()->AddConstraint(constraint, true);
        break;
    }
    case GameModePacket::eMessage_SetAreaBounds:
    {
        GameModePacket::AddAreaBoundsData data;
        packet->ReadPacket(data);
        GameModeConstraints* constraints = m_minecraft->mGameModes[m_playerId]->getGameModeConstraints();
        constraints->ClearAllAreaConstraints();
        AreaConstraint* constraint = new AreaConstraint(-1, data.minX, data.minY, data.minZ, data.maxX, data.maxY,
                                                        data.maxZ, true, true, true, 2);
        constraints->AddConstraint(constraint, true);
        break;
    }
    case GameModePacket::eMessage_ClearAreaBounds:
        m_minecraft->mGameModes[m_playerId]->getGameModeConstraints()->ClearAllAreaConstraints();
        break;
    case GameModePacket::eMessage_ChangeCustomGameMode:
    {
        ClientMasterGameMode* master = m_minecraft->GetClientMasterGameMode();
        if (IsPrimaryPlayer() && !CGameNetworkManager::sInstance.IsHost())
        {
            GameModePacket::ChangeCustomGameMode data;
            packet->ReadPacket(data);
            int previousUniqueId = master->GetCurrentUniqueId();
            master->SetCurrentUniqueId(data.customGameMode);
            m_minecraft->SetupMiniGameInstance(
                *MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(data.gameMode), true), 0);
            if (previousUniqueId != data.customGameMode)
            {
                if (data.customGameMode == -1)
                {
                    std::vector<LevelGenerationOptions*>* generators = gApp.getLevelGenerators();
                    for (std::vector<LevelGenerationOptions*>::iterator it = generators->begin();
                         it != generators->end(); ++it)
                    {
                        LevelGenerationOptions* options = *it;
                        if (options->isGameModeLobby())
                        {
                            options->setBaseSaveData(File(options->getBaseSaveRoot() + options->getBaseSavePath()));
                            gApp.setLevelGenerationOptions(options);
                            break;
                        }
                    }
                    m_minecraft->getLevel(0)->BlockUntilSafeToChangeSaveData(true);
                    m_minecraft->getLevel(0)->refreshFromLevelGen();
                    gRegionFileCache._clear();
                    gApp.setLevelGenerationOptions(NULL);
                }
                else
                {
                    std::vector<LevelRuleset*> rules;
                    gApp.getLevelRules()->getRulesByType(static_cast<LevelRules::ELevelRuleType>(data.gameMode), &rules,
                                                         false);
                    for (std::vector<LevelRuleset*>::iterator it = rules.begin(); it != rules.end(); ++it)
                    {
                        LevelGenerationOptions* options = (*it)->getGenerationOptions();
                        if (options->getUniqueId() == data.customGameMode)
                        {
                            m_minecraft->getLevel(0)->BlockUntilSafeToChangeSaveData(true);
                            gApp.setLevelGenerationOptions(options);
                            master->SetNextLevelGenerationOptions(options, true);
                            break;
                        }
                    }
                }
                if (m_minecraft->IsJoiningFromFriendsTabOrInvite())
                {
                    CMinecraftApp::SetupMiniGameMusic();
                    m_minecraft->SetJoiningFromFriendsTabOrInvite(false);
                }
            }
            if (data.gameMode == eMiniGameId_Glide)
                master->ResetMapDistances();
        }
        if (IsPrimaryPlayer() && CGameNetworkManager::sInstance.IsHost())
            m_minecraft->getLevel(0)->refreshLevelInfoFromLevelGen();
        master->onChangeCustomGameMode();
        break;
    }
    case GameModePacket::eMessage_SetNextCustomGameMode:
    {
        GameModePacket::ChangeCustomGameMode data;
        packet->ReadPacket(data);
        std::vector<LevelRuleset*> rules;
        gApp.getLevelRules()->getRulesByType(static_cast<LevelRules::ELevelRuleType>(data.gameMode), &rules, false);
        if (IsPrimaryPlayer())
        {
            for (std::vector<LevelRuleset*>::iterator it = rules.begin(); it != rules.end(); ++it)
            {
                LevelGenerationOptions* options = (*it)->getGenerationOptions();
                if (options->getUniqueId() == data.customGameMode)
                {
                    m_minecraft->GetClientMasterGameMode()->SetNextLevelGenerationOptions(options, false);
                    break;
                }
            }
        }
        break;
    }
    case GameModePacket::eMessage_LockPlayerLocation:
    {
        GameModePacket::LockPlayerLocation data;
        packet->ReadPacket(data);
        m_minecraft->mLocalPlayers[m_playerId]->SetPositionLocked(data.locked);
        break;
    }
    case GameModePacket::eMessage_ChestRefill:
    {
        GameModePacket::ChestRefill data;
        packet->ReadPacket(data);
        if (m_level != NULL)
        {
            if (gApp.GetLocalPlayerCount() >= 2 || m_minecraft->mPlayer->isAlive())
                gConsoleUIController.ShowChestRefillAnimation(gApp.GetLocalPlayerCount() > 1);
            for (unsigned int i = 0; i < data.positions.size(); ++i)
            {
                boost::shared_ptr<ChestBlockEntity> chest = boost::dynamic_pointer_cast<ChestBlockEntity>(
                    m_level->getBlockEntity(data.positions[i]));
                if (chest != NULL)
                    chest->startRefilledAnimation();
            }
            updateTutorialOnGameMode(eTutorial_GameMode_ChestRefill);
            if (IsPrimaryPlayer())
                m_minecraft->GetClientMasterGameMode()->OnChestRefill();
        }
        break;
    }
    case GameModePacket::eMessage_Tutorial:
    {
        GameModePacket::TutorialGameModeData data;
        packet->ReadPacket(data);
        updateTutorialOnGameMode(static_cast<eTutorial_GameMode>(data.tutorial));
        break;
    }
    case GameModePacket::eMessage_Showdown:
        if (IsPrimaryPlayer())
            m_minecraft->GetClientMasterGameMode()->OnShowdown();
        if (m_minecraft->mLocalPlayers[m_playerId] != NULL)
        {
            std::wstring text = L10N::GetString(0x7ABF0862);
            if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
                text = L10N::GetString(IDS_MG03_HURRY_UP);
            gConsoleUIController.ShowTimedSplash(m_playerId, text, 60);
        }
        break;
    case GameModePacket::eMessage_StateChange:
    {
        GameModePacket::StateChangeData data;
        packet->ReadPacket(data);
        if (IsPrimaryPlayer())
            m_minecraft->GetClientMasterGameMode()->OnStateChange(data.state);
        break;
    }
    case GameModePacket::eMessage_DeathPosition:
        if (IsPrimaryPlayer())
        {
            GameModePacket::SoundPositionData data;
            packet->ReadPacket(data);
            Vec3* position = Vec3::newTemp(data.x, data.y, data.z);
            m_minecraft->GetClientMasterGameMode()->OnDeath(position);
        }
        break;
    case GameModePacket::eMessage_UpdateSeed:
        if (IsPrimaryPlayer())
        {
            GameModePacket::UpdateSeedGameModeData data;
            packet->ReadPacket(data);
            m_minecraft->GetClientMasterGameMode()->SetGeneratorSeed(data.getSeed(), data.getMapSizeMultiplier());
        }
        break;
    case GameModePacket::eMessage_UpdateTheme:
        if (IsPrimaryPlayer())
        {
            GameModePacket::UpdateThemeGameModeData data;
            packet->ReadPacket(data);
            m_minecraft->GetClientMasterGameMode()->setChosenThemeWordId(data.getThemeData());
        }
        break;
    case GameModePacket::eMessage_UpdateThemeId:
        if (IsPrimaryPlayer())
        {
            GameModePacket::UpdateThemeIdData data;
            packet->ReadPacket(data);
            m_minecraft->GetClientMasterGameMode()->SetCurrentTheme(data.themeId, data.previousThemeId);
        }
        break;
    case GameModePacket::eMessage_UpdateStartTime:
        if (IsPrimaryPlayer())
        {
            GameModePacket::UpdateStartTimeData data;
            packet->ReadPacket(data);
            m_minecraft->GetClientMasterGameMode()->SetLocalRoundTimeStart(data.startTime);
        }
        break;
    }
}

void ClientPacketListener::handleMapSelectInfo(boost::shared_ptr<MapSelectInfoPacket> packet)
{
    ByteArrayInputStream bytes(packet->getData());
    DataInputStream input(&bytes);
    std::wstring name(L"");
    ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
    gameMode->ClearPlaylist();
    if (packet->mSize != 0 && packet->mData.data != NULL)
    {
        const int count = input.readInt();
        const int miniGameType = gApp.GetFirstMiniGameType();
        if (miniGameType == 1 || miniGameType == 3)
        {
            for (int i = 0; i < count; ++i)
            {
                const int uniqueId = input.readInt();
                name = input.readUTF();
                const int texturePackId = input.readInt();
                gameMode->AddMapToPlaylist(uniqueId, name, texturePackId);
            }
        }
        else if (miniGameType == 4)
        {
            std::vector<int> layerIds;
            for (int i = 0; i < count; ++i)
                layerIds.push_back(input.readInt());
            gameMode->clearBuildOffVotes();
            gameMode->mThemeWordIds = std::vector<int>(layerIds);
            gameMode->SetCurrentUniqueId(-1);
        }
    }
}

void ClientPacketListener::handlePlayerReady(boost::shared_ptr<PlayerReadyPacket> packet)
{
    ClientMasterGameMode* masterGameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
    masterGameMode->SetPlayerReady(packet->mPlayerId, packet->mReady);
}

void ClientPacketListener::handlePowerup(boost::shared_ptr<ClientboundPowerupPacket> packet)
{
    if (packet->getPowerupTime() > 0)
    {
        PowerupItems::ActivateTimedPowerup(m_minecraft->mLocalPlayers[m_playerId],
                                           static_cast<PowerupItems::eGlide_Timed_Powerup_ID>(packet->getPowerupID()),
                                           packet->getPowerupTime());
    }
}

void ClientPacketListener::handleVote(boost::shared_ptr<VotePacket> packet)
{
    if (packet->voteableId >= -1 && packet->voteableId <= 4)
    {
        ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
        if (gameMode->mVoteables->getVoteable(packet->voteableId) != 0)
            gameMode->mVoteables->getVoteable(packet->voteableId)
                ->setVote(packet->playerId, packet->voteId, packet->vote);
    }
}

void ClientPacketListener::staticCtor()
{
    InitializeCriticalSection(&s_deferredLinkMutex);
}

ClientPacketListener::DeferredEntityLinkPacket::DeferredEntityLinkPacket(
    boost::shared_ptr<ClientboundSetEntityLinkPacket> packet)
{
    mTick = GetTickCount();
    mPacket = packet;
}
