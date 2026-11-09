#include "game/world/chunk/ClientChunkCache.hpp"

#include <algorithm>
#include <climits>
#include <cstring>
#include "game/client/render/GameRenderer.hpp"
#include "game/core/Compression.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/System.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ProgressListener.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/ChunkStorage.hpp"
#include "game/world/chunk/EmptyLevelChunk.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/world/chunk/WaterLevelChunk.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

extern long InterlockedCompareExchangeRelease(volatile long* destination, long exchange, long comparand);
#ifdef MINECRAFT_PC
extern void* InterlockedCompareExchangePointerRelease(void* volatile* destination, void* exchange, void* comparand);
#endif

static const int SAVE_THREAD_STACK_SIZE = 0x100000;
static const int MAX_DROPS_PER_TICK = 100;
static const long long CHUNK_TICK_BUDGET_MS = 5;
static const int MAX_LOCAL_PLAYERS = 4;

ClientChunkCache::QueuedAction::QueuedAction(const boost::shared_ptr<ClientboundBlockEntityDataPacket>& packet)
    : packet(),
      pos(packet->getPos().x >> 4, packet->getPos().y >> 4)
{
    type = eType_BlockEntityData;
    changes = NULL;
    this->packet = packet;
}

ClientChunkCache::QueuedAction::QueuedAction(int x, int z, const std::vector<LevelChunk::BlockChange>* changes)
    : packet(),
      pos(x, z)
{
    this->changes = changes;
    type = eType_ReLoad;
}

ClientChunkCache::QueuedAction& ClientChunkCache::QueuedAction::operator=(const QueuedAction& other)
{
    type = other.type;
    packet = other.packet;
    pos = other.pos;
    changes = other.changes;
    return *this;
}

ClientChunkCache::ClientChunkCache(Level* level, ChunkStorage* storage) : m_reloadPos(0, 0), m_reloadChanges(NULL)
{
    MultiPlayerChunkCacheInit(level, storage);
}

ClientChunkCache::ClientChunkCache(Level* level) : m_reloadPos(0, 0), m_reloadChanges(NULL)
{
    MultiPlayerChunkCacheInit(level, NULL);
}

ClientChunkCache::~ClientChunkCache()
{
    delete m_emptyChunk;
    delete m_waterChunk;

    for (int i = 0; i < m_xzSize * m_xzSize; ++i)
    {
        if (m_cache[i] != NULL)
        {
            delete m_cache[i];
            m_cache[i] = NULL;
        }
    }
    delete m_cache;
    delete m_hasData;
    DeleteCriticalSection(&m_csLoadCreate);

    if (m_storage != NULL)
    {
        m_stopThread = true;
        m_saveThread->WaitForCompletion(-1);
        delete m_saveThread;
        DeleteCriticalSection(&m_csSaveThread);
        delete m_storage;
    }
}

#ifdef MINECRAFT_PC
static const long long PC_RELOAD_BUDGET_MS = 8;
#endif

LevelChunk* ClientChunkCache::load(int x, int z, LevelChunk* chunk)
{
    if (m_storage == NULL)
    {
        return NULL;
    }
    chunk = m_storage->load(m_level, x, z, chunk);
    if (chunk != NULL)
    {
        chunk->setLastSaveTime(m_level->getGameTime());
    }
    return chunk;
}

void ClientChunkCache::reLoad(int x, int z, const std::vector<LevelChunk::BlockChange>* changes)
{
    if (!m_dimension->containsChunk(x, z))
    {
        return;
    }

    int idx = (x + m_xzOffset) * m_xzSize + (z + m_xzOffset);
    LevelChunk* previous = m_cache[idx];
    EnterCriticalSection(&m_csLoadCreate);
    LevelChunk* chunk = load(x, z, NULL);
    if (chunk != NULL)
    {
#ifdef MINECRAFT_PC
        if (InterlockedCompareExchangePointerRelease(reinterpret_cast<void* volatile*>(&m_cache[idx]), chunk, previous)
            == previous)
#else
        if (InterlockedCompareExchangeRelease(reinterpret_cast<volatile long*>(&m_cache[idx]),
                                              reinterpret_cast<long>(chunk), reinterpret_cast<long>(previous))
            == reinterpret_cast<long>(previous))
#endif
        {
            if (previous != NULL)
            {
                previous->unload(true, false);
                chunk->swapEntities(previous);
                std::vector<LevelChunk*>::iterator it = std::find(m_loadedChunkList.begin(), m_loadedChunkList.end(),
                                                                  previous);
                if (it != m_loadedChunkList.end())
                {
                    m_loadedChunkList.erase(it);
                }
                GameRenderer::AddForDelete(previous);
                GameRenderer::FinishedReassigning();
            }
            chunk->load(true);
            chunk->recalcHeightmapOnly();
            m_level->setBlocksDirty(BlockPos(x * 16, 0, z * 16), BlockPos(x * 16 + 15, 255, z * 16 + 15));
            m_hasData[idx] = true;
            for (std::vector<LevelChunk::BlockChange>::const_iterator it = changes->begin(); it != changes->end(); ++it)
            {
                chunk->setBlock(it->GetPos(), it->GetState());
                chunk->setExtraBlock(it->GetPos(), it->GetExtraState());
            }
            m_loadedChunkList.push_back(chunk);
            LeaveCriticalSection(&m_csLoadCreate);
            return;
        }
        delete chunk;
    }
    LeaveCriticalSection(&m_csLoadCreate);
}

