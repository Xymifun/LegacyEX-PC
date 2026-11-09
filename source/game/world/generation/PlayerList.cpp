#include "game/world/generation/PlayerList.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcSettings.hpp"
#include "platform/pc/PcDedicated.hpp"
#include "platform/pc/PcModContent.hpp"
#include "platform/pc/PcScripts.hpp"
#endif
#include "platform/wiiu/CProfile.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/unordered_set.hpp>
#include <cmath>
#include <cstring>
#include <limits>
#include <set>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameRulesInstance.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/Settings.hpp"
#include "game/core/Socket.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PortalForcer.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/world/generation/PlayerIO.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ClientboundLoginPacket.hpp"
#include "game/network/TexturePacket.hpp"
#include "game/network/XZPacket.hpp"
#include "game/network/TextureAndGeometryPacket.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/ClientboundRespawnPacket.hpp"
#include "game/network/ClientboundSetExperiencePacket.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/network/ClientboundSetCarriedItemPacket.hpp"
#include "game/network/ClientboundSetPlayerTeamPacket.hpp"
#include "game/network/ClientboundSetSpawnPositionPacket.hpp"
#include "game/network/ClientboundSetTimePacket.hpp"
#include "game/network/ClientboundUpdateMobEffectPacket.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/network/ProgressListener.hpp"
#include "game/network/ServerConnection.hpp"
#include "game/network/ServerConnectionListener.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/server/ServerScoreboard.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/PlayerChunkMap.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/LevelSettings.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/Level.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/MinecraftServer.hpp"

class MobEffectInstance;

PlayerList::PlayerList(MinecraftServer* server, MasterGameMode* gameMode)
    : mServer(server), mGameMode(gameMode), mPlayerIO(NULL), mUnknownState(false), mOverrideGameMode(NULL),
      mAllowCheatsForAllPlayers(false), mViewDistance(10), mMaxRange(0)
{
#ifdef MINECRAFT_PC
    mMaxPlayers = GameSessionData::MAX_SESSION_PLAYERS;
#else
    mMaxPlayers = server->mSettings->getInt(eServerSetting_MaxPlayers, 20);
#endif
    InitializeCriticalSection(&mPlayersToKickLock);
    InitializeCriticalSection(&mListenersToCloseLock);
}

PlayerList::~PlayerList()
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin(); it < mPlayers.end(); ++it)
        (*it)->mConnection.reset();
    DeleteCriticalSection(&mPlayersToKickLock);
    DeleteCriticalSection(&mListenersToCloseLock);
}

bool PlayerList::canReceiveAllPackets(const boost::shared_ptr<ServerPlayer>& player)
{
    int group = 0;
    if (player->mDimension == -1)
        group = 1;
    else if (player->mDimension == 1)
        group = 2;
    std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it;
    for (it = mReceivingPlayers[group].begin(); it != mReceivingPlayers[group].end(); ++it)
        if (*it == player)
            return true;
    return false;
}

bool PlayerList::isOp(boost::shared_ptr<ServerPlayer> player)
{
    if (!player || gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) == 0)
        return false;
    INetworkPlayer* networkPlayer = player->mConnection != NULL
        ? player->mConnection->getNetworkPlayer() : NULL;
    return player->isModerator() ||
           (networkPlayer != NULL && networkPlayer->IsHost());
}

int PlayerList::getViewDistance()
{
    return mViewDistance;
}

boost::shared_ptr<ServerPlayer> PlayerList::findAlivePlayerOnSystem(boost::shared_ptr<ServerPlayer> player)
{
    int dimension = player->mDimension;
    INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
    if (networkPlayer != NULL)
    {
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
        {
            boost::shared_ptr<ServerPlayer> other = *it;
            INetworkPlayer* otherNetworkPlayer = other->mConnection->getNetworkPlayer();
            if (!other->mRemoved && other != player && other->mDimension == dimension && otherNetworkPlayer != NULL
                && otherNetworkPlayer->IsSameSystem(networkPlayer))
            {
                return other;
            }
        }
    }
    return boost::shared_ptr<ServerPlayer>();
}

int PlayerList::removePlayerFromReceiving(boost::shared_ptr<ServerPlayer> player, bool usePlayerDimension,
                                          int dimension)
{
    int playerDimension;
    if (usePlayerDimension)
        playerDimension = player->mDimension;
    else
        playerDimension = dimension;
    int group = playerDimension;
    if (group == -1)
        group = 1;
    else if (group == 1)
        group = 2;

    bool removed = false;
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator found =
        std::find(mReceivingPlayers[group].begin(), mReceivingPlayers[group].end(), player);
    if (found != mReceivingPlayers[group].end())
    {
        mReceivingPlayers[group].erase(found);
        removed = true;
    }

    INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
    if (networkPlayer != NULL)
    {
        if (removed)
        {
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
            {
                boost::shared_ptr<ServerPlayer> other = *it;
                INetworkPlayer* otherNetworkPlayer = other->mConnection->getNetworkPlayer();
                if (other != player && other->mDimension == playerDimension && otherNetworkPlayer != NULL
                    && otherNetworkPlayer->IsSameSystem(networkPlayer))
                {
                    mReceivingPlayers[group].push_back(other);
                    break;
                }
            }
        }
    }
    else
    {
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
        {
            boost::shared_ptr<ServerPlayer> other = *it;
            INetworkPlayer* otherNetworkPlayer = other->mConnection->getNetworkPlayer();
            if (otherNetworkPlayer != NULL)
            {
                int otherGroup = 0;
                if (other->mDimension == -1)
                    otherGroup = 1;
                else if (other->mDimension == 1)
                    otherGroup = 2;

                bool systemHasReceiver = false;
                for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator receiver =
                         mReceivingPlayers[otherGroup].begin();
                     receiver != mReceivingPlayers[otherGroup].end(); ++receiver)
                {
                    boost::shared_ptr<ServerPlayer> receivingPlayer = *receiver;
                    INetworkPlayer* receivingNetworkPlayer = receivingPlayer->mConnection->getNetworkPlayer();
                    if (receivingNetworkPlayer != NULL && otherNetworkPlayer->IsSameSystem(receivingNetworkPlayer))
                    {
                        systemHasReceiver = true;
                        break;
                    }
                }

                if (!systemHasReceiver)
                    mReceivingPlayers[otherGroup].push_back(other);
            }
        }
    }

    return removed;
}

int PlayerList::addPlayerToReceiving(boost::shared_ptr<ServerPlayer> player, bool force)
{
    int group = 0;
    if (player->mDimension == -1)
        group = 1;
    else if (player->mDimension == 1)
        group = 2;

    int added = 1;
    INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
    if (networkPlayer == NULL)
        added = 0;
    else
    {
        std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
        for (it = mReceivingPlayers[group].begin(); it != mReceivingPlayers[group].end(); ++it)
        {
            boost::shared_ptr<ServerPlayer> other = *it;
            INetworkPlayer* otherNetworkPlayer = other->mConnection->getNetworkPlayer();
            if (otherNetworkPlayer == NULL || !otherNetworkPlayer->IsSameSystem(networkPlayer))
                continue;
            if (force)
                mReceivingPlayers[group].erase(it);
            else
                added = 0;
            break;
        }
        if (added)
            mReceivingPlayers[group].push_back(player);
    }
    return added;
}

