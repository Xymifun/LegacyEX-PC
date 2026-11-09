#include "game/world/chunk/PlayerChunkContainer.hpp"

#include <limits>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/EntityTracker.hpp"
#include "game/network/BlockRegionUpdatePacket.hpp"
#include "game/network/ClientboundBlockUpdatePacket.hpp"
#include "game/network/ClientboundChunkBlocksUpdatePacket.hpp"
#include "game/network/ChunkVisibilityPacket.hpp"
#include "game/network/Packet.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/chunk/LevelChunk.hpp"

#include "game/world/chunk/PlayerChunkMap.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerList.hpp"

PlayerChunkContainer::PlayerChunkContainer(PlayerChunkMap* map, int x, int z) : mPos(x, z)
{
    mMap = map;
    mBlockChangesCount = 0;
    mBlockChanges = arrayWithLength<short>(MAX_BLOCK_CHANGES, true);
    mChunk = NULL;
    mMinX = mMaxX = 0;
    mMinY = mMaxY = 0;
    mMinZ = mMaxZ = 0;
    mUpdateCounter = 0;
    mPrioritiseBlockChanges = false;
    mLastUpdate = 0;
    mHasSentToPlayers = false;
    mChunk = static_cast<ServerLevel*>(map->getLevel())->getChunkSource()->getOrCreateChunk(x, z, false);
}

PlayerChunkContainer::~PlayerChunkContainer()
{
    delete[] mBlockChanges.data;
}

void PlayerChunkContainer::sendToPlayer(boost::shared_ptr<ServerPlayer> player)
{
    static_cast<ServerLevel*>(mMap->getLevel())->getTracker()->playerLoadedChunk(player, mChunk);
}

bool PlayerChunkContainer::hasPlayer(boost::shared_ptr<ServerPlayer> player)
{
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if (it->get() == player.get())
            break;
    }
    return it != mPlayers.end();
}

void PlayerChunkContainer::add(boost::shared_ptr<ServerPlayer> player, bool send)
{
    if (player)
    {
        if (std::find(mPlayers.begin(), mPlayers.end(), player) == mPlayers.end())
        {
            player->mTrackedChunks.insert(mPos);
            bool isLocal = false;
            if (player->mConnection->getNetworkPlayer() != NULL)
            {
                if (player->mConnection->getNetworkPlayer()->IsLocal())
                    isLocal = true;
            }
            if (send | isLocal)
            {
                player->mConnection->send(
                    boost::shared_ptr<ChunkVisibilityPacket>(new ChunkVisibilityPacket(mPos.x, mPos.z, true, false)));
            }

            if (mPlayers.empty())
            {
                mLastUpdate = mMap->getLevel()->getGameTime();
            }

            mPlayers.push_back(player);
            sendToPlayer(player);
            player->mLoadedChunks.push_back(mPos);

            ServerLevel* level = static_cast<ServerLevel*>(mMap->getLevel());
            static_cast<ServerChunkCache*>(level->getChunkSource())->dontDrop(mPos.x, mPos.z);
        }
    }
}

void PlayerChunkContainer::updateInhabitedTime()
{
    long long gameTime = mMap->getLevel()->getGameTime();
    if (mChunk != NULL)
        mChunk->setInhabitedTime(mChunk->getInhabitedTime() + gameTime - mLastUpdate);
    mLastUpdate = gameTime;
}

void PlayerChunkContainer::remove(boost::shared_ptr<ServerPlayer> player, bool send)
{
    PlayerChunkContainer* emptied = NULL;
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = std::find(mPlayers.begin(), mPlayers.end(), player);
    if (it != mPlayers.end())
    {
        mPlayers.erase(it);
        if (mPlayers.empty())
        {
            updateInhabitedTime();

            PlayerChunkMap::ContainerVector::iterator all = std::find(mMap->mAllChunks.begin(), mMap->mAllChunks.end(),
                                                                      this);
            if (all != mMap->mAllChunks.end())
            {
                mMap->mAllChunks.erase(all);
            }

            long long id = (static_cast<long long>(mPos.x) + INT_MAX)
                           | ((static_cast<long long>(mPos.z) + INT_MAX) << 32);
            PlayerChunkMap::ContainerMap::iterator entry = mMap->mChunks.find(id);
            if (entry != mMap->mChunks.end())
            {
                emptied = entry->second;
                mMap->mChunks.erase(entry);
            }

            if (mBlockChangesCount > 0)
            {
                PlayerChunkMap::ContainerVector::iterator changed = std::find(mMap->mChangedChunks.begin(),
                                                                              mMap->mChangedChunks.end(), this);
                mMap->mChangedChunks.erase(changed);
            }

            ServerLevel* level = static_cast<ServerLevel*>(mMap->getLevel());
            static_cast<ServerChunkCache*>(level->getChunkSource())->drop(mPos.x, mPos.z);
        }

        player->mLoadedChunks.remove(mPos);
        if (send && player->mConnection != NULL && player->mTrackedChunks.find(mPos) != player->mTrackedChunks.end())
        {
            INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
            if (networkPlayer != NULL)
            {
                bool sentToSameSystem = false;
                for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator other = mPlayers.begin();
                     other < mPlayers.end(); ++other)
                {
                    boost::shared_ptr<ServerPlayer> otherPlayer = *other;
                    if (otherPlayer->mConnection != NULL)
                    {
                        INetworkPlayer* otherNetworkPlayer = otherPlayer->mConnection->getNetworkPlayer();
                        if (otherNetworkPlayer != NULL && otherNetworkPlayer->IsSameSystem(networkPlayer)
                            && otherPlayer->mTrackedChunks.find(mPos) != otherPlayer->mTrackedChunks.end())
                        {
                            sentToSameSystem = true;
                            break;
                        }
                    }
                }
                if (!sentToSameSystem)
                {
                    player->mConnection->send(boost::shared_ptr<ChunkVisibilityPacket>(
                        new ChunkVisibilityPacket(mPos.x, mPos.z, false, mMap->shouldActuallyDropChunks())));
                }
            }
        }
        delete emptied;
    }
}

