#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/util/Math.hpp"
#include "game/core/GameSettings.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cfloat>
#include <cmath>

#include "game/client/audio/MinecartSoundInstance.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/FireworksParticles__FireworksStarter.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Options.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/MockedLevelStorage.hpp"
#include "game/data/SavedDataStorage.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelSettings.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ClientChunkCache.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/GameType.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"

MultiPlayerLevel::MultiPlayerLevel(boost::shared_ptr<LevelStorage> levelStorage, ClientPacketListener* connection,
                                   LevelSettings* settings, int dimension, const Difficulty* difficulty, int seaLevel)
    : Level(levelStorage, new LevelData(settings, L"MpServer"), DimensionType::getById(dimension)->create(), true)
{
    MultiPlayerLevelInit(connection, settings, difficulty, seaLevel);
}

MultiPlayerLevel::MultiPlayerLevel(ClientPacketListener* connection, LevelSettings* settings, int dimension,
                                   const Difficulty* difficulty, int seaLevel)
    : Level(boost::shared_ptr<MockedLevelStorage>(new MockedLevelStorage()), new LevelData(settings, L"MpServer"),
            DimensionType::getById(dimension)->create(), true)
{
    MultiPlayerLevelInit(connection, settings, difficulty, seaLevel);
}

MultiPlayerLevel::~MultiPlayerLevel()
{
    mSavedDataStorage = NULL;
    delete mClientScoreboard;
}

void MultiPlayerLevel::removeClientPacketListener(ClientPacketListener* connection, bool sendDisconnect)
{
    if (sendDisconnect)
    {
        boost::shared_ptr<Packet> packet(new DisconnectPacket(DisconnectPacket::eDisconnectReason_Quit));
        connection->sendAndDisconnect(packet);
    }

    std::vector<ClientPacketListener*>::iterator found = std::find(mConnections.begin(), mConnections.end(),
                                                                   connection);
    if (found != mConnections.end())
        mConnections.erase(found);
}

ClientScoreboard* MultiPlayerLevel::GetClientScoreboard()
{
    return mClientScoreboard;
}

void MultiPlayerLevel::animateTick(int x, int y, int z)
{
    int sectionX = x >> 3;
    int sectionY = y >> 3;
    int sectionZ = z >> 3;
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dz = -1; dz <= 1; ++dz)
            {
                int sx = sectionX + dx;
                int sy = sectionY + dy;
                int sz = sectionZ + dz;
                if ((unsigned int)(sx + 54) < 108 && (unsigned int)sy < 32 && (unsigned int)(sz + 54) < 108)
                    mAnimatedSections.insert((sx << 20) | ((sy & 255) << 12) | (sz & 4095));
            }
        }
    }
}

void MultiPlayerLevel::animateTickDoWork()
{
    Minecraft::GetInstance()->mAnimateTickLevel = this;
    Random random;
    not_null_ptr<ItemInstance> held = mMinecraft->mPlayer->getMainHandItem();
    bool showBarriers = mMinecraft->mGameMode->GetGameType() == GameType::CREATIVE
                        && Block::byItem(held->getItem()) == Blocks::barrier;

    for (int pass = 0; pass < 16; ++pass)
    {
        if (mAnimatedSections.empty())
        {
            continue;
        }
        for (boost::unordered_set<int>::const_iterator section = mAnimatedSections.begin();
             section != mAnimatedSections.end(); ++section)
        {
            const int packed = *section;
            const int x = (packed >> 20) * 8 + mRandom->nextInt(8);
            const int y = (static_cast<int>(static_cast<unsigned int>(packed) << 12) >> 24) * 8 + mRandom->nextInt(8);
            const int z = (static_cast<int>(static_cast<unsigned int>(packed) << 20) >> 17) + mRandom->nextInt(8);
            const BlockPos pos(x, y, z);
            const BlockState* state = getBlockState(pos);
            state->getBlock()->animateTick(state, this, pos, &random);
            if (showBarriers && state->getBlock() == Blocks::barrier)
            {
                addParticle(PARTICLE_BARRIER, x + 0.5f, y + 0.5f, z + 0.5f, 0.0, 0.0, 0.0, arrayWithLength<int>());
            }
        }
    }

    Minecraft::GetInstance()->mAnimateTickLevel = NULL;
    mAnimatedSections.clear();
}

