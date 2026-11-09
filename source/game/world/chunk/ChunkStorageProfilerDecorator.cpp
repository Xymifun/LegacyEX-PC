#include "game/world/chunk/ChunkStorageProfilerDecorator.hpp"

#include "game/core/System.hpp"

ChunkStorageProfilerDecorator::ChunkStorageProfilerDecorator(ChunkStorage* chunkStorage)
    : mChunkStorage(chunkStorage),
      mLoadTime(0),
      mLoadCount(0),
      mSaveTime(0),
      mSaveCount(0),
      mTickCount(0)
{
}

ChunkStorageProfilerDecorator::~ChunkStorageProfilerDecorator()
{
}

LevelChunk* ChunkStorageProfilerDecorator::load(Level* level, int x, int z, LevelChunk* chunk)
{
    unsigned long long startTime = System::processTimeInNanoSecs();
    LevelChunk* result = mChunkStorage->load(level, x, z, chunk);
    mLoadTime += System::processTimeInNanoSecs() - startTime;
    ++mLoadCount;
    return result;
}

void ChunkStorageProfilerDecorator::save(Level* level, LevelChunk* chunk)
{
    unsigned long long startTime = System::processTimeInNanoSecs();
    mChunkStorage->save(level, chunk);
    mSaveTime += System::processTimeInNanoSecs() - startTime;
    ++mSaveCount;
}

void ChunkStorageProfilerDecorator::saveEntities(Level* level, LevelChunk* chunk)
{
    mChunkStorage->saveEntities(level, chunk);
}

void ChunkStorageProfilerDecorator::tick()
{
    mChunkStorage->tick();
    ++mTickCount;
    if (mTickCount > 500)
    {
        mTickCount = 0;
    }
}

void ChunkStorageProfilerDecorator::flush()
{
    mChunkStorage->flush();
}
