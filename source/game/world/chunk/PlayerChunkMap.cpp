#include "game/world/chunk/PlayerChunkMap.hpp"

#include <algorithm>
#include <limits.h>

#include "game/core/Mth.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/System.hpp"
#include "game/network/ChunkVisibilityAreaPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/minigame/EMiniGameId.hpp"

#ifdef MINECRAFT_PC
#define PLAYER_RADIUS(player) getPlayerRadius(player)
#define PLAYER_CHUNK_IN_RANGE(x, z, centerX, centerZ, radius) inRange(x, z, centerX, centerZ, radius)
#else
#define PLAYER_RADIUS(player) mViewDistance
#define PLAYER_CHUNK_IN_RANGE(x, z, centerX, centerZ, radius) chunkInRange(x, z, centerX, centerZ)
#endif
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/PlayerChunkContainer.hpp"

PlayerChunkMap::PlayerChunkMap(ServerLevel* level, int dimension, int viewDistance)
{
    mViewDistance = viewDistance;
    mPauseBlockUpdatePackets = false;
    mLevel = (Level*)level;
    setViewDistance(level->getServer()->getPlayers()->getViewDistance());
    mDimension = dimension;
    mLastUpdate = 0;
    mTicksUntilNextUpdate = 0;
    mShouldDropChunks = false;
}

PlayerChunkMap::~PlayerChunkMap()
{
    for (ContainerMap::iterator it = mChunks.begin(); it != mChunks.end(); ++it)
    {
        delete it->second;
    }
}

void PlayerChunkMap::flagEntitiesToBeRemoved(unsigned int* count, bool* removed)
{
    for (PlayerVector::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        player->flagEntitiesToBeRemoved(count, removed);
    }
}

Level* PlayerChunkMap::getLevel()
{
    return mLevel;
}

long long PlayerChunkMap::tickAddRequests(boost::shared_ptr<ServerPlayer> player, bool force)
{
    if (!mAddRequests.empty())
    {
        int playerX = static_cast<int>(player->mX);
        int playerZ = static_cast<int>(player->mZ);

        int bestDistance = -1;
        RequestVector::iterator best = mAddRequests.end();
        if (force)
        {
            for (RequestVector::iterator it = mAddRequests.begin(); it != mAddRequests.end(); ++it)
            {
                if (it->player == player)
                {
                    int xd = it->x * 16 + 8 - playerX;
                    int zd = it->z * 16 + 8 - playerZ;
                    int distance = xd * xd + zd * zd;
                    if (bestDistance == -1 || distance < bestDistance)
                    {
                        bestDistance = distance;
                        best = it;
                    }
                }
            }
        }
        else
        {
            for (RequestVector::iterator it = mAddRequests.begin(); it != mAddRequests.end(); ++it)
            {
                if (it->player == player)
                    return System::processTimeInMilliSecs() - player->mChunkRequestTime;
            }
        }

        if (best != mAddRequests.end())
        {
            player->mChunkRequestTime = System::processTimeInMilliSecs();
            getOrCreateChunk(best->x, best->z)->add(best->player, best->add);
            mAddRequests.erase(best);
        }
    }
    return 0;
}

bool PlayerChunkMap::chunkInRange(int x, int z, int centerX, int centerZ)
{
    int xd = x - centerX;
    int zd = z - centerZ;
    if (xd < -mViewDistance || xd > mViewDistance)
    {
        return false;
    }
    if (zd < -mViewDistance || zd > mViewDistance)
    {
        return false;
    }
    return true;
}

bool PlayerChunkMap::shouldActuallyDropChunks()
{
    return mShouldDropChunks;
}

void PlayerChunkMap::setViewDistance(int distance)
{
    int newDistance = Mth::clamp(distance, 3, 15);
    if (newDistance != mViewDistance)
    {
        int delta = newDistance - mViewDistance;
        PlayerVector players(mPlayers);
        for (PlayerVector::iterator it = players.begin(); it != players.end(); ++it)
        {
            boost::shared_ptr<ServerPlayer> player = *it;
            const int chunkX = static_cast<int32_t>(player->mX) >> 4;
            const int chunkZ = static_cast<int32_t>(player->mZ) >> 4;
            if (delta > 0)
            {
                for (int x = chunkX - newDistance; x <= chunkX + newDistance; ++x)
                {
                    for (int z = chunkZ - newDistance; z <= chunkZ + newDistance; ++z)
                    {
                        PlayerChunkContainer* chunk = getOrCreateChunk(x, z);
                        if (!chunk->hasPlayer(player))
                            chunk->add(player, true);
                    }
                }
            }
            else
            {
                for (int x = chunkX - mViewDistance; x <= chunkX + mViewDistance; ++x)
                    for (int z = chunkZ - mViewDistance; z <= chunkZ + mViewDistance; ++z)
                        if (!chunkInRange(x, z, chunkX, chunkZ))
                            getOrCreateChunk(x, z)->remove(player, true);
            }
        }
        mViewDistance = newDistance;
    }
}

