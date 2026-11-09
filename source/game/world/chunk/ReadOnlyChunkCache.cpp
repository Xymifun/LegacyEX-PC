#include "game/world/chunk/ReadOnlyChunkCache.hpp"

#include "game/entity/mob/MobCategory.hpp"
#include "game/network/ProgressListener.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkStorage.hpp"
#include "game/world/chunk/EmptyLevelChunk.hpp"
#include "game/world/chunk/LevelChunk.hpp"

ReadOnlyChunkCache::~ReadOnlyChunkCache()
{
    for (unsigned int i = 0; i < m_chunkCount; ++i)
    {
        if (m_chunks[i])
            delete m_chunks[i];
    }

    delete[] m_chunks;
    delete[] m_data;
}

bool ReadOnlyChunkCache::hasChunk(int x, int z)
{
    LevelChunk* chunk = m_chunks[(x & 15) | ((z & 15) << 4)];
    return chunk != 0 && chunk->isAt(x, z);
}

LevelChunk* ReadOnlyChunkCache::create(int x, int z)
{
    return getChunk(x, z);
}

LevelChunk* ReadOnlyChunkCache::load(int x, int z)
{
    return m_storage->load(m_level, x, z, 0);
}

LevelChunk* ReadOnlyChunkCache::getChunk(int x, int z)
{
    int xx = x & 15;
    int zz = z & 15;
    int chunkIndex = xx | (zz << 4);
    LevelChunk* chunk;
    if (hasChunk(x, z))
    {
        chunk = m_chunks[chunkIndex];
    }
    else
    {
        chunk = load(x, z);
        if (!chunk)
            chunk = new EmptyLevelChunk(m_level, x, z);
        m_chunks[chunkIndex] = chunk;
    }
    return chunk;
}

void ReadOnlyChunkCache::postProcess(ChunkSource*, int, int)
{
}

bool ReadOnlyChunkCache::save(bool, ProgressListener*)
{
    return true;
}

bool ReadOnlyChunkCache::tick()
{
    return false;
}

bool ReadOnlyChunkCache::shouldSave()
{
    return false;
}

std::wstring ReadOnlyChunkCache::gatherStats()
{
    return L"ReadOnlyChunkCache";
}

std::vector<Biome::MobSpawnerData*>* ReadOnlyChunkCache::getMobsAt(MobCategory*, const BlockPos&)
{
    return NULL;
}

BlockPos* ReadOnlyChunkCache::findNearestMapFeature(Level*, const std::wstring&, const BlockPos&, bool)
{
    return 0;
}
