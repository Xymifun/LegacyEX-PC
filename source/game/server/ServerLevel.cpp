#include "game/server/ServerLevel.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/BoundingBox.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/core/LootTables.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/PortalForcer.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/core/System.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/data/SavedDataStorage.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/NaturalMobSpawner.hpp"
#include "game/entity/mob/Npc.hpp"
#include "game/entity/mob/SkeletonHorse.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/ClientboundAddGlobalEntityPacket.hpp"
#include "game/network/ClientboundBlockEventPacket.hpp"
#include "game/network/ClientboundEntityEventPacket.hpp"
#include "game/network/ClientboundExplodePacket.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ClientboundLevelParticlesPacket.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/LevelListener.hpp"
#include "game/network/ProgressListener.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/ServerLevelListener.hpp"
#include "game/server/DerivedServerLevel.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/ServerScoreboard.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/WeighedRandom.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelSettings.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEventData.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/MinigamePlayerChunkMap.hpp"
#include "game/world/chunk/PlayerChunkMap.hpp"
#include "game/world/generation/BonusChestFeature.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/VillageSiege.hpp"
#include "game/world/generation/Villages.hpp"
#include "platform/wiiu/C4JEventArrayImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CStorage.hpp"

#include <algorithm>
#include <typeinfo>

namespace
{
C4JEventArrayImpl* sBlockUpdateEvents = 0;
OSFastMutex sBlockUpdateMutex[3];
C4JThreadImpl* sBlockUpdateThread = 0;
ServerLevel* sBlockUpdateLevels[3] = {0, 0, 0};
int sBlockUpdateChunkCounts[3] = {0, 0, 0};
int sBlockUpdateChunkX[3][0xB48];
int sBlockUpdateChunkZ[3][0xB48];
int sBlockUpdateTickCounts[3] = {0, 0, 0};
int sBlockUpdateRandom[3] = {0, 0, 0};
int sBlockUpdateTickX[3][256];
int sBlockUpdateTickY[3][256];
int sBlockUpdateTickZ[3][256];

inline PlayerList* serverPlayers()
{
    return gMinecraftServer != NULL ? gMinecraftServer->mPlayers : NULL;
}
}

bool canSeeSkyFunction(boost::shared_ptr<Entity> entity)
{
    if (!entity || !entity->isAlive())
        return false;
    return entity->mLevel->canSeeSky(entity->getCommandSenderBlockPosition());
}

ServerLevel::ServerLevel(MinecraftServer* server, boost::shared_ptr<LevelStorage> levelStorage, LevelData* levelData,
                         int dimension)
    : Level(levelStorage, levelData, DimensionType::getById(dimension)->create(), false),
      tickNextTickSet(11),
      entitiesByUuid(11)
{
    InitializeCriticalSection(&entityLimitMutex);
    InitializeCriticalSection(&blockEntityLimitMutex);
    InitializeCriticalSection(&tickNextTickMutex);
    InitializeCriticalSection(&queuedSendBlockUpdateMutex);
    bannerCount = 0;
    primedTntCount = 0;
    fallingBlockCount = 0;
    fireworksRocketCount = 0;
    beaconCount = 0;
    enderCrystalExplosionCount = 0;
    mDimension->init(this);
    somePlayerSleeping = false;
    saveInterval = 40;
    emptyTime = 0;
    unusedFlag = true;
    allPlayersSleeping = false;
    serverScoreboard = NULL;
    activeBlockEventList = 0;
    noSave = false;
    if (gApp.GetResetEnd())
        mLevelData->setModernEnd();
    mChunkSource = createChunkSource();
    mChunkCache = mChunkSource->getCache();
    mChunkCacheSize = mChunkSource->mLevelSize;
    this->server = server;
    server->setLevel(dimension, this);
    serverLevelListener = new ServerLevelListener(server, this);
    addListener(serverLevelListener);
    tracker = new EntityTracker(this);
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        chunkMap = new MinigamePlayerChunkMap(this, dimension, server->getPlayers()->getViewDistance());
    else
        chunkMap = new PlayerChunkMap(this, dimension, server->getPlayers()->getViewDistance());
    mobSpawner = new NaturalMobSpawner(mChunkCacheSize, this);
    portalForcer = new PortalForcer(this);
    villageSiege = new VillageSiege(this);
    updateSkyBrightness();
    prepareWeather();
    primedTntCheckDelay = 0;
}

ServerLevel::~ServerLevel()
{
    delete portalForcer;
    delete mobSpawner;
    delete villageSiege;
    EnterCriticalSection(&queuedSendBlockUpdateMutex);
    for (std::vector<QueuedSendBlockData*>::iterator it = queuedSendBlockUpdates.begin();
         it != queuedSendBlockUpdates.end(); ++it)
        delete *it;
    queuedSendBlockUpdates.clear();
    delete serverLevelListener;
    delete tracker;
    delete chunkMap;
    LeaveCriticalSection(&queuedSendBlockUpdateMutex);
    DeleteCriticalSection(&queuedSendBlockUpdateMutex);
    DeleteCriticalSection(&blockEntityLimitMutex);
    DeleteCriticalSection(&entityLimitMutex);
    DeleteCriticalSection(&tickNextTickMutex);
    for (int dimension = 0; dimension < 3; ++dimension)
    {
        EnterCriticalSection(&sBlockUpdateMutex[dimension]);
        LeaveCriticalSection(&sBlockUpdateMutex[dimension]);
    }
    sBlockUpdateEvents->ClearAll();
    delete serverScoreboard;
}

bool ServerLevel::AllPlayersAreSleeping()
{
    return allPlayersSleeping;
}

EntityTracker* ServerLevel::getTracker()
{
    return tracker;
}

ChunkSource* ServerLevel::getChunkSource()
{
    return Level::getChunkSource();
}

std::vector<boost::shared_ptr<BlockEntity> >* ServerLevel::getBlockEntitiesInRegion(int x0, int y0, int z0, int x1,
                                                                                    int y1, int z1)
{
    std::vector<boost::shared_ptr<BlockEntity> >* result = new std::vector<boost::shared_ptr<BlockEntity> >();
    for (unsigned int i = 0; i < mBlockEntities.size(); ++i)
    {
        boost::shared_ptr<BlockEntity> blockEntity = mBlockEntities[i];
        BlockPos pos = blockEntity->getBlockPos();
        if (pos.x >= x0 && pos.y >= y0 && pos.z >= z0 && pos.x < x1 && pos.y < y1 && pos.z < z1)
            result->push_back(blockEntity);
    }
    return result;
}

PlayerChunkMap* ServerLevel::getChunkMap()
{
    return chunkMap;
}

BlockPos* ServerLevel::getDimensionSpecificSpawn()
{
    return mDimension->getSpawnPos();
}

PortalForcer* ServerLevel::getPortalForcer()
{
    return portalForcer;
}