void PlayerList::broadcastAll(boost::shared_ptr<Packet> packet)
{
    for (unsigned int i = 0; i < mPlayers.size(); i++)
    {
        boost::shared_ptr<ServerPlayer> player = mPlayers[i];
        player->mConnection->send(packet);
    }
}

void PlayerList::save(boost::shared_ptr<ServerPlayer> player)
{
    mPlayerIO->save(player);
}

void PlayerList::saveAll(ProgressListener* listener, bool deleteGuestMaps)
{
    if (listener != NULL)
        listener->progressStart(IDS_PROGRESS_SAVING_PLAYERS);

    if (mPlayerIO)
    {
        mPlayerIO->saveAllCachedData();
        for (unsigned int i = 0; i < mPlayers.size(); i++)
        {
            mPlayerIO->save(mPlayers[i]);
            if (deleteGuestMaps && mPlayers[i]->mIsGuest
#ifdef MINECRAFT_PC
                && (mPlayers[i]->mXuid.flags & PlayerUID::FlagUuid) == 0
#endif
            )
                mPlayerIO->deleteMapFilesForPlayer(mPlayers[i]);
            if (listener != NULL)
                listener->progressStagePercentage((i * 100) / mPlayers.size());
        }
        mPlayerIO->clearOldPlayerFiles();
        mPlayerIO->saveMapIdLookup();
    }
}

void PlayerList::remove(boost::shared_ptr<ServerPlayer> player)
{
#ifdef MINECRAFT_PC
    PcScriptsHostPlayerLeaving(player.get());
#endif
    if (mServer->mMasterGameMode)
        mServer->mMasterGameMode->OnLeave(player);
    else
        save(player);

    if (player->mIsGuest
#ifdef MINECRAFT_PC
        && (player->mXuid.flags & PlayerUID::FlagUuid) == 0
#endif
    )
        mPlayerIO->deleteMapFilesForPlayer(player);

    ServerLevel* level = player->getLevel();
    if (player->isPassenger())
    {
        boost::shared_ptr<Entity> rootVehicle = player->getRootVehicle();
        if (rootVehicle)
        {
            std::set<boost::shared_ptr<Entity> >* players =
                rootVehicle->getIndirectPassengers(EntityType::SERVER_PLAYER);
            const bool onlyPassenger = players->size() == 1;
            delete players;

            if (onlyPassenger)
            {
                player->stopRiding();
                level->removeEntityImmediately(rootVehicle);
                std::set<boost::shared_ptr<Entity> >* passengers =
                    rootVehicle->getIndirectPassengers();
                for (std::set<boost::shared_ptr<Entity> >::const_iterator it =
                         passengers->begin(); it != passengers->end(); ++it)
                    level->removeEntityImmediately(*it);
                delete passengers;
                level->getChunk(player->mChunkX, player->mChunkZ)->markUnsaved();
            }
        }
    }

    level->removeEntity(player);
    level->getChunkMap()->remove(player);
    level->getChunkMap()->onPlayerLeave(player);

    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        if (*it == player)
            it = mPlayers.erase(it);
        else
            ++it;
    }
#ifdef MINECRAFT_PC
    applyViewDistanceCap();
#endif

    removePlayerFromReceiving(player, true, 0);
    player->mConnection = boost::shared_ptr<ServerGamePacketListenerImpl>();
    saveAll(NULL, false);
}

#ifdef MINECRAFT_PC
void PlayerList::applyViewDistanceCap()
{
    static int s_lastCap = -1;
    static size_t s_lastPlayers = 0;
    const int cap = PcSettingsServerRadiusCap(static_cast<int>(mPlayers.size()));
    if (cap != s_lastCap || mPlayers.size() != s_lastPlayers)
    {
        printf("[pc] server distance: %u players, each loads at most %d chunks around them\n",
               static_cast<unsigned int>(mPlayers.size()), cap);
        fflush(stdout);
        s_lastCap = cap;
        s_lastPlayers = mPlayers.size();
    }
    for (size_t i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = mPlayers[i];
        if (player == NULL || player->mConnection == NULL || player->mConnection->mRequestedViewDistance <= 0)
            continue;
        const int wanted = player->mConnection->mRequestedViewDistance;
        const int allowed = wanted < cap ? wanted : cap;
        if (allowed == player->mConnection->mViewDistance)
            continue;
        ServerLevel* level = player->getLevel();
        if (level != NULL && level->getChunkMap() != NULL)
            level->getChunkMap()->setPlayerViewDistance(player, allowed);
        else
            player->mConnection->mViewDistance = allowed;
        if (allowed < wanted)
            printf("[pc] server distance: %ls loads %d of the %d chunks it asked for\n", player->getName().c_str(),
                   allowed, wanted);
    }
}
#endif

void PlayerList::removeAll()
{
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
        mPlayers[i]->mConnection->disconnect(DisconnectPacket::eDisconnectReason_ConnectionClosed);
}

CompoundTag* PlayerList::load(boost::shared_ptr<ServerPlayer> player)
{
    return mPlayerIO->load(player);
}

void PlayerList::validatePlayerSpawnPosition(boost::shared_ptr<ServerPlayer> player)
{
    const double maxSpawnY = 512.0;
    const double maxNetherSpawnY = 125.0;
    const double halfBlock = 0.5;
    const double blockHeight = 1.0;
    const double respawnYOffset = 0.1;

    const bool forced = player->isRespawnForced() != 0;
    ServerLevel* level = mServer->getLevel(player->mDimension);

    double x;
    if (player->mX >= 0.0)
        x = Mth::floor(player->mX) + halfBlock;
    else
        x = Mth::ceil((float)player->mX) - halfBlock;

    double y;
    if (level->mDimension->isHasCeiling())
        y = floor(player->mY);
    else
        y = ceil(player->mY);

    double z;
    if (player->mZ >= 0.0)
        z = Mth::floor(player->mZ) + halfBlock;
    else
        z = Mth::ceil((float)player->mZ) - halfBlock;

    const int startCollisions =
        level->getCollisionAABBs(player, player->mBoundingBox, false, false, false)->size();

    const double originalX = player->mX;
    const double originalY = player->mY;
    const double originalZ = player->mZ;
    player->setPos(x, y, z);

    if (startCollisions == 0)
    {
        while (!level->getCollisionAABBs(player, player->mBoundingBox, false, false, false)->empty()
               && player->mY < maxSpawnY)
            player->setPos(originalX, originalY, originalZ);
    }
    else
    {
        while (!level->getCollisionAABBs(player, player->mBoundingBox, false, false, false)->empty()
               && player->mY < maxSpawnY)
            player->setPos(player->mX, player->mY + blockHeight, player->mZ);
    }

    if ((level->mDimension->getType() == DimensionType::NETHER && player->mY > maxNetherSpawnY)
        || (level->mDimension->getType() == DimensionType::THE_END
            && player->mY == std::numeric_limits<double>::max()))
    {
        player->setLevel(mServer->getLevel(0));
        player->mGameMode->setLevel(mServer->getLevel(0));
        player->mDimension = 0;
        ServerLevel* overworld = mServer->getLevel(player->mDimension);
        BlockPos spawn = overworld->getSharedSpawnPos();
        player->setPos(spawn.x, spawn.y, spawn.z);

        BlockPos* respawnPos = player->getRespawnPosition();
        if (respawnPos != NULL)
        {
            BlockPos* validPos = Player::checkBedValidRespawnPosition(
                mServer->getLevel(player->mDimension), *respawnPos, forced);
            if (validPos != NULL)
            {
                player->moveTo(validPos->x + halfBlock, validPos->y + respawnYOffset,
                               validPos->z + halfBlock, 0.0f, 0.0f);
                player->setRespawnPosition(new BlockPos(*respawnPos), forced);
            }
            delete validPos;
        }

        while (!overworld->getCollisionAABBs(player, player->mBoundingBox, false, false, false)->empty())
            player->setPos(player->mX, player->mY + blockHeight, player->mZ);
    }
    else
    {
        level->mDimension->getType();
    }
}