int ClientChunkCache::StorageThreadProc(void* argument)
{
    ClientChunkCache* cache = static_cast<ClientChunkCache*>(argument);
    Compression::UseDefaultThreadStorage();
    OldChunkStorage::UseDefaultThreadStorage();
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_ClientChunkStorage);

    while (!cache->m_stopThread)
    {
        EnterCriticalSection(&cache->m_csSaveThread);
        if (cache->m_reloadPending)
        {
            int x = cache->m_reloadPos.x;
            int z = cache->m_reloadPos.z;
            LeaveCriticalSection(&cache->m_csSaveThread);
            cache->reLoad(x, z, cache->m_reloadChanges);
            delete cache->m_reloadChanges;
            cache->m_reloadChanges = NULL;
            cache->m_reloadPending = false;
        }
        else
        {
            LeaveCriticalSection(&cache->m_csSaveThread);
            Sleep(1);
        }
    }

    AABB::ReleaseThreadStorage();
    Vec3::ReleaseThreadStorage();
    ElementStorage::ReleaseThreadStorage();
    return 0;
}

void ClientChunkCache::MultiPlayerChunkCacheInit(Level* level, ChunkStorage* storage)
{
    m_xzSize = level->mDimension->getXZSize();
    mLevelSize = m_xzSize;
    m_xzOffset = m_xzSize / 2;
    m_hasData = new bool[m_xzSize * m_xzSize];
    std::memset(m_hasData, 0, m_xzSize * m_xzSize);
    m_emptyChunk = new EmptyLevelChunk(level, 0, 0);
    m_reloadPending = false;
    m_stopThread = false;
    m_storage = storage;

    if (storage != NULL)
    {
        InitializeCriticalSection(&m_csSaveThread);
        m_saveThread = new C4JThreadImpl(&StorageThreadProc, this, "MultiPlayerChunkCache", SAVE_THREAD_STACK_SIZE);
        m_saveThread->SetProcessor(0);
        m_saveThread->Run();
    }
    else
    {
        m_saveThread = NULL;
    }

    if (level->mDimension->getType() == DimensionType::OVERWORLD)
    {
        static const int CHUNK_WIDTH = 16;
        static const int CHUNK_HEIGHT = 128;
        static const int FLAT_GRASS_Y = 3;
        arrayWithLength<unsigned short> elementIds(CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT, true);

        if (level->getLevelData()->getGeneratorType() == LevelType::FLAT)
        {
            for (int x = 0; x < CHUNK_WIDTH; ++x)
                for (int y = 0; y < CHUNK_HEIGHT; ++y)
                    for (int z = 0; z < CHUNK_WIDTH; ++z)
                    {
                        unsigned short element = 0;
                        if (y == FLAT_GRASS_Y)
                            element = Block::getElementId(Blocks::grass);
                        else if (y <= FLAT_GRASS_Y - 1)
                            element = Block::getElementId(Blocks::dirt);
                        elementIds.data[x << 11 | z << 7 | y] = element;
                    }
        }
        else
        {
            for (int x = 0; x < CHUNK_WIDTH; ++x)
                for (int y = 0; y < CHUNK_HEIGHT; ++y)
                    for (int z = 0; z < CHUNK_WIDTH; ++z)
                    {
                        unsigned short element = 0;
                        if (y <= level->getSeaLevel() - 10)
                            element = Block::getElementId(Blocks::stone);
                        else if (y < level->getSeaLevel())
                            element = Block::getElementId(Blocks::water);
                        elementIds.data[x << 11 | z << 7 | y] = element;
                    }
        }

        ChunkPrimer primer(false, elementIds);
        m_waterChunk = new WaterLevelChunk(level, &primer, 0, 0);
        delete[] elementIds.data;
        WaterLevelChunk* waterChunk = static_cast<WaterLevelChunk*>(m_waterChunk);

        if (level->getLevelData()->getGeneratorType() == LevelType::FLAT)
        {
            for (int x = 0; x < CHUNK_WIDTH; ++x)
                for (int y = 0; y < CHUNK_HEIGHT; ++y)
                    for (int z = 0; z < CHUNK_WIDTH; ++z)
                    {
                        if (y >= FLAT_GRASS_Y)
                            waterChunk->setLevelChunkBrightness(LightLayer::SKY, x, y, z, 15);
                    }
        }
        else
        {
            for (int x = 0; x < CHUNK_WIDTH; ++x)
                for (int y = 0; y < CHUNK_HEIGHT; ++y)
                    for (int z = 0; z < CHUNK_WIDTH; ++z)
                    {
                        int brightness = 2;
                        if (y >= level->getSeaLevel() - 1)
                            brightness = 15;
                        waterChunk->setLevelChunkBrightness(LightLayer::SKY, x, y, z, brightness);
                    }
        }
    }
    else
    {
        m_waterChunk = NULL;
    }

    m_dimension = level->mDimension;
    m_level = level;
    m_cache = new LevelChunk*[m_xzSize * m_xzSize];
    std::memset(m_cache, 0, m_xzSize * m_xzSize * sizeof(LevelChunk*));
    InitializeCriticalSectionAndSpinCount(&m_csLoadCreate, 4000);
}