void PlayerChunkMap::tick()
{
    long long gameTime = mLevel->getGameTime();
    if (gameTime - mLastUpdate > 8000)
    {
        mLastUpdate = gameTime;
        for (unsigned int i = 0; i < mAllChunks.size(); i++)
        {
            mAllChunks.at(i)->updateInhabitedTime();
        }
    }

    if (!mPauseBlockUpdatePackets)
    {
        bool sentRegionUpdate = false;
        for (unsigned int i = 0; i < mChangedChunks.size();)
        {
            sentRegionUpdate |= mChangedChunks[i]->broadcastChanges(!sentRegionUpdate);
            if (mChangedChunks[i]->mBlockChangesCount == 0)
            {
                mChangedChunks[i] = mChangedChunks.back();
                mChangedChunks.pop_back();
            }
            else
            {
                ++i;
            }
        }
    }

    mTicksUntilNextUpdate += 25;
    if (mTicksUntilNextUpdate > 25)
        mTicksUntilNextUpdate = 25;

    PIXBeginNamedEvent(0.0f, "PlayerChunkMap tickAddRequests");
    uint64_t tickStart = System::processTimeInMilliSecs();
    while (mTicksUntilNextUpdate > 0)
    {
        uint64_t start = System::processTimeInMilliSecs();

        int bestIndex = -1;
        long long bestWait = 0;
        for (unsigned int i = 0; i < mPlayers.size(); i++)
        {
            long long wait = tickAddRequests(mPlayers[i], false);
            if (wait > bestWait)
            {
                bestIndex = i;
                bestWait = wait;
            }
        }
        if (bestIndex < 0)
            break;

        tickAddRequests(mPlayers[bestIndex], true);

        uint64_t end = System::processTimeInMilliSecs();
        mTicksUntilNextUpdate -= static_cast<int>(end - start);
    }
    uint64_t tickEnd = System::processTimeInMilliSecs();
    if (mLevel->mDimension->getType() == DimensionType::OVERWORLD)
        PIXAddNamedCounter(static_cast<float>(static_cast<long long>(tickEnd - tickStart)), "Tick add request");
    PIXEndNamedEvent();
}

long long PlayerChunkMap::getKeyForChunk(int x, int z)
{
    return (static_cast<long long>(x) + INT_MAX) | ((static_cast<long long>(z) + INT_MAX) << 32);
}

PlayerChunkContainer* PlayerChunkMap::getChunkIfLoaded(int x, int z)
{
    ContainerMap::iterator it = mChunks.find(getKeyForChunk(x, z));
    PlayerChunkContainer* chunk = NULL;
    if (it != mChunks.end())
        chunk = it->second;
    return chunk;
}

PlayerChunkContainer* PlayerChunkMap::getOrCreateChunk(int x, int z)
{
    long long key = getKeyForChunk(x, z);
    ContainerMap::iterator it = mChunks.find(key);
    PlayerChunkContainer* chunk = NULL;
    if (it != mChunks.end())
    {
        chunk = it->second;
    }
    else
    {
        chunk = new PlayerChunkContainer(this, x, z);
        mChunks[key] = chunk;
        mAllChunks.push_back(chunk);
    }
    return chunk;
}

void PlayerChunkMap::getChunkAndAddPlayer(int x, int z, boost::shared_ptr<ServerPlayer> player, bool send)
{
    MasterGameMode* gameMode = gMinecraftServer->mMasterGameMode;
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || !gameMode->IsRoundPlaying()
        || MasterGameMode::IsChunkWithinActiveBounds(gameMode, x, z))
    {
        ContainerMap::iterator it = mChunks.find(getKeyForChunk(x, z));
        if (it != mChunks.end())
            it->second->add(player, send);
        else
            mAddRequests.push_back(PlayerChunkAddRequest(x, z, player, send));
    }
}