void MultiPlayerLevel::removeAllPendingEntityRemovals()
{
    EnterCriticalSection(&mEntityMutex);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin(); it != mEntities.end();)
    {
        if (std::find(mEntitiesToRemove.begin(), mEntitiesToRemove.end(), *it) != mEntitiesToRemove.end())
        {
            it = mEntities.erase(it);
        }
        else
        {
            ++it;
        }
    }
    LeaveCriticalSection(&mEntityMutex);

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = mEntitiesToRemove.begin();
         it != mEntitiesToRemove.end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        int chunkX = entity->mChunkX;
        int chunkZ = entity->mChunkZ;
        if (entity->mInChunk && hasChunk(chunkX, chunkZ, true))
        {
            getChunk(chunkX, chunkZ)->removeEntity(entity);
        }
    }

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = mEntitiesToRemove.begin();
         it != mEntitiesToRemove.end(); ++it)
    {
        entityRemoved(*it);
    }
    mEntitiesToRemove.clear();

    EnterCriticalSection(&mEntityMutex);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin(); it != mEntities.end();)
    {
        boost::shared_ptr<Entity> entity = *it;
        boost::shared_ptr<Entity> vehicle = entity->getVehicle();
        if (vehicle != NULL)
        {
            if (!vehicle->mRemoved && vehicle->hasPassenger(entity))
            {
                ++it;
                continue;
            }
            entity->stopRiding();
        }

        if (!entity->mRemoved)
        {
            ++it;
            continue;
        }

        int chunkX = entity->mChunkX;
        int chunkZ = entity->mChunkZ;
        if (entity->mInChunk && hasChunk(chunkX, chunkZ, true))
        {
            getChunk(chunkX, chunkZ)->removeEntity(entity);
        }
        it = mEntities.erase(it);
        entityRemoved(entity);
    }
    LeaveCriticalSection(&mEntityMutex);
}

void MultiPlayerLevel::playLocalSound(const BlockPos& pos, const SoundEvent* sound, SoundSource::ESoundSource source,
                                      float volume, float pitch, bool distanceDelay, float range)
{
    playLocalSound(pos.x + 0.5, pos.y + 0.5, pos.z + 0.5, sound, source, volume, pitch, distanceDelay, range);
}

void MultiPlayerLevel::MultiPlayerLevelInit(ClientPacketListener* connection, LevelSettings* settings,
                                            const Difficulty* difficulty, int seaLevel)
{
    mMinecraft = Minecraft::GetInstance();
    mDimension->init(this);
    if (seaLevel != -1)
        setSeaLevel(seaLevel);
    mChunkSource = createChunkSource();
    mChunkCache = mChunkSource->getCache();
    mChunkCacheSize = mChunkSource->mLevelSize;
    if (!mLevelData->isInitialized())
    {
        initializeLevel(settings);
        mLevelData->setInitialized(true);
    }
    delete settings;
    if (connection != NULL)
        mConnections.push_back(connection);
    getLevelData()->setDifficulty(difficulty);
    delete mSavedDataStorage;
    mSavedDataStorage = NULL;
    mAnimateTickReady = true;
    mAnimateTickX = 0;
    mAnimateTickY = 0;
    mChunkTickX = 0;
    mChunkTickZ = 0;
    mClientScoreboard = new ClientScoreboard();
    updateSkyBrightness();
    prepareWeather();
    mAmbientSoundTime = mRandom->nextInt(12000);
}