void PlayerList::updatePlayerGameMode(boost::shared_ptr<ServerPlayer> oldPlayer,
                                      boost::shared_ptr<ServerPlayer> newPlayer, Level* level)
{
    if (newPlayer)
    {
        oldPlayer->mGameMode->SetGameType(newPlayer->mGameMode->GetGameType());
    }
    else if (mOverrideGameMode != NULL)
    {
        oldPlayer->mGameMode->SetGameType(mOverrideGameMode);
    }
    else
    {
        oldPlayer->mGameMode->updateGameMode(level->getLevelData()->getGameType());
    }
}

int PlayerList::getMaxPlayers()
{
    return mMaxPlayers;
}

MinecraftServer* PlayerList::getServer()
{
    return mServer;
}

void PlayerList::updateEntireScoreboard(ServerScoreboard* scoreboard,
                                        boost::shared_ptr<ServerPlayer> player)
{
    std::vector<PlayerTeam*>* teams = scoreboard->getPlayerTeams();
    for (std::vector<PlayerTeam*>::iterator it = teams->begin(); it != teams->end(); ++it)
    {
        player->mConnection->send(boost::shared_ptr<Packet>(
            new ClientboundSetPlayerTeamPacket(*it, 0)));
    }
    delete teams;
}

void PlayerList::sendLevelInfo(boost::shared_ptr<ServerPlayer> player, ServerLevel* level)
{
    player->mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundSetTimePacket(level->getGameTime(), level->getDayTime(),
                                     level->getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT))));
    if (level->isRaining())
    {
        player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundGameEventPacket(ClientboundGameEventPacket::START_RAINING, 0.0f)));
    }
    else
    {
        player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundGameEventPacket(ClientboundGameEventPacket::STOP_RAINING, 0.0f)));
    }
    BlockPos spawnPos = level->getSharedSpawnPos();
    player->mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundSetSpawnPositionPacket(spawnPos)));
    if (level->mDimension->getType() == DimensionType::OVERWORLD && level->getLevelData()->getHasStronghold())
    {
        player->mConnection->send(boost::shared_ptr<Packet>(
            new XZPacket(XZPacket::STRONGHOLD, level->getLevelData()->getXStronghold(),
                         level->getLevelData()->getZStronghold())));
    }
}

void PlayerList::changeDimension(boost::shared_ptr<ServerPlayer> player, ServerLevel* level)
{
    BOOST_ASSERT(player);
    ServerLevel* newLevel = player->getLevel();
    if (level != NULL)
        level->getChunkMap()->remove(player);
    newLevel->getChunkMap()->add(player);
    newLevel->getChunkSource()->create(static_cast<int>(player->mX) >> 4,
                                       static_cast<int>(player->mZ) >> 4);
}

void PlayerList::add(boost::shared_ptr<ServerPlayer> player)
{
#ifdef MINECRAFT_PC
    if (player->mConnection->getNetworkPlayer() != NULL && !player->mHiddenHost)
#else
    if (player->mConnection->getNetworkPlayer() != NULL)
#endif
        broadcastAll(boost::shared_ptr<Packet>(new PlayerInfoPacket(player)));

    mPlayers.push_back(player);
    addPlayerToReceiving(player, false);
#ifdef MINECRAFT_PC
    applyViewDistanceCap();
#endif

    ServerLevel* level = mServer->getLevel(player->mDimension);
    changeDimension(player, NULL);
    level->addEntity(player);

    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
#ifdef MINECRAFT_PC
        if ((*it)->mConnection->getNetworkPlayer() != NULL && !(*it)->mHiddenHost)
#else
        if ((*it)->mConnection->getNetworkPlayer() != NULL)
#endif
            player->mConnection->send(boost::shared_ptr<Packet>(new PlayerInfoPacket(*it)));
    }

    if (level->somePlayerSleeping)
    {
        boost::shared_ptr<ServerPlayer> firstSleeper;
        for (unsigned int i = 0; i < mPlayers.size(); ++i)
        {
            boost::shared_ptr<ServerPlayer> sleeper = mPlayers[i];
            if (!sleeper->isSleeping())
                continue;
            if (firstSleeper == NULL)
                firstSleeper = sleeper;
            sleeper->mConnection->send(boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
                sleeper->getName(), ClientboundChatPacket::eChatPacketMessage_BedMeSleep, -1)));
        }
        player->mConnection->send(boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
            firstSleeper->getName(), ClientboundChatPacket::eChatPacketMessage_BedPlayerSleep, -1)));
    }

    if (mGameMode)
        mGameMode->OnJoin(player);
    level->getChunkMap()->onPlayerJoin(player);
}

void PlayerList::placeNewPlayer(Connection* connection, boost::shared_ptr<ServerPlayer> player,
                                boost::shared_ptr<ClientboundLoginPacket> loginPacket)
{
    CompoundTag* playerTag = NULL;
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) == 0)
        playerTag = load(player);

    player->setLevel(mServer->getLevel(player->mDimension));
    player->mGameMode->setLevel(static_cast<ServerLevel*>(player->mLevel));

    INetworkPlayer* networkPlayer = connection->m_socket->getPlayer();
    if (networkPlayer != NULL && networkPlayer->IsHost())
    {
        player->enableAllPlayerPrivileges(true);
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Moderator, 1);
#ifdef MINECRAFT_PC
        if (PcIsDedicatedServer())
        {
            player->mHiddenHost = true;
            player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Invisible, 1);
            player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Invulnerable, 1);
        }
#endif
    }
#ifdef MINECRAFT_PC
    else if (PcIsServerOperator(networkPlayer))
    {
        player->enableAllPlayerPrivileges(true);
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Moderator, 1);
        printf("[pc] server: %ls has host privileges (ops.txt)\n", player->getName().c_str());
        fflush(stdout);
    }
#endif

    validatePlayerSpawnPosition(player);
    ServerLevel* level = mServer->getLevel(player->mDimension);

#ifdef MINECRAFT_PC
    bool occupiedSlots[GameSessionData::MAX_SESSION_PLAYERS];
    const int slotCount = mServer->mMasterGameMode != NULL ? static_cast<int>(GameSessionData::MAX_MINI_GAME_PLAYERS)
                                                           : static_cast<int>(GameSessionData::MAX_SESSION_PLAYERS);
#else
    bool occupiedSlots[8];
    const int slotCount = 8;