void PlayerChunkMap::getChunkAndRemovePlayer(int x, int z, boost::shared_ptr<ServerPlayer> player, bool send)
{
    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (request->x == x && request->z == z && request->player.get() == player.get())
        {
            mAddRequests.erase(request);
            return;
        }
        ++request;
    }
    ContainerMap::iterator it = mChunks.find(getKeyForChunk(x, z));
    if (it != mChunks.end())
    {
        it->second->remove(player, send);
    }
}

void PlayerChunkMap::broadcastBlockUpdate(boost::shared_ptr<Packet> packet, int x, int y, int z)
{
    PlayerChunkContainer* chunk = getChunkIfLoaded(x >> 4, z >> 4);
    if (chunk != NULL)
    {
        chunk->broadcast(packet);
    }
}

void PlayerChunkMap::blockChanged(const BlockPos& pos)
{
    int chunkX = pos.x >> 4;
    int chunkZ = pos.z >> 4;
    PlayerChunkContainer* chunk;
    if (static_cast<ServerChunkCache*>(static_cast<ServerLevel*>(mLevel)->getChunkSource())->isCreatingOnRequest())
    {
        chunk = getOrCreateChunk(chunkX, chunkZ);
    }
    else
    {
        chunk = getChunkIfLoaded(chunkX, chunkZ);
    }
    if (chunk != NULL)
    {
        chunk->blockChanged(pos.x & 15, pos.y, pos.z & 15);
    }
}

bool PlayerChunkMap::isTrackingBlock(int x, int y, int z)
{
    return getChunkIfLoaded(x >> 4, z >> 4) != NULL;
}

void PlayerChunkMap::prioritiseBlockChanges(int x, int y, int z)
{
    PlayerChunkContainer* chunk = getChunkIfLoaded(x >> 4, z >> 4);
    if (chunk != NULL)
    {
        chunk->prioritiseBlockChanges();
    }
}

void PlayerChunkMap::add(boost::shared_ptr<ServerPlayer> player)
{
    static int DIRECTIONS[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

    const int chunkX = static_cast<int>(player->mX) >> 4;
    const int chunkZ = static_cast<int>(player->mZ) >> 4;
    player->mLastSentX = player->mX;
    player->mLastSentZ = player->mZ;

    int direction = 0;
    const int viewDistance = PLAYER_RADIUS(player);
    int offsetX = 0;
    int offsetZ = 0;
    getOrCreateChunk(chunkX, chunkZ)->add(player, false);

    int minX = chunkX;
    int maxX = chunkX;
    int minZ = chunkZ;
    int maxZ = chunkZ;
    for (int length = 1; length <= viewDistance * 2; ++length)
    {
        for (int side = 0; side < 2; ++side)
        {
            const int* step = DIRECTIONS[direction++ % 4];
            for (int i = 0; i < length; ++i)
            {
                offsetX += step[0];
                offsetZ += step[1];
                const int x = chunkX + offsetX;
                const int z = chunkZ + offsetZ;
                if (x > maxX)
                    maxX = x;
                if (x < minX)
                    minX = x;
                if (z > maxZ)
                    maxZ = z;
                if (z < minZ)
                    minZ = z;
                if (length < 14 || (length == 14 && (side == 0 || i < 13)))
                    getOrCreateChunk(x, z)->add(player, false);
                else
                    getChunkAndAddPlayer(x, z, player, false);
            }
        }
    }

    direction %= 4;
    for (int i = 0; i < viewDistance * 2; ++i)
    {
        offsetX += DIRECTIONS[direction][0];
        offsetZ += DIRECTIONS[direction][1];
        const int x = chunkX + offsetX;
        const int z = chunkZ + offsetZ;
        if (x > maxX)
            maxX = x;
        if (x < minX)
            minX = x;
        if (z > maxZ)
            maxZ = z;
        if (z < minZ)
            minZ = z;
        if (viewDistance * 2 < 15)
            getOrCreateChunk(x, z)->add(player, false);
        else
            getChunkAndAddPlayer(x, z, player, false);
    }

    INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
    if (networkPlayer != NULL && !networkPlayer->IsLocal())
    {
        player->mConnection->send(boost::shared_ptr<ChunkVisibilityAreaPacket>(
            new ChunkVisibilityAreaPacket(minX, maxX, minZ, maxZ, true, shouldActuallyDropChunks(), false)));
    }

    static_cast<ServerChunkCache*>(static_cast<ServerLevel*>(getLevel())->getChunkSource())->dontDrop(chunkX, chunkZ);
    mPlayers.push_back(player);
}

void PlayerChunkMap::remove(boost::shared_ptr<ServerPlayer> player)
{
    const int chunkX = static_cast<int>(player->mLastSentX) >> 4;
    const int chunkZ = static_cast<int>(player->mLastSentZ) >> 4;
    const int radius = PLAYER_RADIUS(player);
    for (int x = chunkX - radius; x <= chunkX + radius; ++x)
    {
        for (int z = chunkZ - radius; z <= chunkZ + radius; ++z)
        {
            PlayerChunkContainer* chunk = getChunkIfLoaded(x, z);
            if (chunk != NULL)
                chunk->remove(player, true);
        }
    }

    PlayerVector::iterator it = std::find(mPlayers.begin(), mPlayers.end(), player);
    if (!mPlayers.empty() && it != mPlayers.end())
        mPlayers.erase(std::find(mPlayers.begin(), mPlayers.end(), player));

    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (request->player.get() == player.get())
            request = mAddRequests.erase(request);
        else
            ++request;
    }
}

