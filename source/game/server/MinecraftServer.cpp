#include "game/server/MinecraftServer.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>
#include <cstdlib>
#include <vector>

#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/core/Compression.hpp"
#include "game/core/ConsoleInput.hpp"
#include "game/core/EServerSetting.hpp"
#include "game/core/FileEntry.hpp"
#include "game/core/FillTool.hpp"
#include "game/core/GameRuleManager.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/Settings.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/core/Socket.hpp"
#include "game/core/System.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/ConsoleSaveFileOriginal.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcSaveFileSplit.hpp"
#include "platform/pc/PcModContent.hpp"
#include "platform/pc/PcNativeMods.hpp"
#include "platform/pc/PcScripts.hpp"
#include "platform/pc/PcVoice.hpp"
#endif
#include "game/data/ConsoleSaveFileReadOnly.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/data/McRegionLevelStorage.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/data/SparseLightStorage.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundChangeDifficultyPacket.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ClientboundSetTimePacket.hpp"
#include "game/network/ServerConnection.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/ServerSettingsChangedPacket.hpp"
#include "game/network/UpdateProgressPacket.hpp"
#include "game/server/DerivedServerLevel.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerCommandDispatcher.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcDebugSchematic.hpp"
#endif
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelSettings.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/PlatformTiming.hpp"
#include "vendor/DiskSaveBuffer.hpp"
#include "vendor/_NetworkGameInitData.hpp"

extern CConsoleMinecraftApp gApp;
extern C4JStorage* gStorage;

MinecraftServer* gMinecraftServer = 0;

bool MinecraftServer::s_primaryPlayerSignedOut = false;
bool MinecraftServer::s_haltRequested = false;
bool MinecraftServer::s_saveOnExitAnswered = false;
bool MinecraftServer::s_setTimeAtEndOfTick = false;
long long MinecraftServer::s_setTime = 0;
bool MinecraftServer::s_setDayTimeAtEndOfTick = false;
long long MinecraftServer::s_setDayTime = 0;
long long MinecraftServer::s_chunkPacketTickStart = 0;
bool MinecraftServer::s_chunkPacketBudgetSpent = false;
std::vector<INetworkPlayer*> MinecraftServer::s_chunkPacketRecipients;
boost::unordered_map<std::wstring, int> MinecraftServer::s_countdownTimers;

MinecraftServer::MinecraftServer()
{
    mPlayers = 0;
    tickCount = 0;
    shuttingDown = false;
    unusedValue0 = 0;
    initialised = false;
    mSettings = 0;
    running = true;
    mConnection = 0;
    progress = 0;
    motd = L"";
    serverPaused = false;
    serverPauseTime = 0;
    serverPausedEvent = new C4JEventImpl(C4JEvent::Mode0);
    maxBuildHeight = 256;
    playerIdleTimeout = 0;
    forceGameType = true;
    defaultGameType = 0;
    ugcPlayersVersion = 0;
    texturePackId = 0;
    postProcessThread = 0;
    suspending = false;
    saveOnExit = false;
    commandDispatcher = new ServerCommandDispatcher();
    for (int i = 0; i < 8; ++i)
    {
        schematicFiles[i] = new ConsoleSchematicFile();
        fillTools[i] = new FillTool(schematicFiles[i]);
    }
}

MinecraftServer::~MinecraftServer()
{
    delete serverPausedEvent;
    delete commandDispatcher;
    for (int i = 0; i < 8; ++i)
    {
        delete schematicFiles[i];
        delete fillTools[i];
    }
}

bool MinecraftServer::IsServerPaused() const
{
    return serverPaused;
}
void MinecraftServer::setFlightAllowed(bool allowed)
{
    flightAllowed = allowed;
}
void MinecraftServer::setPlayers(PlayerList* players)
{
    mPlayers = players;
}
void MinecraftServer::setAnimals(bool animals)
{
    this->animals = animals;
}
void MinecraftServer::setNpcsEnabled(bool enabled)
{
    npcsEnabled = enabled;
}
void MinecraftServer::setPvpAllowed(bool allowed)
{
    pvpAllowed = allowed;
}
void MinecraftServer::setMaxBuildHeight(int height)
{
    maxBuildHeight = height;
}
int MinecraftServer::getMaxBuildHeight()
{
    return maxBuildHeight;
}
bool MinecraftServer::isHardcore()
{
    return false;
}

bool MinecraftServer::getSpawnMonsters()
{
    return mSettings->getBoolean(eServerSetting_SpawnMonsters, true);
}

void MinecraftServer::setLevelDifficulty(ServerLevel* level, const Difficulty* difficulty)
{
    if (level->getLevelData()->isHardcore())
    {
        level->getLevelData()->setDifficulty(Difficulty::HARD);
        level->setSpawnSettings(true, true);
    }
    else
    {
        level->getLevelData()->setDifficulty(difficulty);
        level->setSpawnSettings(getSpawnMonsters(), animals);
    }
}

void MinecraftServer::setDifficulty(const Difficulty* difficulty)
{
    for (unsigned int i = 0; i < mLevels.length; ++i)
    {
        ServerLevel* level = mLevels[i];
        if (level != 0)
            setLevelDifficulty(level, difficulty);
    }
    mPlayers->broadcastAll(
        boost::shared_ptr<ClientboundChangeDifficultyPacket>(new ClientboundChangeDifficultyPacket(difficulty, false)));
#ifdef MINECRAFT_PC
    if (gApp.GetGameHostOption(eGameHostOption_Difficulty) != static_cast<unsigned int>(difficulty->getId()))
    {
        gApp.SetGameHostOption(eGameHostOption_Difficulty, difficulty->getId());
        mPlayers->broadcastAll(boost::shared_ptr<Packet>(
            new ServerSettingsChangedPacket(1, gApp.GetGameHostOption(eGameHostOption_All))));
        CGameNetworkManager::sInstance.UpdateAndSetGameSessionData(0);
    }
#endif
}