#endif
    memset(occupiedSlots, 0, sizeof(occupiedSlots));
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = mPlayers.begin();
         it < mPlayers.end(); ++it)
        if ((*it)->mPlayerIndex >= 0 && (*it)->mPlayerIndex < static_cast<int>(sizeof(occupiedSlots)))
            occupiedSlots[(*it)->mPlayerIndex] = true;
    int playerSlot = 0;
    for (int slot = 0; slot < slotCount; ++slot)
    {
        if (!occupiedSlots[slot])
        {
            playerSlot = slot;
            break;
        }
    }
    player->mPlayerIndex = playerSlot;
    player->setCustomSkin(loginPacket->mWorldHeight);
    player->setCustomCape(loginPacket->mSeaLevel);

    boost::shared_ptr<ServerGamePacketListenerImpl> connectionListener(
        new ServerGamePacketListenerImpl(mServer, connection, player));

    if (playerTag == NULL)
    {
        if (gApp.GetGameHostOption(eGameHostOption_MiniGame) == 0)
        {
            player->mInventory->setItem(9, not_null_ptr<ItemInstance>(
                new ItemInstance(Items::map, 1,
                                 level->getAuxValueForMap(player->mXuid, 0, BlockPos(player), 3))));
        }
        if (gApp.getGameRuleDefinitions() != NULL)
            gApp.getGameRuleDefinitions()->postProcessPlayer(player);
    }

    if (!player->mSkinLocation.empty() && player->mSkinLocation.substr(0, 3).compare(L"def") != 0 &&
        !gApp.IsFileInMemoryTextures(player->mSkinLocation))
    {
        if (mServer->getConnection()->addPendingTextureRequest(player->mSkinLocation))
            connectionListener->send(boost::shared_ptr<TextureAndGeometryPacket>(
                new TextureAndGeometryPacket(player->mSkinLocation, NULL, 0)));
    }
    else if (!player->mSkinLocation.empty() && gApp.IsFileInMemoryTextures(player->mSkinLocation))
        gApp.AddMemoryTextureFile(player->mSkinLocation, NULL, 0);

    if (!player->mCapeLocation.empty() && player->mCapeLocation.substr(0, 3).compare(L"def") != 0 &&
        !gApp.IsFileInMemoryTextures(player->mCapeLocation))
    {
        if (mServer->getConnection()->addPendingTextureRequest(player->mCapeLocation))
            connectionListener->send(boost::shared_ptr<TexturePacket>(
                new TexturePacket(player->mCapeLocation, NULL, 0)));
    }
    else if (!player->mCapeLocation.empty() && gApp.IsFileInMemoryTextures(player->mCapeLocation))
        gApp.AddMemoryTextureFile(player->mCapeLocation, NULL, 0);

    player->mIsGuest = loginPacket->mAllowCommands;

    if (mGameMode)
        player->mGameMode->updateGameMode(mGameMode->InitialGameType(player));
    else
        updatePlayerGameMode(player, boost::shared_ptr<ServerPlayer>(), level);
    player->UpdateSize();

    unsigned int privileges = player->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All);
    const GameType* privilegeGameType = LevelSettings::validateGameType(
        (Player::getPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_CreativeMode) ?
            GameType::CREATIVE : GameType::SURVIVAL)->getId());
    if (player->mGameMode->isAdventure())
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CreativeMode, 0);
    else if (player->mGameMode->GetGameType() != privilegeGameType)
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CreativeMode,
                                       player->mGameMode->GetGameType()->getId());

    player->mConnection = connectionListener;
    connectionListener->mFriendsOnly = loginPacket->mHardcore;
    connectionListener->mPlayerUID = loginPacket->mPlayerUID;
    connectionListener->mOldPlayerUID = loginPacket->mPlayerUID2;
    if (loginPacket->mHardcore)
        ++mServer->ugcPlayersVersion;
    addPlayerToReceiving(player, false);

    connectionListener->send(boost::shared_ptr<Packet>(new ClientboundLoginPacket(
        L"", player->getId(), level->getLevelData()->getGeneratorType(), level->getSeed(),
        player->mGameMode->GetGameType()->getId(),
        gApp.GetGameHostOption(eGameHostOption_MiniGame),
        gApp.GetFirstMiniGameType(), static_cast<char>(level->mDimension->getType()->getId()),
        static_cast<unsigned char>(level->getMaxBuildHeight()),
        static_cast<unsigned char>(getMaxPlayers()), static_cast<char>(level->getDifficulty()->getId()),
        telemetryManagerInstance->GetMultiplayerInstanceID(), playerSlot, level->useNewSeaLevel(),
        player->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All),
        level->getLevelData()->getXZSize(), level->getLevelData()->getHellScale(),
        level->getLevelData()->getCloudHeight(), level->getLevelData()->getModernEnd(),
        level->getLevelData()->getBiomeScale(), level->getLevelData()->getBiomeCentreXChunk(),
        level->getLevelData()->getBiomeCentreZChunk(),
        level->useNewSeaLevel() ? level->getSeaLevel() : -1)));

    connectionListener->send(boost::shared_ptr<ClientboundPlayerAbilitiesPacket>(
        new ClientboundPlayerAbilitiesPacket(&player->mAbilities, player->getId())));
    connectionListener->send(boost::shared_ptr<Packet>(
        new ClientboundSetCarriedItemPacket(player->mInventory->m_selected)));

    updateEntireScoreboard(static_cast<ServerScoreboard*>(level->GetScoreboard()), player);
    sendLevelInfo(player, level);

    broadcastAll(boost::shared_ptr<Packet>(new ClientboundChatPacket(
        player->getName(), ClientboundChatPacket::eChatPacketMessage_PlayerJoined, -1)));

    MemSect(14);
    add(player);
    MemSect(0);
    player->doTick(true, true, true);

    connectionListener->teleport(player->mX, player->mY, player->mZ,
                                 player->mYRot, player->mXRot, true, true);
    mServer->getConnection()->addServerGamePacketListenerImpl(connectionListener);

    connectionListener->send(boost::shared_ptr<Packet>(new ClientboundSetTimePacket(
        level->getGameTime(), level->getDayTime(), level->getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT))));

    std::vector<MobEffectInstance*>* effects = player->getActiveEffects();
    if (effects != NULL)
    {
        std::vector<MobEffectInstance*>::iterator it;
        for (it = effects->begin(); it != effects->end(); ++it)
            connectionListener->send(boost::shared_ptr<Packet>(
                new ClientboundUpdateMobEffectPacket(player->getId(), *it)));
        delete effects;
    }

    player->initMenu();
    if (playerTag != NULL && playerTag->contains(L"RootVehicle", Tag::TAG_COMPOUND))
    {
        CompoundTag* rootVehicle = playerTag->getCompound(L"RootVehicle");
        boost::shared_ptr<Entity> vehicle = EntityIO::newEntity(rootVehicle->getCompound(L"Entity"), level);
        if (vehicle)
        {
            if (vehicle->mDimension != player->mDimension)
            {
                vehicle->setLevel(player->mLevel);
                vehicle->mDimension = player->mDimension;
                vehicle->setPos(player->mX, player->mY, player->mZ);
            }
            vehicle->mForcedLoading = true;
            bool added = level->addEntity(vehicle);
            vehicle->mForcedLoading = false;
            if (added)
            {
                const std::wstring attach = rootVehicle->getString(L"Attach");
                if (vehicle->getUUID() == attach)
                    player->startRiding(vehicle);
                else
                {
                    std::set<boost::shared_ptr<Entity> >* passengers = vehicle->getIndirectPassengers();
                    for (std::set<boost::shared_ptr<Entity> >::const_iterator it = passengers->begin();
                         it != passengers->end(); ++it)
                    {
                        boost::shared_ptr<Entity> passenger = *it;
                        if (passenger->getUUID() == attach)
                        {
                            player->startRiding(passenger);
                            break;
                        }
                    }
                    delete passengers;
                }
                if (!player->isPassenger())
                {
                    level->removeEntityImmediately(vehicle);
                    std::set<boost::shared_ptr<Entity> >* passengers = vehicle->getIndirectPassengers();
                    for (std::set<boost::shared_ptr<Entity> >::const_iterator it = passengers->begin();
                         it != passengers->end(); ++it)
                    {
                        boost::shared_ptr<Entity> passenger = *it;
                        level->removeEntityImmediately(passenger);
                    }
                    delete passengers;
                }
            }
        }
    }
    delete playerTag;

    networkPlayer = player->mConnection->getNetworkPlayer();
    if (networkPlayer != NULL)
    {
        for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = mPlayers.begin();
             it != mPlayers.end(); ++it)
        {
            boost::shared_ptr<ServerPlayer> otherPlayer = *it;
            INetworkPlayer* other = otherPlayer->mConnection->getNetworkPlayer();
            if (other != networkPlayer && other != NULL && networkPlayer->IsSameSystem(other) &&
                otherPlayer->mRespawnState)
            {
                player->mRespawnState = true;
                break;
            }
        }
    }