int ServerLevel::runUpdate(void* param)
{
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_ServerLevel);
    ShutdownManager::HasStarted(ShutdownManager::eThreadId_ServerLevelUpdate, sBlockUpdateEvents);
    while (ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ServerLevelUpdate))
    {
        int grassCount = 0;
        int lavaCount = 0;
        for (int dimension = 0; dimension < 3; ++dimension)
        {
            sBlockUpdateEvents->WaitForSingle(dimension, -1);
            PIXBeginNamedEvent(0.0f, "Updating blocks to be ticked %d", dimension);
            if (!ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ServerLevelUpdate))
                break;
            const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
            if (Minecraft::InMiniGame(eMiniGameId_Any, true) && !miniGame->shouldTickBlocks())
                continue;
            EnterCriticalSection(&sBlockUpdateMutex[dimension]);
            ServerLevel* level = sBlockUpdateLevels[dimension];
            for (int index = 0; index < sBlockUpdateChunkCounts[dimension]; ++index)
            {
                int chunkX = sBlockUpdateChunkX[dimension][index];
                int chunkZ = sBlockUpdateChunkZ[dimension][index];
                if (!level->hasChunk(chunkX, chunkZ, true))
                    continue;
                int minX = 0;
                int maxX = 15;
                int minZ = 0;
                int maxZ = 15;
                if (!level->hasChunk(chunkX + 1, chunkZ, true))
                    maxX = 11;
                if (!level->hasChunk(chunkX, chunkZ + 1, true))
                    maxZ = 11;
                if (!level->hasChunk(chunkX - 1, chunkZ, true))
                    minX = 4;
                if (!level->hasChunk(chunkX, chunkZ - 1, true))
                    minZ = 4;
                if (!level->hasChunk(chunkX + 1, chunkZ + 1, true))
                {
                    maxX = 11;
                    maxZ = 11;
                }
                if (!level->hasChunk(chunkX + 1, chunkZ - 1, true))
                {
                    maxX = 11;
                    minZ = 4;
                }
                if (!level->hasChunk(chunkX - 1, chunkZ - 1, true))
                {
                    minX = 4;
                    minZ = 4;
                }
                if (!level->hasChunk(chunkX - 1, chunkZ + 1, true))
                {
                    minX = 4;
                    maxZ = 11;
                }
                LevelChunk* chunk = level->getChunk(chunkX, chunkZ);
                chunk->getElementReadLock();
                for (int sample = 0; sample < 80; ++sample)
                {
                    int random = sBlockUpdateRandom[dimension] * 3 + level->mAddend;
                    sBlockUpdateRandom[dimension] = random;
                    int packed = random >> 2;
                    int x = packed & 15;
                    int z = (packed >> 8) & 15;
                    if (x < minX || x > maxX || z < minZ || z > maxZ)
                        continue;
                    int y = (packed >> 16) & 255;
                    int id;
                    int extraId;
                    chunk->getBlockAndExtraId_NoLock(x, y, z, &id, &extraId);
                    if (sBlockUpdateTickCounts[dimension] >= 256)
                        break;
                    if ((id == 2 && grassCount >= 100) || (id == 11 && lavaCount >= 100))
                        continue;
                    int queued = sBlockUpdateTickCounts[dimension];
                    if (id != 0)
                    {
                        Block* block = Block::byId(id);
                        if (block != 0 && block->isTicking()
                            && (!Minecraft::InMiniGame(eMiniGameId_Any, true) || miniGame->shouldTickBlock(id)))
                        {
                            const BlockState* state = block->getBlockState(chunk->getData(x, y, z));
                            int worldX = x + chunkX * 16;
                            int worldZ = z + chunkZ * 16;
                            if (block->shouldBlockTick(level, BlockPos(worldX, y, worldZ), state))
                            {
                                if (block == Blocks::grass)
                                    ++grassCount;
                                else if (block == Blocks::lava)
                                    ++lavaCount;
                                int index = sBlockUpdateTickCounts[dimension]++;
                                sBlockUpdateTickX[dimension][index] = worldX;
                                sBlockUpdateTickY[dimension][index] = y;
                                sBlockUpdateTickZ[dimension][index] = worldZ;
                            }
                        }
                    }
                    if (queued < sBlockUpdateTickCounts[dimension])
                        continue;
                    if ((extraId == 11 && lavaCount >= 100) || extraId == 0)
                        continue;
                    Block* extraBlock = Block::byId(extraId);
                    if (extraBlock != 0 && extraBlock->isTicking()
                        && (!Minecraft::InMiniGame(eMiniGameId_Any, true) || miniGame->shouldTickBlock(extraId)))
                    {
                        const BlockState* state = extraBlock->getBlockState(chunk->getExtraBlockData(x, y, z));
                        int worldX = x + chunkX * 16;
                        int worldZ = z + chunkZ * 16;
                        if (extraBlock->shouldBlockTick(level, BlockPos(worldX, y, worldZ), state))
                        {
                            if (extraBlock == Blocks::grass)
                                ++grassCount;
                            else if (extraBlock == Blocks::lava)
                                ++lavaCount;
                            int index = sBlockUpdateTickCounts[dimension]++;
                            sBlockUpdateTickX[dimension][index] = worldX;
                            sBlockUpdateTickY[dimension][index] = y;
                            sBlockUpdateTickZ[dimension][index] = worldZ;
                        }
                    }
                }
                chunk->releaseElementReadLock();
            }
            LeaveCriticalSection(&sBlockUpdateMutex[dimension]);
            PIXEndNamedEvent();
        }
    }
    ShutdownManager::HasFinished(ShutdownManager::eThreadId_ServerLevelUpdate);
    return 0;
}

void ServerLevel::staticCtor()
{
    sBlockUpdateEvents = new C4JEventArrayImpl(3, C4JEventArray::Mode0);
    InitializeCriticalSection(&sBlockUpdateMutex[0]);
    InitializeCriticalSection(&sBlockUpdateMutex[1]);
    InitializeCriticalSection(&sBlockUpdateMutex[2]);
    sBlockUpdateThread = new C4JThreadImpl(&ServerLevel::runUpdate, 0, "Block update", 0);
    sBlockUpdateThread->SetProcessor(2);
    sBlockUpdateThread->Run();
}

ChunkSource* ServerLevel::createChunkSource()
{
    ChunkStorage* storage = mLevelStorage->createChunkStorage(mDimension);
    return new ServerChunkCache(this, storage, mDimension->createRandomLevelGenerator());
}

Level* ServerLevel::postConstruct()
{
    mSavedDataStorage = new SavedDataStorage(mLevelStorage.get());
    std::wstring id = Villages::getFileId(mDimension);
    boost::shared_ptr<Villages> villages = boost::static_pointer_cast<Villages>(
        mSavedDataStorage->get(typeid(Villages), id));
    if (!villages)
    {
        mVillages = boost::shared_ptr<Villages>(new Villages(this));
        mSavedDataStorage->set(id, mVillages);
    }
    else
    {
        mVillages = villages;
        mVillages->setLevel(this);
    }
    serverScoreboard = new ServerScoreboard(server);
    mLootTables = new LootTables();
    return this;
}

bool ServerLevel::allPlayersAreSleeping()
{
    if (!allPlayersSleeping || mIsClientSide)
        return false;
    std::vector<boost::shared_ptr<Player> >::iterator end = mPlayers.end();
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != end; ++it)
    {
        if (!(*it)->isSpectator() && !(*it)->isSleepingLongEnough())
            return false;
    }
    return true;
}

void ServerLevel::stopWeather()
{
    mLevelData->setRainTime(0);
    mLevelData->setRaining(false);
    mLevelData->setThunderTime(0);
    mLevelData->setThundering(false);
}

void ServerLevel::awakenAllPlayers()
{
    allPlayersSleeping = false;
    somePlayerSleeping = false;
    std::vector<boost::shared_ptr<Player> >::iterator end = mPlayers.end();
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != end; ++it)
    {
        if ((*it)->isSleeping())
            (*it)->stopSleepInBed(false, false, true);
    }
    if (getGameRules()->getBoolean(GameRules::RULE_WEATHER_CYCLE))
        stopWeather();
}

void ServerLevel::save(bool force, ProgressListener* progressListener, bool autosave)
{
    ChunkSource* chunkSource = getChunkSource();
    if (!chunkSource->shouldSave())
        return;
    if (gStorage->GetSaveDisabled())
        return;

    if (progressListener != NULL)
        progressListener->progressStartNoAbort(autosave ? static_cast<int>(IDS_PROGRESS_AUTOSAVING_LEVEL)
                                                        : static_cast<int>(IDS_PROGRESS_SAVING_LEVEL));
    PIXBeginNamedEvent(0.0f, "Saving level data");
    saveLevelData();
    PIXEndNamedEvent();
    if (progressListener != NULL)
        progressListener->progressStage(static_cast<int>(IDS_PROGRESS_SAVING_CHUNKS));
    chunkSource->save(force, progressListener);
}

bool ServerLevel::doBlockEvent(BlockEventData* event)
{
    const BlockState* state = getBlockState(event->getPos());
    if (state->getBlock() != event->getBlock())
        return false;
    return state->getBlock()->triggerEvent(state, this, event->getPos(), event->getParamA(), event->getParamB());
}

void ServerLevel::runBlockEvents()
{
    while (!blockEvents[activeBlockEventList].empty())
    {
        int current = activeBlockEventList;
        activeBlockEventList ^= 1;
        for (std::vector<BlockEventData>::iterator it = blockEvents[current].begin(); it != blockEvents[current].end();
             ++it)
        {
            if (!doBlockEvent(&*it))
                continue;
            BlockEventData event = *it;
            serverPlayers()->broadcast(event.getPos().x, event.getPos().y, event.getPos().z, 64.0,
                                       mDimension->getType()->getId(),
                                       boost::shared_ptr<ClientboundBlockEventPacket>(new ClientboundBlockEventPacket(
                                           event.getPos(), event.getBlock(), event.getParamA(), event.getParamB())));
        }
        blockEvents[current].clear();
    }
}