int MinecraftServer::runPostUpdate(void* argument)
{
    static int postProcessEventCount = 0;
    MinecraftServer* server = static_cast<MinecraftServer*>(argument);

    ShutdownManager::HasStarted(ShutdownManager::eThreadId_ServerPostUpdate);
    Entity::useSmallIds();
    IntCache::CreateNewThreadStorage();
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    phys::Vec2::CreateNewThreadStorage();
    Compression::UseDefaultThreadStorage();
    Level::enableLightingCache();
    Block::CreateNewThreadStorage();
    LevelChunk::CreateNewThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_ServerPostUpdate);

    do
    {
        EnterCriticalSection(&server->postProcessCritSec);
        if (server->postProcessRequests.size() == 0)
        {
            LeaveCriticalSection(&server->postProcessCritSec);
        }
        else
        {
            postProcessRequest request = server->postProcessRequests.back();
            server->postProcessRequests.pop_back();
            LeaveCriticalSection(&server->postProcessCritSec);
            PIXBeginNamedEvent(0.0f, "Post processing %d ", (postProcessEventCount++) % 8);
#ifdef MINECRAFT_PC
            ServerChunkCache* cache = dynamic_cast<ServerChunkCache*>(request.chunkSource);
            if (cache != 0)
                cache->PcLockGeneration();
#endif
            LevelChunk* chunk = request.chunkSource->getChunk(request.x, request.z);
            if (chunk != 0)
                chunk->doPostProcess(request.chunkSource, request.chunkGenerator);
#ifdef MINECRAFT_PC
            if (cache != 0)
                cache->PcUnlockGeneration();
#endif
            PIXEndNamedEvent();
        }
        Sleep(1);
    } while (!server->postProcessTerminating
             && ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ServerPostUpdate));

    while (true)
    {
        EnterCriticalSection(&server->postProcessCritSec);
        if (server->postProcessRequests.size() == 0
            || !ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ServerPostUpdate))
            break;
        postProcessRequest request = server->postProcessRequests.back();
        server->postProcessRequests.pop_back();
        LeaveCriticalSection(&server->postProcessCritSec);
        LevelChunk* chunk = request.chunkSource->getChunk(request.x, request.z);
        if (chunk != 0)
            chunk->doPostProcess(request.chunkSource, request.chunkGenerator);
    }
    LeaveCriticalSection(&server->postProcessCritSec);

    Block::ReleaseThreadStorage();
    IntCache::ReleaseThreadStorage();
    AABB::ReleaseThreadStorage();
    Vec3::ReleaseThreadStorage();
    phys::Vec2::ReleaseThreadStorage();
    Level::destroyLightingCache();
    ElementStorage::ReleaseThreadStorage();
    LevelChunk::ReleaseThreadStorage();
    ShutdownManager::HasFinished(ShutdownManager::eThreadId_ServerPostUpdate);
    return 0;
}

void MinecraftServer::postProcessTerminate(ProgressRenderer* progressRenderer)
{
    EnterCriticalSection(&gMinecraftServer->postProcessCritSec);
    unsigned int initialRequests = gMinecraftServer->postProcessRequests.size();
    LeaveCriticalSection(&gMinecraftServer->postProcessCritSec);

    while (postProcessThread->WaitForCompletion(50) == C4JEvent::WaitTimeout)
    {
        EnterCriticalSection(&gMinecraftServer->postProcessCritSec);
        unsigned int remainingRequests = gMinecraftServer->postProcessRequests.size();
        LeaveCriticalSection(&gMinecraftServer->postProcessCritSec);
        if (initialRequests != 0)
            progressRenderer->progressStagePercentage((initialRequests - remainingRequests) * 100 / initialRequests);
        CompressedBlockStorage::tick();
        SparseLightStorage::tick();
        SparseDataStorage::tick();
    }
    delete postProcessThread;
    postProcessThread = 0;
    DeleteCriticalSection(&postProcessCritSec);
}

ServerLevel* MinecraftServer::getLevel(int dimension)
{
    if (dimension == -1)
        return mLevels.data[1];
    if (dimension == 1)
        return mLevels.data[2];
    return mLevels.data[0];
}

void MinecraftServer::saveGameRules()
{
    unsigned char* data = 0;
    unsigned int size = 0;
    gApp.mGameRuleManager.saveGameRules(&data, &size);
    if (data == 0)
        return;

    ConsoleSaveFile* saveFile = getLevel(0)->getLevelStorage()->getSaveFile();
    FileEntry* entry = saveFile->createFile(ConsoleSavePath(std::wstring(L"requiredGameRules.grf")));
    saveFile->setFilePointer(entry, 0, 0, 0);
    unsigned int written;
    saveFile->writeFile(entry, data, size, &written);
    delete[] data;
    saveFile->closeHandle(entry);
}

void MinecraftServer::endProgress()
{
    progressName = L"";
    progress = 0;
}