#ifdef MINECRAFT_PC
    if (networkPlayer != NULL && networkPlayer->IsHost() && !PcIsHiddenHost(networkPlayer))
    {
        const std::wstring msg = PcModContentMissingNotice();
        for (size_t start = 0; start < msg.size();)
        {
            size_t end = msg.find(L". ", start);
            end = end == std::wstring::npos ? msg.size() : end + 1;
            player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundChatPacket(
                msg.substr(start, std::min<size_t>(end - start, 120)), ClientboundChatPacket::eChatPacketMessage_Text,
                -1)));
            start = end < msg.size() ? end + 1 : end;
        }
    }
    PcScriptsHostPlayerJoined(player.get());
#endif
}

void PlayerList::setLevel(arrayWithLength<ServerLevel*> levels)
{
    mPlayerIO = levels.data[0]->getLevelStorage()->getPlayerIO();
}

int PlayerList::getMaxRange()
{
    return PlayerChunkMap::convertChunkRangeToBlock(getViewDistance());
}

void PlayerList::move(boost::shared_ptr<ServerPlayer> player)
{
    player->getLevel()->getChunkMap()->move(player);
}

boost::shared_ptr<ServerPlayer> PlayerList::getPlayerForLogin(ServerConnectionListener* listener,
                                                              const std::wstring& name,
                                                              PlayerUID uid, PlayerUID oldUid)
{
    if (getMaxPlayers() <= static_cast<int>(mPlayers.size()))
    {
        listener->disconnect(DisconnectPacket::eDisconnectReason_GameFull);
        return boost::shared_ptr<ServerPlayer>();
    }
#ifdef MINECRAFT_PC
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if ((*it)->getXuid() == uid)
        {
            printf("[pc] login: %ls is already in this game with the same identity; refused\n", name.c_str());
            fflush(stdout);
            listener->disconnect(DisconnectPacket::eDisconnectReason_MiiInUse);
            return boost::shared_ptr<ServerPlayer>();
        }
    }
    std::wstring playerName = name;
    INetworkPlayer* sessionPlayer = listener->m_connection->m_socket->getPlayer();
    if (sessionPlayer != NULL && sessionPlayer->GetOnlineName() != NULL && sessionPlayer->GetOnlineName()[0] != 0)
        playerName = sessionPlayer->GetOnlineName();
    ServerPlayerGameMode* gameMode = new ServerPlayerGameMode(mServer->getLevel(0), mGameMode);
    boost::shared_ptr<ServerPlayer> newPlayer(
        new ServerPlayer(mServer, mServer->getLevel(0), playerName, gameMode));
#else
    ServerPlayerGameMode* gameMode = new ServerPlayerGameMode(mServer->getLevel(0), mGameMode);
    boost::shared_ptr<ServerPlayer> newPlayer(
        new ServerPlayer(mServer, mServer->getLevel(0), name, gameMode));
#endif
    newPlayer->setXuid(uid);
    newPlayer->mOnlineXuid = oldUid;

    INetworkPlayer* networkPlayer = listener->m_connection->m_socket->getPlayer();
    if (networkPlayer != NULL && !networkPlayer->IsHost())
    {
        newPlayer->enableAllPlayerPrivileges(gApp.GetGameHostOption(eGameHostOption_TrustPlayers) != 0);
    }

    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset != NULL)
    {
        newPlayer->mGameMode->setGameRules(static_cast<GameRulesInstance*>(GameRuleDefinition::generateNewGameRulesInstance(
            GameRulesInstance::eGameRulesInstanceType_Player, ruleset, listener->m_connection)));
        newPlayer->UpdateSize();
    }
    return newPlayer;
}

std::vector<boost::shared_ptr<ServerPlayer> >* PlayerList::getPlayers()
{
    return &mPlayers;
}