void ServerLevel::runQueuedSendBlockUpdates()
{
    EnterCriticalSection(&queuedSendBlockUpdateMutex);
    for (std::vector<QueuedSendBlockData*>::iterator it = queuedSendBlockUpdates.begin();
         it != queuedSendBlockUpdates.end(); ++it)
    {
        sendBlockUpdated((*it)->pos, (*it)->oldState, (*it)->newState, 3, true);
        delete *it;
    }
    queuedSendBlockUpdates.clear();
    LeaveCriticalSection(&queuedSendBlockUpdateMutex);
}

void ServerLevel::tick()
{
    Level::tick();
    if (getLevelData()->isHardcore() && getDifficulty() != Difficulty::HARD)
        getLevelData()->setDifficulty(Difficulty::HARD);

    mDimension->getBiomeSource()->update();
    if (allPlayersAreSleeping())
    {
        if (getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT))
        {
            long long newTime = mLevelData->getDayTime() + 24000;
            newTime -= newTime % 24000;
            long long oldTime = mLevelData->getDayTime();
            setDayTime(newTime);
            for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
                (*it)->AdvanceTimerAchievement(Player::eTimerAchievement_Castaway, static_cast<int>(newTime - oldTime));
        }
        awakenAllPlayers();
    }

    PIXBeginNamedEvent(0.0f, "Mob spawner tick");
    if (getGameRules()->getBoolean(GameRules::RULE_DOMOBSPAWNING))
    {
        bool spawnEnemies = mSpawnEnemies && (mLevelData->getGameTime() & 1) == 0;
        bool spawnFriendlies = mSpawnFriendlies && mLevelData->getGameTime() % 40 == 0;
        bool spawnPersistent = spawnFriendlies && mLevelData->getGameTime() % 80 == 0;
        mobSpawner->tick(this, spawnEnemies, spawnFriendlies, spawnPersistent);
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Chunk source tick");
    mChunkSource->tick();
    PIXEndNamedEvent();

    int skyDarken = getOldSkyDarken(1.0f);
    if (skyDarken != getSkyDarken())
        setSkyDarken(skyDarken);

    int dimensionId = mDimension->getType()->getId();
    if ((mLevelData->getGameTime() + 1) % saveInterval
        == dimensionId * mDimension->getType()->getId() * (saveInterval / 2))
    {
        PIXBeginNamedEvent(0.0f, "Incremental save");
        save(false, NULL, false);
        PIXEndNamedEvent();
    }

    setGameTime(mLevelData->getGameTime() + 1);
    if (getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT))
        setDayTime(mLevelData->getDayTime() + 1);

    PIXBeginNamedEvent(0.0f, "Tick pending ticks");
    tickPendingTicks(false);
    PIXEndNamedEvent();

    unsigned long long startTime = System::processTimeInMilliSecs();
    PIXBeginNamedEvent(0.0f, "Tick blocks");
    MemSect(0x12);
    tickBlocks();
    MemSect(0);
    PIXEndNamedEvent();
    unsigned long long endTime = System::processTimeInMilliSecs();
    if (mDimension->getType()->getId() == 0)
        PIXAddNamedCounter(static_cast<float>(static_cast<long long>(endTime - startTime)), "Tick blocks");

    chunkMap->tick();

    PIXBeginNamedEvent(0.0f, "Tick villages");
    mVillages->tick();
    villageSiege->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Tick portal forcer");
    portalForcer->tick(getGameTime());
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "runBlockEvents");
    runBlockEvents();
    PIXEndNamedEvent();

    runQueuedSendBlockUpdates();

    EnterCriticalSection(&entityLimitMutex);
    enderCrystalExplosionCount = 0;
    LeaveCriticalSection(&entityLimitMutex);
    --primedTntCheckDelay;
}

Biome::MobSpawnerData* ServerLevel::getRandomMobSpawnAt(MobCategory* category, const BlockPos& pos)
{
    std::vector<Biome::MobSpawnerData*>* mobs = getChunkSource()->getMobsAt(category, pos);
    if (mobs == NULL || mobs->empty())
        return NULL;
    return static_cast<Biome::MobSpawnerData*>(
        WeighedRandom::getRandomItem(mRandom, reinterpret_cast<std::vector<WeighedRandomItem*>*>(mobs)));
}

bool ServerLevel::canMobSpawnAt(MobCategory* category, Biome::MobSpawnerData* spawnerData, const BlockPos& pos)
{
    std::vector<Biome::MobSpawnerData*>* mobs = getChunkSource()->getMobsAt(category, pos);
    if (mobs == NULL || mobs->empty())
        return false;
    return std::find(mobs->begin(), mobs->end(), spawnerData) != mobs->end();
}

void ServerLevel::updateSleepingPlayerList()
{
    somePlayerSleeping = false;
    if (mPlayers.empty())
    {
        allPlayersSleeping = false;
        return;
    }

    int spectators = 0;
    unsigned int sleeping = 0;
    allPlayersSleeping = false;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if ((*it)->isSpectator())
        {
            ++spectators;
        }
        else if ((*it)->isSleeping())
        {
            ++sleeping;
            somePlayerSleeping = true;
        }
    }
    allPlayersSleeping = sleeping > 0 && static_cast<unsigned int>(mPlayers.size() - spectators) <= sleeping;
}

void ServerLevel::validateSpawn()
{
    if (mLevelData->getYSpawn() <= 0)
        mLevelData->setYSpawn(getSeaLevel() + 1);

    int x = mLevelData->getXSpawn();
    int z = mLevelData->getZSpawn();
    int attempts = 0;
    while (getTopBlockState(BlockPos(x, 0, z))->getMaterial() == Material::AIR)
    {
        x += mRandom->nextInt(8) - mRandom->nextInt(8);
        z += mRandom->nextInt(8) - mRandom->nextInt(8);
        if (++attempts == 10000)
            break;
    }
    mLevelData->setXSpawn(x);
    mLevelData->setZSpawn(z);
}

bool ServerLevel::hasChunk(int chunkX, int chunkZ, bool allowEmpty)
{
    return getChunkSource()->hasChunk(chunkX, chunkZ);
}

LevelChunk* ServerLevel::getChunkIfHasChunk(int chunkX, int chunkZ, bool allowEmpty)
{
    return getChunkSource()->getChunkIfHasChunk(chunkX, chunkZ);
}

BlockPos ServerLevel::findLightingTargetAround(const BlockPos& pos)
{
    BlockPos target = getTopRainBlockPos(pos);
    AABB* box = AABB::newTemp(target, BlockPos(target.x, getMaxBuildHeight(), target.z))->grow(3.0, 3.0, 3.0);
    static Predicate<boost::shared_ptr<Entity> >* canSeeSky =
        new Predicates<boost::shared_ptr<Entity> >::FunctionPredicate(canSeeSkyFunction);
    std::vector<boost::shared_ptr<Entity> >* entities = getEntitiesOfClass(typeid(LivingEntity), box, canSeeSky, false);
    if (!entities->empty())
    {
        BlockPos result =
            entities->at(mRandom->nextInt(static_cast<int>(entities->size())))->getCommandSenderBlockPosition();
        delete entities;
        return result;
    }
    if (target.y == -1)
        target = target.above(2);
    delete entities;
    return target;
}

