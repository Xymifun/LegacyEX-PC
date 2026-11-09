#include "game/world/chunk/MinigamePlayerChunkMap.hpp"

#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/ChunkVisibilityAreaPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/chunk/PlayerChunkContainer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/minigame/EMiniGameId.hpp"

MinigamePlayerChunkMap::MinigamePlayerChunkMap(ServerLevel* level, int dimension, int viewDistance)
    : PlayerChunkMap(level, dimension, viewDistance)
{
}

MinigamePlayerChunkMap::~MinigamePlayerChunkMap()
{
}

void MinigamePlayerChunkMap::updatePlayerReferences(boost::shared_ptr<ServerPlayer> player)
{
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> existing = mPlayers[i];
        if (existing->getXuid() == player->getXuid())
        {
            mPlayers[i] = player;
            break;
        }
    }

    ContainerVector chunks;
    chunks.assign(mAllChunks.begin(), mAllChunks.end());
    for (ContainerVector::iterator it = chunks.begin(); it != chunks.end(); ++it)
    {
        PlayerChunkContainer* chunk = *it;
        chunk->updatePlayerReference(player);
    }

    for (RequestVector::iterator it = mAddRequests.begin(); it != mAddRequests.end(); ++it)
    {
        if (it->player->getXuid() == player->getXuid())
            it->player = player;
    }
}

void MinigamePlayerChunkMap::add(boost::shared_ptr<ServerPlayer> player)
{
    updatePlayerReferences(player);
}

void MinigamePlayerChunkMap::remove(boost::shared_ptr<ServerPlayer>)
{
}

void MinigamePlayerChunkMap::move(boost::shared_ptr<ServerPlayer> player)
{
    player->mLastSentX = player->mX;
    player->mLastSentZ = player->mZ;
}

void MinigamePlayerChunkMap::removeOutsideAreaForAllPlayers(int minX, int minZ, int maxX, int maxZ)
{
}

void MinigamePlayerChunkMap::loadPlayerInActiveArea(boost::shared_ptr<ServerPlayer> player)
{
    int minX;
    int minZ;
    int maxX;
    int maxZ;
    gMinecraftServer->mMasterGameMode->GetMaxActiveBounds(minX, minZ, maxX, maxZ);

    if (!Minecraft::InMiniGame(eMiniGameId_Glide, false))
    {
        minX -= 5;
        minZ -= 5;
        maxX += 5;
        maxZ += 5;
    }

    for (int x = minX; x <= maxX; ++x)
    {
        for (int z = minZ; z <= maxZ; ++z)
            getChunkAndAddPlayer(x, z, player, false);
    }

    if ((gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->canReceiveAllPackets(player))
    {
        player->mConnection->send(boost::shared_ptr<ChunkVisibilityAreaPacket>(
            new ChunkVisibilityAreaPacket(minX, maxX, minZ, maxZ, true, shouldActuallyDropChunks(), false)));
    }
}

void MinigamePlayerChunkMap::reloadAllPlayers(int minX, int minZ, int maxX, int maxZ)
{
    ContainerVector chunks(mAllChunks);
    mShouldDropChunks = true;
    for (ContainerVector::iterator it = chunks.begin(); it != chunks.end(); ++it)
    {
        PlayerChunkContainer* chunk = *it;
        const int chunkX = chunk->getPos().x;
        const int chunkZ = chunk->getPos().z;
        if (chunkX < minX || chunkX > maxX || chunkZ < minZ || chunkZ > maxZ)
            PlayerChunkContainer::removeAllPlayers(chunk, false);
    }

    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = mPlayers[i];
        if ((gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->canReceiveAllPackets(player))
        {
            player->mConnection->send(boost::shared_ptr<ChunkVisibilityAreaPacket>(
                new ChunkVisibilityAreaPacket(minX, maxX, minZ, maxZ, false, shouldActuallyDropChunks(), true)));
        }
    }
    mShouldDropChunks = false;
    mAddRequests.clear();

    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = mPlayers[i];
        updatePlayerReferences(player);
        loadPlayerInActiveArea(player);
    }
}

void MinigamePlayerChunkMap::onPlayerJoin(boost::shared_ptr<ServerPlayer> player)
{
    mPlayers.push_back(player);
    loadPlayerInActiveArea(player);
}

void MinigamePlayerChunkMap::onPlayerLeave(boost::shared_ptr<ServerPlayer> player)
{
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> existing = mPlayers[i];
        if (existing->getXuid() == player->getXuid())
        {
            mPlayers.erase(mPlayers.begin() + i);
            break;
        }
    }

    ContainerVector chunks;
    chunks.assign(mAllChunks.begin(), mAllChunks.end());
    for (ContainerVector::iterator it = chunks.begin(); it != chunks.end(); ++it)
    {
        PlayerChunkContainer* chunk = *it;
        chunk->updatePlayerReference(player);
        chunk->remove(player, false);
    }

    for (RequestVector::iterator request = mAddRequests.begin(); request != mAddRequests.end();)
    {
        if (request->player->getXuid() == player->getXuid())
            request = mAddRequests.erase(request);
        else
            ++request;
    }
}

uint32_t MinigamePlayerChunkMap::containsPlayer(boost::shared_ptr<ServerPlayer> player)
{
    uint32_t contained = 0;
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> existing = mPlayers[i];
        if (existing->getXuid() == player->getXuid())
        {
            contained = 1;
            break;
        }
    }
    return contained;
}