boost::shared_ptr<ServerPlayer> PlayerList::respawn(boost::shared_ptr<ServerPlayer> player,
                                                    int dimension, bool conqueredEnd)
{
    const bool receivingAllPackets = canReceiveAllPackets(player);
    const int oldDimension = player->mDimension;
    bool leavingSystem = dimension != oldDimension;
    if (leavingSystem)
    {
        INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
        for (unsigned int i = 0; i < mPlayers.size(); ++i)
        {
            boost::shared_ptr<ServerPlayer> other = mPlayers[i];
            if (other == player)
                continue;
            if (other->mDimension != oldDimension)
                continue;
            INetworkPlayer* otherNetworkPlayer = other->mConnection->getNetworkPlayer();
            if (otherNetworkPlayer != NULL && networkPlayer->IsSameSystem(otherNetworkPlayer))
                leavingSystem = false;
        }
    }
    if (receivingAllPackets)
    {
        if (leavingSystem)
            player->getLevel()->getTracker()->clear(player);
        else
            player->getLevel()->getTracker()->removeEntity(player);
    }
    else
    {
        player->getLevel()->getTracker()->removeEntity(player);
    }

    player->getLevel()->getChunkMap()->remove(player);
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator oldPlayer =
        std::find(mPlayers.begin(), mPlayers.end(), player);
    if (oldPlayer != mPlayers.end())
        mPlayers.erase(oldPlayer);
    mServer->getLevel(player->mDimension)->removeEntityImmediately(player);

    BlockPos* respawnPosition = player->getRespawnPosition();
    const bool respawnForced = player->isRespawnForced();
    const bool wasReceiving = removePlayerFromReceiving(player, true, 0) != 0;
    player->mDimension = dimension;
    const EDefaultSkins defaultSkin = static_cast<EDefaultSkins>(player->mField0x7d8);
    const int playerIndex = player->mPlayerIndex;
    const PlayerUID xuid = player->mXuid;
    const PlayerUID onlineXuid = player->mOnlineXuid;

    boost::shared_ptr<ServerPlayer> replacement(new ServerPlayer(
        mServer, mServer->getLevel(player->mDimension), player->getName(),
        new ServerPlayerGameMode(mServer->getLevel(player->mDimension), mGameMode)));
    replacement->mConnection = player->mConnection;
    replacement->restoreFrom(player, conqueredEnd);

    replacement->setMainArm(player->getMainArm());
    if (conqueredEnd)
        replacement->mInventory->m_selected = player->mInventory->m_selected;
    replacement->mIsGuest = player->mIsGuest;
    replacement->setXuid(xuid);
    replacement->mOnlineXuid = onlineXuid;
    replacement->setPlayerDefaultSkin(defaultSkin);
    replacement->mPlayerIndex = playerIndex;
    replacement->setCustomSkin(player->mCustomSkin);
    replacement->setCustomCape(player->mCustomCape);
    replacement->setPlayerGamePrivilege(
        Player::ePlayerGamePrivilege_All,
        player->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All));
    replacement->mGameMode->setGameRules(player->mGameMode->mGameRules);
    player->mGameMode->nullifyGameRules();
    replacement->mDimension = dimension;
    replacement->mConnection->mPlayer = replacement;

    addPlayerToReceiving(replacement, wasReceiving && oldDimension == dimension);
    ServerLevel* newLevel = mServer->getLevel(player->mDimension);
    updatePlayerGameMode(replacement, player, newLevel);
    replacement->resetPos();
    if (player->mRespawnState && dimension == oldDimension && player->getHealth() > 0.0f)
    {
        replacement->moveTo(player->mX, player->mY, player->mZ,
                            player->mYRot, player->mXRot);
        if (respawnPosition != NULL)
            replacement->setRespawnPosition(new BlockPos(*respawnPosition), respawnForced);
        replacement->mInventory->m_selected = player->mInventory->m_selected;
    }
    else if (respawnPosition != NULL)
    {
        BlockPos* validPosition = Player::checkBedValidRespawnPosition(
            mServer->getLevel(player->mDimension), *respawnPosition, respawnForced);
        if (validPosition != NULL)
        {
            replacement->moveTo(validPosition->x + 0.5f, validPosition->y + 0.1f,
                                validPosition->z + 0.5f, 0.0f, 0.0f);
            replacement->setRespawnPosition(new BlockPos(*respawnPosition), respawnForced);
        }
        else
        {
            replacement->mConnection->send(boost::shared_ptr<ClientboundChatPacket>(
                new ClientboundChatPacket(
                    ClientboundChatPacket::eChatPacketMessage_BedNotValid, -1)));
        }
        delete validPosition;
    }

    if (mGameMode != NULL)
        mGameMode->OnSpawn(replacement, player, true);

    newLevel->getChunkSource()->create(static_cast<int>(replacement->mX) >> 4,
                                       static_cast<int>(replacement->mZ) >> 4);
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        while (!newLevel->getCollisionAABBs(replacement, replacement->mBoundingBox,
                                            false, false, false)->empty())
            replacement->setPos(replacement->mX, replacement->mY + 1.0, replacement->mZ);
    }

    replacement->mConnection->send(boost::shared_ptr<ClientboundRespawnPacket>(
        new ClientboundRespawnPacket(
            replacement->mX, replacement->mY, replacement->mZ, replacement->mYRot,
            replacement->mXRot, static_cast<char>(replacement->mDimension),
            replacement->mLevel->getSeed(), replacement->mGameMode->GetGameType(),
            newLevel->getDifficulty(), newLevel->getLevelData()->getGeneratorType(),
            replacement->mLevel->useNewSeaLevel(), replacement->getId(),
            newLevel->getLevelData()->getXZSize(), newLevel->getLevelData()->getHellScale(),
            newLevel->getLevelData()->getCloudHeight(), newLevel->getLevelData()->getModernEnd(),
            static_cast<unsigned char>(newLevel->getLevelData()->getBiomeScale()),
            newLevel->getLevelData()->getBiomeCentreXChunk(),
            newLevel->getLevelData()->getBiomeCentreZChunk(), conqueredEnd,
            newLevel->useNewSeaLevel() ? newLevel->getSeaLevel() : -1)));
    replacement->mConnection->teleport(replacement->mX, replacement->mY, replacement->mZ,
                                       replacement->mYRot, replacement->mXRot, true, true);
    replacement->mConnection->send(boost::shared_ptr<ClientboundSetExperiencePacket>(
        new ClientboundSetExperiencePacket(replacement->mExperienceProgress,
                                           replacement->mTotalExperience,
                                           replacement->mExperienceLevel)));

    if (conqueredEnd)
    {
        std::vector<MobEffectInstance*>* effects = player->getActiveEffects();
        for (std::vector<MobEffectInstance*>::iterator it = effects->begin();
             it != effects->end(); ++it)
            replacement->addEffect(new MobEffectInstance(*it), boost::shared_ptr<Entity>());
        delete effects;

        boost::shared_ptr<SynchedEntityData> entityData = replacement->getEntityData();
        entityData->markDirty(LivingEntity::DATA_EFFECT_COLOR_ID);
    }

    sendLevelInfo(replacement, newLevel);
    newLevel->getChunkMap()->add(replacement);
    newLevel->addEntity(replacement);
    mPlayers.push_back(replacement);
    replacement->initMenu();

    if (conqueredEnd)
        replacement->setHealth(player->getHealth());
    else
        replacement->setHealth(replacement->getHealth());

    if (Minecraft::GetInstance()->isTutorial())
    {
        if (Minecraft::GetInstance()->mGameMode->getTutorial() != NULL
            && !Minecraft::GetInstance()->mGameMode->getTutorial()->isStateCompleted(
                   eTutorial_State_FoodBar))
            gApp.getGameRuleDefinitions()->postProcessPlayer(replacement);
    }
    if (oldDimension == 1 && replacement->mDimension != 1)
        replacement->displayClientMessage(IDS_PLAYER_LEFT_END, true);
    if (mGameMode != NULL)
        mGameMode->OnSpawned(replacement, true);

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = getPlayers()->begin();
         it != getPlayers()->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> other = *it;
        other->notifyPlayerRespawn(player, replacement);
    }

    return replacement;
}