void ServerLevel::tickBlocks()
{
    bool wasRaining = isRaining();
    int dimension = 0;
    if (mDimension->getType() == DimensionType::NETHER)
        dimension = 1;
    else if (mDimension->getType() == DimensionType::THE_END)
        dimension = 2;
    mChunksToPoll.clear();
    if (mDimension->getType() == DimensionType::OVERWORLD)
        PIXAddNamedCounter(static_cast<float>(sBlockUpdateTickCounts[dimension]), "Blocks to be ticked");

    EnterCriticalSection(&sBlockUpdateMutex[dimension]);
    PIXBeginNamedEvent(0.0f, "Ticking %d blocks", sBlockUpdateTickCounts[dimension]);
    for (int index = 0; index < sBlockUpdateTickCounts[dimension]; ++index)
    {
        BlockPos pos(sBlockUpdateTickX[dimension][index], sBlockUpdateTickY[dimension][index],
                     sBlockUpdateTickZ[dimension][index]);
        if (!hasChunkAt(pos))
            continue;
        Block* block;
        Block* extraBlock;
        getBlockAndExtraBlock(pos, &block, &extraBlock);
        if (block != 0 && block->isTicking())
            block->tick(this, pos, getBlockState(pos), mRandom);
        if (extraBlock != 0 && extraBlock->isTicking())
        {
            const BlockState* extraState = getExtraBlockState(pos);
            extraState->getBlock()->tick(this, pos, extraState, mRandom);
        }
    }
    PIXEndNamedEvent();
    sBlockUpdateTickCounts[dimension] = 0;
    sBlockUpdateChunkCounts[dimension] = 0;
    LeaveCriticalSection(&sBlockUpdateMutex[dimension]);

    PIXBeginNamedEvent(0.0f, "Level::tickBlocks");
    Level::tickBlocks();
    PIXEndNamedEvent();

    int lightningChance = 100000;
    if (GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(0xd), -1))
        lightningChance = 100;

    MutableBlockPos pos(0, 0, 0);
    for (int index = 0; index < mChunksToPoll.end(); ++index)
    {
        ChunkPos chunkPos = mChunksToPoll.get(index);
        int xo = chunkPos.x * 16;
        int zo = chunkPos.z * 16;
        if (!hasChunk(chunkPos.x, chunkPos.z, true))
            continue;
        int queueIndex = sBlockUpdateChunkCounts[dimension]++;
        sBlockUpdateChunkX[dimension][queueIndex] = chunkPos.x;
        sBlockUpdateChunkZ[dimension][queueIndex] = chunkPos.z;
        LevelChunk* chunk = getChunk(chunkPos.x, chunkPos.z);
        tickClientSideBlocks(xo, zo, chunk);
        chunk->tick(false);

        if (mRandom->nextInt(lightningChance) == 0 && isRaining() && isThundering())
        {
            mRandValue = mRandValue * 3 + mAddend;
            int value = mRandValue >> 2;
            pos.x = xo + (value & 15);
            pos.y = 0;
            pos.z = zo + ((value >> 8) & 15);
            BlockPos target = findLightingTargetAround(pos);
            if (isRainingAt(target))
            {
                DifficultyInstance difficulty = getCurrentDifficultyAt(target);
                if (getGameRules()->getBoolean(GameRules::RULE_DOMOBSPAWNING)
                    && mRandom->nextDouble() < difficulty.getEffectiveDifficulty() * 0.01)
                {
                    boost::shared_ptr<SkeletonHorse> horse(new SkeletonHorse(this));
                    horse->setTrap(true);
                    horse->setAge(0);
                    horse->setPos(target.x, target.y, target.z);
                    addEntity(horse);
                    addGlobalEntity(
                        boost::shared_ptr<Entity>(new LightningBolt(this, target.x, target.y, target.z, true)));
                }
                else
                {
                    addGlobalEntity(
                        boost::shared_ptr<Entity>(new LightningBolt(this, target.x, target.y, target.z, false)));
                }
            }
        }

        if (mRandom->nextInt(16) == 0)
        {
            mRandValue = mRandValue * 3 + mAddend;
            int value = mRandValue >> 2;
            pos.x = xo + (value & 15);
            pos.y = 0;
            pos.z = zo + ((value >> 8) & 15);
            BlockPos top = getTopRainBlockPos(pos);
            BlockPos below = top.below();
            if (shouldFreeze(below))
                setBlockAndUpdate(below, Blocks::ice->defaultBlockState());
            Biome* biome = getBiome(top);
            float downfall = getRainLevel(0.0f) * biome->getDownfall();
            if (downfall > 0.0f)
            {
                int range = static_cast<int>(Mth::Lerp(256.0f, -256.0f, downfall));
                if (range < 16)
                    range = 16;
                int limit = getBlock(top) == Blocks::snow_layer ? 2 : 16;
                if (mRandom->nextInt(range) <= limit)
                    tryToPlaceTopSnow(top, true, true);
                if (wasRaining && getBiome(below)->hasRain())
                    getBlock(below)->handleRain(this, below);
            }
        }

        if (mRandom->nextInt(8) == 0)
        {
            int x = mRandom->nextInt(16);
            int y = mRandom->nextInt(128);
            int z = mRandom->nextInt(16);
            pos.x = xo + x;
            pos.y = y;
            pos.z = zo + z;
            checkLight(pos, false, false);
        }
    }

    sBlockUpdateLevels[dimension] = this;
    sBlockUpdateRandom[dimension] = mRandValue;
    sBlockUpdateEvents->Set(dimension);
}

bool ServerLevel::isBlockToBeTickedAt(const BlockPos& pos, Block* block)
{
    TickNextTickData data(pos, block);
    return std::find(toBeTicked.begin(), toBeTicked.end(), data) != toBeTicked.end();
}

bool ServerLevel::hasTickNextTick(const BlockPos& pos, Block* block)
{
    TickNextTickData data(pos, block);
    return tickNextTickSet.find(data) != tickNextTickSet.end();
}

void ServerLevel::addToTickNextTick(const BlockPos& pos, Block* block, int delay)
{
    addToTickNextTick(pos, block, delay, 0);
}

void ServerLevel::addToTickNextTick(const BlockPos& pos, Block* block, int delay, int priority)
{
    MemSect(0x1b);
    const Material* material = block->defaultBlockState()->getMaterial();
    if (Level::getInstaTick() && material != Material::AIR)
    {
        if (block->canInstantlyTick())
        {
            if (hasChunksAt(pos.offset(-8, -8, -8), pos.offset(8, 8, 8)))
            {
                const BlockState* state = getBlockState(pos);
                if (state->getMaterial() != Material::AIR && state->getBlock() == block)
                {
                    state->getBlock()->tick(this, pos, state, mRandom);
                    MemSect(0);
                    return;
                }
            }
            MemSect(0);
            return;
        }
        delay = 1;
    }

    TickNextTickData data(pos, block);
    if (hasChunkAt(pos))
    {
        if (block->getCraftingMaterial() != Material::AIR)
        {
            data.delay(mLevelData->getGameTime() + delay);
            data.setPriorityTilt(priority);
        }
        EnterCriticalSection(&tickNextTickMutex);
        if (tickNextTickSet.find(data) == tickNextTickSet.end())
        {
            tickNextTickSet.insert(data);
            tickNextTickList.insert(data);
        }
        LeaveCriticalSection(&tickNextTickMutex);
    }
    MemSect(0);
}

void ServerLevel::forceAddBlockTick(const BlockPos& pos, Block* block, int delay, int priority)
{
    TickNextTickData data(pos, block);
    data.setPriorityTilt(priority);
    if (block->defaultBlockState()->getMaterial() != Material::AIR)
        data.delay(mLevelData->getGameTime() + delay);

    EnterCriticalSection(&tickNextTickMutex);
    if (tickNextTickSet.find(data) == tickNextTickSet.end())
    {
        tickNextTickSet.insert(data);
        tickNextTickList.insert(data);
    }
    LeaveCriticalSection(&tickNextTickMutex);
}

void ServerLevel::resetEmptyTime()
{
    emptyTime = 0;
}

void ServerLevel::tickEntities()
{
    if (mPlayers.empty())
    {
        if (emptyTime++ >= 300)
            return;
    }
    else
    {
        resetEmptyTime();
    }
    mDimension->tick();
    Level::tickEntities();
}

void ServerLevel::tickServerPlayers()
{
    Level::tickServerPlayers();
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<Entity> player = mPlayers.at(i);
        boost::shared_ptr<Entity> vehicle = player->getVehicle();
        if (vehicle)
        {
            if (!vehicle->mRemoved && vehicle->hasPassenger(player))
                continue;
            player->stopRiding();
        }

        if (!player->mRemoved)
            Level::tick(player);

        if (player->mRemoved)
        {
            int chunkX = player->mChunkX;
            int chunkZ = player->mChunkZ;
            if (player->mInChunk && hasChunk(chunkX, chunkZ, true))
                getChunk(chunkX, chunkZ)->removeEntity(player);
            std::vector<boost::shared_ptr<Entity> >::iterator it = std::find(mEntities.begin(), mEntities.end(),
                                                                             player);
            if (it != mEntities.end())
                mEntities.erase(it);
            entityRemoved(player);
        }
    }
}