namespace
{

const int BLOCK_REGION_UPDATE_PACKET_ID = 51;
const int CHUNK_BLOCKS_UPDATE_PACKET_ID = 52;
const int BLOCK_UPDATE_PACKET_ID = 53;
const int BLOCK_ENTITY_DATA_PACKET_ID = 132;

}

bool PlayerChunkContainer::broadcast(boost::shared_ptr<Packet> packet)
{
    bool hasRemotePlayer = false;
    std::vector<boost::shared_ptr<ServerPlayer> > sentTo;

    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = mPlayers[i];
        bool dontSend = false;
        INetworkPlayer* thisPlayer = NULL;
        if (player->mConnection != NULL)
        {
            thisPlayer = player->mConnection->getNetworkPlayer();
            if (thisPlayer != NULL && !thisPlayer->IsLocal())
                hasRemotePlayer = true;
        }

        if (!sentTo.empty())
        {
            if (thisPlayer == NULL)
            {
                dontSend = true;
            }
            else
            {
                for (unsigned int j = 0; j < sentTo.size(); ++j)
                {
                    boost::shared_ptr<ServerPlayer> player2 = sentTo[j];
                    INetworkPlayer* otherPlayer = player2->mConnection->getNetworkPlayer();
                    if (otherPlayer != NULL && thisPlayer->IsSameSystem(otherPlayer))
                        dontSend = true;
                }
            }
        }

        if (dontSend)
            continue;

        const int flagIndex = ServerPlayer::getFlagIndexForChunk(mPos, mMap->mDimension);
        if (player->mTrackedChunks.find(mPos) != player->mTrackedChunks.end()
            && CGameNetworkManager::sInstance.SystemFlagGet(thisPlayer, flagIndex))
        {
            player->mConnection->send(packet);
            sentTo.push_back(player);
        }
    }

    if (packet->getPacketId() == BLOCK_REGION_UPDATE_PACKET_ID || packet->getPacketId() == CHUNK_BLOCKS_UPDATE_PACKET_ID
        || packet->getPacketId() == BLOCK_UPDATE_PACKET_ID || packet->getPacketId() == BLOCK_ENTITY_DATA_PACKET_ID)
    {
        for (unsigned int i = 0; i < mMap->getLevel()->getServer()->getPlayers()->mPlayers.size(); ++i)
        {
            boost::shared_ptr<ServerPlayer> player = mMap->getLevel()->getServer()->getPlayers()->mPlayers[i];
            if (player->mConnection == NULL)
                continue;

            const int flagIndex = ServerPlayer::getFlagIndexForChunk(mPos, mMap->mDimension);
            if (!CGameNetworkManager::sInstance.SystemFlagGet(player->mConnection->getNetworkPlayer(), flagIndex))
                continue;

            bool dontSend = false;
            if (!sentTo.empty())
            {
                INetworkPlayer* thisPlayer = player->mConnection->getNetworkPlayer();
                if (thisPlayer == NULL)
                {
                    dontSend = true;
                }
                else
                {
                    for (unsigned int j = 0; j < sentTo.size(); ++j)
                    {
                        boost::shared_ptr<ServerPlayer> player2 = sentTo[j];
                        INetworkPlayer* otherPlayer = player2->mConnection->getNetworkPlayer();
                        if (otherPlayer != NULL && thisPlayer->IsSameSystem(otherPlayer))
                            dontSend = true;
                    }
                }
            }

            if (!dontSend)
            {
                player->mConnection->send(packet);
                sentTo.push_back(player);
            }
        }
    }

    return hasRemotePlayer;
}

void PlayerChunkContainer::broadcast(boost::shared_ptr<BlockEntity> blockEntity)
{
    if (blockEntity)
    {
        boost::shared_ptr<Packet> packet = blockEntity->getUpdatePacket();
        if (packet)
            broadcast(packet);
    }
}

bool PlayerChunkContainer::IsGoingToSendBRUP()
{
    return mBlockChangesCount >= MAX_BLOCK_CHANGES;
}