void ClientChunkCache::BlockUntilSafeToChangeSaveData(bool processQueue)
{
    if (m_storage == NULL)
    {
        return;
    }

    bool reloadPending;
    do
    {
        EnterCriticalSection(&m_csSaveThread);
        reloadPending = m_reloadPending;
        LeaveCriticalSection(&m_csSaveThread);
    } while (reloadPending);

    if (processQueue)
    {
        while (!m_queuedActions.empty())
        {
            QueuedAction& action = m_queuedActions.front();
            if (action.type == QueuedAction::eType_ReLoad)
            {
                reLoad(action.pos.x, action.pos.z, action.changes);
                delete action.changes;
            }
            else
            {
                ClientPacketListener::handleBlockEntityDataInternal(action.packet, m_level);
            }
            m_queuedActions.erase(m_queuedActions.begin());
        }
    }
}

LevelChunk* ClientChunkCache::getChunkIfLoaded(int x, int z)
{
    if (!m_dimension->containsChunk(x, z))
    {
        return m_waterChunk != NULL ? m_waterChunk : m_emptyChunk;
    }
    int ix = x + m_xzOffset;
    int iz = z + m_xzOffset;
    int idx = ix * m_xzSize + iz;
    LevelChunk* chunk = m_cache[idx];
    return chunk;
}

LevelChunk* ClientChunkCache::getOrCreateChunk(int x, int z, bool)
{
    LevelChunk* chunk = getChunkIfLoaded(x, z);
    if (chunk == NULL)
    {
        chunk = m_emptyChunk;
    }
    return chunk;
}

int ClientChunkCache::getLoadedChunksCount()
{
    return 0;
}

bool ClientChunkCache::isChunkGeneratedAt(int, int)
{
    return false;
}

LevelChunk* ClientChunkCache::getChunkIfHasChunk(int x, int z)
{
    return getChunk(x, z);
}

bool ClientChunkCache::hasChunk(int, int)
{
    return true;
}

#ifdef MINECRAFT_PC
bool ClientChunkCache::pcHoldsChunk(int x, int z)
{
    const int ix = x + m_xzOffset;
    const int iz = z + m_xzOffset;
    if (m_cache == NULL || ix < 0 || iz < 0 || ix >= m_xzSize || iz >= m_xzSize)
        return false;
    return m_cache[ix * m_xzSize + iz] != NULL;
}

void ClientChunkCache::pcLockCreate()
{
    EnterCriticalSection(&m_csLoadCreate);
}

void ClientChunkCache::pcUnlockCreate()
{
    LeaveCriticalSection(&m_csLoadCreate);
}
#endif