void PlayerList::repositionAcrossDimension(boost::shared_ptr<Entity> entity, int lastDimension,
                                           ServerLevel* oldLevel, ServerLevel* newLevel)
{
    const double originalX = entity->mX;
    const double originalZ = entity->mZ;
    double x = entity->mX;
    double z = entity->mZ;
    const float rotation = entity->mYRot;
    const double scale = static_cast<double>(newLevel->getLevelData()->getHellScale());

    if (entity->mDimension == -1)
    {
        x /= scale;
        z /= scale;
        entity->moveTo(x, entity->mY, z, entity->mYRot, entity->mXRot);
        if (entity->isAlive())
        {
            oldLevel->tick(entity, false);
        }
    }
    else if (entity->mDimension == 0)
    {
        x *= scale;
        z *= scale;
        entity->moveTo(x, entity->mY, z, entity->mYRot, entity->mXRot);
        if (entity->isAlive())
        {
            oldLevel->tick(entity, false);
        }
    }
    else
    {
        BlockPos spawn;
        if (lastDimension == 1)
        {
            spawn = newLevel->getSharedSpawnPos();
        }
        else
        {
            spawn = *newLevel->getDimensionSpecificSpawn();
        }
        x = static_cast<double>(spawn.x);
        entity->mY = static_cast<double>(spawn.y);
        z = static_cast<double>(spawn.z);
        entity->moveTo(x, entity->mY, z, 90.0f, 0.0f);
        if (entity->isAlive())
        {
            oldLevel->tick(entity, false);
        }
    }

    if (eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, entity->GetType()))
    {
        boost::shared_ptr<ServerPlayer> serverPlayer = boost::dynamic_pointer_cast<ServerPlayer>(entity);
        removePlayerFromReceiving(serverPlayer, false, lastDimension);
        addPlayerToReceiving(serverPlayer, false);
    }

    if (lastDimension != 1)
    {
        newLevel->clampWithinLevelBounds(x, z, 0.0);
        if (entity->isAlive())
        {
            entity->moveTo(x, entity->mY, z, entity->mYRot, entity->mXRot);
            ServerChunkCache* chunkCache = static_cast<ServerChunkCache*>(newLevel->getChunkSource());
            chunkCache->m_createOnRequest = true;
            newLevel->getPortalForcer()->force(entity, rotation, static_cast<int>(originalX),
                                               static_cast<int>(originalZ));
            chunkCache->m_createOnRequest = false;
            newLevel->addEntity(entity);
            newLevel->tick(entity, false);
        }
    }

    entity->setLevel(newLevel);
}

void PlayerList::sendAllPlayerInfo(boost::shared_ptr<ServerPlayer> player)
{
    player->refreshContainer(player->mInventoryMenu);
    player->resetSentInfo();
    player->mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundSetCarriedItemPacket(player->mInventory->m_selected)));
}

void PlayerList::toggleDimension(boost::shared_ptr<ServerPlayer> player, int dimension)
{
    const int oldDimension = player->mDimension;
    const bool receivingAllPackets = canReceiveAllPackets(player);
    bool anotherLocalPlayerInDimension = true;
    INetworkPlayer* localNetworkPlayer = player->mConnection->getNetworkPlayer();

    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> candidate = mPlayers[i];
        if (candidate.get() == player.get())
            continue;
        if (candidate->mDimension == oldDimension)
        {
            INetworkPlayer* candidateNetworkPlayer = candidate->mConnection->getNetworkPlayer();
            if (candidateNetworkPlayer != NULL && localNetworkPlayer->IsSameSystem(candidateNetworkPlayer))
                anotherLocalPlayerInDimension = false;
        }
    }

    if (!receivingAllPackets)
        player->getLevel()->getTracker()->removeEntity(player);
    else if (anotherLocalPlayerInDimension)
        player->getLevel()->getTracker()->clear(player);
    else
        player->getLevel()->getTracker()->removeEntity(player);

    ServerLevel* oldLevel = mServer->getLevel(player->mDimension);
    player->getLevel()->getChunkMap()->remove(player);

    if (player->mDimension == 1 || dimension != 1)
    {
        if (player->mDimension == 1)
            player->displayClientMessage(static_cast<int>(IDS_PLAYER_LEFT_END), true);
    }
    else
    {
        player->displayClientMessage(static_cast<int>(IDS_PLAYER_ENTERED_END), true);
    }

    player->mDimension = dimension;
    ServerLevel* newLevel = mServer->getLevel(player->mDimension);
    player->clearFire();
    player->flushEntitiesToRemove();

    player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundRespawnPacket(
        static_cast<char>(player->mDimension), newLevel->getSeed(), player->mGameMode->GetGameType(),
        newLevel->getDifficulty(), newLevel->getLevelData()->getGeneratorType(), newLevel->useNewSeaLevel(),
        player->getId(), newLevel->getLevelData()->getXZSize(), newLevel->getLevelData()->getHellScale(),
        newLevel->getLevelData()->getCloudHeight(), newLevel->getLevelData()->getModernEnd(),
        static_cast<unsigned char>(newLevel->getLevelData()->getBiomeScale()),
        newLevel->getLevelData()->getBiomeCentreXChunk(), newLevel->getLevelData()->getBiomeCentreZChunk(),
        true, newLevel->useNewSeaLevel() ? newLevel->getSeaLevel() : -1)));

    oldLevel->removeEntityImmediately(player);
    player->mRemoved = false;
    repositionAcrossDimension(player, oldDimension, oldLevel, newLevel);
    changeDimension(player, newLevel);
    player->mGameMode->setLevel(newLevel);

    INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
    if (!CGameNetworkManager::sInstance.SystemFlagGet(
            networkPlayer,
            ServerPlayer::getFlagIndexForChunk(ChunkPos(player->mChunkX, player->mChunkZ),
                                               player->mLevel->mDimension->getType()->getId())))
    {
        player->mConnection->teleport(player->mX, player->mY, player->mZ,
                                      player->mYRot, player->mXRot, false, true);
        player->doTick(true, true, true);
    }
    player->mConnection->teleport(player->mX, player->mY, player->mZ,
                                  player->mYRot, player->mXRot, true, true);

    std::vector<MobEffectInstance*>* effects = player->getActiveEffects();
    for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
    {
        player->mConnection->send(boost::shared_ptr<Packet>(
            new ClientboundUpdateMobEffectPacket(player->getId(), *it)));
    }
    delete effects;

    player->getEntityData()->markDirty(LivingEntity::DATA_EFFECT_COLOR_ID);
    player->markAttributesDirty();
    sendLevelInfo(player, newLevel);
    sendAllPlayerInfo(player);
}

void PlayerList::tick()
{
    EnterCriticalSection(&mListenersToCloseLock);
    while (!mListenersToClose.empty())
    {
        unsigned char smallId = mListenersToClose.front();
        mListenersToClose.pop_front();
        boost::shared_ptr<ServerPlayer> closing;
        for (unsigned int i = 0; i < mPlayers.size(); ++i)
        {
            boost::shared_ptr<ServerPlayer> player = mPlayers.at(i);
            if (player && player->mConnection && player->mConnection->mConnection
                && player->mConnection->mConnection->m_socket
                && player->mConnection->mConnection->m_socket->mPlayerId == smallId)
            {
                closing = player;
                break;
            }
        }
        if (closing)
            closing->mConnection->disconnect(DisconnectPacket::eDisconnectReason_ConnectionClosed);
    }
    LeaveCriticalSection(&mListenersToCloseLock);

    EnterCriticalSection(&mPlayersToKickLock);
    while (!mPlayersToKick.empty())
    {
        unsigned char smallId = mPlayersToKick.front();
        mPlayersToKick.pop_front();
        INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId);
        if (networkPlayer == NULL || networkPlayer->IsLocal())
            continue;

        PlayerUID uid = networkPlayer->GetUID();
        boost::shared_ptr<ServerPlayer> kicked;
        for (unsigned int i = 0; i < mPlayers.size(); ++i)
        {
            boost::shared_ptr<ServerPlayer> player = mPlayers.at(i);
            if (player && gProfile->AreXUIDSEqual(player->mOnlineXuid, uid))
            {
                kicked = player;
                break;
            }
        }
        if (kicked)
        {
            mBannedPlayers.push_back(kicked->mOnlineXuid);
            kicked->enableAllPlayerPrivileges(false);
            kicked->mConnection->mUseAltLeaveMessage = true;
            kicked->mConnection->send(boost::shared_ptr<Packet>(
                new DisconnectPacket(DisconnectPacket::eDisconnectReason_Kicked)));
        }
    }
    LeaveCriticalSection(&mPlayersToKickLock);

    for (int dimension = 0; dimension < 3; ++dimension)
    {
        for (unsigned int i = 0; i < mReceivingPlayers[dimension].size(); ++i)
        {
            boost::shared_ptr<ServerPlayer> player = mReceivingPlayers[dimension][i];
            if (player && player->mRemoved)
            {
                boost::shared_ptr<ServerPlayer> replacement = findAlivePlayerOnSystem(player);
                if (replacement)
                    mReceivingPlayers[dimension][i] = replacement;
            }
        }
    }
}

