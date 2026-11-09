#ifndef GAME_WORLD_CHUNK_READONLYCHUNKCache_HPP
#define GAME_WORLD_CHUNK_READONLYCHUNKCache_HPP

#include <string>
#include <vector>

#include "game/world/chunk/ChunkSource.hpp"

class BlockPos;
class ChunkStorage;
class Level;
class LevelChunk;
class Mob;
class MobCategory;
class ProgressListener;

class ReadOnlyChunkCache : public ChunkSource
{
public:
    ReadOnlyChunkCache(Level* level, ChunkStorage* storage);
    virtual ~ReadOnlyChunkCache();
    virtual bool hasChunk(int x, int z);
    virtual LevelChunk* create(int x, int z);
    LevelChunk* load(int x, int z);
    virtual LevelChunk* getChunk(int x, int z);
    virtual void postProcess(ChunkSource* source, int x, int z);
    virtual bool save(bool force, ProgressListener* listener);
    virtual bool tick();
    virtual bool shouldSave();
    virtual std::wstring gatherStats();
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                            bool takeBlock);

private:
    LevelChunk** m_chunks;
    unsigned int m_chunkCount;
    Level* m_level;
    ChunkStorage* m_storage;
    unsigned char* m_data;
};

#endif