bool MinecraftServer::loadLevel(LevelStorageSource* storageSource, const std::wstring& name, long long seed,
                                LevelType* levelType, _NetworkGameInitData* initData)
{
    static int createEventCount = 0;
    ProgressRenderer* progressRenderer = Minecraft::GetInstance()->mProgressRenderer;

    mLevels = arrayWithLength<ServerLevel*>(3, true);
    defaultGameType = LevelSettings::validateGameType(mSettings->getInt(eServerSetting_GameMode, initData->m_gameType));

    boost::shared_ptr<McRegionLevelStorage> storage;
    DiskSaveBuffer* saveData = initData->m_saveData;
    if (saveData != 0)
    {
#ifdef MINECRAFT_PC
        LevelGenerationOptions* levelGen = gApp.getLevelGenerationOptions();
        ConsoleSaveFile* saveFile;
        if (levelGen == 0 || !levelGen->requiresBaseSave())
            saveFile = new PcSaveFileSplit(saveData->name, saveData->data, saveData->length,
                                           static_cast<ESavePlatform>(initData->m_savePlatform));
        else
            saveFile = new ConsoleSaveFileOriginal(saveData->name, saveData->data, saveData->length, false,
                                                   static_cast<ESavePlatform>(initData->m_savePlatform));
#else
        ConsoleSaveFile* saveFile = new ConsoleSaveFileOriginal(saveData->name, saveData->data, saveData->length, false,
                                                                static_cast<ESavePlatform>(initData->m_savePlatform));
#endif
        saveFile->isSaveEndianDifferent();
        saveFile->ConvertToLocalPlatform();
        storage = boost::shared_ptr<McRegionLevelStorage>(new McRegionLevelStorage(
            saveFile, File(std::wstring(L".")), name, true, Minecraft::GetInstance()->getFixerUpper()));
    }
    else
    {
        ConsoleSaveFile* saveFile;
        LevelGenerationOptions* levelGen = gApp.getLevelGenerationOptions();
        unsigned int baseSaveSize = 0;
        if (levelGen != 0 && levelGen->requiresBaseSave() && levelGen->getBaseSaveData(baseSaveSize) != 0
            && baseSaveSize != 0 && levelGen->getRequiredGameRules() != 0 && levelGen->isLocal()
            && !levelGen->isTutorial()
            && (levelGen->isGameModeLobby() || levelGen->getMapSize() != static_cast<unsigned int>(-1)))
        {
            saveFile = new ConsoleSaveFileReadOnly(std::wstring(L""), 0, 0, false, kLocalSavePlatform);
        }
#ifdef MINECRAFT_PC
        else if (levelGen == 0 || !levelGen->requiresBaseSave())
        {
            saveFile = new PcSaveFileSplit(std::wstring(L""), 0, 0, kLocalSavePlatform);
        }
#endif
        else
        {
            saveFile = new ConsoleSaveFileOriginal(std::wstring(L""), 0, 0, false, kLocalSavePlatform);
        }
        storage = boost::shared_ptr<McRegionLevelStorage>(new McRegionLevelStorage(
            saveFile, File(std::wstring(L".")), name, true, Minecraft::GetInstance()->getFixerUpper()));
    }

    LevelData* levelData = storage->prepareLevel();
    LevelSettings* levelSettings;
    if (levelData == 0)
    {
        levelSettings = new LevelSettings(
            seed, defaultGameType, gApp.GetGameHostOption(eGameHostOption_Structures) != 0, isHardcore(), true,
            levelType, initData->m_xzSize, initData->m_hellScale, 0x80, true, initData->m_biomeScale,
            initData->m_biomeCentreXChunk, initData->m_biomeCentreZChunk);
        if (gApp.GetGameHostOption(eGameHostOption_BonusChest) != 0)
            levelSettings->enableStartingBonusItems();
        if (initData->m_levelTypeOptions != 0)
            levelSettings->setLevelTypeOptions(initData->m_levelTypeOptions);
        levelData = new LevelData(levelSettings, name);
    }
    else
    {
        levelData->setLevelName(name);
        levelSettings = new LevelSettings(levelData);
    }
#ifdef MINECRAFT_PC
    PcModContentBeginWorld(levelData);
#endif

    ConsoleSavePath gameRulesPath(std::wstring(L"requiredGameRules.grf"));
    ConsoleSaveFile* saveFile = storage->getSaveFile();
    if (saveFile->doesFileExist(gameRulesPath))
    {
        FileEntry* entry = saveFile->createFile(gameRulesPath);
        unsigned int size = entry->m_data.m_fileSize;
        arrayWithLength<unsigned char> ruleData(size, true);
        saveFile->setFilePointer(entry, 0, 0, 0);
        unsigned int read;
        saveFile->readFile(entry, ruleData.data, size, &read);
        gApp.mGameRuleManager.loadGameRules(ruleData.data, size);
        saveFile->closeHandle(entry);
        if (saveFile->getSaveVersion() < 10)
        {
            BlockPos* spawnPos = gApp.mGameRuleManager.getLevelGenerationOptions()->getSpawnPos();
            if (levelSettings->getSeed() == 0x32f2d7ff && spawnPos->x == -1 && spawnPos->y == 0x60
                && spawnPos->z == 0x89)
                gApp.SetResetEnd(true);
        }
    }

    for (unsigned int i = 0; i < mLevels.length; ++i)
    {
        if (s_haltRequested || !CGameNetworkManager::sInstance.IsInSession())
        {
            delete levelSettings;
            return false;
        }

        int dimension = 0;
        if (i == 1)
            dimension = -1;
        else if (i == 2)
            dimension = 1;

        bool inMiniGame;
        if (i == 0)
        {
            mLevels.data[i] = static_cast<ServerLevel*>(
                (new ServerLevel(this, storage, levelData, 0))->postConstruct());
            setLevelDifficulty(mLevels.data[i], Difficulty::byId(gApp.GetGameHostOption(eGameHostOption_Difficulty)));
            if (gApp.getLevelGenerationOptions() != 0)
            {
                LevelGenerationOptions* levelGen = gApp.getLevelGenerationOptions();
                BlockPos* spawnPos = levelGen->getSpawnPos();
                if (spawnPos != 0)
                    mLevels.data[i]->setSpawnPos(*spawnPos);
                mLevels.data[i]->getLevelData()->setHasBeenInCreative(
                    mLevels.data[i]->getLevelData()->getHasBeenInCreative() || levelGen->isFromDLC());
                mLevels.data[i]->getLevelData()->setCloudHeight(levelGen->getCloudHeight());
                mLevels.data[i]->setFogDistance(levelGen->getFogDistance());
                mLevels.data[i]->initializeLevel(levelSettings);
            }
            else
            {
                if (initData->m_levelTypeOptions != 0)
                    mLevels.data[i]->getLevelData()->setHasBeenInCreative(true);
                mLevels.data[i]->initializeLevel(levelSettings);
            }
            inMiniGame = Minecraft::InMiniGame(eMiniGameId_Any, true);
        }
        else
        {
            mLevels.data[i] = static_cast<ServerLevel*>(
                (new DerivedServerLevel(this, storage, dimension, mLevels.data[0]))->postConstruct());
            setLevelDifficulty(mLevels.data[i], Difficulty::byId(gApp.GetGameHostOption(eGameHostOption_Difficulty)));
            inMiniGame = Minecraft::InMiniGame(eMiniGameId_Any, true);
        }

        if (inMiniGame)
            mMasterGameMode->OnLevelLoad(mLevels.data[i]);
        mLevels.data[i]->getLevelData()->setGameType(defaultGameType);
        if (gApp.getLevelGenerationOptions() != 0)
        {
            LevelGenerationOptions* levelGen = gApp.getLevelGenerationOptions();
            if (levelGen->getSrc() != LevelGenerationOptions::eSrc_Save)
                mLevels.data[i]->getLevelData()->setHasBeenInCreative(levelGen->getLevelHasBeenInCreative());
        }
    }
    delete levelSettings;

    mPlayers->setLevel(mLevels);
    setDifficulty(Difficulty::byId(gApp.GetGameHostOption(eGameHostOption_Difficulty)));

    if (mLevels.data[0]->mIsNew)
        progressRenderer->progressStage(IDS_PROGRESS_GENERATING_SPAWN_AREA);
    else
        progressRenderer->progressStage(IDS_PROGRESS_LOADING_SPAWN_AREA);
    gApp.SetGameHostOption(eGameHostOption_HasBeenInCreative,
                           defaultGameType == GameType::CREATIVE || mLevels.data[0]->getHasBeenInCreative());
    gApp.SetGameHostOption(eGameHostOption_Structures, mLevels.data[0]->isGenerateMapFeatures());

    if (s_haltRequested || !CGameNetworkManager::sInstance.IsInSession())
        return false;

    InitializeCriticalSection(&postProcessCritSec);
    postProcessThread = new C4JThreadImpl(&MinecraftServer::runPostUpdate, this, "Post processing", 0x40000);
    postProcessTerminating = false;
    postProcessThread->SetProcessor(2);
    postProcessThread->Run();

    const int radius = 196;
    const int span = radius * 2 + 1;
    for (unsigned int i = 0; i < mLevels.length; ++i)
    {
        if (i != 0 && !mSettings->getBoolean(eServerSetting_AllowNether, true))
            continue;

        ServerLevel* level = mLevels.data[i];
        long long lastStorageTick = System::processTimeInMilliSecs();
        BlockPos spawnPos = level->getSharedSpawnPos();
        for (int x = -radius; x <= radius && running; x += 16)
        {
            for (int z = -radius; z <= radius && running; z += 16)
            {
                if (s_haltRequested || !CGameNetworkManager::sInstance.IsInSession())
                {
                    postProcessTerminating = true;
                    postProcessTerminate(progressRenderer);
                    return false;
                }
                progressRenderer->progressStagePercentage(((x + radius) * span + (z + radius) + 1) * 100
                                                          / (span * span));
                PIXBeginNamedEvent(0.0f, "Creating %d", (createEventCount++) % 8);
                static_cast<ServerChunkCache*>(level->getChunkSource())
                    ->create((spawnPos.x + x) >> 4, (spawnPos.z + z) >> 4, true);
                PIXEndNamedEvent();
                if (System::processTimeInMilliSecs() - lastStorageTick > 50)
                {
                    CompressedBlockStorage::tick();
                    SparseLightStorage::tick();
                    SparseDataStorage::tick();
                    lastStorageTick = System::processTimeInMilliSecs();
                }
            }
        }
        if (mSettings->getBoolean(eServerSetting_DisableSaving, false))
            level->noSave = true;
    }

    postProcessTerminating = true;
    postProcessTerminate(progressRenderer);

    if (mLevels.data[0]->mDimension->getType() == DimensionType::OVERWORLD)
    {
        int strongholdX;
        int strongholdZ;
        if (!mLevels.data[0]->getLevelData()->getHasStronghold()
            && gApp.GetTerrainFeaturePosition(eTerrainFeatureType_Stronghold, &strongholdX, &strongholdZ))
        {
            mLevels.data[0]->getLevelData()->setXStronghold(strongholdX);
            mLevels.data[0]->getLevelData()->setZStronghold(strongholdZ);
            mLevels.data[0]->getLevelData()->setHasStronghold();
        }
    }

    if (s_haltRequested || !CGameNetworkManager::sInstance.IsInSession())
        return false;
    if (mLevels.data[1]->mIsNew)
        mLevels.data[1]->save(true, progressRenderer, false);

    if (s_haltRequested || !CGameNetworkManager::sInstance.IsInSession())
        return false;
    if (mLevels.data[2]->mIsNew)
        mLevels.data[2]->save(true, progressRenderer, false);

    if (s_haltRequested || !CGameNetworkManager::sInstance.IsInSession())
        return false;
    if (mLevels.data[0]->mIsNew)
    {
        saveGameRules();
        if (mLevels.data[0]->mIsNew)
            mLevels.data[0]->save(true, progressRenderer, false);
    }

    if (s_haltRequested || !CGameNetworkManager::sInstance.IsInSession())
        return false;
    if (mLevels.data[0]->mIsNew || mLevels.data[1]->mIsNew || mLevels.data[2]->mIsNew)
        mLevels.data[0]->saveToDisc(progressRenderer, false);

    if (s_haltRequested || !CGameNetworkManager::sInstance.IsInSession())
        return false;
    endProgress();
    return true;
}