void PlayerChunkMap::removeOutsideArea(boost::shared_ptr<ServerPlayer> player, int minX, int minZ, int maxX, int maxZ)
{
    const int chunkX = static_cast<int>(player->mLastSentX) >> 4;
    const int chunkZ = static_cast<int>(player->mLastSentZ) >> 4;
    const int radius = PLAYER_RADIUS(player);
    for (int x = chunkX - radius; x <= chunkX + radius; ++x)
    {
        for (int z = chunkZ - radius; z <= chunkZ + radius; ++z)
        {
            if (x < minX || x > maxX || z < minZ || z > maxZ)
            {
                PlayerChunkContainer* chunk = getChunkIfLoaded(x, z);
                if (chunk != NULL)
                    chunk->remove(player, false);
            }
        }
    }

    player->mConnection->send(boost::shared_ptr<ChunkVisibilityAreaPacket>(
        new ChunkVisibilityAreaPacket(minX, maxX, minZ, maxZ, false, shouldActuallyDropChunks(), true)));

    PlayerVector::iterator it = std::find(mPlayers.begin(), mPlayers.end(), player);
    if (!mPlayers.empty() && it != mPlayers.end())
        mPlayers.erase(std::find(mPlayers.begin(), mPlayers.end(), player));

    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (request->player.get() == player.get())
            request = mAddRequests.erase(request);
        else
            ++request;
    }
}

void PlayerChunkMap::move(boost::shared_ptr<ServerPlayer> player)
{
    const int chunkX = static_cast<int>(player->mX) >> 4;
    const int chunkZ = static_cast<int>(player->mZ) >> 4;
    const double deltaXPos = player->mLastSentX - player->mX;
    const double deltaZPos = player->mLastSentZ - player->mZ;
    if (deltaXPos * deltaXPos + deltaZPos * deltaZPos < 64.0)
        return;

    const int lastChunkX = static_cast<int>(player->mLastSentX) >> 4;
    const int lastChunkZ = static_cast<int>(player->mLastSentZ) >> 4;
    const int deltaX = chunkX - lastChunkX;
    const int deltaZ = chunkZ - lastChunkZ;
    if (deltaX == 0 && deltaZ == 0)
        return;

    const bool inMiniGame = Minecraft::InMiniGame(eMiniGameId_Any, true);
    const int radius = PLAYER_RADIUS(player);
    std::vector<std::pair<int, int> > addedChunks;
    for (int x = chunkX - radius; x <= chunkX + radius; ++x)
    {
        for (int z = chunkZ - radius; z <= chunkZ + radius; ++z)
        {
            if (!PLAYER_CHUNK_IN_RANGE(x, z, lastChunkX, lastChunkZ, radius))
            {
                MasterGameMode* gameMode = gMinecraftServer->mMasterGameMode;
                if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || !gameMode->IsRoundPlaying()
                    || MasterGameMode::IsChunkWithinActiveBounds(gameMode, x, z))
                {
                    getChunkAndAddPlayer(x, z, player, false);
                    addedChunks.push_back(std::make_pair(x, z));
                }
            }

            if (!PLAYER_CHUNK_IN_RANGE(x - deltaX, z - deltaZ, chunkX, chunkZ, radius) && !inMiniGame)
                getChunkAndRemovePlayer(x - deltaX, z - deltaZ, player, true);
        }
    }

    INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
    if (networkPlayer != NULL && !networkPlayer->IsLocal())
    {
        while (!addedChunks.empty())
        {
            const int firstX = addedChunks[0].first;
            const int firstZ = addedChunks[0].second;
            int sameRow = 0;
            int sameColumn = 0;
            for (unsigned int i = 0; i < addedChunks.size(); ++i)
            {
                if (addedChunks[i].second == firstZ)
                    ++sameRow;
                if (addedChunks[i].first == firstX)
                    ++sameColumn;
            }

            if (sameRow > sameColumn)
            {
                std::vector<std::pair<int, int> >::iterator it = addedChunks.begin();
                while (it != addedChunks.end())
                {
                    if (it->second == firstZ)
                        it = addedChunks.erase(it);
                    else
                        ++it;
                }
                player->mConnection->send(boost::shared_ptr<ChunkVisibilityAreaPacket>(new ChunkVisibilityAreaPacket(
                    firstX, firstX + sameRow, firstZ, firstZ, true, shouldActuallyDropChunks(), false)));
            }
            else
            {
                std::vector<std::pair<int, int> >::iterator it = addedChunks.begin();
                while (it != addedChunks.end())
                {
                    if (it->first == firstX)
                        it = addedChunks.erase(it);
                    else
                        ++it;
                }
                player->mConnection->send(boost::shared_ptr<ChunkVisibilityAreaPacket>(new ChunkVisibilityAreaPacket(
                    firstX, firstX, firstZ, firstZ + sameColumn, true, shouldActuallyDropChunks(), false)));
            }
        }
    }

    player->mLastSentX = player->mX;
    player->mLastSentZ = player->mZ;
}