void MultiPlayerLevel::tick()
{
    PIXBeginNamedEvent(0.0f, "Sky color changing");
    setGameTime(getGameTime() + 1);
    if (getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT))
    {
        setDayTime(getDayTime() + 1);
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Entity re-entry");
    EnterCriticalSection(&mEntityMutex);
    for (int i = 0; i < 10 && !mPendingEntityAdditions.empty(); ++i)
    {
        boost::shared_ptr<Entity> entity = *mPendingEntityAdditions.begin();
        mPendingEntityAdditions.erase(mPendingEntityAdditions.begin());
        if (std::find(mEntities.begin(), mEntities.end(), entity) == mEntities.end())
        {
            addEntity(entity);
        }
    }
    LeaveCriticalSection(&mEntityMutex);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Connection ticking");
    std::vector<ClientPacketListener*> connections(mConnections);
    for (std::vector<ClientPacketListener*>::iterator it = connections.begin(); it < connections.end(); ++it)
    {
        (*it)->tick();
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Updating resets");
    bool reset = false;
    for (unsigned int i = 0; i < mResetInfos.size(); ++i)
    {
        ResetInfo& info = mResetInfos[i];
        if (--info.delay == 0)
        {
            Level::setBlockAndUpdate(info.pos, info.state);
            Level::sendBlockUpdated(info.pos, NULL, NULL, 3, true);
            --i;
            reset = true;
        }
    }
    if (reset)
    {
        mResetInfos.erase(mResetInfos.begin(), mResetInfos.begin());
    }
    PIXEndNamedEvent();

    mClientChunkCache->tick();
    tickBlocks();

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        return;
    }

    int size = mDimension->getXZSize();
    int halfSize = size / 2;
    for (int i = 0; i < size; ++i)
    {
        if (mChunkTickX + 1 < size)
        {
            ++mChunkTickX;
        }
        else
        {
            mChunkTickX = 0;
            if (mChunkTickZ + 1 < size)
            {
                ++mChunkTickZ;
            }
            else
            {
                mChunkTickZ = 0;
            }
        }

        if (reallyHasChunk(mChunkTickX - halfSize, mChunkTickZ - halfSize))
        {
            LevelChunk* chunk = getChunk(mChunkTickX - halfSize, mChunkTickZ - halfSize);
            chunk->compressLighting();
            chunk->compressBlocks();
            chunk->compressData();
            break;
        }
    }
}

void MultiPlayerLevel::clearResetRegion(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    for (unsigned int i = 0; i < mResetInfos.size(); ++i)
    {
        ResetInfo& info = mResetInfos[i];
        if (info.pos.x >= minX && info.pos.y >= minY && info.pos.z >= minZ && info.pos.x <= maxX && info.pos.y <= maxY
            && info.pos.z <= maxZ)
        {
            mResetInfos.erase(mResetInfos.begin() + i);
            --i;
        }
    }
}

ChunkSource* MultiPlayerLevel::createChunkSource()
{
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || CGameNetworkManager::sInstance.IsHost())
        mClientChunkCache = new ClientChunkCache(this);
    else
        mClientChunkCache = new ClientChunkCache(this, mLevelStorage->createChunkStorage(mDimension));
    return mClientChunkCache;
}

ChunkSource* MultiPlayerLevel::getChunkSource()
{
    return Level::getChunkSource();
}

bool MultiPlayerLevel::hasChunk(int x, int z, bool allowEmpty)
{
    return allowEmpty || !getChunkSource()->getOrCreateChunk(x, z, false)->isEmpty();
}

bool MultiPlayerLevel::hasChunksAt(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, bool allowEmpty)
{
    if (allowEmpty)
        return true;
    return Level::hasChunksAt(minX, minY, minZ, maxX, maxY, maxZ, false);
}

LevelChunk* MultiPlayerLevel::getChunkIfHasChunk(int x, int z, bool allowEmpty)
{
    return hasChunk(x, z, allowEmpty) ? getChunk(x, z) : NULL;
}

void MultiPlayerLevel::validateSpawn()
{
}

void MultiPlayerLevel::tickBlocks()
{
    if (mAmbientSoundTime > 0)
        --mAmbientSoundTime;

    mChunksToPoll.clear();

    PIXBeginNamedEvent(0.0f, "Ticking blocks (multiplayer)");
    PIXBeginNamedEvent(0.0f, "buildAndPrepareChunksToPoll");
    Level::tickBlocks();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Ticking client side blocks");
    for (int i = 0; i < mChunksToPoll.end(); ++i)
    {
        ChunkPos pos = mChunksToPoll.get(i);
        LevelChunk* chunk = getChunk(pos.x, pos.z);
        tickClientSideBlocks(pos.x * 16, pos.z * 16, chunk);
    }
    PIXEndNamedEvent();
    PIXEndNamedEvent();
}

void MultiPlayerLevel::ReloadChunk(int x, int z, const std::vector<LevelChunk::BlockChange>* changes, bool replace)
{
    mClientChunkCache->queueForReLoad(x, z, changes, replace);
}

