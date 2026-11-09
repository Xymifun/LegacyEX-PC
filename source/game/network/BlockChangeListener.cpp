#include "game/network/BlockChangeListener.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include <utility>
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/LevelChunk.hpp"

namespace
{
const int MAX_CLIENT_LOADED_CHANGES = 20;
}

BlockChangeListener::BlockChangeListener(ServerLevel* level)
    : mLevel(level),
      mChunks(new boost::unordered_map<long long, Chunk>())
{
}

BlockChangeListener::~BlockChangeListener()
{
    delete mChunks;
}

long long BlockChangeListener::GetChunkKey(int x, int z)
{
    return (static_cast<long long>(x) + 0x7fffffffLL) | ((static_cast<long long>(z) + 0x7fffffffLL) << 32);
}

bool BlockChangeListener::HasChunk(long long key) const
{
    return mChunks->find(key) != mChunks->end();
}

BlockChangeListener::Chunk& BlockChangeListener::GetChunk(long long key) const
{
    return mChunks->at(key);
}

BlockChangeListener::Chunk& BlockChangeListener::GetChunk(long long key)
{
    return mChunks->at(key);
}

std::vector<short>* BlockChangeListener::GetChangesForChunk(int x, int z) const
{
    std::vector<short>* changes = NULL;
    long long key = GetChunkKey(x, z);
    if (HasChunk(key))
        changes = &GetChunk(key).GetChanges();
    return changes;
}

bool BlockChangeListener::ShouldLoadOnClient(int x, int z) const
{
    Chunk* chunk = NULL;
    long long key = GetChunkKey(x, z);
    if (HasChunk(key))
        chunk = &GetChunk(key);
    return chunk == NULL || chunk->GetChangeCount() <= MAX_CLIENT_LOADED_CHANGES;
}

BlockChangeListener::Chunk& BlockChangeListener::GetChunk(int x, int z, bool create)
{
    long long key = GetChunkKey(x, z);
    if (create && !HasChunk(key))
        mChunks->insert(std::make_pair(key, Chunk(x, z)));
    return mChunks->at(key);
}

int BlockChangeListener::GetBlockKey(const BlockPos& pos)
{
    return LevelChunk::PackBlockPos(pos);
}

void BlockChangeListener::blockChanged(Level*, const BlockPos& pos, const BlockState*, const BlockState* newState, int,
                                       bool)
{
    int chunkX = pos.x >> 4;
    int chunkZ = pos.z >> 4;
    Chunk& chunk = GetChunk(chunkX, chunkZ, true);
    chunk.AddBlockChange(pos);
    MinecraftServer* server = gMinecraftServer;
    if (server != NULL && newState != NULL && newState->getBlock() == Blocks::air)
        server->mMasterGameMode->BlockDestroyedAt(pos);
}

void BlockChangeListener::invalidateChunksOutsideArea(int minChunkX, int minChunkZ, int maxChunkX, int maxChunkZ)
{
    boost::unordered_map<long long, Chunk>::iterator it = mChunks->begin();
    while (it != mChunks->end())
    {
        int chunkX = it->second.GetX();
        int chunkZ = it->second.GetZ();
        if (chunkX < minChunkX || chunkX > maxChunkX || chunkZ < minChunkZ || chunkZ > maxChunkZ)
            it = mChunks->erase(it);
        else
            it++;
    }
}

void BlockChangeListener::allChanged()
{
}
void BlockChangeListener::blockLightChanged(const BlockPos&)
{
}
void BlockChangeListener::setBlocksDirty(int, int, int, int, int, int, Level*)
{
}
void BlockChangeListener::playStreamingMusic(const SoundEvent*, const BlockPos&, bool)
{
}
void BlockChangeListener::playSound(boost::shared_ptr<Player>, const SoundEvent*, SoundSource::ESoundSource, double,
                                    double, double, float, float, float)
{
}
void BlockChangeListener::addParticle(Level*, ePARTICLE_TYPE, bool, double, double, double, double, double, double,
                                      arrayWithLength<int>)
{
}
void BlockChangeListener::addParticle(Level*, ePARTICLE_TYPE, bool, bool, double, double, double, double, double,
                                      double, arrayWithLength<int>)
{
}
void BlockChangeListener::entityAdded(boost::shared_ptr<Entity>)
{
}
void BlockChangeListener::entityRemoved(boost::shared_ptr<Entity>)
{
}
void BlockChangeListener::playerRemoved(boost::shared_ptr<Entity>)
{
}
void BlockChangeListener::globalLevelEvent(int, const BlockPos&, int)
{
}
void BlockChangeListener::levelEvent(boost::shared_ptr<Player>, int, const BlockPos&, int)
{
}
void BlockChangeListener::destroyBlockProgress(int, const BlockPos&, int)
{
}

BlockChangeListener::Chunk::Chunk(int x, int z) : mChangeCount(0), mChanges(), mX(x), mZ(z)
{
    mChanges.reserve(20);
}

std::vector<short>& BlockChangeListener::Chunk::GetChanges()
{
    return mChanges;
}

int BlockChangeListener::Chunk::GetChangeCount()
{
    return mChangeCount;
}

bool BlockChangeListener::Chunk::HasBlockChange(short blockKey)
{
    std::vector<short>::iterator it = mChanges.begin();
    for (; it != mChanges.end(); ++it)
    {
        if (*it == blockKey)
            break;
    }
    return it != mChanges.end();
}

void BlockChangeListener::Chunk::AddBlockChange(const BlockPos& pos)
{
    int changeCount = mChangeCount;
    if (changeCount <= MAX_CLIENT_LOADED_CHANGES)
    {
        if (changeCount < MAX_CLIENT_LOADED_CHANGES)
        {
            short blockKey = static_cast<short>(BlockChangeListener::GetBlockKey(pos));
            if (HasBlockChange(blockKey))
                return;
            ++mChangeCount;
            mChanges.push_back(blockKey);
            return;
        }
        mChanges.clear();
    }
    mChangeCount = changeCount + 1;
}

int BlockChangeListener::Chunk::GetX() const
{
    return mX;
}

int BlockChangeListener::Chunk::GetZ() const
{
    return mZ;
}