bool PlayerChunkMap::isPlayerIn(boost::shared_ptr<ServerPlayer> player, int x, int z)
{
    PlayerChunkContainer* chunk = getChunkIfLoaded(x, z);
    if (chunk == NULL)
        return false;

    std::list<ChunkPos>::iterator pending = std::find(player->mLoadedChunks.begin(), player->mLoadedChunks.end(),
                                                      chunk->getPos());
    return chunk->hasPlayer(player) && pending == player->mLoadedChunks.end();
}

int PlayerChunkMap::convertChunkRangeToBlock(int range)
{
    return (range << 4) - 0x10;
}

void PlayerChunkMap::setRadius(int radius)
{
    if (radius <= mViewDistance)
        return;

    static_cast<ServerLevel*>(mLevel)->getServer();
    PlayerList* players = gMinecraftServer != NULL ? gMinecraftServer->mPlayers : NULL;
    for (unsigned int i = 0; i < players->mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = players->mPlayers[i];
        if (player->mLevel != mLevel)
            continue;

        const int chunkX = static_cast<int>(player->mX) >> 4;
        const int chunkZ = static_cast<int>(player->mZ) >> 4;
        const int oldRadius = mViewDistance;
        for (int x = chunkX - radius; x < chunkX - oldRadius; ++x)
        {
            for (int z = chunkZ - radius; z <= chunkZ + radius; ++z)
                getChunkAndAddPlayer(x, z, player, true);
        }
        for (int x = chunkX + oldRadius; x <= chunkX + radius; ++x)
        {
            for (int z = chunkZ - radius; z <= chunkZ + radius; ++z)
                getChunkAndAddPlayer(x, z, player, true);
        }
        for (int z = chunkZ - radius; z < chunkZ - oldRadius; ++z)
        {
            for (int x = chunkX - oldRadius; x < chunkX + oldRadius; ++x)
                getChunkAndAddPlayer(x, z, player, true);
        }
        for (int z = chunkZ + oldRadius; z <= chunkZ + radius; ++z)
        {
            for (int x = chunkX - oldRadius; x < chunkX + oldRadius; ++x)
                getChunkAndAddPlayer(x, z, player, true);
        }
    }
    mViewDistance = radius;
}

void PlayerChunkMap::removeOutsideAreaForAllPlayers(int minX, int minZ, int maxX, int maxZ)
{
    (void)minX;
    (void)minZ;
    (void)maxX;
    (void)maxZ;
}