void MultiPlayerLevel::setChunkVisible(int x, int z, bool visible, bool force)
{
    if (visible)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Any, true) && !CGameNetworkManager::sInstance.IsHost()
            && !hasChunk(x, z, false))
            ReloadChunk(x, z, new std::vector<LevelChunk::BlockChange>(), false);
        mClientChunkCache->create(x, z);
    }
    else
    {
        mClientChunkCache->drop(x, z, force);
        setBlocksDirty(BlockPos(x * 16, 0, z * 16), BlockPos(x * 16 + 15, 256, z * 16 + 15));
    }
}

bool MultiPlayerLevel::addEntity(boost::shared_ptr<Entity> entity)
{
    bool added = Level::addEntity(entity);
    mTrackedEntities.insert(entity);
    if (!added)
        mPendingEntityAdditions.insert(entity);
    else if (eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType()))
        static_cast<ConsoleSoundEngine*>(mMinecraft->mSoundEngine)
            ->play(new MinecartSoundInstance(boost::static_pointer_cast<Minecart>(entity)));
    return added;
}

void MultiPlayerLevel::removeEntity(boost::shared_ptr<Entity> entity)
{
    EntitySet::iterator pending = mPendingEntityAdditions.find(entity);
    if (pending != mPendingEntityAdditions.end())
        mPendingEntityAdditions.erase(pending);
    Level::removeEntity(entity);
    mTrackedEntities.erase(entity);
}

void MultiPlayerLevel::entityAdded(boost::shared_ptr<Entity> entity)
{
    Level::entityAdded(entity);
    EntitySet::iterator pending = mPendingEntityAdditions.find(entity);
    if (pending != mPendingEntityAdditions.end())
        mPendingEntityAdditions.erase(pending);
}

void MultiPlayerLevel::entityRemoved(boost::shared_ptr<Entity> entity)
{
    Level::entityRemoved(entity);
    if (mTrackedEntities.find(entity) != mTrackedEntities.end())
    {
        entity->stopCurrentLerp();
        mPendingEntityAdditions.insert(entity);
    }
}

void MultiPlayerLevel::putEntity(int id, boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<Entity> oldEntity = getEntity(id);
    if (oldEntity)
        removeEntity(oldEntity);
    mTrackedEntities.insert(entity);
    entity->setId(id);
    if (!addEntity(entity))
        mPendingEntityAdditions.insert(entity);
    mEntitiesById[id] = entity;
}

boost::shared_ptr<Entity> MultiPlayerLevel::getEntity(int id)
{
    for (int i = 0; i < 4; ++i)
    {
        if (mMinecraft->mLocalPlayers[i] && mMinecraft->mLocalPlayers[i]->getId() == id)
            return mMinecraft->mLocalPlayers[i];
    }

    EntityMap::iterator found = mEntitiesById.find(id);
    if (found == mEntitiesById.end())
        return boost::shared_ptr<Entity>();
    return found->second;
}

boost::shared_ptr<Entity> MultiPlayerLevel::removeEntity(int id)
{
    boost::shared_ptr<Entity> entity;
    EntityMap::iterator found = mEntitiesById.find(id);
    if (found != mEntitiesById.end())
    {
        entity = found->second;
        mEntitiesById.erase(found);
        mTrackedEntities.erase(entity);
        removeEntity(entity);
    }
    return entity;
}

void MultiPlayerLevel::removeEntities(std::vector<boost::shared_ptr<Entity> >* entities)
{
    std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
    while (it != entities->end())
    {
        boost::shared_ptr<Entity> entity = *it;
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            it = entities->erase(it);
        }
        else
        {
            EntitySet::iterator pending = mPendingEntityAdditions.find(entity);
            if (pending != mPendingEntityAdditions.end())
                mPendingEntityAdditions.erase(pending);
            mTrackedEntities.erase(entity);
            ++it;
        }
    }
    Level::removeEntities(entities);
}

bool MultiPlayerLevel::setBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate)
{
    return Level::setBlock(pos, state, flags, false);
}