bool MinecraftServer::initServer(long long seed, _NetworkGameInitData* initData, unsigned int hostSettings,
                                 bool findSeed)
{
    mSettings = new Settings();
    setFlightAllowed(true);
    mConnection = new ServerConnection(this);
    Socket::Initialise(mConnection);

    while (gApp.getLevelGenerationOptions() != 0 && !gApp.getLevelGenerationOptions()->hasLoadedData())
        Sleep(1);
    if (gApp.getLevelGenerationOptions() != 0)
        gApp.getLevelGenerationOptions()->ready();

    if (gApp.getGameRuleDefinitions() != 0 && initData->m_levelGenerationOptions != 0
        && initData->m_levelGenerationOptions->isGameModeLobby())
    {
        mMasterGameMode = new MasterGameMode(&initData->m_ruleGroups);
        mMasterGameMode->SetupServerSettings(mSettings);
        setPlayers(new PlayerList(this, mMasterGameMode));
        setAnimals(mSettings->getBoolean(eServerSetting_SpawnAnimals, true));
        setNpcsEnabled(mSettings->getBoolean(eServerSetting_SpawnNpcs, true));
        setPvpAllowed(mSettings->getBoolean(eServerSetting_Pvp, gApp.GetGameHostOption(eGameHostOption_PvP) != 0));
    }
    else
    {
        mMasterGameMode = 0;
        setPlayers(new PlayerList(this, mMasterGameMode));
        setAnimals(mSettings->getBoolean(eServerSetting_SpawnAnimals, true));
        setNpcsEnabled(mSettings->getBoolean(eServerSetting_SpawnNpcs, true));
        setPvpAllowed(mSettings->getBoolean(eServerSetting_Pvp, gApp.GetGameHostOption(eGameHostOption_PvP) != 0));
    }

    std::wstring levelName = mSettings->getString(eServerSetting_LevelName, std::wstring(L"world"));
    std::wstring levelTypeName = L"";
    if ((gApp.getLevelGenerationOptions() != 0 && gApp.getLevelGenerationOptions()->getuseFlatWorld())
        || gApp.GetGameHostOption(eGameHostOption_LevelType) > 0)
    {
        levelTypeName = mSettings->getString(eServerSetting_LevelType, std::wstring(L"flat"));
    }
    else
    {
        levelTypeName = mSettings->getString(eServerSetting_LevelType, std::wstring(L"default"));
    }
    LevelType* levelType = LevelType::getLevelType(levelTypeName);
    if (initData->m_levelType != NULL)
    {
        levelType = initData->m_levelType;
    }
    else if (levelType == 0)
    {
        levelType = LevelType::DEFAULT;
    }
    Minecraft::GetInstance()->mProgressRenderer->progressStart(static_cast<int>(0xf990f518));

    if (findSeed)
        seed = BiomeSource::findSeed(levelType);

    setMaxBuildHeight(mSettings->getInt(eServerSetting_MaxBuildHeight, 256));
    setMaxBuildHeight((getMaxBuildHeight() + 8) / 16 * 16);
    setMaxBuildHeight(Mth::clamp(getMaxBuildHeight(), 64, 256));

    if (initData->m_recordLevelResume)
    {
        int saveUniqueNumber = 0;
        gStorage->GetSaveUniqueNumber(&saveUniqueNumber);
        telemetryManagerInstance->RecordLevelResume(
            gProfile->GetPrimaryPad(), eSen_FriendOrMatch_Friend, eSen_CompeteOrCoop_Both,
            GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_Difficulty),
            gApp.GetLocalPlayerCount(), CGameNetworkManager::sInstance.GetOnlinePlayerCount(), 0);
    }

    initialised = loadLevel(0, levelName, seed, levelType, initData);
    if (initData->m_saveData != 0)
    {
        std::free(initData->m_saveData->data);
        initData->m_saveData->data = 0;
        initData->m_saveData->length = 0;
    }
    CGameNetworkManager::sInstance.ServerReady();
    return initialised;
}