bool PlayerChunkContainer::broadcastChanges(bool allowRegionUpdate)
{
    Level* level = mMap->getLevel();
    if (mUpdateCounter > 0)
    {
        --mUpdateCounter;
    }

    if (mBlockChangesCount == 0)
    {
        mPrioritiseBlockChanges = false;
        return false;
    }

    if (mBlockChangesCount == 1)
    {
        BlockPos pos(mPos.x * 16 + mMinX, mMinY, mPos.z * 16 + mMinZ);
        {
            boost::shared_ptr<Packet> packet(new ClientboundBlockUpdatePacket(level, pos));
            broadcast(packet);
        }
        if (level->getBlockState(pos)->getBlock()->mBlockEntity)
        {
            broadcast(level->getBlockEntity(pos));
        }
    }
    else if (mBlockChangesCount >= MAX_BLOCK_CHANGES)
    {
        if (!mPrioritiseBlockChanges && (!allowRegionUpdate || mUpdateCounter > 0))
        {
            return false;
        }

        mMinY = (mMinY / 2) * 2;
        mMaxY = (mMaxY / 2) * 2 + 2;
        int x = mPos.x * 16 + mMinX;
        int z = mPos.z * 16 + mMinZ;
        int width = mMaxX - mMinX + 1;
        int height = mMaxY - mMinY + 2;
        int depth = mMaxZ - mMinZ + 1;
        if (height > 256)
        {
            height = 256;
        }

        {
            boost::shared_ptr<Packet> packet(
                new BlockRegionUpdatePacket(x, mMinY, z, width, height, depth, level, false, NULL));
            broadcast(packet);
        }

        std::vector<boost::shared_ptr<BlockEntity> >* blockEntities =
            static_cast<ServerLevel*>(level)->getBlockEntitiesInRegion(x, mMinY, z, x + width, mMinY + height,
                                                                       z + depth);
        for (unsigned int i = 0; i < blockEntities->size(); ++i)
        {
            broadcast(blockEntities->at(i));
        }
        delete blockEntities;

        mBlockChangesCount = 0;
        mPrioritiseBlockChanges = false;
        mUpdateCounter = 1;
        return true;
    }
    else
    {
        {
            boost::shared_ptr<Packet> packet(new ClientboundChunkBlocksUpdatePacket(
                mPos.x, mPos.z, mBlockChanges, static_cast<unsigned char>(mBlockChangesCount), level));
            broadcast(packet);
        }

        for (int i = 0; i < mBlockChangesCount; ++i)
        {
            short packed = mBlockChanges[i];
            BlockPos pos((mPos.x << 4) | ((packed >> 12) & 15), packed & 255, (mPos.z << 4) | ((packed >> 8) & 15));
            if (level->getBlockState(pos)->getBlock()->mBlockEntity)
            {
                broadcast(level->getBlockEntity(pos));
            }
        }
    }

    mBlockChangesCount = 0;
    mPrioritiseBlockChanges = false;
    return false;
}

void PlayerChunkContainer::blockChanged(int x, int y, int z)
{
    if (mBlockChangesCount == 0)
    {
        mMap->mChangedChunks.push_back(this);
        mMinX = mMaxX = x;
        mMinY = mMaxY = y;
        mMinZ = mMaxZ = z;
    }

    if (mMinX > x)
        mMinX = x;
    if (mMaxX < x)
        mMaxX = x;
    if (mMinY > y)
        mMinY = y;
    if (mMaxY < y)
        mMaxY = y;
    if (mMinZ > z)
        mMinZ = z;
    if (mMaxZ < z)
        mMaxZ = z;

    if (mBlockChangesCount < MAX_BLOCK_CHANGES)
    {
        short packed = static_cast<short>((x << 12) | (z << 8) | y);
        int i;
        for (i = 0; i < mBlockChangesCount; ++i)
            if (mBlockChanges[i] == packed)
                return;
        mBlockChanges[mBlockChangesCount] = packed;
    }
    ++mBlockChangesCount;
}

void PlayerChunkContainer::prioritiseBlockChanges()
{
    mPrioritiseBlockChanges = true;
}

ChunkPos PlayerChunkContainer::getPos()
{
    return mPos;
}

double PlayerChunkContainer::getDistanceSqrToClosestPlayer()
{
    double result = std::numeric_limits<double>::max();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        double distance = mPos.distanceToSqr(player);
        if (distance < result)
            result = distance;
    }
    return result;
}

bool PlayerChunkContainer::updatePlayerReference(boost::shared_ptr<ServerPlayer> player)
{
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> existing = mPlayers[i];
        if (existing->getXuid() == player->getXuid())
        {
            mPlayers[i] = player;
            return true;
        }
    }
    return false;
}

void PlayerChunkContainer::removeAllPlayers(PlayerChunkContainer* container, bool send)
{
    std::vector<boost::shared_ptr<ServerPlayer> > players(container->mPlayers);
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players.begin(); it != players.end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        container->remove(player, send);
    }
}