bool ServerLevel::tickPendingTicks(bool runAll)
{
    EnterCriticalSection(&tickNextTickMutex);
    int count = static_cast<int>(tickNextTickList.size());
    if (count > 1000)
        count = 1000;

    for (int i = 0; i < count; ++i)
    {
        TickNextTickData data = *tickNextTickList.begin();
        if (!runAll && data.triggerTick > mLevelData->getGameTime())
            break;
        tickNextTickList.erase(tickNextTickList.begin());
        tickNextTickSet.erase(data);
        toBeTicked.push_back(data);
    }

    PIXBeginNamedEvent(0.0f, "Tick toBeTicked ticks (length=%i)", static_cast<int>(toBeTicked.size()));
    for (std::vector<TickNextTickData>::iterator it = toBeTicked.begin(); it != toBeTicked.end();)
    {
        TickNextTickData data = *it;
        it = toBeTicked.erase(it);
        if (!hasChunksAt(data.pos.offset(0, 0, 0), data.pos.offset(0, 0, 0)))
        {
            addToTickNextTick(data.pos, data.getBlock(), 0);
            continue;
        }

        const BlockState* state = getBlockState(data.pos);
        Block* block = state->getBlock();
        const BlockState* extraState = getExtraBlockState(data.pos);
        Block* extraBlock = extraState->getBlock();
        if (block->getCraftingMaterial() != Material::AIR && Block::isMatching(block, data.getBlock()))
        {
            PIXBeginNamedEvent(0.0f, "Tick block '%i'", Block::getId(block));
            block->tick(this, data.pos, state, mRandom);
            PIXEndNamedEvent();
        }
        else if (extraBlock->getCraftingMaterial() != Material::AIR && Block::isMatching(extraBlock, data.getBlock()))
        {
            PIXBeginNamedEvent(0.0f, "Tick Extra block '%i'", Block::getId(block));
            extraBlock->tick(this, data.pos, extraState, mRandom);
            PIXEndNamedEvent();
        }
    }
    PIXEndNamedEvent();

    bool pending = !tickNextTickList.empty();
    toBeTicked.clear();
    LeaveCriticalSection(&tickNextTickMutex);
    return pending;
}

std::vector<TickNextTickData>* ServerLevel::fetchTicksInChunk(LevelChunk* chunk, bool removeTicks)
{
    ChunkPos chunkPos = chunk->getPos();
    int x0 = chunkPos.x * 16;
    int z0 = chunkPos.z * 16;
    BoundingBox box(x0 - 2, 0, z0 - 2, x0 + 16, 256, z0 + 16);
    return fetchTicksInArea(&box, removeTicks);
}

std::vector<TickNextTickData>* ServerLevel::fetchTicksInArea(BoundingBox* box, bool removeTicks)
{
    EnterCriticalSection(&tickNextTickMutex);
    std::vector<TickNextTickData>* ticks = new std::vector<TickNextTickData>();
    for (int pass = 0; pass < 2; ++pass)
    {
        if (pass == 0)
        {
            for (std::set<TickNextTickData, TickNextTickDataKeyCompare>::iterator it = tickNextTickList.begin();
                 it != tickNextTickList.end(); ++it)
            {
                TickNextTickData data = *it;
                if (data.pos.x < box->x0 || data.pos.x >= box->x1 || data.pos.z < box->z0 || data.pos.z >= box->z1)
                    continue;
                if (removeTicks)
                    tickNextTickSet.erase(data);
                ticks->push_back(data);
            }
        }
        else if (removeTicks)
        {
            for (std::vector<TickNextTickData>::iterator it = toBeTicked.begin(); it != toBeTicked.end();)
            {
                TickNextTickData data = *it;
                if (data.pos.x < box->x0 || data.pos.x >= box->x1 || data.pos.z < box->z0 || data.pos.z >= box->z1)
                {
                    ++it;
                    continue;
                }
                std::pair<std::set<TickNextTickData, TickNextTickDataKeyCompare>::iterator,
                          std::set<TickNextTickData, TickNextTickDataKeyCompare>::iterator>
                    range = tickNextTickList.equal_range(data);
                tickNextTickList.erase(range.first, range.second);
                it = toBeTicked.erase(it);
                ticks->push_back(data);
            }
        }
        else
        {
            for (std::vector<TickNextTickData>::iterator it = toBeTicked.begin(); it != toBeTicked.end(); ++it)
            {
                TickNextTickData data = *it;
                if (data.pos.x >= box->x0 && data.pos.x < box->x1 && data.pos.z >= box->z0 && data.pos.z < box->z1)
                    ticks->push_back(data);
            }
        }
    }
    LeaveCriticalSection(&tickNextTickMutex);
    return ticks;
}

bool ServerLevel::isAnimalsAllowed()
{
    return server->isAnimals();
}

bool ServerLevel::isNpcsAllowed()
{
    return server->isNpcsEnabled();
}

void ServerLevel::tick(boost::shared_ptr<Entity> entity, bool updateChunk)
{
    if (!isAnimalsAllowed()
        && (eTYPE_DERIVED_FROM(EntityType::ANIMAL, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::WATER_ANIMAL, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::AGABLE_WATER_ANIMAL, entity->GetType())))
        entity->remove();
    if (!isNpcsAllowed() && boost::dynamic_pointer_cast<Npc>(entity))
        entity->remove();
    Level::tick(entity, updateChunk);
}

void ServerLevel::forceTick(boost::shared_ptr<Entity> entity, bool updateChunk)
{
    Level::tick(entity, updateChunk);
}

bool ServerLevel::mayInteract(boost::shared_ptr<Player> player, const BlockPos& pos, Block* block)
{
    if (block == Blocks::flowing_lava && mDimension->getType() == DimensionType::OVERWORLD)
        return !server->isUnderSpawnProtection(this, pos, player) && getWorldBorder()->isWithinBounds(pos);
    return true;
}

void ServerLevel::initializeLevel(LevelSettings* settings)
{
    if (!mLevelData->isInitialized())
    {
        setInitialSpawn(settings);
        Level::initializeLevel(settings);
        mLevelData->setInitialized(true);
    }
}

void ServerLevel::setupDebugLevel()
{
    mLevelData->setGenerateMapFeatures(false);
    mLevelData->setAllowCommands(true);
    mLevelData->setRaining(false);
    mLevelData->setThundering(false);
    mLevelData->setClearWeatherTime(1000000000);
    mLevelData->setDayTime(6000);
    mLevelData->setGameType(GameType::SPECTATOR);
    mLevelData->setHardcore(false);
    mLevelData->setDifficulty(Difficulty::PEACEFUL);
    getGameRules()->set(8, false);
}

void ServerLevel::generateBonusItemsNearSpawn()
{
    int spawnX = mLevelData->getXSpawn();
    int spawnZ = mLevelData->getZSpawn();
    for (int dx = -20; dx <= 20; ++dx)
    {
        for (int dz = -20; dz <= 20; ++dz)
        {
            BlockPos pos = getTopSolidBlockPos(BlockPos(spawnX + dx, 0, spawnZ + dz)).below();
            if (getBlock(pos) != Blocks::chest)
                continue;
            boost::shared_ptr<ChestBlockEntity> chest = boost::dynamic_pointer_cast<ChestBlockEntity>(
                getBlockEntity(pos));
            if (chest && chest->mIsBonusChest)
                return;
        }
    }

    BonusChestFeature* feature = new BonusChestFeature();
    for (int attempt = 0; attempt < 16; attempt += 2)
    {
        BlockPos pos(mLevelData->getXSpawn() + mRandom->nextInt(6) - mRandom->nextInt(6), 0,
                     mLevelData->getZSpawn() + mRandom->nextInt(6) - mRandom->nextInt(6));
        if (feature->place(this, *mRandom, getTopSolidBlockPos(pos).above(), attempt == 15))
            break;
        BlockPos retryPos(mLevelData->getXSpawn() + mRandom->nextInt(6) - mRandom->nextInt(6), 0,
                          mLevelData->getZSpawn() + mRandom->nextInt(6) - mRandom->nextInt(6));
        if (feature->place(this, *mRandom, getTopSolidBlockPos(retryPos).above(), attempt == 14))
            break;
    }
    delete feature;
}