void MinecraftServer::saveAllChunks()
{
    for (unsigned int i = 0; i < mLevels.length; ++i)
    {
        if (s_primaryPlayerSignedOut)
            return;
        ServerLevel* level = mLevels.data[mLevels.length - 1 - i];
        if (level != 0)
        {
            level->save(true, Minecraft::GetInstance()->mProgressRenderer, false);
            if (i == mLevels.length - 1)
                level->closeLevelStorage();
        }
    }
}

void MinecraftServer::stopServer(bool didInit)
{
#ifdef MINECRAFT_PC
    PcScriptsHostStop();
    PcNativeModsHostStop();
#endif
    Minecraft::GetInstance()->mGameRenderer->DisableUpdateThread(0);
    mConnection->stop();

    if (!s_primaryPlayerSignedOut && gProfile->IsSignedIn(gProfile->GetPrimaryPad()) && saveOnExit
        && gProfile->IsFullVersion() && !gStorage->GetSaveDisabled() && didInit)
    {
        if (mPlayers != 0)
        {
            mPlayers->saveAll(Minecraft::GetInstance()->mProgressRenderer, true);
            mPlayers->removeAll();
        }
        saveAllChunks();
        saveGameRules();
        gApp.mGameRuleManager.unloadCurrentGameRules();
        if (mLevels.data[0] != 0)
            mLevels.data[0]->saveToDisc(Minecraft::GetInstance()->mProgressRenderer, false);
    }

    s_primaryPlayerSignedOut = false;
    s_haltRequested = false;
    if (mMasterGameMode != 0)
    {
        mMasterGameMode->OnExitedGame();
        delete mMasterGameMode;
        mMasterGameMode = 0;
    }

    delete mConnection;
    mConnection = 0;
    delete mPlayers;
    mPlayers = 0;
    unsigned int levelCount = mLevels.length;
    for (unsigned int i = 0; i < levelCount; ++i)
    {
        ServerLevel* level = mLevels.data[i];
        if (level != 0)
        {
            delete level;
            mLevels.data[i] = 0;
        }
    }
    delete mSettings;
    mSettings = 0;

    SparseLightStorage::tick();
    SparseLightStorage::tick();
    SparseLightStorage::tick();
#ifdef MINECRAFT_PC
    PcModContentEndWorld();
#endif
    CGameNetworkManager::sInstance.ServerStopped();
}

PlayerList* MinecraftServer::getPlayers()
{
    return mPlayers;
}

void MinecraftServer::setDefaultGameMode(const GameType* gameType)
{
    defaultGameType = gameType;
    for (unsigned int i = 0; i < mLevels.length; ++i)
        gMinecraftServer->mLevels.data[i]->getLevelData()->setGameType(gameType);
}

const GameType* MinecraftServer::getDefaultGameType()
{
    return defaultGameType;
}
ServerConnection* MinecraftServer::getConnection()
{
    return mConnection;
}
bool MinecraftServer::isAnimals()
{
    return animals;
}
bool MinecraftServer::isNpcsEnabled()
{
    return npcsEnabled;
}
bool MinecraftServer::isPvpAllowed()
{
    return pvpAllowed;
}
bool MinecraftServer::isFlightAllowed()
{
    return flightAllowed;
}
bool MinecraftServer::isCommandBlockEnabled()
{
    return false;
}
bool MinecraftServer::isNetherEnabled()
{
    return true;
}
CommandDispatcher* MinecraftServer::getCommandDispatcher()
{
    return commandDispatcher;
}
int MinecraftServer::getSpawnProtectionRadius()
{
    return 16;
}

bool MinecraftServer::isUnderSpawnProtection(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player)
{
    if (level->mDimension->getType() != DimensionType::OVERWORLD)
        return false;
    if (getPlayers()->isOp(boost::dynamic_pointer_cast<ServerPlayer>(player)))
        return false;
    if (getSpawnProtectionRadius() <= 0)
        return false;

    BlockPos spawnPos = level->getSharedSpawnPos();
    int xDistance = Mth::abs(pos.x - spawnPos.x);
    int zDistance = Mth::abs(pos.z - spawnPos.z);
    int distance = xDistance > zDistance ? xDistance : zDistance;
    return distance <= getSpawnProtectionRadius();
}

bool MinecraftServer::getForceGameType()
{
    return forceGameType;
}

unsigned long long MinecraftServer::getCurrentTimeMillis()
{
    return System::processTimeInMilliSecs();
}

int MinecraftServer::getSpawnRadius(ServerLevel* level)
{
    if (level == 0)
        return 10;
    return level->getGameRules()->getInt(GameRules::RULE_SPAWN_RADIUS);
}

void MinecraftServer::halt()
{
    running = false;
}

void MinecraftServer::fjSetServerPaused(bool paused)
{
    if (serverPaused == paused)
        return;
    serverPaused = paused;
    if (paused)
    {
        serverPauseTime = System::processTimeInMilliSecs();
        serverPausedEvent->Set();
    }
    else if (mMasterGameMode != 0)
    {
        long long pausedTime = System::processTimeInMilliSecs() - serverPauseTime;
        mMasterGameMode->OnServerResume(pausedTime);
    }
}

