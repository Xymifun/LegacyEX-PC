#include "game/world/chunk/ChunkSource.hpp"

class Level;

void ChunkSource::flagPostProcessComplete(short index, int chunkX, int chunkZ)
{
}

bool ChunkSource::isChunkGeneratedAt(int chunkX, int chunkZ)
{
    return false;
}

bool ChunkSource::reallyHasChunk(int chunkX, int chunkZ)
{
    return hasChunk(chunkX, chunkZ);
}

LevelChunk* ChunkSource::getChunkLoadedOrUnloaded(int chunkX, int chunkZ)
{
    return getChunk(chunkX, chunkZ);
}

void ChunkSource::lightChunk(LevelChunk* chunk)
{
}

bool ChunkSource::saveAllEntities()
{
    return false;
}

LevelChunk** ChunkSource::getCache()
{
    return 0;
}

void ChunkSource::dataReceived(int chunkX, int chunkZ)
{
}