bool MultiPlayerLevel::doSetBlockAndData(const BlockPos& pos, const BlockState* state, const BlockState* extraState)
{
    clearResetRegion(pos.x, pos.y, pos.z, pos.x, pos.y, pos.z);
    const BlockState* oldState = getBlockState(pos);

    bool liquidFlowChange = true;
    if (!((oldState->getBlock() == Blocks::flowing_water && state->getBlock() == Blocks::water)
          || (oldState->getBlock() == Blocks::water && state->getBlock() == Blocks::flowing_water)
          || (oldState->getBlock() == Blocks::flowing_lava && state->getBlock() == Blocks::lava)
          || (oldState->getBlock() == Blocks::lava && state->getBlock() == Blocks::lava)
          || (oldState->getBlock() == Blocks::lava && state->getBlock() == Blocks::flowing_lava)))
    {
        liquidFlowChange = false;
    }

    bool changed;
    if (extraState == NULL)
    {
        changed = setBlockAndUpdate(pos, state);
    }
    else
    {
        bool extraChanged = setExtraBlock(pos, extraState, Block::UPDATE_ALL, false);
        changed = setBlockAndUpdate(pos, state) || extraChanged;
    }
    if (!changed && !CGameNetworkManager::sInstance.IsHost())
        return false;

    if (CGameNetworkManager::sInstance.IsHost() && !liquidFlowChange)
    {
        sendBlockUpdated(pos, oldState, state, 3, true);
        blockUpdated(pos, state->getBlock(), true);
    }
    return true;
}

void MultiPlayerLevel::disconnect(bool save)
{
    if (save)
    {
        for (std::vector<ClientPacketListener*>::iterator it = mConnections.begin(); it < mConnections.end(); ++it)
        {
            boost::shared_ptr<Packet> packet(new DisconnectPacket(DisconnectPacket::eDisconnectReason_Quit));
            (*it)->sendAndDisconnect(packet);
        }
    }
    else
    {
        for (std::vector<ClientPacketListener*>::iterator it = mConnections.begin(); it < mConnections.end(); ++it)
            (*it)->close();
    }
}

SoundInstance* MultiPlayerLevel::makeSoundUpdater(boost::shared_ptr<Minecart>)
{
    return NULL;
}

void MultiPlayerLevel::tickWeather(bool)
{
}

int MultiPlayerLevel::getChunkPollRange()
{
    return mMinecraft->mOptions->mViewDistance;
}

void MultiPlayerLevel::tickClientSideBlocks(int x, int z, LevelChunk* chunk)
{
    Level::tickClientSideBlocks(x, z, chunk);
    if (mAmbientSoundTime != 0)
        return;

    mRandValue = mRandValue * 3 + mAddend;
    int value = mRandValue >> 2;
    int localX = value & 15;
    int localY = (value >> 16) & 127;
    int localZ = (value >> 8) & 15;
    BlockPos pos(localX, localY, localZ);
    if (chunk->getBlock(pos)->mMaterial != Material::AIR)
        return;
    if (getDaytimeRawBrightness(pos) > mRandom->nextInt(8))
        return;
    if (chunk->getBrightness(LightLayer::SKY, pos) > 0)
        return;

    double soundX = localX + x + 0.5;
    double soundY = localY + 0.5;
    double soundZ = localZ + z + 0.5;
    boost::shared_ptr<Player> player = getNearestPlayer(soundX, soundY, soundZ, 8.0, false, -1.0);
    Player* nearest = player.get();
    if (nearest != NULL && nearest->distanceToSqr(soundX, soundY, soundZ) > 4.0)
    {
        playLocalSound(soundX, soundY, soundZ, SoundEvents::AMBIENT_CAVE, SoundSource::eSoundSource_Ambient, 0.7f,
                       mRandom->nextFloat() * 0.2f + 0.8f, false, 16.0f);
        mAmbientSoundTime = mRandom->nextInt(12000) + 6000;
    }
}

void MultiPlayerLevel::playSound(boost::shared_ptr<Player> player, double x, double y, double z,
                                 const SoundEvent* sound, SoundSource::ESoundSource source, float volume, float pitch,
                                 float range)
{
    if ((mMinecraft->mLocalPlayers[0] && player == mMinecraft->mLocalPlayers[0])
        || (mMinecraft->mLocalPlayers[1] && player == mMinecraft->mLocalPlayers[1])
        || (mMinecraft->mLocalPlayers[2] && player == mMinecraft->mLocalPlayers[2])
        || (mMinecraft->mLocalPlayers[3] && player == mMinecraft->mLocalPlayers[3]))
        playLocalSound(x, y, z, sound, source, volume, pitch, false, range);
}