void MinecraftServer::tick()
{
    std::vector<std::wstring> expiredTimers;
    for (boost::unordered_map<std::wstring, int>::iterator it = s_countdownTimers.begin();
         it != s_countdownTimers.end(); it++)
    {
        int remaining = it->second;
        if (remaining > 0)
            s_countdownTimers[it->first] = remaining - 1;
        else
            expiredTimers.push_back(it->first);
    }
    for (unsigned int i = 0; i < expiredTimers.size(); ++i)
        s_countdownTimers.erase(expiredTimers[i]);

    AABB::resetPool();
    Vec3::resetPool();

    ++tickCount;
    for (unsigned int i = 0; i < mLevels.length; ++i)
    {
        ServerLevel* level = mLevels.data[i];
        if (tickCount % 20 == 0)
        {
            if (level->getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT) || level->ShouldForceTimeUpdate())
            {
                mPlayers->broadcastAll(boost::shared_ptr<ClientboundSetTimePacket>(new ClientboundSetTimePacket(
                                           level->getGameTime(), level->getDayTime(),
                                           level->getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT))),
                                       level->mDimension->getType()->getId());
            }
        }

        PIXBeginNamedEvent(0.0f, "Level tick %d", i);
        level->tick();
        PIXEndNamedEvent();

        PIXBeginNamedEvent(0.0f, "Entity tick %d", i);
        if (mPlayers->getPlayerCount(level) > 0 || level->hasEntitiesToRemove())
        {
            long long entityTickStart = System::processTimeInMilliSecs();
            level->tickEntities();
            long long entityTickEnd = System::processTimeInMilliSecs();
            if (level->mDimension->getType() == DimensionType::OVERWORLD)
                PIXAddNamedCounter(static_cast<float>(entityTickEnd - entityTickStart), "Tick entities");
        }
        PIXEndNamedEvent();

        PIXBeginNamedEvent(0.0f, "Entity tracker tick");
        level->getTracker()->tick(boost::shared_ptr<ServerPlayer>());
        PIXEndNamedEvent();
    }
    Entity::tickExtraWandering();

    PIXBeginNamedEvent(0.0f, "Connection tick");
    mConnection->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Players tick");
    mPlayers->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Game Mode tick");
    if (mMasterGameMode != 0)
        mMasterGameMode->Tick();
    PIXEndNamedEvent();
#ifdef MINECRAFT_PC
    PcScriptsHostTick();
    PcNativeModsHostTick();
    PcVoiceServerTick();
#endif
}

void MinecraftServer::chunkPacketManagement_PreTick()
{
    s_chunkPacketBudgetSpent = false;
    s_chunkPacketTickStart = System::processTimeInMilliSecs();
    s_chunkPacketRecipients.clear();

    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >& players = mConnection->getPlayers();
    if (players.size() == 0)
        return;

    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> > unsorted(players);
    players.clear();
    do
    {
        int longestWait = 0;
        std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >::iterator longest = unsorted.begin();
        for (std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >::iterator it = unsorted.begin();
             it != unsorted.end(); ++it)
        {
            int wait = 0;
            INetworkPlayer* networkPlayer = (*it)->getNetworkPlayer();
            if (networkPlayer != 0)
                wait = networkPlayer->GetTimeSinceLastChunkPacket_ms();
            if (wait > longestWait)
            {
                longestWait = wait;
                longest = it;
            }
        }
        players.push_back(*longest);
        unsorted.erase(longest);
    } while (unsorted.size() != 0);
}