void ServerLevel::setInitialSpawn(LevelSettings* settings)
{
    if (!mDimension->mayRespawn())
    {
        mLevelData->setSpawn(BlockPos::ZERO.above(mDimension->getSpawnYPosition()));
        return;
    }

    mFindingSpawn = true;
    BiomeSource* biomeSource = mDimension->getBiomeSource();
    std::vector<Biome*> playerSpawnBiomes = biomeSource->getPlayerSpawnBiomes();
    Random random(getSeed());
    BlockPos* found = biomeSource->findBiome(0, 0, 256, playerSpawnBiomes, &random);
    int x = 8;
    int y = mDimension->getSpawnYPosition();
    int z = 8;
    int minBound = -(mDimension->getXZSize() * 16) / 2;
    int maxBound = (mDimension->getXZSize() * 16) / 2 - 1;
    if (found != NULL)
    {
        x = found->x;
        z = found->z;
        delete found;
    }

    int attempts = 0;
    while (!mDimension->isValidSpawn(x, z))
    {
        x += random.nextInt(64) - random.nextInt(64);
        if (x > maxBound)
            x = 0;
        if (x < minBound)
            x = 0;
        z += random.nextInt(64) - random.nextInt(64);
        if (z > maxBound)
            z = 0;
        if (z < minBound)
            z = 0;
        if (++attempts == 1000)
            break;
    }

    mLevelData->setSpawn(BlockPos(x, y, z));
    if (settings->hasStartingBonusItems())
        generateBonusItemsNearSpawn();
    mFindingSpawn = false;
}

void ServerLevel::Suspend()
{
    if (!gStorage->GetSaveDisabled())
    {
        saveLevelData();
        mChunkSource->saveAllEntities();
    }
}

void ServerLevel::saveToDisc(ProgressListener* progressListener, bool autosave)
{
    if (gStorage->GetSaveDisabled())
        return;
    TexturePack* selected = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    if (selected != TexturePackRepository::defaultTexturePack
        && !static_cast<DLCTexturePack*>(selected)->getDLCInfoParentPack()->hasPurchasedPack())
        return;
    if (progressListener != NULL)
        progressListener->progressStage(static_cast<int>(0xaebf0574u));
    mLevelStorage->flushSaveFile(autosave);
}

bool ServerLevel::removeDuplicateAndPendingEntity(const boost::shared_ptr<Entity>& entity)
{
    if (entity->mRemoved)
        return false;
    std::wstring uuid = entity->getUUID();
    if (entitiesByUuid.find(uuid) != entitiesByUuid.end())
    {
        boost::shared_ptr<Entity> duplicate = entitiesByUuid.at(uuid);
        bool pending = false;
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = mEntitiesToRemove.begin();
             it != mEntitiesToRemove.end(); ++it)
        {
            if (*it == duplicate)
            {
                mEntitiesToRemove.erase(it);
                pending = true;
                break;
            }
        }
        if (!pending && !eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
            return false;
        removeEntityImmediately(duplicate);
    }
    return true;
}

void ServerLevel::addEntities(std::vector<boost::shared_ptr<Entity> >* entities)
{
    EnterCriticalSection(&mEntityMutex);
    bool addedDragon = false;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        if (!removeDuplicateAndPendingEntity(*it))
            continue;
        mEntities.push_back(*it);
        entityAdded(*it);
        if ((*it)->GetType() == EntityType::ENDER_DRAGON)
            addedDragon = true;
    }

    if (addedDragon)
    {
        bool foundDragon = false;
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin(); it != mEntities.end(); ++it)
        {
            if ((*it)->GetType() != EntityType::ENDER_DRAGON)
                continue;
            if (foundDragon)
                (*it)->remove();
            else
                foundDragon = true;
        }
    }
    LeaveCriticalSection(&mEntityMutex);
}

void ServerLevel::saveLevelData()
{
    checkSession();
    for (unsigned int i = 0; i < server->mLevels.length; ++i)
    {
        DerivedServerLevel* derived = dynamic_cast<DerivedServerLevel*>(server->mLevels[i]);
        if (derived != NULL)
            derived->saveDimensionData();
    }
    mLevelStorage->saveLevelData(mLevelData, &mPlayers);
    mSavedDataStorage->save();
}

void ServerLevel::entityAddedExtra(boost::shared_ptr<Entity> entity)
{
    if (eTYPE_DERIVED_FROM(EntityType::ITEM_ENTITY, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        itemEntities.push_back(entity);
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::HANGING_ENTITY, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        hangingEntities.push_back(entity);
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::ARROW, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        arrowEntities.push_back(entity);
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::EXPERIENCE_ORB, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        experienceOrbs.push_back(entity);
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::PRIMED_TNT, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        ++primedTntCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::FALLING_BLOCK_ENTITY, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        ++fallingBlockCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::FIREWORKS_ROCKET_ENTITY, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        ++fireworksRocketCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::AREA_EFFECT_CLOUD, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        areaEffectClouds.push_back(entity);
        LeaveCriticalSection(&entityLimitMutex);
    }
}

void ServerLevel::entityAdded(boost::shared_ptr<Entity> entity)
{
    Level::entityAdded(entity);
    mEntityMap[entity->getId()] = entity;
    entitiesByUuid[entity->getUUID()] = entity;
    std::vector<boost::shared_ptr<Entity> >* subEntities = entity->getSubEntities();
    if (subEntities != NULL)
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = subEntities->begin(); it != subEntities->end();
             ++it)
            mEntityMap[(*it)->getId()] = *it;
    }
    entityAddedExtra(entity);
}

void ServerLevel::entityRemovedExtra(boost::shared_ptr<Entity> entity)
{
    if (eTYPE_DERIVED_FROM(EntityType::ITEM_ENTITY, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        std::list<boost::shared_ptr<Entity> >::iterator it = std::find(itemEntities.begin(), itemEntities.end(),
                                                                       entity);
        if (it != itemEntities.end())
            itemEntities.erase(it);
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::HANGING_ENTITY, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        std::list<boost::shared_ptr<Entity> >::iterator it = std::find(hangingEntities.begin(), hangingEntities.end(),
                                                                       entity);
        if (it != hangingEntities.end())
            hangingEntities.erase(it);
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::ARROW, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        std::list<boost::shared_ptr<Entity> >::iterator it = std::find(arrowEntities.begin(), arrowEntities.end(),
                                                                       entity);
        if (it != arrowEntities.end())
            arrowEntities.erase(it);
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::EXPERIENCE_ORB, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        std::list<boost::shared_ptr<Entity> >::iterator it = std::find(experienceOrbs.begin(), experienceOrbs.end(),
                                                                       entity);
        if (it != experienceOrbs.end())
            experienceOrbs.erase(it);
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::PRIMED_TNT, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        --primedTntCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::FALLING_BLOCK_ENTITY, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        --fallingBlockCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::FIREWORKS_ROCKET_ENTITY, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        --fireworksRocketCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::AREA_EFFECT_CLOUD, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        std::list<boost::shared_ptr<Entity> >::iterator it = std::find(areaEffectClouds.begin(), areaEffectClouds.end(),
                                                                       entity);
        if (it != areaEffectClouds.end())
            areaEffectClouds.erase(it);
        LeaveCriticalSection(&entityLimitMutex);
    }
}

void ServerLevel::entityRemoved(boost::shared_ptr<Entity> entity)
{
    Level::entityRemoved(entity);
    mEntityMap.erase(entity->getId());
    entitiesByUuid.erase(entity->getUUID());
    std::vector<boost::shared_ptr<Entity> >* subEntities = entity->getSubEntities();
    if (subEntities != NULL)
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = subEntities->begin(); it != subEntities->end();
             ++it)
            mEntityMap.erase((*it)->getId());
    }
    entityRemovedExtra(entity);
}

boost::shared_ptr<Entity> ServerLevel::getEntity(int id)
{
    boost::unordered_map<int, boost::shared_ptr<Entity>, IntKeyHash2, IntKeyEq>::iterator it = mEntityMap.find(id);
    if (it != mEntityMap.end())
        return it->second;
    return boost::shared_ptr<Entity>();
}

bool ServerLevel::addGlobalEntity(boost::shared_ptr<Entity> entity)
{
    if (Level::addGlobalEntity(entity))
    {
        server->getPlayers()->broadcast(entity->mX, entity->mY, entity->mZ, 512.0, mDimension->getType()->getId(),
                                        boost::shared_ptr<Packet>(new ClientboundAddGlobalEntityPacket(entity)));
        return true;
    }
    return false;
}

void ServerLevel::broadcastEntityEvent(boost::shared_ptr<Entity> entity, unsigned char event, int data)
{
    getTracker()->broadcastAndSend(entity,
                                   boost::shared_ptr<Packet>(new ClientboundEntityEventPacket(entity, event, data)));
}