void MultiPlayerLevel::playLocalSound(double x, double y, double z, const SoundEvent* sound,
                                      SoundSource::ESoundSource source, float volume, float pitch, bool distanceDelay,
                                      float range)
{
    if (GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_CaveSounds) == 0
        && sound == SoundEvents::AMBIENT_CAVE)
        return;
    if (volume > 1.0f)
        range *= volume;
    if (sound == SoundEvents::MG02_FALLING)
    {
        mMinecraft->mSoundEngine->playUI(sound, volume, pitch);
        return;
    }

    float nearest = FLT_MAX;
    for (int i = 0; i < 4; ++i)
    {
        if (mMinecraft->mLocalPlayers[i] != NULL)
        {
            const float distance = static_cast<float>(mMinecraft->mLocalPlayers[i]->distanceToSqr(x, y, z));
            if (distance < nearest)
                nearest = distance;
        }
    }
    if (nearest >= range * range)
        return;
    if (distanceDelay && nearest > 100.0f)
    {
        const int delay = static_cast<int>(Math::round(sqrtf(nearest) / 40.0 * 20.0));
        mMinecraft->mSoundEngine->schedule(sound, SoundSource::eSoundSource_Neutral, static_cast<float>(x),
                                           static_cast<float>(y), static_cast<float>(z), volume, pitch, delay);
        return;
    }
    mMinecraft->mSoundEngine->play(sound, static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), volume,
                                   pitch, NULL);
}

void MultiPlayerLevel::createFireworks(double x, double y, double z, double xa, double ya, double za,
                                       CompoundTag* fireworks)
{
    ParticleEngine* particleEngine = mMinecraft->mParticleEngine;
    FireworksParticles::FireworksStarter* particle = new FireworksParticles::FireworksStarter(
        this, x, y, z, xa, ya, za, particleEngine, fireworks);
    particleEngine->add(particle);
}

void MultiPlayerLevel::sendPacketToServer(boost::shared_ptr<Packet>)
{
}

void MultiPlayerLevel::setDayTime(long long time)
{
    Level::setDayTime(time);
}

void MultiPlayerLevel::dataReceivedForChunk(int x, int z)
{
    mClientChunkCache->dataReceived(x, z);
}

void MultiPlayerLevel::removeUnusedBlockEntitiesInRegion(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    EnterCriticalSection(&mBlockEntityMutex);
    for (unsigned int index = 0; index < mBlockEntities.size(); ++index)
    {
        boost::shared_ptr<BlockEntity> blockEntity = mBlockEntities[index];
        BlockPos pos = blockEntity->getBlockPos();
        if (pos.x >= minX && pos.y >= minY && pos.z >= minZ && pos.x < maxX && pos.y < maxY && pos.z < maxZ)
        {
            LevelChunk* chunk = getChunk(blockEntity->getBlockPos().x >> 4, blockEntity->getBlockPos().z >> 4);
            if (chunk != NULL)
            {
                Block* block = chunk->getBlock(blockEntity->getBlockPos());
                if (block == NULL || !block->mBlockEntity)
                {
                    mBlockEntities[index] = mBlockEntities.back();
                    mBlockEntities.pop_back();
                    mScheduledUpdatesAreImmediate = true;
                    chunk->removeBlockEntity(blockEntity->getBlockPos());
                    mScheduledUpdatesAreImmediate = false;
                    --index;
                }
            }
        }
    }
    LeaveCriticalSection(&mBlockEntityMutex);
}

Scoreboard* MultiPlayerLevel::GetScoreboard()
{
    return mClientScoreboard;
}

void MultiPlayerLevel::DropChunksOutsideArea(int minX, int minZ, int maxX, int maxZ, bool force)
{
    mClientChunkCache->dropOutsideArea(minX, minZ, maxX, maxZ, force);
}

void MultiPlayerLevel::StopReloadChunk(int x, int z)
{
    mClientChunkCache->unqueueForReLoad(x, z);
}

bool MultiPlayerLevel::MustQueueBlockEntityDataPackets()
{
    return mClientChunkCache->mustQueueBlockEntityDataPackets();
}

void MultiPlayerLevel::QueueBlockEntityDataPacket(const boost::shared_ptr<ClientboundBlockEntityDataPacket>& packet)
{
    mClientChunkCache->queueBlockEntityDataPacket(packet);
}

void MultiPlayerLevel::BlockUntilSafeToChangeSaveData(bool block)
{
    mClientChunkCache->BlockUntilSafeToChangeSaveData(block);
}