void MinecraftServer::run(long long seed, void* argument)
{
    _NetworkGameInitData* initData = static_cast<_NetworkGameInitData*>(argument);
    unsigned int hostSettings = 0;
    bool findSeed = false;
    if (initData != 0)
    {
        hostSettings = gApp.GetGameHostOption(eGameHostOption_All);
        findSeed = initData->m_findSeed;
        texturePackId = initData->m_texturePackId;
    }

    bool didInit = false;
    if (initServer(seed, initData, hostSettings, findSeed))
    {
        didInit = true;

        LevelData* levelData = mLevels.data[0]->getLevelData();
        int strongholdX;
        int strongholdZ;
        if (levelData != 0 && !levelData->getHasStronghold()
            && gApp.GetTerrainFeaturePosition(eTerrainFeatureType_Stronghold, &strongholdX, &strongholdZ))
        {
            levelData->setXStronghold(strongholdX);
            levelData->setZStronghold(strongholdZ);
            levelData->setHasStronghold();
        }

        if (GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(0x1d), -1))
            fjSetServerPaused(true);

#ifdef MINECRAFT_PC
        PcScriptsHostStart();
        PcNativeModsHostStart();
#endif
        long long lastTime = getCurrentTimeMillis();
        long long unprocessedTime = 0;
        while (running && !s_haltRequested)
        {
            long long now = getCurrentTimeMillis();
            long long passedTime = now - lastTime;
            if (passedTime > 2000)
            {
                unprocessedTime += 2000;
            }
            else
            {
                if (passedTime < 0)
                    passedTime = 0;
                unprocessedTime += passedTime;
            }
            lastTime = now;

            if (!serverPaused)
            {
                if (mLevels.data[0]->allPlayersAreSleeping())
                {
                    tick();
                    unprocessedTime = 0;
                }
                else
                {
                    while (unprocessedTime > 50)
                    {
                        unprocessedTime -= 50;
                        chunkPacketManagement_PreTick();
                        PIXBeginNamedEvent(0.0f, "Server tick");
                        tick();
                        PIXEndNamedEvent();
                    }
                }
            }
            else
            {
                while (unprocessedTime > 50)
                {
                    unprocessedTime -= 50;
                    mConnection->tick();
                }
            }

            if (s_setTimeAtEndOfTick)
            {
                s_setTimeAtEndOfTick = false;
                for (unsigned int i = 0; i < mLevels.length; ++i)
                    mLevels.data[i]->setGameTime(s_setTime);
            }
            if (s_setDayTimeAtEndOfTick)
            {
                s_setDayTimeAtEndOfTick = false;
                for (unsigned int i = 0; i < mLevels.length; ++i)
                {
                    if (i == 0 || mSettings->getBoolean(eServerSetting_AllowNether, true))
                        mLevels.data[i]->setDayTime(s_setDayTime);
                }
            }

            for (int pad = 0; pad < 4; ++pad)
            {
                eXuiServerAction action = static_cast<eXuiServerAction>(gApp.GetXuiServerAction(pad));
                void* param = gApp.GetXuiServerActionParam(pad);
                switch (action)
                {
                case eXuiServerAction_SaveGame:
                case eXuiServerAction_AutoSaveGame:
                {
                    gApp.EnterSaveNotificationSection();
                    if (mPlayers != 0)
                        mPlayers->saveAll(Minecraft::GetInstance()->mProgressRenderer, false);
                    mPlayers->broadcastAll(boost::shared_ptr<UpdateProgressPacket>(new UpdateProgressPacket(20)));
                    if (!s_haltRequested)
                    {
                        int saveProgress = 33;
                        for (unsigned int i = 0; i < mLevels.length && !s_haltRequested; ++i)
                        {
                            mLevels.data[mLevels.length - 1 - i]->save(true,
                                                                       Minecraft::GetInstance()->mProgressRenderer,
                                                                       action == eXuiServerAction_AutoSaveGame);
                            mPlayers->broadcastAll(
                                boost::shared_ptr<UpdateProgressPacket>(new UpdateProgressPacket(saveProgress)));
                            saveProgress += 33;
                        }
                        if (!s_haltRequested)
                        {
                            saveGameRules();
                            mLevels.data[0]->saveToDisc(Minecraft::GetInstance()->mProgressRenderer,
                                                        action == eXuiServerAction_AutoSaveGame);
                        }
                    }
                    gApp.LeaveSaveNotificationSection();
                    break;
                }
                case eXuiServerAction_DropItem:
                {
                    boost::shared_ptr<ServerPlayer> player = mPlayers->mPlayers.at(0);
                    player->drop(
                        not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(static_cast<int>(reinterpret_cast<intptr_t>(param))), 1, 0)),
                        false);
                    break;
                }
                case eXuiServerAction_SpawnMob:
                {
                    boost::shared_ptr<ServerPlayer> player = mPlayers->mPlayers.at(0);
                    boost::shared_ptr<Mob> mob = boost::dynamic_pointer_cast<Mob>(EntityIO::newEntity(
                        static_cast<unsigned long long>(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(param))), player->mLevel));
                    mob->moveTo(player->mX + 1.0, player->mY, player->mZ + 1.0,
                                player->mLevel->mRandom->nextFloat() * 360.0f, 0.0f);
                    mob->setDespawnProtected();
                    player->mLevel->addEntity(mob);
                    break;
                }
                case eXuiServerAction_PauseServer:
                    fjSetServerPaused(reinterpret_cast<intptr_t>(param) == 1);
                    break;
                case eXuiServerAction_ToggleRain:
                {
                    bool raining = mLevels.data[0]->getLevelData()->isRaining();
                    mLevels.data[0]->getLevelData()->setRaining(!raining);
                    mLevels.data[0]->getLevelData()->setRainTime(mLevels.data[0]->mRandom->nextInt(0x29040) + 12000);
                    break;
                }
                case eXuiServerAction_ToggleThunder:
                {
                    bool thundering = mLevels.data[0]->getLevelData()->isThundering();
                    mLevels.data[0]->getLevelData()->setThundering(!thundering);
                    mLevels.data[0]->getLevelData()->setThunderTime(mLevels.data[0]->mRandom->nextInt(0x29040) + 12000);
                    break;
                }
                case eXuiServerAction_SetCameraLocation:
                {
                    const DebugSetCameraPosition* position = static_cast<const DebugSetCameraPosition*>(param);
                    if (position != NULL && !mPlayers->mPlayers.empty())
                    {
                        boost::shared_ptr<ServerPlayer> player = mPlayers->mPlayers.at(0);
                        player->mConnection->teleport(position->m_camX, position->m_camY, position->m_camZ,
                                                      static_cast<float>(position->m_yRot),
                                                      static_cast<float>(position->m_elev), true, true);
                    }
                    delete position;
                    break;
                }
                case eXuiServerAction_FillArea:
                {
                    const DebugFillAreaRequest* request = static_cast<const DebugFillAreaRequest*>(param);
                    if (request != NULL && !mPlayers->mPlayers.empty())
                        DebugFillArea(mPlayers->mPlayers.at(0)->mLevel, *request);
                    delete request;
                    break;
                }
                case eXuiServerAction_UndoFillArea:
                    if (!mPlayers->mPlayers.empty())
                        DebugUndoFillArea(mPlayers->mPlayers.at(0)->mLevel);
                    break;
#ifdef MINECRAFT_PC
                case eXuiServerAction_SchematicCopy:
                    if (!mPlayers->mPlayers.empty())
                        PcDebugSchematicCopy(mPlayers->mPlayers.at(0)->mLevel, *static_cast<PcSchematicCopy*>(param));
                    delete static_cast<PcSchematicCopy*>(param);
                    break;
                case eXuiServerAction_SchematicPaste:
                    if (!mPlayers->mPlayers.empty())
                        PcDebugSchematicPaste(mPlayers->mPlayers.at(0)->mLevel, *static_cast<PcSchematicPaste*>(param));
                    delete static_cast<PcSchematicPaste*>(param);
                    break;
                case eXuiServerAction_SchematicUndo:
                    if (!mPlayers->mPlayers.empty())
                        PcDebugSchematicUndo(mPlayers->mPlayers.at(0)->mLevel);
                    break;
                case eXuiServerAction_SchematicSave:
                    PcDebugSchematicSave(*static_cast<std::wstring*>(param));
                    delete static_cast<std::wstring*>(param);
                    break;
                case eXuiServerAction_SchematicLoad:
                    PcDebugSchematicLoad(*static_cast<std::wstring*>(param));
                    delete static_cast<std::wstring*>(param);
                    break;
#endif
                case eXuiServerAction_ServerSettingChanged:
                    mPlayers->broadcastAll(boost::shared_ptr<ServerSettingsChangedPacket>(
                        new ServerSettingsChangedPacket(0, gApp.GetGameHostOption(eGameHostOption_Gamertags))));
                    break;
                default:
                    break;
                }
                gApp.SetXuiServerAction(pad, eXuiServerAction_Idle, 0);
            }
#ifdef MINECRAFT_PC
            PcDebugSchematicTick();
#endif
            Sleep(1);
        }
    }

    shuttingDown = true;
    stopServer(didInit);
}

namespace
{
struct FilledBlock
{
    BlockPos pos;
    const BlockState* previous;
};
std::vector<FilledBlock> s_fillUndo;
Level* s_fillUndoLevel = NULL;
const long long MAX_FILL_BLOCKS = 32768;
}

