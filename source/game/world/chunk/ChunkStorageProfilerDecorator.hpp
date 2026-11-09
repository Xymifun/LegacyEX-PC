#ifndef GAME_WORLD_CHUNK_CHUNKSTORAGEPROFILERDECORATOR_HPP
#define GAME_WORLD_CHUNK_CHUNKSTORAGEPROFILERDECORATOR_HPP

#include "game/world/chunk/ChunkStorage.hpp"

class ChunkStorageProfilerDecorator : public ChunkStorage
{
public:
    ChunkStorageProfilerDecorator(ChunkStorage* chunkStorage);
    virtual ~ChunkStorageProfilerDecorator();

    virtual LevelChunk* load(Level* level, int x, int z, LevelChunk* chunk);
    virtual void save(Level* level, LevelChunk* chunk);
    virtual void saveEntities(Level* level, LevelChunk* chunk);
    virtual void tick();
    virtual void flush();

private:
    ChunkStorage* mChunkStorage;
    unsigned long long mLoadTime;
    unsigned long long mLoadCount;
    unsigned long long mSaveTime;
    unsigned long long mSaveCount;
    int mTickCount;
};

#endif