boost::shared_ptr<Explosion> ServerLevel::explode(boost::shared_ptr<Entity> source, double x, double y, double z,
                                                  float radius, bool fire, bool breaksBlocks, bool playersOnly,
                                                  bool noBlockDrops, bool silent, float damageRadius)
{
    boost::shared_ptr<Explosion> explosion(new Explosion(this, source, x, y, z, radius, fire, breaksBlocks, playersOnly,
                                                         noBlockDrops, silent, damageRadius, false));
    explosion->explode();
    explosion->finalizeExplosion(false, NULL);
    if (!breaksBlocks)
        explosion->clearToBlow();

    std::vector<boost::shared_ptr<ServerPlayer> > sentTo;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = boost::dynamic_pointer_cast<ServerPlayer>(*it);
        if (player->mDimension != mDimension->getType()->getId())
            continue;

        bool knockbackOnly = false;
        if (!sentTo.empty())
        {
            INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
            if (networkPlayer == NULL)
                continue;
            for (unsigned int i = 0; i < sentTo.size(); ++i)
            {
                INetworkPlayer* sentToPlayer = sentTo[i]->mConnection->getNetworkPlayer();
                if (sentToPlayer != NULL && networkPlayer->IsSameSystem(sentToPlayer))
                    knockbackOnly = true;
            }
        }

        if (player->distanceToSqr(x, y, z) < 4096.0)
        {
            Vec3* knockback = explosion->getHitPlayerKnockback(player);
            player->mConnection->send(boost::shared_ptr<Packet>(
                new ClientboundExplodePacket(x, y, z, radius, explosion->getToBlow(), knockback, knockbackOnly)));
            sentTo.push_back(player);
        }
    }
    return explosion;
}

void ServerLevel::blockEvent(const BlockPos& pos, Block* block, int event, int data)
{
    BlockEventData eventData(pos, block, event, data);
    for (std::vector<BlockEventData>::iterator it = blockEvents[activeBlockEventList].begin();
         it != blockEvents[activeBlockEventList].end(); ++it)
    {
        if (it->equals(eventData))
            return;
    }
    blockEvents[activeBlockEventList].push_back(eventData);
}

void ServerLevel::closeLevelStorage()
{
    mLevelStorage->closeAll();
}

void ServerLevel::tickWeather(bool blockEvents)
{
    bool wasRaining = isRaining();
    bool wasThundering = isThundering();
    Level::tickWeather(blockEvents);

    if (mOldRainLevel != mRainLevel)
        serverPlayers()->broadcastAll(boost::shared_ptr<ClientboundGameEventPacket>(new ClientboundGameEventPacket(
                                          ClientboundGameEventPacket::RAIN_LEVEL_CHANGE, mRainLevel)),
                                      mDimension->getType()->getId());
    if (wasThundering != isThundering() || mOldThunderLevel != mThunderLevel)
        serverPlayers()->broadcastAll(boost::shared_ptr<ClientboundGameEventPacket>(new ClientboundGameEventPacket(
                                          ClientboundGameEventPacket::THUNDER_LEVEL_CHANGE, mThunderLevel)),
                                      mDimension->getType()->getId());

    if (wasRaining == isRaining())
        return;
    if (wasRaining)
        serverPlayers()->broadcastAll(boost::shared_ptr<ClientboundGameEventPacket>(new ClientboundGameEventPacket(
                                          ClientboundGameEventPacket::STOP_RAINING, blockEvents ? 1.0f : 0.0f)),
                                      mDimension->getType()->getId());
    else
        serverPlayers()->broadcastAll(boost::shared_ptr<ClientboundGameEventPacket>(new ClientboundGameEventPacket(
                                          ClientboundGameEventPacket::START_RAINING, blockEvents ? 1.0f : 0.0f)),
                                      mDimension->getType()->getId());
    serverPlayers()->broadcastAll(boost::shared_ptr<ClientboundGameEventPacket>(new ClientboundGameEventPacket(
                                      ClientboundGameEventPacket::RAIN_LEVEL_CHANGE, mRainLevel)),
                                  mDimension->getType()->getId());
    serverPlayers()->broadcastAll(boost::shared_ptr<ClientboundGameEventPacket>(new ClientboundGameEventPacket(
                                      ClientboundGameEventPacket::THUNDER_LEVEL_CHANGE, mThunderLevel)),
                                  mDimension->getType()->getId());
}

int ServerLevel::getChunkPollRange()
{
    return serverPlayers()->getViewDistance();
}

MinecraftServer* ServerLevel::getServer()
{
    return server;
}

void ServerLevel::setTimeAndAdjustBlockTicks(long long newTime)
{
    long long delta = newTime - mLevelData->getGameTime();
    std::vector<TickNextTickData> ticks;
    for (std::set<TickNextTickData, TickNextTickDataKeyCompare>::iterator it = tickNextTickList.begin();
         it != tickNextTickList.end(); ++it)
    {
        ticks.push_back(*it);
        ticks.back().triggerTick += delta;
    }
    tickNextTickList.clear();
    for (unsigned int i = 0; i < ticks.size(); ++i)
        tickNextTickList.insert(ticks[i]);
    setGameTime(newTime);
}

void ServerLevel::sendParticles(boost::shared_ptr<ServerPlayer> player, bool longDistance,
                                boost::shared_ptr<ClientboundLevelParticlesPacket> packet)
{
    BlockPos pos = player->getCommandSenderBlockPosition();
    double distance = pos.distSqr(packet->getX(), packet->getY(), packet->getZ());
    if (distance <= 1024.0 || (packet->isOverrideLimiter() && distance <= 262144.0))
        player->mConnection->send(packet);
}

void ServerLevel::sendParticles(const ParticleType* type, bool longDistance, double x, double y, double z, int count,
                                double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params)
{
    boost::shared_ptr<ClientboundLevelParticlesPacket> packet(
        new ClientboundLevelParticlesPacket(type, longDistance, static_cast<float>(x), static_cast<float>(y),
                                            static_cast<float>(z), static_cast<float>(xDist), static_cast<float>(yDist),
                                            static_cast<float>(zDist), static_cast<float>(maxSpeed), count, params));
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
        sendParticles(boost::dynamic_pointer_cast<ServerPlayer>(mPlayers.at(i)), longDistance, packet);
}

void ServerLevel::sendParticles(const ParticleType* type, double x, double y, double z, int count, double xDist,
                                double yDist, double zDist, double maxSpeed, arrayWithLength<int> params)
{
    sendParticles(type, false, x, y, z, count, xDist, yDist, zDist, maxSpeed, params);
}

void ServerLevel::sendParticles(const ParticleType* type, double x, double y, double z, int count, double xDist,
                                double yDist, double zDist, double maxSpeed, arrayWithLength<int> params, double xAux,
                                double yAux, double zAux)
{
    boost::shared_ptr<ClientboundLevelParticlesPacket> packet(new ClientboundLevelParticlesPacket(
        type, false, static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(xDist),
        static_cast<float>(yDist), static_cast<float>(zDist), static_cast<float>(maxSpeed), count, params,
        static_cast<float>(xAux), static_cast<float>(yAux), static_cast<float>(zAux)));
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
        sendParticles(boost::static_pointer_cast<ServerPlayer>(mPlayers.at(i)), false, packet);
}

void ServerLevel::sendParticles(boost::shared_ptr<ServerPlayer> player, const ParticleType* type, bool longDistance,
                                double x, double y, double z, int count, double xDist, double yDist, double zDist,
                                double maxSpeed, arrayWithLength<int> params)
{
    boost::shared_ptr<ClientboundLevelParticlesPacket> packet(
        new ClientboundLevelParticlesPacket(type, longDistance, static_cast<float>(x), static_cast<float>(y),
                                            static_cast<float>(z), static_cast<float>(xDist), static_cast<float>(yDist),
                                            static_cast<float>(zDist), static_cast<float>(maxSpeed), count, params));
    sendParticles(player, longDistance, packet);
}

boost::shared_ptr<Entity> ServerLevel::getEntityByUUID(const std::wstring& uuid)
{
    boost::unordered_map<std::wstring, boost::shared_ptr<Entity> >::iterator it = entitiesByUuid.find(uuid);
    if (it != entitiesByUuid.end())
        return it->second;
    return boost::shared_ptr<Entity>();
}

