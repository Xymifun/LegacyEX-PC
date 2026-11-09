#ifndef GAME_WORLD_CHUNK_CHUNKSOURCE_HPP
#define GAME_WORLD_CHUNK_CHUNKSOURCE_HPP

#include <string>
#include <vector>

#include "game/world/biome/Biome.hpp"

class BlockPos;
class Level;
class LevelChunk;
class Mob;
class MobCategory;
class ProgressListener;

class ChunkSource
{
public:
    int mLevelSize;

    virtual ~ChunkSource() {}
    virtual LevelChunk* getChunkIfLoaded(int chunkX, int chunkZ) = 0;
    virtual LevelChunk* getOrCreateChunk(int chunkX, int chunkZ, bool allowEmpty) = 0;
    virtual bool tick() = 0;
    virtual std::wstring gatherStats() = 0;
    virtual int getLoadedChunksCount() = 0;
    virtual void flagPostProcessComplete(short index, int chunkX, int chunkZ);
    virtual bool isChunkGeneratedAt(int chunkX, int chunkZ);
    virtual bool hasChunk(int chunkX, int chunkZ) = 0;
    virtual bool reallyHasChunk(int chunkX, int chunkZ);
    virtual LevelChunk* getChunk(int chunkX, int chunkZ) = 0;
    virtual LevelChunk* getChunkAt(const BlockPos& pos) = 0;
    virtual LevelChunk* getChunkLoadedOrUnloaded(int chunkX, int chunkZ);
    virtual LevelChunk* getChunkIfHasChunk(int chunkX, int chunkZ) = 0;
    virtual void lightChunk(LevelChunk* chunk);
    virtual LevelChunk* create(int chunkX, int chunkZ) = 0;
    virtual bool saveAllEntities();
    virtual bool save(bool force, ProgressListener* progressListener) = 0;
    virtual bool shouldSave() = 0;
    virtual LevelChunk** getCache();
    virtual void dataReceived(int chunkX, int chunkZ);
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(MobCategory* category, const BlockPos& pos) = 0;
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                            bool takeBlock) = 0;
    virtual int getLoadedChunks() = 0;
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ) = 0;
};

#endif