void PlayerChunkMap::reloadAllPlayers(int minX, int minZ, int maxX, int maxZ)
{
    PlayerVector players;
    players.insert(players.end(), mPlayers.begin(), mPlayers.end());

    mShouldDropChunks = true;
    while (!mPlayers.empty())
    {
        removeOutsideArea(mPlayers[0], minX, minZ, maxX, maxZ);
    }
    mShouldDropChunks = false;

    int oldRadius = mViewDistance;
    setRadius(3);
    for (PlayerVector::iterator it = players.begin(); it != players.end(); it++)
    {
        add(*it);
    }
    setRadius(oldRadius);
}

int PlayerChunkMap::getBlockRegionPacketsToBeSent(boost::shared_ptr<ServerPlayer> player)
{
    int count = 0;
    for (unsigned int i = 0; i < mChangedChunks.size(); ++i)
    {
        if (mChangedChunks[i]->IsGoingToSendBRUP() && mChangedChunks[i]->hasPlayer(player))
            ++count;
    }
    return count;
}

void PlayerChunkMap::PauseBlockUpdatePackets(bool pause)
{
    mPauseBlockUpdatePackets = pause;
}

void PlayerChunkMap::onPlayerJoin(boost::shared_ptr<ServerPlayer> player)
{
    (void)player;
}

void PlayerChunkMap::onPlayerLeave(boost::shared_ptr<ServerPlayer> player)
{
    (void)player;
}

#ifdef MINECRAFT_PC
bool PlayerChunkMap::inRange(int x, int z, int centerX, int centerZ, int radius)
{
    return x >= centerX - radius && x <= centerX + radius && z >= centerZ - radius && z <= centerZ + radius;
}

int PlayerChunkMap::getPlayerRadius(const boost::shared_ptr<ServerPlayer>& player)
{
    if (player == NULL || player->mConnection == NULL || player->mConnection->mViewDistance <= 0
        || Minecraft::InMiniGame(eMiniGameId_Any, true))
        return mViewDistance;
    return player->mConnection->mViewDistance;
}

void PlayerChunkMap::sendVisibleArea(boost::shared_ptr<ServerPlayer> player, int minX, int maxX, int minZ, int maxZ)
{
    if (minX > maxX || minZ > maxZ)
        return;
    INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
    if (networkPlayer == NULL || networkPlayer->IsLocal())
        return;
    player->mConnection->send(boost::shared_ptr<ChunkVisibilityAreaPacket>(
        new ChunkVisibilityAreaPacket(minX, maxX, minZ, maxZ, true, shouldActuallyDropChunks(), false)));
}

void PlayerChunkMap::setPlayerViewDistance(boost::shared_ptr<ServerPlayer> player, int distance)
{
    if (player == NULL || player->mConnection == NULL)
        return;
    const int oldRadius = getPlayerRadius(player);
    player->mConnection->mViewDistance = distance;
    const int newRadius = getPlayerRadius(player);
    if (newRadius == oldRadius || std::find(mPlayers.begin(), mPlayers.end(), player) == mPlayers.end())
        return;

    const int chunkX = static_cast<int>(player->mLastSentX) >> 4;
    const int chunkZ = static_cast<int>(player->mLastSentZ) >> 4;
    if (newRadius > oldRadius)
    {
        for (int x = chunkX - newRadius; x <= chunkX + newRadius; ++x)
            for (int z = chunkZ - newRadius; z <= chunkZ + newRadius; ++z)
                if (!inRange(x, z, chunkX, chunkZ, oldRadius))
                    getChunkAndAddPlayer(x, z, player, false);
        sendVisibleArea(player, chunkX - newRadius, chunkX - oldRadius - 1, chunkZ - newRadius, chunkZ + newRadius);
        sendVisibleArea(player, chunkX + oldRadius + 1, chunkX + newRadius, chunkZ - newRadius, chunkZ + newRadius);
        sendVisibleArea(player, chunkX - oldRadius, chunkX + oldRadius, chunkZ - newRadius, chunkZ - oldRadius - 1);
        sendVisibleArea(player, chunkX - oldRadius, chunkX + oldRadius, chunkZ + oldRadius + 1, chunkZ + newRadius);
    }
    else
    {
        for (int x = chunkX - oldRadius; x <= chunkX + oldRadius; ++x)
            for (int z = chunkZ - oldRadius; z <= chunkZ + oldRadius; ++z)
                if (!inRange(x, z, chunkX, chunkZ, newRadius))
                    getChunkAndRemovePlayer(x, z, player, true);
    }
}
#endif