bool ClientChunkCache::reallyHasChunk(int x, int z)
{
    if (!m_dimension->containsChunk(x, z))
    {
        return true;
    }
    int ix = x + m_xzOffset;
    int iz = z + m_xzOffset;
    int idx = ix * m_xzSize + iz;
    LevelChunk* chunk = m_cache[idx];
    if (chunk == NULL)
    {
        return false;
    }
    return m_hasData[idx];
}

void ClientChunkCache::unqueueForReLoad(int x, int z)
{
    EnterCriticalSection(&m_csLoadCreate);
    for (int i = 0; i < m_queuedActions.size(); ++i)
    {
        QueuedAction action = m_queuedActions[i];
        if (action.type == QueuedAction::eType_ReLoad && action.pos.x == x && action.pos.z == z)
        {
            m_queuedActions.erase(m_queuedActions.begin() + i);
            break;
        }
    }
    LeaveCriticalSection(&m_csLoadCreate);
}

void ClientChunkCache::drop(int x, int z, bool force)
{
    LevelChunk* chunk = getOrCreateChunk(x, z, false);
    if (!chunk->isEmpty())
    {
        chunk->unload(force, true);
        if (CGameNetworkManager::sInstance.IsHost())
        {
            chunk->dropLighting();
        }
        chunk->mLoaded = true;
    }

    if (force)
    {
        if (!m_dimension->containsChunk(x, z))
        {
            return;
        }
        unqueueForReLoad(x, z);
        int ix = x + m_xzOffset;
        int iz = z + m_xzOffset;
        int idx = ix * m_xzSize + iz;
        EnterCriticalSection(&m_csLoadCreate);
#ifdef MINECRAFT_PC
        if (InterlockedCompareExchangePointerRelease(reinterpret_cast<void* volatile*>(&m_cache[idx]), NULL, chunk)
            == chunk)
#else
        if (InterlockedCompareExchangeRelease(reinterpret_cast<volatile long*>(&m_cache[idx]), 0,
                                              reinterpret_cast<long>(chunk))
            == reinterpret_cast<long>(chunk))
#endif
        {
            std::vector<LevelChunk*>::iterator it = std::find(m_loadedChunkList.begin(), m_loadedChunkList.end(),
                                                              chunk);
            if (it != m_loadedChunkList.end())
            {
                m_loadedChunkList.erase(it);
            }
            GameRenderer::AddForDelete(chunk);
            GameRenderer::FinishedReassigning();
            LeaveCriticalSection(&m_csLoadCreate);
            m_level->setBlocksDirty(BlockPos(x * 16, 0, z * 16), BlockPos(x * 16 + 15, 255, z * 16 + 15));
            m_hasData[idx] = false;
            return;
        }
        LeaveCriticalSection(&m_csLoadCreate);
    }
    else if (chunk != NULL)
    {
        m_toDrop.push_back(chunk);
    }
}

void ClientChunkCache::dropOutsideArea(int minX, int minZ, int maxX, int maxZ, bool force)
{
    for (int x = -m_xzSize / 2; x < m_xzSize / 2; ++x)
    {
        for (int z = -m_xzSize / 2; z < m_xzSize / 2; ++z)
        {
            if (x < minX || x > maxX || z < minZ || z > maxZ)
            {
                drop(x, z, force);
            }
        }
    }
}

