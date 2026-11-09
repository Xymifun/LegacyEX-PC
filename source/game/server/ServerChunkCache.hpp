#ifndef GAME_SERVER_SERVERCHUNKCACHE_HPP
#define GAME_SERVER_SERVERCHUNKCACHE_HPP

#include "game/world/chunk/ChunkSource.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/world/biome/Biome.hpp"
#include <deque>
#include <vector>
#include <string>

class ServerLevel;
class Dimension;
class ChunkStorage;
class ChunkGenerator;
class LevelChunk;
class BlockPos;
class Level;
class MobCategory;
class ProgressListener;

class ServerChunkCache : public ChunkSource
{
public:
    bool isCreatingOnRequest() const { return m_createOnRequest; }
    ServerChunkCache(ServerLevel* level, ChunkStorage* storage, ChunkGenerator* generator);
    virtual ~ServerChunkCache();
    void dontDrop(int x, int z);
    int computeIdx(int x, int z);
    bool inBounds(int x, int z);
    void drop(int x, int z);
    void dropAll();
    void invalidateChunksOutsideArea(int minX, int minZ, int maxX, int maxZ);
    virtual LevelChunk* getChunkIfLoaded(int x, int z);
    LevelChunk* getChunkIfLoadedOrInvalid(int x, int z);
    virtual LevelChunk* getChunkIfHasChunk(int x, int z);
    LevelChunk* load(int x, int z, LevelChunk* chunk);
    void updatePostProcessFlag(short index, int x, int z, int dx, int dz, LevelChunk* chunk);
    void updatePostProcessFlags(int x, int z);
    LevelChunk* updateCacheAndPostProcess(int x, int z, LevelChunk* newChunk, LevelChunk* oldChunk, bool allowEmpty);
    LevelChunk* getChunkIfGenerated(int x, int z, bool allowEmpty);
    virtual LevelChunk* getOrCreateChunk(int x, int z, bool allowEmpty);
    virtual int getLoadedChunksCount();
    virtual bool hasChunk(int x, int z);
    virtual bool isChunkGeneratedAt(int x, int z);
    std::vector<LevelChunk*>& getLoadedChunkList();
    LevelChunk* create(int x, int z, bool allowEmpty);
    virtual LevelChunk* create(int x, int z);
    virtual LevelChunk* getChunk(int x, int z);
    virtual LevelChunk* getChunkLoadedOrUnloaded(int x, int z);
    void saveEntities(LevelChunk* chunk);
    void save(LevelChunk* chunk);
    virtual void flagPostProcessComplete(short index, int x, int z);
    virtual bool saveAllEntities();
    virtual bool save(bool force, ProgressListener* listener);
    virtual bool tick();
    virtual bool shouldSave();
    virtual std::wstring gatherStats();
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                            bool takeBlock);
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int x, int z);
    virtual LevelChunk* getChunkAt(const BlockPos& pos);
    static unsigned int runSaveThreadProc(void* data);
    virtual int getLoadedChunks();
    void setOverrideBoundFunction(bool (*function)(void*, int, int), void* data);
    virtual LevelChunk** getCache();
#ifdef MINECRAFT_PC
    void PcLockGeneration();
    void PcUnlockGeneration();
#endif

private:
    LevelChunk* m_emptyChunk;
    ChunkGenerator* m_source;
    ChunkStorage* m_storage;

public:
    bool m_createOnRequest;

private:
    LevelChunk** m_cache;
    std::vector<LevelChunk*> m_loadedChunkList;
    ServerLevel* m_level;
    Dimension* m_dimension;
    std::deque<LevelChunk*> m_toDrop;
    LevelChunk** m_unloadedCache;
#ifdef MINECRAFT_PC
    void pcTrimUnloaded();
    std::deque<int> m_pcUnloadedOrder;
    int m_pcUnloadedCount;
#endif
    OSFastMutex m_csLoadCreate;
    int m_xzSize;
    int m_xzOffset;
    bool (*m_boundFunction)(void*, int, int);
    void* m_boundData;
};

#endif