bool PlayerList::isTrackingBlock(const BlockPos& pos, const Dimension* dimension)
{
    if (!mServer || !dimension)
        return false;
    ServerLevel* level = mServer->getLevel(dimension->getType()->getId());
    return level != NULL && level->getChunkMap()->isTrackingBlock(pos.x, pos.y, pos.z);
}

void PlayerList::prioritiseBlockChanges(const BlockPos& pos, int dimension)
{
    ServerLevel* level = mServer->getLevel(dimension);
    if (level != NULL)
        level->getChunkMap()->prioritiseBlockChanges(pos.x, pos.y, pos.z);
}

void PlayerList::broadcastAll(boost::shared_ptr<Packet> packet, int dimension)
{
    for (unsigned int i = 0; i < mPlayers.size(); i++)
    {
        boost::shared_ptr<ServerPlayer> player = mPlayers[i];
        if (player->mDimension == dimension)
        {
            player->mConnection->send(packet);
        }
    }
}

boost::shared_ptr<ServerPlayer> PlayerList::getPlayer(PlayerUID uid)
{
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = mPlayers[i];
        if (player->mXuid == uid || player->mOnlineXuid == uid)
            return player;
    }
    return boost::shared_ptr<ServerPlayer>();
}

void PlayerList::broadcastToTeam(boost::shared_ptr<Player> source,
                               boost::shared_ptr<Packet> packet)
{
    Team* team = source->getTeam();
    if (!team)
        return;
    boost::unordered_set<PlayerUID, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >* members =
        team->getPlayers();
    for (boost::unordered_set<PlayerUID, boost::hash<PlayerUID>,
                              std::equal_to<PlayerUID> >::iterator it = members->begin();
         it != members->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> target = getPlayer(*it);
        if (!target || target.get() == source.get())
            continue;
        target->mConnection->send(packet);
    }
}

void PlayerList::broadcastToAllExceptTeam(boost::shared_ptr<Player> source,
                                          boost::shared_ptr<Packet> packet)
{
    Team* sourceTeam = source->getTeam();
    if (!sourceTeam)
    {
        broadcastAll(packet);
        return;
    }
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        if (player->getTeam() != sourceTeam)
            player->mConnection->send(packet);
    }
}

void PlayerList::broadcastPlaylistInfo(std::vector<LevelRules::LevelRulesetGroup*>* groups)
{
    if (!mGameMode)
        return;
    mGameMode->UpdateRulePlayList(groups);
    mGameMode->GeneratePlaylistSyncInfo();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        if (*it)
            mGameMode->SendPlaylistInfo(*it);
    }
}

std::wstring PlayerList::getPlayerNames()
{
    std::wstring names;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        if (it != mPlayers.begin())
            names.append(L", ");
        BOOST_ASSERT(*it);
        const std::wstring playerName = (*it)->getName();
        names.append(playerName);
    }
    return names;
}

boost::shared_ptr<ServerPlayer> PlayerList::getPlayerByName(const std::wstring& name)
{
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = mPlayers[i];
        if (player->getName() == name)
            return player;
    }
    return boost::shared_ptr<ServerPlayer>();
}

boost::shared_ptr<ServerPlayer> PlayerList::getPlayerFromStatsUID(const StatsUID& uid)
{
    return getPlayer(uid.GetID());
}

void PlayerList::broadcast(boost::shared_ptr<Player> source, double x, double y, double z,
                           double radius, int dimension, boost::shared_ptr<Packet> packet)
{
    std::vector<boost::shared_ptr<ServerPlayer> > sentTo;
    if (source)
    {
        sentTo.push_back(boost::dynamic_pointer_cast<ServerPlayer>(source));
    }

    for (unsigned int i = 0; i < mPlayers.size(); i++)
    {
        boost::shared_ptr<ServerPlayer> candidate = mPlayers[i];
        if (candidate == source)
            continue;
        if (candidate->mDimension != dimension)
            continue;

        bool dontSend = false;
        if (sentTo.size())
        {
            INetworkPlayer* networkPlayer = candidate->mConnection->getNetworkPlayer();
            if (networkPlayer == NULL)
            {
                dontSend = true;
            }
            else
            {
                for (unsigned int j = 0; j < sentTo.size(); j++)
                {
                    boost::shared_ptr<ServerPlayer> sentPlayer = sentTo[j];
                    INetworkPlayer* sentNetworkPlayer = sentPlayer->mConnection->getNetworkPlayer();
                    if (sentNetworkPlayer != NULL && networkPlayer->IsSameSystem(sentNetworkPlayer))
                    {
                        dontSend = true;
                    }
                }
            }
        }
        if (dontSend)
            continue;

        double dx = x - candidate->mX;
        double dy = y - candidate->mY;
        double dz = z - candidate->mZ;
        if (dx * dx + dy * dy + dz * dz < radius * radius)
        {
            candidate->mConnection->send(packet);
            sentTo.push_back(candidate);
        }
    }
}

void PlayerList::broadcast(double x, double y, double z, double radius, int dimension,
                           boost::shared_ptr<Packet> packet)
{
    broadcast(boost::shared_ptr<Player>(), x, y, z, radius, dimension, packet);
}

int PlayerList::getPlayerCount()
{
    return static_cast<int>(mPlayers.size());
}

int PlayerList::getPlayerCount(ServerLevel* level)
{
    int count = 0;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        if ((*it)->mLevel == level)
            ++count;
    }
    return count;
}

void PlayerList::kickPlayerByShortId(unsigned char id)
{
    EnterCriticalSection(&mPlayersToKickLock);
    mPlayersToKick.push_back(id);
    LeaveCriticalSection(&mPlayersToKickLock);
}

void PlayerList::closeServerGamePacketListenerImplBySmallId(unsigned char id)
{
    EnterCriticalSection(&mListenersToCloseLock);
    mListenersToClose.push_back(id);
    LeaveCriticalSection(&mListenersToCloseLock);
}

bool PlayerList::isXuidBanned(PlayerUID uid)
{
    if (uid == PlayerUID(0))
        return false;

    bool banned = false;
    for (std::vector<PlayerUID>::iterator it = mBannedPlayers.begin(); it != mBannedPlayers.end(); ++it)
    {
        if (gProfile->AreXUIDSEqual(uid, *it))
        {
            banned = true;
            break;
        }
    }
    return banned;
}

void PlayerList::resetAllChunkFlagsOutsideArea(int minX, int minZ, int maxX, int maxZ, int flags)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin(); it < mPlayers.end(); ++it)
        (*it)->resetChunkFlagsOutsideArea(minX, minZ, maxX, maxZ, flags);
}