LevelChunk* ClientChunkCache::create(int x, int z)
{
    if (!m_dimension->containsChunk(x, z))
    {
        return m_waterChunk != NULL ? m_waterChunk : m_emptyChunk;
    }

    int ix = x + m_xzOffset;
    int iz = z + m_xzOffset;
    int idx = ix * m_xzSize + iz;
    LevelChunk* chunk = m_cache[idx];
    LevelChunk* previous = chunk;
    if (chunk == NULL)
    {
        EnterCriticalSection(&m_csLoadCreate);
        if (CGameNetworkManager::sInstance.IsHost())
        {
            if (MinecraftServer::s_haltRequested)
            {
                LeaveCriticalSection(&m_csLoadCreate);
                return NULL;
            }
            ServerLevel* serverLevel = gMinecraftServer->getLevel(m_level->mDimension->getType()->getId());
            LevelChunk* serverChunk = serverLevel->getChunkSource()->getChunkLoadedOrUnloaded(x, z);
            if (serverChunk->isEmpty())
            {
                LeaveCriticalSection(&m_csLoadCreate);
                return m_emptyChunk;
            }
            chunk = new LevelChunk(m_level, x, z, serverChunk);
            m_level->setBlocksDirty(BlockPos(x * 16, 0, z * 16), BlockPos(x * 16 + 15, 255, z * 16 + 15));
            m_hasData[idx] = true;
        }
        else
        {
            ChunkPrimer* primer = new ChunkPrimer(true, arrayWithLength<unsigned short>());
            chunk = new LevelChunk(m_level, primer, x, z);
            delete primer;
            chunk->setSkyLightDataAllBright();
        }
        chunk->mLoaded = true;
#ifdef MINECRAFT_PC
        if (InterlockedCompareExchangePointerRelease(reinterpret_cast<void* volatile*>(&m_cache[idx]), chunk, previous)
            == previous)
#else
        if (InterlockedCompareExchangeRelease(reinterpret_cast<volatile long*>(&m_cache[idx]),
                                              reinterpret_cast<long>(chunk), reinterpret_cast<long>(previous))
            == reinterpret_cast<long>(previous))
#endif
        {
            m_loadedChunkList.push_back(chunk);
            LeaveCriticalSection(&m_csLoadCreate);
        }
        else
        {
            LeaveCriticalSection(&m_csLoadCreate);
            delete chunk;
            return m_cache[idx];
        }
    }
    else if (!chunk->mLoaded || chunk->mUnloaded)
    {
        chunk->load(false);
    }

    if (CGameNetworkManager::sInstance.IsHost() && !chunk->isEmpty())
    {
        chunk->recalcHeightmapOnly();
        if (chunk->hasLightingBeenDropped())
        {
            chunk->reSyncLighting();
        }
    }
    return chunk;
}

LevelChunk* ClientChunkCache::getChunk(int x, int z)
{
    if (!m_dimension->containsChunk(x, z))
    {
        return m_waterChunk != NULL ? m_waterChunk : m_emptyChunk;
    }
    int ix = x + m_xzOffset;
    int iz = z + m_xzOffset;
    int idx = ix * m_xzSize + iz;
    LevelChunk* chunk = m_cache[idx];
    if (chunk == NULL)
    {
        return m_emptyChunk;
    }
    else
    {
        return chunk;
    }
}

LevelChunk* ClientChunkCache::getChunkAt(const BlockPos& pos)
{
    return getChunk(pos.x, pos.z);
}

bool ClientChunkCache::save(bool, ProgressListener*)
{
    return true;
}

bool ClientChunkCache::tick()
{
    EnterCriticalSection(&m_csLoadCreate);

#ifdef MINECRAFT_PC
    const long long reloadStart = System::processTimeInMilliSecs();
#endif
    if (m_storage != NULL && !Minecraft::GetInstance()->GetClientMasterGameMode()->isNewLevelDataPending())
    {
        while (!m_queuedActions.empty())
        {
            int closestDistance = INT_MAX;
            unsigned int closestIndex = 0;
            QueuedAction* closestAction = &m_queuedActions.front();
            for (unsigned int i = 0; i < m_queuedActions.size(); ++i)
            {
                QueuedAction& action = m_queuedActions[i];
                int actionDistance = INT_MAX;
                for (int playerIdx = 0; playerIdx < MAX_LOCAL_PLAYERS; ++playerIdx)
                {
                    boost::shared_ptr<MultiplayerLocalPlayer> player =
                        Minecraft::GetInstance()->mLocalPlayers[playerIdx];
                    if (player != NULL)
                    {
                        int distance = Mth::abs(Mth::floor(player->mX) - action.pos.x * 16)
                                       + Mth::abs(Mth::floor(player->mZ) - action.pos.z * 16);
                        if (distance < actionDistance)
                        {
                            actionDistance = distance;
                        }
                    }
                }
                if (actionDistance < closestDistance)
                {
                    closestDistance = actionDistance;
                    closestIndex = i;
                    closestAction = &action;
                }
            }

            if (closestAction->type == QueuedAction::eType_ReLoad)
            {
                EnterCriticalSection(&m_csSaveThread);
                if (m_reloadPending)
                {
                    LeaveCriticalSection(&m_csSaveThread);
                }
#ifdef MINECRAFT_PC
                else if (System::processTimeInMilliSecs() - reloadStart < PC_RELOAD_BUDGET_MS)
                {
                    LeaveCriticalSection(&m_csSaveThread);
                    const int x = closestAction->pos.x;
                    const int z = closestAction->pos.z;
                    const std::vector<LevelChunk::BlockChange>* changes = closestAction->changes;
                    m_queuedActions.erase(m_queuedActions.begin() + closestIndex);
                    reLoad(x, z, changes);
                    delete changes;
                    continue;
                }
#endif
                else
                {
                    m_reloadPending = true;
                    m_reloadPos.x = closestAction->pos.x;
                    m_reloadPos.z = closestAction->pos.z;
                    m_reloadChanges = closestAction->changes;
                    LeaveCriticalSection(&m_csSaveThread);
                    m_queuedActions.erase(m_queuedActions.begin() + closestIndex);
                }
                break;
            }

            ClientPacketListener::handleBlockEntityDataInternal(closestAction->packet, m_level);
            m_queuedActions.erase(m_queuedActions.begin() + closestIndex);
        }
    }

    for (int dropped = 0; dropped < MAX_DROPS_PER_TICK && !m_toDrop.empty(); ++dropped)
    {
        LevelChunk* chunk = m_toDrop.front();
        m_toDrop.pop_front();
        std::vector<LevelChunk*>::iterator it = std::find(m_loadedChunkList.begin(), m_loadedChunkList.end(), chunk);
        if (it != m_loadedChunkList.end())
        {
            m_loadedChunkList.erase(it);
        }
    }

    long long start = System::processTimeInMilliSecs();
    std::vector<LevelChunk*>::iterator it = m_loadedChunkList.begin();
    std::vector<LevelChunk*>::iterator end = m_loadedChunkList.end();
    for (; System::processTimeInMilliSecs() - start < CHUNK_TICK_BUDGET_MS && it != end; ++it)
    {
        (*it)->tick(false);
    }
    for (; it != end; ++it)
    {
        (*it)->tick(true);
    }

    LeaveCriticalSection(&m_csLoadCreate);
    return false;
}