void MinecraftServer::DebugFillArea(Level* level, const DebugFillAreaRequest& request)
{
    const int minX = std::min(request.x0, request.x1), maxX = std::max(request.x0, request.x1);
    const int minY = std::max(0, std::min(request.y0, request.y1)), maxY = std::min(255, std::max(request.y0, request.y1));
    const int minZ = std::min(request.z0, request.z1), maxZ = std::max(request.z0, request.z1);
    const long long count = static_cast<long long>(maxX - minX + 1) * (maxY - minY + 1) * (maxZ - minZ + 1);
    if (request.fill == NULL || maxY < minY || count > MAX_FILL_BLOCKS)
    {
        printf("[pc] fill area: refused (%lld blocks; at most %lld)\n", count, MAX_FILL_BLOCKS);
        fflush(stdout);
        return;
    }
    s_fillUndo.clear();
    s_fillUndo.reserve(static_cast<size_t>(count));
    s_fillUndoLevel = level;
    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                const bool onEdge = x == minX || x == maxX || y == minY || y == maxY || z == minZ || z == maxZ;
                const BlockState* state = onEdge && request.edge != NULL ? request.edge : request.fill;
                BlockPos pos(x, y, z);
                FilledBlock saved = {pos, level->getBlockState(pos)};
                s_fillUndo.push_back(saved);
                level->setBlock(pos, state, Block::UPDATE_CLIENTS, false);
            }
        }
    }
    printf("[pc] fill area: %lld blocks from %d %d %d to %d %d %d\n", count, minX, minY, minZ, maxX, maxY, maxZ);
    fflush(stdout);
}

void MinecraftServer::DebugUndoFillArea(Level* level)
{
    if (level != s_fillUndoLevel || s_fillUndo.empty())
        return;
    for (std::vector<FilledBlock>::reverse_iterator it = s_fillUndo.rbegin(); it != s_fillUndo.rend();
         ++it)
        level->setBlock(it->pos, it->previous, Block::UPDATE_CLIENTS, false);
    printf("[pc] fill area: undid %u blocks\n", static_cast<unsigned int>(s_fillUndo.size()));
    fflush(stdout);
    s_fillUndo.clear();
}

void MinecraftServer::broadcastStartSavingPacket()
{
    boost::shared_ptr<ClientboundGameEventPacket> packet(
        new ClientboundGameEventPacket(ClientboundGameEventPacket::START_SAVING, 0.0f));
    mPlayers->broadcastAll(boost::shared_ptr<Packet>(packet));
}

void MinecraftServer::broadcastStopSavingPacket()
{
    if (!s_haltRequested)
        mPlayers->broadcastAll(boost::shared_ptr<ClientboundGameEventPacket>(
            new ClientboundGameEventPacket(ClientboundGameEventPacket::STOP_SAVING, 0.0f)));
}

unsigned long long MinecraftServer::main(long long seed, void* argument)
{
    gMinecraftServer = new MinecraftServer();
    gMinecraftServer->run(seed, argument);
    delete gMinecraftServer;
    gMinecraftServer = 0;
    ShutdownManager::HasFinished(ShutdownManager::eThreadId_ServerMain);
    return 0;
}

void MinecraftServer::HaltServer(bool primaryPlayerSignedOut)
{
    s_haltRequested = true;
    if (gMinecraftServer != 0)
    {
        s_primaryPlayerSignedOut = primaryPlayerSignedOut;
        gMinecraftServer->halt();
    }
}

void MinecraftServer::info(const std::wstring&)
{
}
void MinecraftServer::warn(const std::wstring&)
{
}
std::wstring MinecraftServer::getConsoleName()
{
    return L"CONSOLE";
}

void MinecraftServer::setLevel(int dimension, ServerLevel* level)
{
    if (dimension == -1)
        mLevels.data[1] = level;
    else if (dimension == 1)
        mLevels.data[2] = level;
    else
        mLevels.data[0] = level;
}

bool MinecraftServer::flagEntitiesToBeRemoved(unsigned int* entityIds)
{
    bool removed = false;
    for (unsigned int i = 0; i < mLevels.length; ++i)
    {
        ServerLevel* level = mLevels.data[i];
        if (level != 0)
            level->flagEntitiesToBeRemoved(entityIds, &removed);
    }
    return removed;
}

void MinecraftServer::addPostProcessRequest(ChunkSource* chunkSource, ChunkGenerator* chunkGenerator, int x, int z)
{
    EnterCriticalSection(&postProcessCritSec);
    postProcessRequest request;
    request.x = x;
    request.z = z;
    request.chunkSource = chunkSource;
    request.chunkGenerator = chunkGenerator;
    postProcessRequests.push_back(request);
    LeaveCriticalSection(&postProcessCritSec);
}

bool MinecraftServer::chunkPacketManagement_CanSendTo(INetworkPlayer* player)
{
    if (s_chunkPacketBudgetSpent || player == 0)
        return false;
    for (unsigned int i = 0; i < s_chunkPacketRecipients.size(); ++i)
    {
        if (s_chunkPacketRecipients[i]->IsSameSystem(player))
            return false;
    }
    return player->GetOutstandingAckCount() < 2;
}

#ifdef MINECRAFT_PC
bool MinecraftServer::chunkPacketManagement_CanSendTo(INetworkPlayer* player, int perTick)
{
    if (s_chunkPacketBudgetSpent || player == 0)
        return false;
    int sent = 0;
    for (unsigned int i = 0; i < s_chunkPacketRecipients.size(); ++i)
    {
        if (s_chunkPacketRecipients[i]->IsSameSystem(player))
            ++sent;
    }
    if (sent >= perTick)
        return false;
    return player->GetOutstandingAckCount() < 2 * perTick;
}
#endif

void MinecraftServer::chunkPacketManagement_DidSendTo(INetworkPlayer* player)
{
    if (System::processTimeInMilliSecs() - s_chunkPacketTickStart >= 35)
        s_chunkPacketBudgetSpent = true;
    if (player != 0)
    {
        player->SentChunkPacket();
        s_chunkPacketRecipients.push_back(player);
    }
}

void MinecraftServer::Suspend()
{
    PIXBeginNamedEvent(0.0f, "Suspending server");
    suspending = true;
    _LARGE_INTEGER frequency;
    _LARGE_INTEGER start;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    if (initialised && gProfile->IsFullVersion() && !gStorage->GetSaveDisabled())
    {
        if (mPlayers != 0)
            mPlayers->saveAll(0, false);
        if (!s_haltRequested)
        {
            bool halted = false;
            for (unsigned int i = 0; i < mLevels.length; ++i)
            {
                mLevels.data[mLevels.length - 1 - i]->Suspend();
                if (s_haltRequested)
                {
                    halted = true;
                    break;
                }
            }
            if (!halted)
            {
                saveGameRules();
                mLevels.data[0]->saveToDisc(0, true);
            }
        }
    }
    _LARGE_INTEGER end;
    QueryPerformanceCounter(&end);
    suspending = false;
    PIXEndNamedEvent();
}