bool ServerLevel::isEntityIdInUse(int id)
{
    return mEntityMap.find(id) != mEntityMap.end();
}

BlockPos* ServerLevel::findNearestMapFeature(const std::wstring& feature, const BlockPos& pos, bool findUnexplored)
{
    return getChunkSource()->findNearestMapFeature(this, feature, pos, findUnexplored);
}

boost::shared_ptr<Player> ServerLevel::getPlayer(const PlayerUID& uid) const
{
    for (std::vector<boost::shared_ptr<Player> >::const_iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if ((*it)->getXuid() == uid)
            return *it;
    }
    return boost::shared_ptr<Player>();
}

void ServerLevel::queueSendBlockUpdate(int x, int y, int z, const BlockState* oldState, const BlockState* newState)
{
    EnterCriticalSection(&queuedSendBlockUpdateMutex);
    QueuedSendBlockData* update = new QueuedSendBlockData(x, y, z, oldState, newState);
    queuedSendBlockUpdates.push_back(update);
    LeaveCriticalSection(&queuedSendBlockUpdateMutex);
}

bool ServerLevel::atBlockEntityLimit(Block* block)
{
    bool atLimit = false;
    if (block == Blocks::wall_banner || block == Blocks::standing_banner)
    {
        EnterCriticalSection(&blockEntityLimitMutex);
        atLimit = bannerCount >= 100;
        LeaveCriticalSection(&blockEntityLimitMutex);
    }
    if (block == Blocks::beacon)
    {
        EnterCriticalSection(&blockEntityLimitMutex);
        atLimit = beaconCount >= 250;
        LeaveCriticalSection(&blockEntityLimitMutex);
    }
    return atLimit;
}

bool ServerLevel::setBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate)
{
    Block* block = state->getBlock();
    if (block->mBlockEntity && atBlockEntityLimit(block))
        return false;
    return Level::setBlock(pos, state, flags, forceUpdate);
}

bool ServerLevel::addEntity(boost::shared_ptr<Entity> entity)
{
    if (removeDuplicateAndPendingEntity(entity))
    {
        if (eTYPE_DERIVED_FROM(EntityType::ITEM_ENTITY, entity->GetType()))
        {
            EnterCriticalSection(&entityLimitMutex);
            if (itemEntities.size() >= 200)
                removeEntityImmediately(itemEntities.front());
            LeaveCriticalSection(&entityLimitMutex);
        }
        else if (eTYPE_DERIVED_FROM(EntityType::HANGING_ENTITY, entity->GetType()))
        {
            EnterCriticalSection(&entityLimitMutex);
            if (hangingEntities.size() >= 400)
            {
                LeaveCriticalSection(&entityLimitMutex);
                return false;
            }
            LeaveCriticalSection(&entityLimitMutex);
        }
        else if (eTYPE_DERIVED_FROM(EntityType::ARROW, entity->GetType()))
        {
            EnterCriticalSection(&entityLimitMutex);
            if (arrowEntities.size() >= 200)
                removeEntityImmediately(arrowEntities.front());
            LeaveCriticalSection(&entityLimitMutex);
        }
        else if (eTYPE_DERIVED_FROM(EntityType::EXPERIENCE_ORB, entity->GetType()))
        {
            EnterCriticalSection(&entityLimitMutex);
            if (experienceOrbs.size() >= 50)
                removeEntityImmediately(experienceOrbs.front());
            LeaveCriticalSection(&entityLimitMutex);
        }
        else if (eTYPE_DERIVED_FROM(EntityType::AREA_EFFECT_CLOUD, entity->GetType()))
        {
            EnterCriticalSection(&entityLimitMutex);
            if (areaEffectClouds.size() >= 20)
                removeEntityImmediately(areaEffectClouds.front());
            LeaveCriticalSection(&entityLimitMutex);
        }
        return Level::addEntity(entity);
    }
    return false;
}

bool ServerLevel::atEntityLimit(boost::shared_ptr<Entity> entity)
{
    bool atLimit = false;
    if (eTYPE_DERIVED_FROM(EntityType::ITEM_ENTITY, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        atLimit = itemEntities.size() >= 200;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::HANGING_ENTITY, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        atLimit = hangingEntities.size() >= 400;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::ARROW, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        atLimit = arrowEntities.size() >= 200;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::PRIMED_TNT, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        atLimit = primedTntCount >= 20;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::EXPERIENCE_ORB, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        atLimit = experienceOrbs.size() >= 50;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::FIREWORKS_ROCKET_ENTITY, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        atLimit = fireworksRocketCount >= 20;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::AREA_EFFECT_CLOUD, entity->GetType()))
    {
        EnterCriticalSection(&entityLimitMutex);
        atLimit = areaEffectClouds.size() >= 20;
        LeaveCriticalSection(&entityLimitMutex);
    }
    return atLimit;
}

bool ServerLevel::newPrimedTntAllowed()
{
    EnterCriticalSection(&entityLimitMutex);
    bool allowed = primedTntCount < 20;
    if (!allowed && primedTntCheckDelay < 0)
    {
        primedTntCheckDelay = 20;
        std::vector<boost::shared_ptr<Entity> > entities(mEntities);
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities.begin(); it != entities.end(); ++it)
        {
            if ((*it)->GetType() != EntityType::PRIMED_TNT || (*it)->mRemoved)
                continue;
            int x = Mth::floor((*it)->mX);
            int z = Mth::floor((*it)->mZ);
            if (!hasChunksAt(x - 32, 0, z - 32, x + 32, 0, z + 32, true))
                removeEntityImmediately(*it);
        }
        allowed = primedTntCount < 20;
    }
    LeaveCriticalSection(&entityLimitMutex);
    return allowed;
}

bool ServerLevel::newFallingBlockAllowed()
{
    EnterCriticalSection(&entityLimitMutex);
    unsigned int count = fallingBlockCount;
    LeaveCriticalSection(&entityLimitMutex);
    return count < 20;
}

bool ServerLevel::explodeNextEnderCrystal()
{
    EnterCriticalSection(&entityLimitMutex);
    unsigned int count = enderCrystalExplosionCount++;
    LeaveCriticalSection(&entityLimitMutex);
    return count < 30;
}

void ServerLevel::flagEntitiesToBeRemoved(unsigned int* flags, bool* removedFound)
{
    if (chunkMap != NULL)
        chunkMap->flagEntitiesToBeRemoved(flags, removedFound);
}

Scoreboard* ServerLevel::GetScoreboard()
{
    return serverScoreboard;
}

ServerScoreboard* ServerLevel::GetServerScoreboard()
{
    return serverScoreboard;
}

void ServerLevel::invalidateChunksOutsideArea(int minChunkX, int minChunkZ, int maxChunkX, int maxChunkZ)
{
    static_cast<ServerChunkCache*>(getChunkSource())
        ->invalidateChunksOutsideArea(minChunkX, minChunkZ, maxChunkX, maxChunkZ);
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
        (*it)->invalidateChunksOutsideArea(minChunkX, minChunkZ, maxChunkX, maxChunkZ);
}

void ServerLevel::blockEntityAddedExtra(boost::shared_ptr<BlockEntity> blockEntity)
{
    if (eTYPE_DERIVED_FROM(BlockEntityType::BANNER, blockEntity->GetType()))
    {
        EnterCriticalSection(&blockEntityLimitMutex);
        ++bannerCount;
        LeaveCriticalSection(&blockEntityLimitMutex);
    }
    if (eTYPE_DERIVED_FROM(BlockEntityType::BEACON, blockEntity->GetType()))
    {
        EnterCriticalSection(&blockEntityLimitMutex);
        ++beaconCount;
        LeaveCriticalSection(&blockEntityLimitMutex);
    }
}

void ServerLevel::blockEntityRemovedExtra(boost::shared_ptr<BlockEntity> blockEntity)
{
    if (eTYPE_DERIVED_FROM(BlockEntityType::BANNER, blockEntity->GetType()))
    {
        EnterCriticalSection(&blockEntityLimitMutex);
        --bannerCount;
        LeaveCriticalSection(&blockEntityLimitMutex);
    }
    if (eTYPE_DERIVED_FROM(BlockEntityType::BEACON, blockEntity->GetType()))
    {
        EnterCriticalSection(&blockEntityLimitMutex);
        --beaconCount;
        LeaveCriticalSection(&blockEntityLimitMutex);
    }
}