bool ClientChunkCache::shouldSave()
{
    return false;
}

std::wstring ClientChunkCache::gatherStats()
{
    EnterCriticalSection(&m_csLoadCreate);
    int loadedCount = m_loadedChunkList.size();
    LeaveCriticalSection(&m_csLoadCreate);
    return L"MultiplayerChunkCache: " + _toString<int>(loadedCount);
}

std::vector<Biome::MobSpawnerData*>* ClientChunkCache::getMobsAt(MobCategory*, const BlockPos&)
{
    return NULL;
}

BlockPos* ClientChunkCache::findNearestMapFeature(Level*, const std::wstring&, const BlockPos&, bool)
{
    return NULL;
}

int ClientChunkCache::getLoadedChunks()
{
    return m_loadedChunkList.size();
}

void ClientChunkCache::recreateLogicStructuresForChunk(LevelChunk*, int, int)
{
}

void ClientChunkCache::dataReceived(int x, int z)
{
    if (m_dimension->containsChunk(x, z))
    {
        int ix = x + m_xzOffset;
        int iz = z + m_xzOffset;
        int idx = ix * m_xzSize + iz;
        m_hasData[idx] = true;
    }
}

void ClientChunkCache::queueBlockEntityDataPacket(const boost::shared_ptr<ClientboundBlockEntityDataPacket>& packet)
{
    EnterCriticalSection(&m_csLoadCreate);
    m_queuedActions.push_back(QueuedAction(packet));
    LeaveCriticalSection(&m_csLoadCreate);
}

void ClientChunkCache::queueForReLoad(int x, int z, const std::vector<LevelChunk::BlockChange>* changes, bool replace)
{
    if (!m_dimension->containsChunk(x, z))
    {
        return;
    }

    EnterCriticalSection(&m_csLoadCreate);
    if (replace)
    {
        for (unsigned int i = 0; i < m_queuedActions.size(); ++i)
        {
            QueuedAction& action = m_queuedActions[i];
            if (action.type == QueuedAction::eType_ReLoad && action.pos.x == x && action.pos.z == z)
            {
                const std::vector<LevelChunk::BlockChange>* previous = action.changes;
                action.changes = changes;
                delete previous;
                LeaveCriticalSection(&m_csLoadCreate);
                return;
            }
        }
    }
    m_queuedActions.push_back(QueuedAction(x, z, changes));
    LeaveCriticalSection(&m_csLoadCreate);
}

bool ClientChunkCache::mustQueueBlockEntityDataPackets()
{
    return !m_queuedActions.empty();
}

LevelChunk** ClientChunkCache::getCache()
{
    return m_cache;
}
