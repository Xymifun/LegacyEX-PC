#ifndef GAME_WORLD_CHUNK_CLIENTCHUNKCACHE_HPP
#define GAME_WORLD_CHUNK_CLIENTCHUNKCACHE_HPP

#include <deque>
#include <string>
#include <vector>
#include <stdint.h>
#include <boost/shared_ptr.hpp>

#include "game/core/CriticalSection.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"

class C4JThreadImpl;
class ChunkStorage;
class Dimension;
class Level;

class ClientChunkCache : public ChunkSource
{
public:
    class QueuedAction
    {
    public:
        enum EType
        {
            eType_ReLoad,
            eType_BlockEntityData
        };

        QueuedAction(const boost::shared_ptr<ClientboundBlockEntityDataPacket>& packet);
        QueuedAction(int x, int z, const std::vector<LevelChunk::BlockChange>* changes);
        QueuedAction& operator=(const QueuedAction& other);

        EType type;
        boost::shared_ptr<ClientboundBlockEntityDataPacket> packet;
        ChunkPos pos;
        const std::vector<LevelChunk::BlockChange>* changes;
    };

    ClientChunkCache(Level* level, ChunkStorage* storage);
    ClientChunkCache(Level* level);
    virtual ~ClientChunkCache();

    LevelChunk* load(int x, int z, LevelChunk* chunk);
    void reLoad(int x, int z, const std::vector<LevelChunk::BlockChange>* changes);
    static int StorageThreadProc(void* argument);
    void MultiPlayerChunkCacheInit(Level* level, ChunkStorage* storage);
    void BlockUntilSafeToChangeSaveData(bool block);

    virtual LevelChunk* getChunkIfLoaded(int x, int z);
    virtual LevelChunk* getOrCreateChunk(int x, int z, bool allowEmpty);
    virtual int getLoadedChunksCount();
    virtual bool isChunkGeneratedAt(int x, int z);
    virtual LevelChunk* getChunkIfHasChunk(int x, int z);
    virtual bool hasChunk(int x, int z);
    virtual bool reallyHasChunk(int x, int z);
#ifdef MINECRAFT_PC
    bool pcHoldsChunk(int x, int z);
    void pcLockCreate();
    void pcUnlockCreate();
#endif
    void unqueueForReLoad(int x, int z);
    void drop(int x, int z, bool force);
    void dropOutsideArea(int minX, int minZ, int maxX, int maxZ, bool force);
    virtual LevelChunk* create(int x, int z);
    virtual LevelChunk* getChunk(int x, int z);
    virtual LevelChunk* getChunkAt(const BlockPos& pos);
    virtual bool save(bool force, ProgressListener* listener);
    virtual bool tick();
    virtual bool shouldSave();
    virtual std::wstring gatherStats();
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                            bool takeBlock);
    virtual int getLoadedChunks();
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int x, int z);
    virtual void dataReceived(int x, int z);
    void queueBlockEntityDataPacket(const boost::shared_ptr<ClientboundBlockEntityDataPacket>& packet);
    void queueForReLoad(int x, int z, const std::vector<LevelChunk::BlockChange>* changes, bool replace);
    bool mustQueueBlockEntityDataPackets();
    virtual LevelChunk** getCache();

private:
    LevelChunk* m_emptyChunk;
    LevelChunk* m_waterChunk;
    ChunkStorage* m_storage;
    C4JThreadImpl* m_saveThread;
    ChunkPos m_reloadPos;
    const std::vector<LevelChunk::BlockChange>* m_reloadChanges;
    bool m_reloadPending;
    bool m_stopThread;
    OSFastMutex m_csSaveThread;
    std::vector<LevelChunk*> m_loadedChunkList;
    std::vector<QueuedAction> m_queuedActions;
    LevelChunk** m_cache;
    std::deque<LevelChunk*> m_toDrop;
    OSFastMutex m_csLoadCreate;
    int m_xzSize;
    int m_xzOffset;
    bool* m_hasData;
    Level* m_level;
    Dimension* m_dimension;
};

#endif
