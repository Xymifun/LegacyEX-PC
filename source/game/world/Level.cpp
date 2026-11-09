#include "game/world/Level.hpp"
#include "game/util/Math.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcSettings.hpp"
#endif
#include "game/world/LevelEvent.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstddef>

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/BoundingBox.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/LootTables.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/HierarchicalElementStorage.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/data/SavedData.hpp"
#include "game/data/SavedDataStorage.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityTickCache.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/entity/ai/EntitySelector.hpp"
#include "game/network/LevelListener.hpp"
#include "game/network/PathNavigationListener.hpp"
#include "game/server/ObserverBlock.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelSettings.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DiodeBlock.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/RedstoneTorchBlock.hpp"
#include "game/world/block/SnowLayerBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/XMem.hpp"
#include "game/core/MemSect.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

static unsigned int sInstaTickTlsIndex = TlsAlloc();
static unsigned int sLightingCacheTlsIndex = TlsAlloc();
static int sEntityTickRange = 32;

const int Level::maxBuildHeight;

Level::Level(boost::shared_ptr<LevelStorage> levelStorage, LevelData* levelData, Dimension* dimension,
             bool isClientSide)
{
    mSeaLevel = dimension->getSeaLevel();
    _init();
    mLevelStorage = levelStorage;
    mDimension = dimension;
    mIsClientSide = isClientSide;
    mLevelData = levelData;
    mWorldBorder = dimension->createWorldBorder(this);

    if (!mLevelData->useNewSeaLevel() && dimension->isNaturalDimension())
    {
        mSeaLevel = 64;
    }
}

Level::~Level()
{
    if (mPathListener != NULL)
    {
        delete mPathListener;
    }

    delete[] mLightUpdateQueue;

    if (mDimension != NULL)
    {
        delete mDimension;
    }

    if (mChunkSource != NULL)
    {
        delete mChunkSource;
    }

    if (mLevelData != NULL)
    {
        delete mLevelData;
    }

    delete mRandom;

    if (mWorldBorder != NULL)
    {
        delete mWorldBorder;
    }

    if (!mIsClientSide)
    {
        RedstoneTorchBlock::removeLevelReferences(this);
    }

    DeleteCriticalSection(&mLightMutex);
    if (mSavedDataStorage != NULL)
    {
        delete mSavedDataStorage;
    }
    mSavedDataStorage = NULL;
    DeleteCriticalSection(&mEntityMutex);
    DeleteCriticalSection(&mBlockEntityMutex);
}

void Level::_init()
{
    mCloudColour = 0xFFFFFF;
    mSkyDarken = 0;

    Random random;
    mRandValue = random.nextInt();
    mAddend = 1013904223;

    mPathListener = new PathNavigationListener();
    mListeners.push_back(mPathListener);

    mSkyFlashTime = 0;
    mOldRainLevel = 0.0f;
    mRainLevel = 0.0f;
    mThunderLevel = 0.0f;
    mOldThunderLevel = 0.0f;

    mRandom = new Random();
    mVillages.reset();
    mScheduledUpdatesAreImmediate = false;
    mChunkSource = NULL;
    mChunkCache = NULL;
    mChunkCacheSize = 0;
    mDimension = NULL;

    mLevelData = NULL;
    mIsClientSide = false;
    mSavedDataStorage = NULL;
    mSpawnEnemies = true;
    mSpawnFriendlies = true;
    mFindingSpawn = false;

    InitializeCriticalSection(&mEntityMutex);
    InitializeCriticalSection(&mBlockEntityMutex);
    mProcessingBlockEntities = false;

    mLightUpdateQueue = new unsigned int[0x8000];

    InitializeCriticalSectionAndSpinCount(&mLightMutex, 0x1400);

    mIsNew = false;
    mAmbientTickCountdown = 0;
    mFogDistance = -1;
    mForceTimeUpdate = false;
    mLootTables = NULL;
}

bool Level::isWaterAt(const BlockPos& pos)
{
    return getBlockState(pos)->getMaterial() == Material::WATER;
}

bool Level::shouldFreeze(const BlockPos& pos, bool checkNeighbours)
{
    if (getBiome(pos)->getTemperature(pos) > 0.15f)
    {
        return false;
    }

    if ((unsigned int)pos.y >= 256)
    {
        return false;
    }

    if (getBrightness(LightLayer::BLOCK, pos) >= 10)
    {
        return false;
    }

    const BlockState* state = getBlockState(pos);
    Block* block = state->getBlock();

    if (block != Blocks::water && block != Blocks::flowing_water)
    {
        return false;
    }

    if (state->getValue(LiquidBlock::LEVEL) != 0)
    {
        return false;
    }

    if (checkNeighbours && isWaterAt(pos.west()) && isWaterAt(pos.east()) && isWaterAt(pos.north())
        && isWaterAt(pos.south()))
    {
        return false;
    }

    return true;
}

bool Level::shouldFreezeIgnoreNeighbors(const BlockPos& pos)
{
    return shouldFreeze(pos, false);
}

long long Level::getSeed()
{
    return mLevelData->getSeed();
}

LevelData* Level::getLevelData()
{
    return mLevelData;
}

void Level::clampWithinLevelBounds(double& x, double& z, double margin)
{
    if (mDimension->getType() == DimensionType::THE_END)
    {
        const AABB* bounds = mDimension->getPlayerConstraints(BlockPos((int)x, 64, (int)z));
        x = Mth::clamp(x, bounds->mMinX + margin, bounds->mMaxX - margin);
        z = Mth::clamp(z, bounds->mMinZ + margin, bounds->mMaxZ - margin);
    }
    else
    {
        int bound = 30000000;
        LevelData* levelData = getLevelData();

        if (levelData != NULL)
        {
            bound = levelData->getXZSize() * 8;
        }

        x = Mth::clamp(x, margin - (double)bound, (double)(bound - 1) - margin);
        z = Mth::clamp(z, margin - (double)bound, (double)(bound - 1) - margin);
    }
}

bool Level::isWithinLevelBounds(int x, int z)
{
    if (mDimension->getType() == DimensionType::THE_END)
    {
        return mDimension->getPlayerConstraints(BlockPos(x, 64, z))
            ->containsIncludingLowerBound(Vec3::newTemp((double)x, 64.0, (double)z));
    }

    int bound = 30000000;
    LevelData* levelData = getLevelData();

    if (levelData != NULL)
    {
        bound = levelData->getXZSize() * 8;
    }

    return x >= -bound && z >= -bound && x < bound && z < bound;
}

void Level::clampWithinLevelHeight(double& y, double margin)
{
    if (y + margin > 512.0)
    {
        y -= (y + margin) - 512.0;
    }
}

void Level::enableLightingCache()
{
    TlsSetValue(sLightingCacheTlsIndex, XPhysicalAlloc(0x40000, 0xFFFFFFFF, 0x20000, 0x20000004));
}

void Level::destroyLightingCache()
{
    XPhysicalFree(TlsGetValue(sLightingCacheTlsIndex));
}

bool Level::getInstaTick()
{
    return TlsGetValue(sInstaTickTlsIndex) != NULL;
}

void Level::setInstaTick(bool instaTick)
{
    TlsSetValue(sInstaTickTlsIndex, (void*)(instaTick ? 1 : 0));
}

bool Level::hasEntitiesToRemove()
{
    return !mEntitiesToRemove.empty();
}

Level* Level::postConstruct()
{
    return this;
}

LevelChunk* Level::getChunk(int chunkX, int chunkZ)
{
    return mChunkSource->getChunk(chunkX, chunkZ);
}

LevelChunk* Level::getChunkAt(const BlockPos& pos)
{
    return getChunk(pos.x >> 4, pos.z >> 4);
}

bool Level::hasChunkAt(const BlockPos& pos, bool allowEmpty)
{
    return hasChunk(pos.x >> 4, pos.z >> 4, allowEmpty);
}

bool Level::hasChunkAt(const BlockPos& pos)
{
    return hasChunkAt(pos, true);
}

Biome* Level::getBiome(const BlockPos& pos)
{
    if (hasChunkAt(pos))
    {
        return getChunkAt(pos)->getBiome(pos, mDimension->getBiomeSource());
    }

    return mDimension->getBiomeSource()->getBiome(pos, Biome::PLAINS);
}

BiomeSource* Level::getBiomeSource()
{
    return mDimension->getBiomeSource();
}

void Level::initializeLevel(LevelSettings* settings)
{
    mLevelData->setInitialized(true);
}

MinecraftServer* Level::getServer()
{
    return NULL;
}

void Level::setSpawnPos(const BlockPos& pos)
{
    mLevelData->setSpawn(pos);
}

void Level::validateSpawn()
{
    setSpawnPos(BlockPos(8, 64, 8));
}

int Level::getSeaLevel()
{
    return mSeaLevel;
}

const BlockState* Level::getTopBlockState(const BlockPos& pos)
{
    if (mLevelData->getGeneratorType() == LevelType::FLAT)
    {
        return Blocks::grass->defaultBlockState();
    }

    BlockPos top(pos.x, getSeaLevel(), pos.z);

    while (!isEmptyBlock(top.above()))
    {
        top = top.above();
    }

    return getBlockState(top);
}

bool Level::isInWorldBounds(const BlockPos& pos)
{
    return (pos.y & ~255) == 0;
}

bool Level::isInWorldBounds(int x, int y, int z)
{
    return (y & ~255) == 0;
}

bool Level::isOutsideBuildHeight(const BlockPos& pos)
{
    return (unsigned int)pos.y > 255;
}

bool Level::isEmptyBlock(const BlockPos& pos)
{
    return getBlockId(pos.x, pos.y, pos.z) == BlockId::AIR;
}

bool Level::isEmptyWaterBlock(const BlockPos& pos)
{
    return getBlockState(pos)->getMaterial() == Material::WATER;
}

bool Level::hasChunksAt(const BlockPos& pos, int radius, bool allowEmpty)
{
    return hasChunksAt(pos.x - radius, pos.y - radius, pos.z - radius, pos.x + radius, pos.y + radius, pos.z + radius,
                       allowEmpty);
}

bool Level::hasChunksAt(const BlockPos& pos, int radius)
{
    return hasChunksAt(pos, radius, true);
}

bool Level::hasChunksAt(const BlockPos& from, const BlockPos& to, bool allowEmpty)
{
    return hasChunksAt(from.x, from.y, from.z, to.x, to.y, to.z, allowEmpty);
}

bool Level::hasChunksAt(const BlockPos& from, const BlockPos& to)
{
    return hasChunksAt(from, to, true);
}

bool Level::hasChunksAt(BoundingBox box, bool allowEmpty)
{
    return hasChunksAt(box.x0, box.y0, box.z0, box.x1, box.y1, box.z1, allowEmpty);
}

bool Level::hasChunksAt(BoundingBox box)
{
    return hasChunksAt(box, true);
}

bool Level::reallyHasChunk(int chunkX, int chunkZ)
{
    return mChunkSource->reallyHasChunk(chunkX, chunkZ);
}

bool Level::reallyHasChunkAt(const BlockPos& pos)
{
    if (!isInWorldBounds(pos))
    {
        return false;
    }

    return reallyHasChunk(pos.x >> 4, pos.z >> 4);
}

bool Level::reallyHasChunksAt(const BlockPos& from, const BlockPos& to)
{
    for (int chunkX = from.x >> 4; chunkX <= (to.x >> 4); ++chunkX)
    {
        for (int chunkZ = from.z >> 4; chunkZ <= (to.z >> 4); ++chunkZ)
        {
            if (!reallyHasChunk(chunkX, chunkZ))
            {
                return false;
            }
        }
    }

    return true;
}

bool Level::hasChunksAt(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, bool allowEmpty)
{
    if (maxY < 0 || minY > 255)
    {
        return false;
    }

    for (int chunkX = minX >> 4; chunkX <= (maxX >> 4); ++chunkX)
    {
        for (int chunkZ = minZ >> 4; chunkZ <= (maxZ >> 4); ++chunkZ)
        {
            if (!hasChunk(chunkX, chunkZ, allowEmpty))
            {
                return false;
            }
        }
    }

    return true;
}

LevelChunk* Level::getChunkAt(int x, int z)
{
    return getChunk(x >> 4, z >> 4);
}

LevelChunk* Level::getChunkLoadedOrUnloadedAt(int x, int z)
{
    return mChunkSource->getChunkLoadedOrUnloaded(x >> 4, z >> 4);
}

bool Level::isChunkGeneratedAt(int chunkX, int chunkZ)
{
    if (hasChunk(chunkX, chunkZ, false))
    {
        return true;
    }

    return mChunkSource->reallyHasChunk(chunkX, chunkZ);
}

bool Level::setBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate)
{
    return internalSetBlock(false, pos, state, flags, forceUpdate);
}

bool Level::setExtraBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate)
{
    return internalSetBlock(true, pos, state, flags, forceUpdate);
}

void Level::setBlockBrightnessChanged(const BlockPos& pos)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->blockLightChanged(pos);
    }
}

void Level::setBrightness(LightLayer::variety layer, const BlockPos& pos, int brightness, bool markDirty)
{
    if (!isInWorldBounds(pos) || !hasChunkAt(pos))
    {
        return;
    }

    getChunkAt(pos)->setBrightness(layer, pos, brightness);

    if (mIsClientSide && markDirty)
    {
        if (mHasDirtyRegion)
        {
            if (pos.x < mDirtyMinX)
            {
                mDirtyMinX = pos.x;
            }

            if (pos.x > mDirtyMaxX)
            {
                mDirtyMaxX = pos.x;
            }

            if (pos.y < mDirtyMinY)
            {
                mDirtyMinY = pos.y;
            }

            if (pos.y > mDirtyMaxY)
            {
                mDirtyMaxY = pos.y;
            }

            if (pos.z < mDirtyMinZ)
            {
                mDirtyMinZ = pos.z;
            }

            if (pos.z > mDirtyMaxZ)
            {
                mDirtyMaxZ = pos.z;
            }
        }
        else
        {
            mDirtyMinX = pos.x;
            mDirtyMaxX = pos.x;
            mDirtyMinY = pos.y;
            mDirtyMaxY = pos.y;
            mDirtyMinZ = pos.z;
            mDirtyMaxZ = pos.z;
            mHasDirtyRegion = true;
        }
    }
    else
    {
        setBlockBrightnessChanged(pos);
    }
}

#ifdef MINECRAFT_PC
static int LightCacheCoord(unsigned int lowBits, int reference)
{
    int offset = (static_cast<int>(lowBits) - reference) & 0x3FF;
    if (offset >= 0x200)
        offset -= 0x400;
    return reference + offset;
}
#else
static int LightCacheCoord(unsigned int lowBits, int)
{
    return static_cast<int>(lowBits << 22) >> 22;
}
#endif

int Level::getBrightnessCached(unsigned int* cache, LightLayer::variety layer, int x, int y, int z)
{
    if (cache == NULL || (y & 0xFFFFFF00) != 0)
    {
        return getBrightness(layer, BlockPos(x, y, z));
    }

    int index = ((x & 0xF) << 8) | ((y & 0xF) << 4) | (z & 0xF);
    unsigned int entry = cache[index];
    unsigned int key = ((unsigned int)(y & 0xF0) << 2) | ((unsigned int)(x & 0x3F0) << 6)
                       | ((unsigned int)(z >> 4) & 0x3F);

    if ((entry & 0xFFFF) == key)
    {
        if ((entry & 0x10000000) != 0)
        {
            return (entry >> 24) & 0xF;
        }

        int brightness = getBrightness(layer, BlockPos(x, y, z));
        cache[index] = entry | ((unsigned int)brightness << 24) | 0x10000000;
        return brightness & 0xF;
    }

    if ((entry & 0x80000000) != 0)
    {
        setBrightness(layer,
                      BlockPos(LightCacheCoord(((entry >> 6) & 0x3F0) | (x & 0xF), x),
                               ((entry >> 2) & 0xF0) | (y & 0xF),
                               LightCacheCoord(((entry & 0x3F) << 4) | (z & 0xF), z)),
                      (entry >> 24) & 0xF, true);
    }

    int brightness = getBrightness(layer, BlockPos(x, y, z));
    cache[index] = key | 0x10000000 | ((unsigned int)brightness << 24);
    return brightness & 0xF;
}

bool Level::canSeeSky(const BlockPos& pos)
{
    return getChunkAt(pos)->isSkyLit(pos);
}

int Level::getEmissionCached(unsigned int* cache, Block* block, const BlockPos& pos)
{
    if (cache == NULL)
    {
        return block->defaultBlockState()->getLightEmission();
    }

    int index = ((pos.x & 0xF) << 8) | ((pos.y & 0xF) << 4) | (pos.z & 0xF);
    unsigned int entry = cache[index];
    unsigned int key = ((unsigned int)(pos.y & 0xF0) << 2) | ((unsigned int)(pos.x & 0x3F0) << 6)
                       | ((unsigned int)(pos.z >> 4) & 0x3F);

    if ((entry & 0xFFFF) == key && (entry & 0x40000000) != 0)
    {
        return (entry >> 16) & 0xF;
    }

    if ((entry & 0xFFFF) != key && (entry & 0x80000000) != 0)
    {
        setBrightness(LightLayer::BLOCK,
                      BlockPos(LightCacheCoord(((entry >> 6) & 0x3F0) | (pos.x & 0xF), pos.x),
                               ((entry >> 2) & 0xF0) | (pos.y & 0xF),
                               LightCacheCoord(((entry & 0x3F) << 4) | (pos.z & 0xF), pos.z)),
                      (entry >> 24) & 0xF, true);
    }

    Block* blockAt = getBlock(pos);
    Block* extraBlockAt = getExtraBlock(pos);
    int emission = blockAt->defaultBlockState()->getLightEmission();
    int blocking = blockAt->defaultBlockState()->getLightBlock();
    int extraBlocking = extraBlockAt->defaultBlockState()->getLightBlock();

    if (blocking < extraBlocking)
    {
        blocking = extraBlocking;
    }

    if ((entry & 0xFFFF) == key)
    {
        cache[index] = entry | 0x60000000 | ((unsigned int)(emission & 0xF) << 16)
                       | ((unsigned int)(blocking & 0xF) << 20);
    }
    else
    {
        cache[index] = key | 0x60000000 | ((unsigned int)(emission & 0xF) << 16)
                       | ((unsigned int)(blocking & 0xF) << 20);
    }

    return emission & 0xF;
}

int Level::getExpectedLight(unsigned int* cache, const BlockPos& pos, LightLayer::variety layer, bool ignoreEmission)
{
    if (layer == LightLayer::SKY && canSeeSky(pos))
    {
        return 15;
    }

    const BlockState* state = getBlockState(pos);
    const BlockState* extraState = getExtraBlockState(pos);

    int light = (layer == LightLayer::SKY) ? 0 : state->getLightEmission();
    int blocking = state->getLightBlock();
    int extraBlocking = extraState->getLightBlock();

    if (blocking < extraBlocking)
    {
        blocking = extraBlocking;
    }

    if (blocking >= 15)
    {
        if (light < 1)
        {
            if (!ignoreEmission && layer != LightLayer::SKY)
            {
                return getEmissionCached(cache, Blocks::air, pos);
            }

            return 0;
        }

        blocking = 1;
    }
    else if (blocking < 1)
    {
        blocking = 1;
    }

    if (light < 14)
    {
        int limit = 15 - blocking;
        int neighbour = getBrightnessCached(cache, layer, pos.x, pos.y, pos.z + 1) - blocking;

        if (light < neighbour)
        {
            light = neighbour;
        }

        if (light < limit)
        {
            neighbour = getBrightnessCached(cache, layer, pos.x, pos.y, pos.z - 1) - blocking;

            if (light < neighbour)
            {
                light = neighbour;
            }
        }

        if (light < limit)
        {
            neighbour = getBrightnessCached(cache, layer, pos.x, pos.y + 1, pos.z) - blocking;

            if (light < neighbour)
            {
                light = neighbour;
            }
        }

        if (light < limit)
        {
            neighbour = getBrightnessCached(cache, layer, pos.x + 1, pos.y, pos.z) - blocking;

            if (light < neighbour)
            {
                light = neighbour;
            }
        }

        if (light < limit)
        {
            neighbour = getBrightnessCached(cache, layer, pos.x - 1, pos.y, pos.z) - blocking;

            if (light < neighbour)
            {
                light = neighbour;
            }
        }

        if (light < limit)
        {
            neighbour = getBrightnessCached(cache, layer, pos.x, pos.y - 1, pos.z) - blocking;

            if (light < neighbour)
            {
                light = neighbour;
            }
        }
    }

    return light;
}

int Level::getBlockingCached(unsigned int* cache, LightLayer::variety layer, Block* block, Block* extraBlock,
                             const BlockPos& pos)
{
    if (cache == NULL)
    {
        if (block == NULL)
        {
            block = getBlock(pos);
        }

        if (extraBlock == NULL)
        {
            extraBlock = getExtraBlock(pos);
        }

        int blocking = block->defaultBlockState()->getLightBlock();
        int extraBlocking = extraBlock->defaultBlockState()->getLightBlock();
        return (extraBlocking < blocking) ? blocking : extraBlocking;
    }

    int index = ((pos.x & 0xF) << 8) | ((pos.y & 0xF) << 4) | (pos.z & 0xF);
    unsigned int entry = cache[index];
    unsigned int key = ((unsigned int)(pos.y & 0xF0) << 2) | ((unsigned int)(pos.x & 0x3F0) << 6)
                       | ((unsigned int)(pos.z >> 4) & 0x3F);

    if ((entry & 0xFFFF) == key && (entry & 0x40000000) != 0)
    {
        return (entry >> 20) & 0xF;
    }

    if ((entry & 0xFFFF) != key && (entry & 0x80000000) != 0)
    {
        setBrightness(layer,
                      BlockPos(LightCacheCoord(((entry >> 6) & 0x3F0) | (pos.x & 0xF), pos.x),
                               ((entry >> 2) & 0xF0) | (pos.y & 0xF),
                               LightCacheCoord(((entry & 0x3F) << 4) | (pos.z & 0xF), pos.z)),
                      (entry >> 24) & 0xF, true);
    }

    Block* blockAt = getBlock(pos);
    Block* extraBlockAt = getExtraBlock(pos);
    int emission = blockAt->defaultBlockState()->getLightEmission();
    int blocking = blockAt->defaultBlockState()->getLightBlock();
    int extraBlocking = extraBlockAt->defaultBlockState()->getLightBlock();

    if (blocking < extraBlocking)
    {
        blocking = extraBlocking;
    }

    if ((entry & 0xFFFF) == key)
    {
        cache[index] = entry | 0x60000000 | ((unsigned int)(emission & 0xF) << 16)
                       | ((unsigned int)(blocking & 0xF) << 20);
    }
    else
    {
        cache[index] = key | 0x60000000 | ((unsigned int)(emission & 0xF) << 16)
                       | ((unsigned int)(blocking & 0xF) << 20);
    }

    return blocking & 0xF;
}

void Level::initCachePartial(unsigned int* cache, int x, int y, int z)
{
    mHasDirtyRegion = false;

    if (cache == NULL)
    {
        return;
    }

    if ((y & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | (z & 0xF)] = 0;
        cache[(((x - 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)] = 0;
        cache[(((x + 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)] = 0;
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z - 1) & 0xF)] = 0;
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z + 1) & 0xF)] = 0;
    }

    if (((y - 1) & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | (((y - 1) << 4) & 0xF0) | (z & 0xF)] = 0;
    }

    if (((y + 1) & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | (((y + 1) << 4) & 0xF0) | (z & 0xF)] = 0;
    }
}

void Level::initCacheComplete(unsigned int* cache, int x, int y, int z)
{
    unsigned int centre = 0;
    unsigned int west = 0;
    unsigned int east = 0;
    unsigned int below = 0;
    unsigned int above = 0;
    unsigned int north = 0;
    unsigned int south = 0;

    if ((y & 0xFFFFFF00) == 0)
    {
        centre = cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | (z & 0xF)];
        west = cache[(((x - 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)];
        east = cache[(((x + 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)];
        north = cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z - 1) & 0xF)];
        south = cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z + 1) & 0xF)];
    }

    if (((y - 1) & 0xFFFFFF00) == 0)
    {
        below = cache[((x & 0xF) << 8) | (((y - 1) << 4) & 0xF0) | (z & 0xF)];
    }

    if (((y + 1) & 0xFFFFFF00) == 0)
    {
        above = cache[((x & 0xF) << 8) | (((y + 1) << 4) & 0xF0) | (z & 0xF)];
    }

    XMemSet128(cache, 0, 0x4000);

    if ((y & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | (z & 0xF)] = centre;
        cache[(((x - 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)] = west;
        cache[(((x + 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)] = east;
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z - 1) & 0xF)] = north;
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z + 1) & 0xF)] = south;
    }

    if (((y - 1) & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | (((y - 1) << 4) & 0xF0) | (z & 0xF)] = below;
    }

    if (((y + 1) & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | (((y + 1) << 4) & 0xF0) | (z & 0xF)] = above;
    }
}

void Level::setBrightnessCached(unsigned int* cache, unsigned long long* dirty, LightLayer::variety layer, int x, int y,
                                int z, int brightness)
{
    if (cache == NULL)
    {
        setBrightness(layer, BlockPos(x, y, z), brightness, true);
        return;
    }

    if ((y & 0xFFFFFF00) != 0)
    {
        return;
    }

    int index = ((x & 0xF) << 8) | ((y & 0xF) << 4) | (z & 0xF);
    unsigned int entry = cache[index];
    unsigned int key = ((unsigned int)(y & 0xF0) << 2) | ((unsigned int)(x & 0x3F0) << 6)
                       | ((unsigned int)(z >> 4) & 0x3F);

    if ((entry & 0xFFFF) != key)
    {
        if ((entry & 0x80000000) != 0)
        {
            setBrightness(layer,
                          BlockPos(LightCacheCoord(((entry >> 6) & 0x3F0) | (x & 0xF), x),
                                   ((entry >> 2) & 0xF0) | (y & 0xF),
                                   LightCacheCoord(((entry & 0x3F) << 4) | (z & 0xF), z)),
                          (entry >> 24) & 0xF, true);
        }

        entry = key;
    }

    *dirty |= (1ULL << (x & 0xF)) | (0x10000ULL << (y & 0xF)) | (1ULL << (32 + (z & 0xF)));
    cache[index] = (entry & 0xF0FFFFFF) | ((unsigned int)brightness << 24) | 0x90000000;
}

void Level::setBlocksDirty(const BlockPos& from, const BlockPos& to)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->setBlocksDirty(from.x, from.y, from.z, to.x, to.y, to.z, this);
    }
}

#ifdef MINECRAFT_PC
void Level::flushCache(unsigned int* cache, unsigned long long dirty, LightLayer::variety layer, int referenceX,
                       int referenceZ)
#else
void Level::flushCache(unsigned int* cache, unsigned long long dirty, LightLayer::variety layer)
#endif
{
    if (dirty == 0)
    {
        return;
    }

    if (cache != NULL)
    {
        unsigned int* entryPtr = cache;

        for (int x = 0; x < 16; ++x)
        {
            if ((dirty & (1ULL << x)) == 0)
            {
                entryPtr += 256;
                continue;
            }

            for (int y = 0; y < 16; ++y)
            {
                if ((dirty & (0x10000ULL << y)) == 0)
                {
                    entryPtr += 16;
                    continue;
                }

                for (int z = 0; z < 16; ++z, ++entryPtr)
                {
                    if ((dirty & (1ULL << (32 + z))) == 0)
                    {
                        continue;
                    }

                    unsigned int entry = *entryPtr;

                    if ((entry & 0x80000000) != 0)
                    {
#ifdef MINECRAFT_PC
                        setBrightness(layer,
                                      BlockPos(LightCacheCoord(((entry >> 6) & 0x3F0) | x, referenceX),
                                               ((entry >> 2) & 0xF0) | (y & 0xF),
                                               LightCacheCoord((z & 0xF) | ((entry & 0x3F) << 4), referenceZ)),
                                      (entry >> 24) & 0xF, true);
                        *entryPtr = entry & 0x7FFFFFFF;
#else
                        setBrightness(layer,
                                      BlockPos((int)((((entry >> 6) & 0x3F0) | x) << 22) >> 22,
                                               ((entry >> 2) & 0xF0) | (y & 0xF),
                                               (int)(((z & 0xF) | ((entry & 0x3F) << 4)) << 22) >> 22),
                                      (entry >> 24) & 0xF, true);
#endif
                    }
                }
            }
        }
    }

    if (mIsClientSide && mHasDirtyRegion)
    {
        setBlocksDirty(BlockPos(mDirtyMinX, mDirtyMinY, mDirtyMinZ), BlockPos(mDirtyMaxX, mDirtyMaxY, mDirtyMaxZ));
    }
}

bool Level::checkLight(LightLayer::variety layer, const BlockPos& pos, bool onlyLoadedChunks, bool skipCentre)
{
    unsigned int* cache = (unsigned int*)TlsGetValue(sLightingCacheTlsIndex);
    unsigned long long dirty = 0;
#ifdef MINECRAFT_PC
    if (cache != NULL)
    {
        const int chunkX = pos.x >> 4;
        const int chunkZ = pos.z >> 4;
        const int anchorX = static_cast<int>(cache[0xFFFE]);
        const int anchorZ = static_cast<int>(cache[0xFFFF]);
        if (chunkX - anchorX > 16 || anchorX - chunkX > 16 || chunkZ - anchorZ > 16 || anchorZ - chunkZ > 16)
        {
            memset(cache, 0, 0x1000 * sizeof(unsigned int));
            cache[0xFFFE] = static_cast<unsigned int>(chunkX);
            cache[0xFFFF] = static_cast<unsigned int>(chunkZ);
        }
    }
#endif

    if (!hasChunksAt(pos, onlyLoadedChunks ? 0 : 17))
    {
        return false;
    }

    EnterCriticalSection(&mLightMutex);

    int centreX = pos.x;
    int centreY = pos.y;
    int centreZ = pos.z;

    PIXBeginNamedEvent(0.0f, "initCachePartial");
    initCachePartial(cache, centreX, centreY, centreZ);
    PIXEndNamedEvent();

    unsigned int* queue = (cache == NULL) ? mLightUpdateQueue : cache + 0x1000;
    int checked = 0;
    int toCheck = 0;
    int minBound = -(mDimension->getXZSize() * 16) / 2;
    int maxBound = mDimension->getXZSize() * 16 / 2 - 1;

    if (centreX > maxBound || centreX < minBound || centreZ > maxBound || centreZ < minBound)
    {
        LeaveCriticalSection(&mLightMutex);
        return false;
    }

    PIXBeginNamedEvent(0.0f, "First block");

    int currentLight = getBrightnessCached(cache, layer, pos.x, pos.y, pos.z);
    int expectedLight = getExpectedLight(cache, pos, layer, false);

    if (expectedLight != currentLight && cache != NULL)
    {
        initCacheComplete(cache, centreX, centreY, centreZ);
    }

    if (expectedLight > currentLight)
    {
        queue[0] = 0x20820;
        toCheck = 1;
    }
    else if (expectedLight < currentLight)
    {
        queue[0] = ((unsigned int)currentLight << 18) | 0x20820;

        int count = 1;
        MutableBlockPos current(0, 0, 0);
        unsigned int* keep = queue;

        do
        {
            unsigned int entry = queue[checked];
            ++checked;

            int x = (int)(entry & 0x3F) - 32 + centreX;
            int light = (entry >> 18) & 0xF;
            int y = (int)((entry >> 6) & 0x3F) - 32 + centreY;
            int z = (int)((entry >> 12) & 0x3F) - 32 + centreZ;
            current.set(x, y, z);

            if (getBrightnessCached(cache, layer, x, y, z) != light)
            {
                continue;
            }

            setBrightnessCached(cache, &dirty, layer, x, y, z, 0);

            if (light == 0)
            {
                continue;
            }

            if (Mth::abs(x - centreX) + Mth::abs(y - centreY) + Mth::abs(z - centreZ) >= 17)
            {
                continue;
            }

            bool stillLit = false;

            for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
                 ++it)
            {
                const Direction* direction = *it;
                int nx = x + direction->getStepX();
                int ny = y + direction->getStepY();
                int nz = z + direction->getStepZ();

                if (nx > maxBound || nx < minBound || nz > maxBound || nz < minBound || (unsigned int)ny >= 256)
                {
                    continue;
                }

                current.set(nx, ny, nz);

                int emission;

                if (layer == LightLayer::SKY)
                {
                    emission = 0;
                }
                else
                {
                    emission = getBlock(current)->defaultBlockState()->getLightEmission();
                }

                int blocking = std::max(
                    getBlockingCached(cache, layer, getBlock(current), getExtraBlock(current), current), 1);

                if (blocking >= 15 && emission > 0)
                {
                    blocking = 1;
                }

                int neighbourLight = getBrightnessCached(cache, layer, nx, ny, nz);
                int fallOff = light - blocking;

                if (neighbourLight == fallOff && count < 0x8000)
                {
                    queue[count] = (nx - centreX + 32) | ((ny - centreY + 32) << 6) | ((nz - centreZ + 32) << 12)
                                   | (fallOff << 18);
                    ++count;
                }
                else if (neighbourLight > fallOff)
                {
                    stillLit = true;
                }
            }

            if (stillLit)
            {
                *keep = entry;
                ++keep;
                ++toCheck;
            }
        } while (checked < count);
        checked = 0;
    }

    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Second block: checked=%i toCheck=%i", 0, toCheck);

    MutableBlockPos current(0, 0, 0);

    while (checked < toCheck)
    {
        unsigned int entry = queue[checked];
        ++checked;

        int x = (int)(entry & 0x3F) - 32 + centreX;
        int y = (int)((entry >> 6) & 0x3F) - 32 + centreY;
        int z = (int)((entry >> 12) & 0x3F) - 32 + centreZ;
        current.set(x, y, z);

        if (onlyLoadedChunks && !hasChunkAt(current))
        {
            continue;
        }

        int light = getBrightnessCached(cache, layer, x, y, z);
        bool ignoreEmission = false;
        if (layer == LightLayer::BLOCK && skipCentre)
        {
            ignoreEmission = x != centreX || y != centreY || z != centreZ;
        }
        int expected = getExpectedLight(cache, current, layer, ignoreEmission);

        if (expected == light)
        {
            continue;
        }

        setBrightnessCached(cache, &dirty, layer, x, y, z, expected);

        if (expected <= light)
        {
            continue;
        }

        bool hasSpace = toCheck < 0x7FFA;
        if (abs(x - centreX) + abs(y - centreY) + abs(z - centreZ) < 17 && hasSpace)
        {
            if (x - 1 >= minBound && getBrightnessCached(cache, layer, x - 1, y, z) < expected)
            {
                queue[toCheck] = (x - 1 - centreX + 32) + ((y - centreY + 32) << 6) + ((z - centreZ + 32) << 12);
                ++toCheck;
            }

            if (x + 1 <= maxBound && getBrightnessCached(cache, layer, x + 1, y, z) < expected)
            {
                queue[toCheck] = (x + 1 - centreX + 32) + ((y - centreY + 32) << 6) + ((z - centreZ + 32) << 12);
                ++toCheck;
            }

            if (y - 1 >= 0 && getBrightnessCached(cache, layer, x, y - 1, z) < expected)
            {
                queue[toCheck] = (x - centreX + 32) + ((y - 1 - centreY + 32) << 6) + ((z - centreZ + 32) << 12);
                ++toCheck;
            }

            if (y + 1 < 256 && getBrightnessCached(cache, layer, x, y + 1, z) < expected)
            {
                queue[toCheck] = (x - centreX + 32) + ((y + 1 - centreY + 32) << 6) + ((z - centreZ + 32) << 12);
                ++toCheck;
            }

            if (z - 1 >= minBound && getBrightnessCached(cache, layer, x, y, z - 1) < expected)
            {
                queue[toCheck] = (x - centreX + 32) + ((y - centreY + 32) << 6) + ((z - 1 - centreZ + 32) << 12);
                ++toCheck;
            }

            if (z + 1 <= maxBound && getBrightnessCached(cache, layer, x, y, z + 1) < expected)
            {
                queue[toCheck] = (x - centreX + 32) + ((y - centreY + 32) << 6) + ((z + 1 - centreZ + 32) << 12);
                ++toCheck;
            }
        }
    }

    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "flushCache");
#ifdef MINECRAFT_PC
    flushCache(cache, dirty, layer, centreX, centreZ);
#else
    flushCache(cache, dirty, layer);
#endif
    PIXEndNamedEvent();

    LeaveCriticalSection(&mLightMutex);
    return true;
}

bool Level::checkLight(const BlockPos& pos, bool onlyLoadedChunks, bool skipCentre)
{
    bool changed = false;

    if (mDimension->isHasSkyLight())
    {
        PIXBeginNamedEvent(0.0f, "Check sky light");
        changed = checkLight(LightLayer::SKY, pos, onlyLoadedChunks, false);
        PIXEndNamedEvent();
    }

    PIXBeginNamedEvent(0.0f, "Check block light");
    bool blockChanged = checkLight(LightLayer::BLOCK, pos, onlyLoadedChunks, skipCentre);
    PIXEndNamedEvent();

    return changed || blockChanged;
}

void Level::neighborChanged(const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    if (mIsClientSide)
    {
        return;
    }

    const BlockState* state = getBlockState(pos);
    state->neighborChanged(this, pos, block, fromPos);
    state = Block::getStateByElementId(getExtraElement(pos.x, pos.y, pos.z));
    state->neighborChanged(this, pos, block, fromPos);
}

void Level::observedNeighborChanged(const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    if (mIsClientSide)
    {
        return;
    }

    const BlockState* state = getBlockState(pos);

    if (state->getBlock() == Blocks::observer)
    {
        static_cast<ObserverBlock*>(state->getBlock())->observedNeighborChanged(state, this, pos, block, fromPos);
    }
}

void Level::updateObservingBlocksAt(const BlockPos& pos, Block* block)
{
    observedNeighborChanged(pos.west(), block, pos);
    observedNeighborChanged(pos.east(), block, pos);
    observedNeighborChanged(pos.below(), block, pos);
    observedNeighborChanged(pos.above(), block, pos);
    observedNeighborChanged(pos.north(), block, pos);
    observedNeighborChanged(pos.south(), block, pos);
}

void Level::updateNeighborsAt(const BlockPos& pos, Block* block, bool updateObservers)
{
    neighborChanged(pos.west(), block, pos);
    neighborChanged(pos.east(), block, pos);
    neighborChanged(pos.below(), block, pos);
    neighborChanged(pos.above(), block, pos);
    neighborChanged(pos.north(), block, pos);
    neighborChanged(pos.south(), block, pos);

    if (updateObservers)
    {
        updateObservingBlocksAt(pos, block);
    }
}

void Level::blockUpdated(const BlockPos& pos, Block* block, bool updateObservers)
{
    updateNeighborsAt(pos, block, updateObservers);
}

bool Level::internalSetBlock(bool extraBlock, const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate)
{
    if (isOutsideBuildHeight(pos))
    {
        return false;
    }

    LevelChunk* chunk = getChunkAt(pos);
    Block* block = state->getBlock();

    int oldBlocking = chunk->getBlock(pos)->defaultBlockState()->getLightBlock();
    int oldExtraBlocking = chunk->getExtraBlock(pos)->defaultBlockState()->getLightBlock();

    if (oldBlocking < oldExtraBlocking)
    {
        oldBlocking = oldExtraBlocking;
    }

    const BlockState* oldState = extraBlock ? chunk->setExtraBlock(pos, state) : chunk->setBlock(pos, state);
    if (oldState == NULL)
    {
        return false;
    }

    bool sameLight = false;
    int newBlocking = chunk->getBlock(pos)->defaultBlockState()->getLightBlock();
    int newExtraBlocking = chunk->getExtraBlock(pos)->defaultBlockState()->getLightBlock();

    if (newBlocking < newExtraBlocking)
    {
        newBlocking = newExtraBlocking;
    }

    if (oldBlocking == newBlocking && state->getLightEmission() == oldState->getLightEmission())
    {
        sameLight = true;
    }

    if ((flags & Block::UPDATE_LIGHT_IF_CHANGED) == 0 || !sameLight)
    {
        PIXBeginNamedEvent(0.0f, "Checking light %d %d %d", pos.x, pos.y, pos.z);
        checkLight(pos, false, false);
        PIXEndNamedEvent();
    }

    bool sendUpdate = forceUpdate;

    if (!sendUpdate && (flags & Block::UPDATE_CLIENTS) != 0)
    {
        if (mIsClientSide)
        {
            if ((flags & Block::UPDATE_INVISIBLE) != 0)
            {
                return true;
            }

            sendUpdate = true;
        }
        else if (chunk->isReady())
        {
            sendUpdate = true;
        }
    }

    if (sendUpdate)
    {
        sendBlockUpdated(pos, oldState, state, flags, true);
    }

    if (mIsClientSide)
    {
        return true;
    }

    if (!forceUpdate && (flags & Block::UPDATE_NEIGHBORS) == 0)
    {
        if ((flags & Block::UPDATE_NO_OBSERVERS) == 0)
        {
            updateObservingBlocksAt(pos, block);
        }

        return true;
    }

    blockUpdated(pos, oldState->getBlock(), (flags & Block::UPDATE_NO_OBSERVERS) == 0);

    if (state->hasAnalogOutputSignal())
    {
        updateNeighbourForOutputSignal(pos, block);
    }

    if (extraBlock && state != oldState)
    {
        neighborChanged(pos, block, pos);
    }

    return true;
}

bool Level::setData(const BlockPos& pos, int data, int flags, bool forceUpdate)
{
    return setData(pos.x, pos.y, pos.z, data, flags, forceUpdate);
}

bool Level::setData(int x, int y, int z, int data, int flags, bool forceUpdate)
{
    if (!isInWorldBounds(x, y, z))
    {
        return false;
    }

    LevelChunk* chunk = getChunk(x >> 4, z >> 4);
    BlockPos local(x & 0xF, y, z & 0xF);
    bool lightChanged;
    bool changed = chunk->setData(x & 0xF, y, z & 0xF, data, getBlock(local)->mBlockData, &lightChanged);

    if (!changed && !forceUpdate)
    {
        return changed;
    }

    Block* block = chunk->getBlock(local);
    BlockPos pos(x, y, z);

    if (forceUpdate || (flags & Block::UPDATE_CLIENTS) != 0)
    {
        if (forceUpdate || !mIsClientSide || (flags & Block::UPDATE_INVISIBLE) == 0)
        {
            sendBlockUpdated(pos, block->defaultBlockState(), block->defaultBlockState(), flags, true);
        }
        else
        {
            return changed;
        }
    }

    if (mIsClientSide)
    {
        return changed;
    }

    if (!forceUpdate && (flags & Block::UPDATE_NEIGHBORS) == 0)
    {
        if ((flags & Block::UPDATE_NO_OBSERVERS) == 0)
        {
            updateObservingBlocksAt(pos, block);
        }
    }
    else
    {
        blockUpdated(pos, block, false);

        if (block != NULL && block->getBlockState(data)->hasAnalogOutputSignal())
        {
            updateNeighbourForOutputSignal(pos, block);
        }
    }

    return changed;
}

bool Level::setBlockAndData(int x, int y, int z, int blockId, int data, int flags)
{
    if (!isInWorldBounds(x, y, z))
    {
        return false;
    }

    LevelChunk* chunk = getChunk(x >> 4, z >> 4);

    if ((flags & Block::UPDATE_CHUNK_ONLY) != 0)
    {
        chunk->setBlockAndData(x & 0xF, y, z & 0xF, blockId, data, false, true);
        return true;
    }

    int oldBlockId = chunk->getBlockId(x & 0xF, y, z & 0xF);
    bool changed = chunk->setBlockAndData(x & 0xF, y, z & 0xF, blockId, data, true, false);
    BlockPos pos(x, y, z);

    if ((flags & Block::UPDATE_LIGHT_IF_CHANGED) == 0)
    {
        checkLight(pos, false, false);
    }

    if (!changed)
    {
        return changed;
    }

    Block* block = Block::byId(blockId);
    Block* oldBlock = Block::byId(oldBlockId);
    bool immediate = (flags & Block::UPDATE_CLIENTS) != 0 && (!mIsClientSide || (flags & Block::UPDATE_INVISIBLE) == 0);

    sendBlockUpdated(pos, block->defaultBlockState(), oldBlock->defaultBlockState(), flags, immediate);

    if (mIsClientSide)
    {
        return changed;
    }

    if ((flags & Block::UPDATE_NEIGHBORS) != 0)
    {
        blockUpdated(pos, oldBlock, (flags & Block::UPDATE_NO_OBSERVERS) == 0);

        if (block != NULL && block->getBlockState(data)->hasAnalogOutputSignal())
        {
            updateNeighbourForOutputSignal(pos, block);
        }
    }
    else if ((flags & Block::UPDATE_NO_OBSERVERS) == 0)
    {
        updateObservingBlocksAt(pos, block);
    }

    return changed;
}

bool Level::removeBlock(const BlockPos& pos)
{
    return setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
}

void Level::levelEvent(boost::shared_ptr<Player> player, int type, const BlockPos& pos, int data)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->levelEvent(player, type, pos, data);
    }
}

void Level::levelEvent(int type, const BlockPos& pos, int data)
{
    levelEvent(boost::shared_ptr<Player>(), type, pos, data);
}

bool Level::destroyBlock(const BlockPos& pos, bool dropResources)
{
    const BlockState* state = getBlockState(pos);
    Block* block = state->getBlock();

    if (state->getMaterial() == Material::AIR)
    {
        return false;
    }

    levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, pos, Block::getIdWithData(state));

    if (dropResources)
    {
        block->spawnResources(this, pos, state, 0);
    }

    return setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
}

bool Level::setBlockAndUpdate(const BlockPos& pos, const BlockState* state)
{
    return setBlock(pos, state, Block::UPDATE_ALL, false);
}

void Level::sendBlockUpdated(const BlockPos& pos, const BlockState* oldState, const BlockState* newState, int flags,
                             bool immediate)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->blockChanged(this, pos, oldState, newState, flags, immediate);
    }
}

void Level::lightColumnChanged(int x, int z, int fromY, int toY)
{
    PIXBeginNamedEvent(0.0f, "LightColumnChanged (%d,%d) %d to %d", x, z, fromY, toY);

    int minY = fromY;
    int maxY = toY;

    if (toY < fromY)
    {
        minY = toY;
        maxY = fromY;
    }

    if (mDimension->isHasSkyLight())
    {
        PIXBeginNamedEvent(0.0f, "Checking lights");

        for (int y = minY; y <= maxY; ++y)
        {
            PIXBeginNamedEvent(0.0f, "Checking light %d", y);
            checkLight(LightLayer::SKY, BlockPos(x, y, z), false, false);
            PIXEndNamedEvent();
        }

        PIXEndNamedEvent();
    }

    PIXBeginNamedEvent(0.0f, "Setting blocks dirty");
    setBlocksDirty(BlockPos(x, minY, z), BlockPos(x, maxY, z));
    PIXEndNamedEvent();

    PIXEndNamedEvent();
}

void Level::updateNeighborsAtExceptFromFacing(const BlockPos& pos, Block* block, const Direction* skipSide)
{
    if (skipSide != Direction::WEST)
    {
        neighborChanged(pos.west(), block, pos);
    }

    if (skipSide != Direction::EAST)
    {
        neighborChanged(pos.east(), block, pos);
    }

    if (skipSide != Direction::DOWN)
    {
        neighborChanged(pos.below(), block, pos);
    }

    if (skipSide != Direction::UP)
    {
        neighborChanged(pos.above(), block, pos);
    }

    if (skipSide != Direction::NORTH)
    {
        neighborChanged(pos.north(), block, pos);
    }

    if (skipSide != Direction::SOUTH)
    {
        neighborChanged(pos.south(), block, pos);
    }
}

bool Level::isBlockToBeTickedAt(const BlockPos& pos, Block* block)
{
    return false;
}

bool Level::canSeeSkyFromBelowWater(const BlockPos& pos)
{
    if (pos.y > 62)
    {
        return canSeeSky(pos);
    }

    BlockPos current(pos.x, 63, pos.z);

    if (!canSeeSky(current))
    {
        return false;
    }

    for (current = current.below(); current.y > pos.y; current = current.below())
    {
        const BlockState* state = getBlockState(current);
        const BlockState* extraState = getExtraBlockState(current);
        int blocking = state->getLightBlock();
        int extraBlocking = extraState->getLightBlock();

        if (blocking < extraBlocking)
        {
            blocking = extraBlocking;
        }

        if (blocking > 0 && !state->getMaterial()->isLiquid() && !extraState->getMaterial()->isLiquid())
        {
            return false;
        }
    }

    return true;
}

int Level::getDaytimeRawBrightness(const BlockPos& pos)
{
    if (pos.y < 0)
    {
        return 0;
    }

    BlockPos clamped = pos;

    if (pos.y > 255)
    {
        clamped = BlockPos(pos.x, 255, pos.z);
    }

    return getChunkAt(clamped)->getRawBrightness(clamped, 0);
}

int Level::getRawBrightness(const BlockPos& pos, bool useNeighbourBrightness)
{
    if (!isWithinLevelBounds(pos.x, pos.z))
    {
        return 15;
    }

    if (useNeighbourBrightness && getBlockState(pos)->doesPropagate())
    {
        int brightest = getRawBrightness(pos.above(), false);
        int east = getRawBrightness(pos.east(), false);
        int west = getRawBrightness(pos.west(), false);
        int south = getRawBrightness(pos.south(), false);
        int north = getRawBrightness(pos.north(), false);

        if (brightest < east)
        {
            brightest = east;
        }

        if (brightest < west)
        {
            brightest = west;
        }

        if (brightest < south)
        {
            brightest = south;
        }

        if (brightest < north)
        {
            brightest = north;
        }

        return brightest;
    }

    if (pos.y < 0)
    {
        return 0;
    }

    BlockPos clamped = pos;

    if (pos.y > 255)
    {
        clamped = BlockPos(pos.x, 255, pos.z);
    }

    return getChunkAt(clamped)->getRawBrightness(clamped, mSkyDarken);
}

int Level::getRawBrightness(const BlockPos& pos)
{
    return getRawBrightness(pos, true);
}

int Level::getHeightmapHeight(int x, int z)
{
    if (!isWithinLevelBounds(x, z))
    {
        return getSeaLevel() + 1;
    }

    if (!hasChunk(x >> 4, z >> 4, true))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getHeightmap(x & 0xF, z & 0xF);
}

BlockPos Level::getHeightmapPos(const BlockPos& pos)
{
    return BlockPos(pos.x, getHeightmapHeight(pos.x, pos.z), pos.z);
}

int Level::getHeightmap(int x, int z)
{
    if (!isWithinLevelBounds(x, z))
    {
        return 64;
    }

    if (!hasChunk(x >> 4, z >> 4, true))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getHeightmap(x & 0xF, z & 0xF);
}

int Level::getLowestHeightmap(int x, int z)
{
    if (!isWithinLevelBounds(x, z))
    {
        return 64;
    }

    if (!hasChunk(x >> 4, z >> 4, true))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getLowestHeightmap();
}

int Level::getBrightnessPropagate(LightLayer::variety layer, const BlockPos& pos)
{
    if (!mDimension->isHasSkyLight() && layer == LightLayer::SKY)
    {
        return 0;
    }

    BlockPos target = pos;

    if (pos.y < 0)
    {
        target = BlockPos(pos.x, 0, pos.z);
    }

    if (!isInWorldBounds(target) || !hasChunkAt(target))
    {
        return layer;
    }

    if (!getBlockState(target)->doesPropagate())
    {
        return getChunkAt(target)->getBrightness(layer, target);
    }

    int brightest = getBrightness(layer, target.above());
    int east = getBrightness(layer, target.east());
    int west = getBrightness(layer, target.west());
    int south = getBrightness(layer, target.south());
    int north = getBrightness(layer, target.north());

    if (brightest < east)
    {
        brightest = east;
    }

    if (brightest < west)
    {
        brightest = west;
    }

    if (brightest < south)
    {
        brightest = south;
    }

    if (brightest < north)
    {
        brightest = north;
    }

    return brightest;
}

void Level::getNeighbourBrightnesses(int* brightnesses, LightLayer::variety layer, int x, int y, int z)
{
    if ((x & 0xF) != 0 && (x & 0xF) != 15 && (z & 0xF) != 0 && (z & 0xF) != 15 && (unsigned int)(y - 1) < 126)
    {
        int size = mChunkCacheSize;
        int half = size / 2;
        int cacheX = (x >> 4) + half;
        int cacheZ = (z >> 4) + half;

        if (cacheX >= 0 && cacheX < size && cacheZ >= 0 && cacheZ < size)
        {
            LevelChunk* chunk = mChunkCache[cacheX * size + cacheZ];

            if (chunk != NULL)
            {
                chunk->getNeighbourBrightnesses(brightnesses, layer, x & 0xF, y, z & 0xF);
                return;
            }
        }

        brightnesses[0] = layer;
        brightnesses[1] = layer;
        brightnesses[2] = layer;
        brightnesses[3] = layer;
        brightnesses[4] = layer;
        brightnesses[5] = layer;
        return;
    }

    brightnesses[0] = getBrightness(layer, BlockPos(x - 1, y, z));
    brightnesses[1] = getBrightness(layer, BlockPos(x + 1, y, z));
    brightnesses[2] = getBrightness(layer, BlockPos(x, y - 1, z));
    brightnesses[3] = getBrightness(layer, BlockPos(x, y + 1, z));
    brightnesses[4] = getBrightness(layer, BlockPos(x, y, z - 1));
    brightnesses[5] = getBrightness(layer, BlockPos(x, y, z + 1));
}

int Level::getBrightness(LightLayer::variety layer, const BlockPos& pos)
{
    int size = mChunkCacheSize;
    int half = size / 2;
    int cacheX = (pos.x >> 4) + half;
    int cacheZ = (pos.z >> 4) + half;

    if (cacheX < 0 || cacheX >= size || cacheZ < 0 || cacheZ >= size)
    {
        return 0;
    }

    LevelChunk* chunk = mChunkCache[cacheX * size + cacheZ];

    if (chunk != NULL)
    {
        BlockPos target(pos.x, pos.y, pos.z);

        if (target.y < 0)
        {
            target = BlockPos(target.x, 0, target.z);
        }

        if (isInWorldBounds(target))
        {
            return chunk->getBrightness(layer, target);
        }
    }

    return layer;
}

int Level::getLightColor(const BlockPos& pos, int lightValue, int cacheIndex)
{
    int skyLight = getBrightnessPropagate(LightLayer::SKY, pos);
    int blockLight = getBrightnessPropagate(LightLayer::BLOCK, pos);

    if (blockLight < lightValue)
    {
        blockLight = lightValue;
    }

    return (skyLight << 20) | (blockLight << 4);
}

float Level::getBrightness(const BlockPos& pos, int lightValue)
{
    int brightness = getRawBrightness(pos);

    if (brightness < lightValue)
    {
        brightness = lightValue;
    }

    return mDimension->getBrightnessRamp()[brightness];
}

float Level::getBrightness(const BlockPos& pos)
{
    return mDimension->getBrightnessRamp()[getRawBrightness(pos)];
}

const BlockState* Level::getBlockState(const BlockPos& pos)
{
    if (isOutsideBuildHeight(pos))
    {
        return Blocks::air->defaultBlockState();
    }

    return getChunkAt(pos)->getBlockState(pos);
}

const BlockState* Level::getBlockState(EntityTickCache& tickCache, const BlockPos& pos)
{
    const BlockState* state = tickCache.getBlockState(pos.x, pos.y, pos.z);

    if (state == NULL)
    {
        return getBlockState(pos);
    }

    return state;
}

const BlockState* Level::getBlockState(LevelChunk* chunk, EntityTickCache& tickCache, const BlockPos& pos)
{
    const BlockState* state = tickCache.getBlockState(pos.x, pos.y, pos.z);

    if (state != NULL)
    {
        return state;
    }

    if (isOutsideBuildHeight(pos))
    {
        return Blocks::air->defaultBlockState();
    }

    return chunk->getBlockState(pos);
}

int Level::getBlockId(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getBlockId(x & 0xF, y, z & 0xF);
}

int Level::getBlockId(EntityTickCache& tickCache, int x, int y, int z)
{
    int element = tickCache.getElement(x, y, z);

    if (element == 0xFFFF)
    {
        element = getElement(x, y, z);
    }

    return element >> 4;
}

unsigned short Level::getElement(EntityTickCache& tickCache, int x, int y, int z)
{
    unsigned short element = tickCache.getElement(x, y, z);

    if (element == 0xFFFF)
    {
        return getElement(x, y, z);
    }

    return element;
}

unsigned short Level::getElement(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getElement(x & 0xF, y, z & 0xF);
}

unsigned short Level::getExtraElement(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getExtraElement(x & 0xF, y, z & 0xF);
}

void Level::getElementAndExtraElement(int x, int y, int z, unsigned short* element, unsigned short* extraElement)
{
    if (!isInWorldBounds(x, y, z))
    {
        *element = 0;
        *extraElement = 0;
        return;
    }

    getChunk(x >> 4, z >> 4)->getElementAndExtraElement(x & 0xF, y, z & 0xF, element, extraElement);
}

bool Level::sectionContainsBlockId(int x, int y, int z, int blockId)
{
    if (!isInWorldBounds(x, y, z))
    {
        return false;
    }

    return getChunk(x >> 4, z >> 4)->sectionContainsBlockId(x & 0xF, y, z & 0xF, blockId);
}

int Level::getExtraBlockId(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getExtraBlockId(x & 0xF, y, z & 0xF);
}

void Level::getBlockAndExtraBlockId(int x, int y, int z, int* blockId, int* extraBlockId)
{
    if (!isInWorldBounds(x, y, z))
    {
        *blockId = 0;
        *extraBlockId = 0;
        return;
    }

    getChunk(x >> 4, z >> 4)->getBlockAndExtraId(x & 0xF, y, z & 0xF, blockId, extraBlockId);
}

Block* Level::getExtraBlock(const BlockPos& pos)
{
    return Block::byId(getExtraBlockId(pos.x, pos.y, pos.z));
}

void Level::getBlockAndExtraBlock(const BlockPos& pos, Block** block, Block** extraBlock)
{
    int blockId = 0;
    int extraBlockId = 0;
    getBlockAndExtraBlockId(pos.x, pos.y, pos.z, &blockId, &extraBlockId);
    *block = Block::byId(blockId);
    *extraBlock = Block::byId(extraBlockId);
}

Block* Level::getLiquidBlock(const BlockPos& pos)
{
    Block* extraBlock = getExtraBlock(pos);

    if (extraBlock == NULL || extraBlock == Blocks::air)
    {
        return getBlock(pos);
    }

    return extraBlock;
}

const BlockState* Level::getLiquidBlockState(const BlockPos& pos)
{
    const BlockState* extraState = getExtraBlockState(pos);

    if (extraState != NULL && extraState != Blocks::air->defaultBlockState())
    {
        return extraState;
    }

    return getBlockState(pos);
}

const BlockState* Level::getExtraBlockState(const BlockPos& pos)
{
    if (isOutsideBuildHeight(pos))
    {
        return Blocks::air->defaultBlockState();
    }

    return getChunkAt(pos)->getExtraBlockState(pos);
}

int Level::getBlockData(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getData(x & 0xF, y, z & 0xF);
}

int Level::getExtraBlockData(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getExtraBlockData(x & 0xF, y, z & 0xF);
}

Block* Level::getBlock(const BlockPos& pos)
{
    return Block::byId(getBlockId(pos.x, pos.y, pos.z));
}

bool Level::isSolidRender(const BlockPos& pos)
{
    const BlockState* state = getBlockState(pos);

    if (state == NULL)
    {
        return false;
    }

    if (!LeafBlock::isLeaves(state->getBlock()))
    {
        return state->getBlock()->isSolidRender(state);
    }

    int xOffsets[6] = {1, -1, 0, 0, 0, 0};
    int yOffsets[6] = {0, 0, 1, -1, 0, 0};
    int zOffsets[6] = {0, 0, 0, 0, 1, -1};

    for (int i = 0; i < 6; ++i)
    {
        const BlockState* neighbour = getBlockState(pos.offset(xOffsets[i], yOffsets[i], zOffsets[i]));

        if (!LeafBlock::isLeaves(neighbour->getBlock()) || !neighbour->isSolidRender())
        {
            return false;
        }
    }

    return true;
}

bool Level::isDay()
{
    return mSkyDarken < 4;
}

HitResult* Level::clip(
    Vec3* from, Vec3* to, bool stopOnLiquid, bool ignoreBlocksWithoutShape, bool returnLastUncollidableBlock,
    bool testUseBlocks,
    boost::unordered_set<Block*, boost::hash<Block*>, std::equal_to<Block*>, std::allocator<Block*> >* alwaysClipBlocks,
    bool clipExtraBlocks)
{
    int targetX = Mth::fastFloor(to->x);
    int targetY = Mth::fastFloor(to->y);
    int targetZ = Mth::fastFloor(to->z);
    int currentX = Mth::fastFloor(from->x);
    int currentY = Mth::fastFloor(from->y);
    int currentZ = Mth::fastFloor(from->z);

    MutableBlockPos current(currentX, currentY, currentZ);
    MutableBlockPos target(targetX, targetY, targetZ);

    const BlockState* state = getBlockState(current);
    const BlockState* extraState = getExtraBlockState(current);
    Block* block = state->getBlock();
    Block* extraBlock = extraState->getBlock();

    if (!ignoreBlocksWithoutShape || state->getClipAABB(this, current) != NULL || (testUseBlocks && block->TestUse())
        || (alwaysClipBlocks != NULL && alwaysClipBlocks->find(block) != alwaysClipBlocks->end()))
    {
        if (block->mayPick(state, stopOnLiquid))
        {
            HitResult* hit = state->clip(this, current, from, to);

            if (hit != NULL)
            {
                return hit;
            }
        }
        else if (extraBlock->mayPick(extraState, stopOnLiquid))
        {
            HitResult* hit = extraState->clip(this, current, from, to);

            if (hit != NULL)
            {
                return hit;
            }
        }
    }

    HitResult* lastUncollidable = NULL;
    int steps = 199;

    while (true)
    {
        if (currentX == targetX && currentY == targetY && currentZ == targetZ)
        {
            if (returnLastUncollidableBlock)
            {
                return lastUncollidable;
            }

            delete lastUncollidable;
            return NULL;
        }

        bool crossX = true;
        bool crossY = true;
        bool crossZ = true;
        double planeX = 999.0;
        double planeY = 999.0;
        double planeZ = 999.0;

        if (targetX > currentX)
        {
            planeX = (double)currentX + 1.0;
        }
        else if (targetX < currentX)
        {
            planeX = (double)currentX;
        }
        else
        {
            crossX = false;
        }

        if (targetY > currentY)
        {
            planeY = (double)currentY + 1.0;
        }
        else if (targetY < currentY)
        {
            planeY = (double)currentY;
        }
        else
        {
            crossY = false;
        }

        if (targetZ > currentZ)
        {
            planeZ = (double)currentZ + 1.0;
        }
        else if (targetZ < currentZ)
        {
            planeZ = (double)currentZ;
        }
        else
        {
            crossZ = false;
        }

        double deltaX = to->x - from->x;
        double deltaY = to->y - from->y;
        double deltaZ = to->z - from->z;
        double partX = 999.0;
        double partY = 999.0;
        double partZ = 999.0;

        if (crossX)
        {
            partX = (planeX - from->x) / deltaX;
        }

        if (crossY)
        {
            partY = (planeY - from->y) / deltaY;
        }

        if (crossZ)
        {
            partZ = (planeZ - from->z) / deltaZ;
        }

        if (partX == -0.0)
        {
            partX = -1.0E-4;
        }

        if (partY == -0.0)
        {
            partY = -1.0E-4;
        }

        if (partZ == -0.0)
        {
            partZ = -1.0E-4;
        }

        const Direction* side;

        if (partX < partY && partX < partZ)
        {
            side = (targetX > currentX) ? Direction::WEST : Direction::EAST;
            from = Vec3::newTemp(planeX, from->y + deltaY * partX, from->z + deltaZ * partX);
        }
        else if (partY < partZ)
        {
            side = (targetY > currentY) ? Direction::DOWN : Direction::UP;
            from = Vec3::newTemp(from->x + deltaX * partY, planeY, from->z + deltaZ * partY);
        }
        else
        {
            side = (targetZ > currentZ) ? Direction::NORTH : Direction::SOUTH;
            from = Vec3::newTemp(from->x + deltaX * partZ, from->y + deltaY * partZ, planeZ);
        }

        currentX = Mth::fastFloor(from->x) - ((side == Direction::EAST) ? 1 : 0);
        currentY = Mth::fastFloor(from->y) - ((side == Direction::UP) ? 1 : 0);
        currentZ = Mth::fastFloor(from->z) - ((side == Direction::SOUTH) ? 1 : 0);
        current.set(currentX, currentY, currentZ);

        state = getBlockState(current);
        block = state->getBlock();
        extraState = getExtraBlockState(current);
        extraBlock = extraState->getBlock();

        if (!ignoreBlocksWithoutShape || state->getMaterial() == Material::PORTAL
            || state->getClipAABB(this, current) != NULL || (testUseBlocks && block->TestUse())
            || (alwaysClipBlocks != NULL && alwaysClipBlocks->find(block) != alwaysClipBlocks->end()))
        {
            if (block->mayPick(state, stopOnLiquid))
            {
                HitResult* hit = state->clip(this, current, from, to);

                if (hit != NULL)
                {
                    delete lastUncollidable;
                    return hit;
                }
            }
            else if (extraBlock->mayPick(extraState, stopOnLiquid))
            {
                HitResult* hit = extraState->clip(this, current, from, to);

                if (hit != NULL)
                {
                    return hit;
                }
            }
            else
            {
                delete lastUncollidable;
                lastUncollidable = new HitResult(HitResult::MISS, from, side, current);
            }
        }

        if (steps < 0)
        {
            break;
        }

        --steps;
    }

    if (returnLastUncollidableBlock)
    {
        return lastUncollidable;
    }

    delete lastUncollidable;
    return NULL;
}

HitResult* Level::clip(Vec3* from, Vec3* to)
{
    return clip(from, to, false, false, false, false, NULL, false);
}

HitResult* Level::clip(Vec3* from, Vec3* to, bool stopOnLiquid)
{
    return clip(from, to, stopOnLiquid, false, false, false, NULL, false);
}

void Level::playSoundToAllPlayers(const BlockPos& pos, const SoundEvent* sound, SoundSource::ESoundSource source,
                                  float volume, float pitch, float range)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->playSoundToAllPlayers(sound, source, (double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5,
                                     volume, pitch, range);
    }
}

void Level::playSound(boost::shared_ptr<Player> player, const BlockPos& pos, const SoundEvent* sound,
                      SoundSource::ESoundSource source, float volume, float pitch, float range)
{
    playSound(player, (double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5, sound, source, volume, pitch,
              16.0f);
}

void Level::playSound(boost::shared_ptr<Player> player, double x, double y, double z, const SoundEvent* sound,
                      SoundSource::ESoundSource source, float volume, float pitch, float range)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->playSound(player, sound, source, x, y, z, volume, pitch, range);
    }
}

void Level::playLocalSound(double x, double y, double z, const SoundEvent* sound, SoundSource::ESoundSource source,
                           float volume, float pitch, bool distanceDelay, float range)
{
}

void Level::playStreamingMusic(const BlockPos& pos, const SoundEvent* sound, bool playing)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->playStreamingMusic(sound, pos, playing);
    }
}

void Level::addParticle(ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z, double xa, double ya,
                        double za, arrayWithLength<int> params)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->addParticle(this, type, overrideLimiter, x, y, z, xa, ya, za, params);
    }
}

void Level::addParticle(const ParticleType* type, double x, double y, double z, double xa, double ya, double za,
                        arrayWithLength<int> params)
{
    addParticle(type->getId(), type->getOverrideLimiter(), x, y, z, xa, ya, za, params);
}

void Level::addAlwaysVisibleParticle(ePARTICLE_TYPE type, double x, double y, double z, double xa, double ya, double za,
                                     arrayWithLength<int> params)
{
    for (unsigned int i = 0; i < mListeners.size(); ++i)
    {
        mListeners[i]->addParticle(this, type, false, true, x, y, z, xa, ya, za, params);
    }
}

void Level::addParticle(const ParticleType* type, bool overrideLimiter, double x, double y, double z, double xa,
                        double ya, double za, arrayWithLength<int> params)
{
    addParticle(type->getId(), type->getOverrideLimiter() || overrideLimiter, x, y, z, xa, ya, za, params);
}

bool Level::addGlobalEntity(boost::shared_ptr<Entity> entity)
{
    mGlobalEntities.push_back(entity);
    return true;
}

bool Level::addEntity(boost::shared_ptr<Entity> entity)
{
    entity->fjCheckDerivedConstruction();

    int chunkX = Mth::floor(entity->mX * 0.0625);
    int chunkZ = Mth::floor(entity->mZ * 0.0625);
    if (entity == NULL)
    {
        return false;
    }

    bool forcedLoading = entity->mForcedLoading;
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        forcedLoading = true;
    }

    if (forcedLoading || hasChunk(chunkX, chunkZ, false))
    {
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            std::vector<boost::shared_ptr<Player> >::iterator it = std::find(mPlayers.begin(), mPlayers.end(), entity);
            if (it == mPlayers.end())
            {
                mPlayers.push_back(player);
            }

            updateSleepingPlayerList();
        }

        MemSect(0x2A);
        getChunk(chunkX, chunkZ)->addEntity(entity);
        MemSect(0);

        EnterCriticalSection(&mEntityMutex);
        MemSect(0x2B);
        mEntities.push_back(entity);
        MemSect(0);
        LeaveCriticalSection(&mEntityMutex);

        MemSect(0x2C);
        entityAdded(entity);
        MemSect(0);
        return true;
    }
    return false;
}

void Level::entityAdded(boost::shared_ptr<Entity> entity)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->entityAdded(entity);
    }
}

void Level::entityRemoved(boost::shared_ptr<Entity> entity)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->entityRemoved(entity);
    }
}

void Level::playerRemoved(boost::shared_ptr<Entity> player)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->playerRemoved(player);
    }
}

void Level::removeEntity(boost::shared_ptr<Entity> entity)
{
    if (entity->isVehicle())
    {
        entity->ejectPassengers();
    }

    if (entity->isPassenger())
    {
        entity->stopRiding();
    }

    entity->remove();

    bool playerEntity = eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType());
    if (playerEntity)
    {
        std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin();
        while (it != mPlayers.end() && *it != boost::static_pointer_cast<Player>(entity))
        {
            ++it;
        }
        if (it != mPlayers.end())
        {
            mPlayers.erase(it);
        }
        updateSleepingPlayerList();
        playerRemoved(entity);
    }
}

void Level::removeEntityImmediately(boost::shared_ptr<Entity> entity)
{
    entity->setDropContainerContent(false);
    entity->remove();

    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        std::vector<boost::shared_ptr<Player> >::iterator playerIt = mPlayers.begin();

        while (playerIt != mPlayers.end() && *playerIt != boost::static_pointer_cast<Player>(entity))
        {
            ++playerIt;
        }

        if (playerIt != mPlayers.end())
        {
            mPlayers.erase(playerIt);
        }

        updateSleepingPlayerList();
        playerRemoved(entity);
    }

    if (entity->mInChunk && hasChunk(entity->mChunkX, entity->mChunkZ, true))
    {
        getChunk(entity->mChunkX, entity->mChunkZ)->removeEntity(entity);
    }

    EnterCriticalSection(&mEntityMutex);

    std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin();

    while (it != mEntities.end() && *it != entity)
    {
        ++it;
    }

    if (it != mEntities.end())
    {
        mEntities.erase(it);
    }

    LeaveCriticalSection(&mEntityMutex);
    entityRemoved(entity);
}

void Level::addListener(LevelListener* listener)
{
    mListeners.push_back(listener);
}

void Level::removeListener(LevelListener* listener)
{
    std::vector<LevelListener*>::iterator it = mListeners.begin();

    while (it != mListeners.end() && *it != listener)
    {
        ++it;
    }

    if (it == mListeners.end())
    {
        return;
    }

    mListeners.erase(it);
}

bool Level::isInsideWorldBorder(boost::shared_ptr<Entity> entity)
{
    const AABB* constraints = mDimension->getPlayerConstraints(BlockPos(entity));
    double minX = constraints->mMinX;
    double maxX = constraints->mMaxX;
    double minZ = constraints->mMinZ;
    double maxZ = constraints->mMaxZ;

    if (entity->mOutsideWorldBorder)
    {
        minX += 1.0;
        maxX -= 1.0;
        minZ += 1.0;
        maxZ -= 1.0;
    }
    else
    {
        minX -= 1.0;
        maxX += 1.0;
        minZ -= 1.0;
        maxZ += 1.0;
    }

    return entity->mX > minX && entity->mX < maxX && entity->mZ > minZ && entity->mZ < maxZ;
}

bool Level::checkAndHandleWater(AABB* box, Material* material, boost::shared_ptr<Entity> entity)
{
    int minX = Mth::floor(box->mMinX);
    int maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    int minY = Mth::floor(box->mMinY);
    int maxY = Mth::ceil(static_cast<float>(box->mMaxY));
    int minZ = Mth::floor(box->mMinZ);
    int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));

    if (!hasChunksAt(minX, minY, minZ, maxX, maxY, maxZ, true))
    {
        return false;
    }
    bool found = false;
    Vec3* flow = Vec3::newTemp(0.0, 0.0, 0.0);

    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                BlockPos pos(x, y, z);
                const BlockState* state = getBlockState(entity->mTickCache, pos);
                Block* block = state->getBlock();

                if (state->getMaterial() == material)
                {
                    int data = state->getValue(LiquidBlock::LEVEL);
                    double surface = (y + 1) - LiquidBlock::getHeightFromData(data);

                    if (surface <= maxY)
                    {
                        found = true;
                        flow = block->handleEntityInside(this, pos, entity, flow);
                    }
                }
            }
        }
    }

    if (flow->length() > 0.0 && entity->isPushedByWater())
    {
        flow = flow->normalize();
        entity->mXd += flow->x * 0.014;
        entity->mYd += flow->y * 0.014;
        entity->mZd += flow->z * 0.014;
    }

    return found;
}

float Level::getTimeOfDay(float partialTick)
{
    return mDimension->getTimeOfDay(mLevelData->getDayTime(), partialTick);
}

float Level::getRainLevel(float partialTick)
{
    return mOldRainLevel + (mRainLevel - mOldRainLevel) * partialTick;
}

float Level::getThunderLevel(float partialTick)
{
    return (mOldThunderLevel + (mThunderLevel - mOldThunderLevel) * partialTick) * getRainLevel(partialTick);
}

int Level::getOldSkyDarken(float partialTick)
{
    float daylight = Mth::clamp(1.0f - (Mth::cos(getTimeOfDay(partialTick) * 6.2831855f) * 2.0f + 0.5f), 0.0f, 1.0f);
    daylight = 1.0f - daylight;
    daylight *= 1.0f - getRainLevel(partialTick) * 5.0f / 16.0f;
    daylight *= 1.0f - getThunderLevel(partialTick) * 5.0f / 16.0f;
    return static_cast<int>((1.0f - daylight) * 11.0f);
}

float Level::getSkyDarken(float partialTick)
{
    float daylight = Mth::clamp(1.0f - (Mth::cos(getTimeOfDay(partialTick) * 6.2831855f) * 2.0f + 0.2f), 0.0f, 1.0f);
    daylight = 1.0f - daylight;
    daylight *= 1.0f - getRainLevel(partialTick) * 5.0f / 16.0f;
    daylight *= 1.0f - getThunderLevel(partialTick) * 5.0f / 16.0f;
    return daylight * 0.8f + 0.2f;
}

int Level::getMoonPhase()
{
    return mDimension->getMoonPhase(mLevelData->getDayTime());
}

float Level::getMoonBrightness()
{
    return Dimension::MOON_BRIGHTNESS_PER_PHASE[mDimension->getMoonPhase(mLevelData->getDayTime())];
}

float Level::getSunAngle(float partialTick)
{
    return getTimeOfDay(partialTick) * 6.2831855f;
}

Vec3* Level::getCloudColor(float partialTick)
{
    float daylight = Mth::clamp(Mth::cos(getTimeOfDay(partialTick) * 6.2831855f) * 2.0f + 0.5f, 0.0f, 1.0f);
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_In_Cloud_Base_Colour);
    float red = static_cast<float>((colour >> 16) & 255) / 255.0f;
    float green = static_cast<float>((colour >> 8) & 255) / 255.0f;
    float blue = static_cast<float>(colour & 255) / 255.0f;
    float rain = getRainLevel(partialTick);
    if (rain > 0.0f)
    {
        float grey = (red * 0.3f + green * 0.59f + blue * 0.11f) * 0.6f;
        float clear = 1.0f - rain * 0.95f;
        red = red * clear + grey * (1.0f - clear);
        green = green * clear + grey * (1.0f - clear);
        blue = blue * clear + grey * (1.0f - clear);
    }
    red *= daylight * 0.9f + 0.1f;
    green *= daylight * 0.9f + 0.1f;
    blue *= daylight * 0.85f + 0.15f;
    float thunder = getThunderLevel(partialTick);
    if (thunder > 0.0f)
    {
        float grey = (red * 0.3f + green * 0.59f + blue * 0.11f) * 0.2f;
        float clear = 1.0f - thunder * 0.95f;
        red = red * clear + grey * (1.0f - clear);
        green = green * clear + grey * (1.0f - clear);
        blue = blue * clear + grey * (1.0f - clear);
    }
    return Vec3::newTemp(red, green, blue);
}

bool Level::biomeHasRain(const BlockPos& pos)
{
    return getChunkAt(pos)->biomeHasRain(pos.x & 15, pos.z & 15);
}

bool Level::biomeHasSnow(const BlockPos& pos)
{
    return getChunkAt(pos)->biomeHasSnow(pos.x & 15, pos.z & 15);
}

float Level::getStarBrightness(float partialTick)
{
    float brightness = Mth::clamp(1.0f - (Mth::cos(getTimeOfDay(partialTick) * 6.2831855f) * 2.0f + 0.25f), 0.0f, 1.0f);
    return brightness * brightness * 0.5f;
}

const Difficulty* Level::getDifficulty()
{
    return getLevelData()->getDifficulty();
}

DifficultyInstance Level::getCurrentDifficultyAt(const BlockPos& pos)
{
    float moonBrightness = 0.0f;
    long long inhabitedTime = 0;
    if (hasChunkAt(pos))
    {
        moonBrightness = getMoonBrightness();
        inhabitedTime = getChunkAt(pos)->getInhabitedTime();
    }
    return DifficultyInstance(getDifficulty(), getDayTime(), inhabitedTime, moonBrightness);
}

int Level::getSkyDarken()
{
    return mSkyDarken;
}

void Level::setSkyDarken(int value)
{
    mSkyDarken = value;
}

int Level::getSkyFlashTime()
{
    return mSkyFlashTime;
}

void Level::setSkyFlashTime(int value)
{
    mSkyFlashTime = value;
}

bool Level::isFindingSpawn()
{
    return mFindingSpawn;
}

void Level::setFindingSpawn(bool value)
{
    mFindingSpawn = value;
}

GameRules* Level::getGameRules()
{
    return mLevelData->getGameRules();
}

int Level::getMaxBuildHeight()
{
    return 256;
}

int Level::getHeight()
{
    return mDimension->isUltraWarm() ? 128 : 256;
}

WorldBorder* Level::getWorldBorder()
{
    return mWorldBorder;
}

int Level::getCloudHeight()
{
    return mLevelData->getCloudHeight();
}

int Level::getFogDistance()
{
    return mFogDistance;
}

void Level::setFogDistance(int value)
{
    mFogDistance = value;
}

bool Level::useNewSeaLevel()
{
    return mLevelData->useNewSeaLevel();
}

bool Level::isFlatLand()
{
    return getLevelData()->getGeneratorType() == LevelType::FLAT;
}

bool Level::AllPlayersAreSleeping()
{
    return false;
}

bool Level::isSpawnChunk(int chunkX, int chunkZ)
{
    BlockPos spawn = getSharedSpawnPos();
    return static_cast<unsigned int>(chunkX * 16 - spawn.x + 136) < 257
           && static_cast<unsigned int>(chunkZ * 16 - spawn.z + 136) < 257;
}

bool Level::getHasBeenInCreative()
{
    return mLevelData->getHasBeenInCreative();
}

bool Level::isGenerateMapFeatures()
{
    return mLevelData->isGenerateMapFeatures();
}

BlockPos Level::getTopRainBlockPos(const BlockPos& pos)
{
    return getChunkAt(pos)->getTopRainBlock(pos);
}

bool Level::isSnowTemperature(const BlockPos& pos)
{
    return getBiome(pos)->getTemperature(pos) < 0.15f;
}

bool Level::isRainingAt(const BlockPos& pos)
{
    if (!isRaining() || !canSeeSky(pos))
    {
        return false;
    }
    BlockPos top = getTopRainBlockPos(pos);
    return top.y <= pos.y && !biomeHasSnow(pos) && !isSnowTemperature(pos) && biomeHasRain(pos);
}

bool Level::isSnowingAt(const BlockPos& pos)
{
    if (!isRaining() || !canSeeSky(pos))
    {
        return false;
    }
    return getTopRainBlockPos(pos).y <= pos.y && isSnowTemperature(pos);
}

void Level::setThunderLevel(float value)
{
    mOldThunderLevel = value;
    mThunderLevel = value;
}

void Level::setRainLevel(float value)
{
    mOldRainLevel = value;
    mRainLevel = value;
}

bool Level::isThundering()
{
    return getThunderLevel(1.0f) > 0.9f;
}

bool Level::isRaining()
{
    return getRainLevel(1.0f) > 0.2f;
}

void Level::setSavedDataStorage(SavedDataStorage* storage)
{
    mSavedDataStorage = storage;
}

SavedDataStorage* Level::getSavedDataStorage()
{
    return mSavedDataStorage;
}

void Level::setSavedData(const std::wstring& id, boost::shared_ptr<SavedData> data)
{
    mSavedDataStorage->set(id, data);
}

boost::shared_ptr<SavedData> Level::getSavedData(const std::type_info& type, const std::wstring& id)
{
    return mSavedDataStorage->get(type, id);
}

int Level::getFreeAuxValueFor(const std::wstring& id)
{
    return mSavedDataStorage->getFreeAuxValueFor(id);
}

int Level::getAuxValueForMap(PlayerUID uid, int dimension, int x, int z, int scale)
{
    return mSavedDataStorage->getAuxValueForMap(uid, dimension, x, z, scale);
}

void Level::globalLevelEvent(int type, const BlockPos& pos, int data)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->globalLevelEvent(type, pos, data);
    }
}

Random* Level::getRandomFor(int x, int y, int z)
{
    mRandom->setSeed(static_cast<long long>(x) * 341873128712LL + static_cast<long long>(y) * 132897987541LL
                     + getLevelData()->getSeed() + z);
    return mRandom;
}

long long Level::fjGetRandomFor(long long seed, int x, int y, int z)
{
    return (long long)x * 341873128712LL + (long long)y * 132897987541LL + seed + z;
}

bool Level::isAllEmpty()
{
    return false;
}

double Level::getHorizonHeight()
{
    return mLevelData->getGeneratorType() == LevelType::FLAT ? 0.0 : 63.0;
}

void Level::destroyBlockProgress(int breakerId, const BlockPos& pos, int progress)
{
    std::vector<LevelListener*>::iterator end = mListeners.end();
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != end; ++it)
    {
        (*it)->destroyBlockProgress(breakerId, pos, progress);
    }
}

void Level::setSeaLevel(int value)
{
    mSeaLevel = value;
}

LevelType* Level::getGeneratorType()
{
    return mLevelData->getGeneratorType();
}

void Level::setSpawnSettings(bool spawnEnemies, bool spawnFriendlies)
{
    mSpawnEnemies = spawnEnemies;
    mSpawnFriendlies = spawnFriendlies;
}

int Level::getDirectSignal(const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = getBlockState(pos);
    return state->getDirectSignal(this, pos, direction);
}

int Level::getDirectSignalTo(const BlockPos& pos)
{
    int signal = 0;
    int value = getDirectSignal(pos.below(), Direction::DOWN);
    if (value > signal)
        signal = value;
    if (signal < 15)
    {
        value = getDirectSignal(pos.above(), Direction::UP);
        if (value > signal)
            signal = value;
    }
    if (signal < 15)
    {
        int value = getDirectSignal(pos.north(), Direction::NORTH);
        if (value > signal)
            signal = value;
    }
    if (signal < 15)
    {
        int value = getDirectSignal(pos.south(), Direction::SOUTH);
        if (value > signal)
            signal = value;
    }
    if (signal < 15)
    {
        int value = getDirectSignal(pos.west(), Direction::WEST);
        if (value > signal)
            signal = value;
    }
    if (signal < 15)
    {
        int value = getDirectSignal(pos.east(), Direction::EAST);
        if (value > signal)
            signal = value;
    }
    return signal;
}

int Level::getSignal(const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = getBlockState(pos);
    if (state->isSolidBlockingCubeAndNotSignalSource())
    {
        return getDirectSignalTo(pos);
    }
    return state->getSignal(this, pos, direction);
}

bool Level::hasSignal(const BlockPos& pos, const Direction* direction)
{
    return getSignal(pos, direction) > 0;
}

bool Level::hasNeighborSignal(const BlockPos& pos)
{
    if (getSignal(pos.below(), Direction::DOWN) > 0)
    {
        return true;
    }

    if (getSignal(pos.above(), Direction::UP) > 0)
    {
        return true;
    }

    if (getSignal(pos.north(), Direction::NORTH) > 0)
    {
        return true;
    }

    if (getSignal(pos.south(), Direction::SOUTH) > 0)
    {
        return true;
    }

    if (getSignal(pos.west(), Direction::WEST) > 0)
    {
        return true;
    }

    if (getSignal(pos.east(), Direction::EAST) > 0)
    {
        return true;
    }

    return false;
}

int Level::getBestNeighborSignal(const BlockPos& pos)
{
    int signal = 0;
    for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
    {
        const Direction* direction = *it;
        int value = getSignal(pos.relative(direction), direction);
        if (value >= 15)
        {
            return 15;
        }
        if (value > signal)
        {
            signal = value;
        }
    }
    return signal;
}

boost::shared_ptr<Player> Level::getNearestPlayer(boost::shared_ptr<Entity> entity, double distance,
                                                  double maxYDistance)
{
    return getNearestPlayer(entity, distance, false, maxYDistance);
}

boost::shared_ptr<Player> Level::getNearestPlayer(boost::shared_ptr<Entity> entity, double distance,
                                                  bool includeCreative, double maxYDistance)
{
    const Predicate<boost::shared_ptr<Entity> >* predicate = includeCreative ? EntitySelector::NO_CREATIVE_OR_SPECTATOR
                                                                             : EntitySelector::NO_SPECTATORS;
    return getNearestPlayerPredicate(entity, distance, predicate);
}

boost::shared_ptr<Player> Level::getNearestPlayer(double x, double y, double z, double distance, bool includeCreative,
                                                  double maxYDistance)
{
    const Predicate<boost::shared_ptr<Entity> >* predicate = includeCreative ? EntitySelector::NO_CREATIVE_OR_SPECTATOR
                                                                             : EntitySelector::NO_SPECTATORS;
    return getNearestPlayerPredicate(x, y, z, distance, predicate);
}

boost::shared_ptr<Player> Level::getNearestPlayer(double x, double z, double distance)
{
    boost::shared_ptr<Player> nearest;
    double nearestDistance = -1.0;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<Player> player = *it;
        double currentDistance = player->distanceToSqr(x, player->mY, z);
        if ((distance < 0.0 || currentDistance < distance * distance)
            && (nearestDistance == -1.0 || currentDistance < nearestDistance))
        {
            nearest = player;
            nearestDistance = currentDistance;
        }
    }
    return nearest;
}

boost::shared_ptr<Player> Level::getNearestPlayerNotCreative(boost::shared_ptr<Entity> entity, double distance,
                                                             double maxYDistance)
{
    return getNearestPlayer(entity, distance, true, maxYDistance);
}

boost::shared_ptr<Player> Level::getNearestPlayerPredicate(boost::shared_ptr<Entity> entity, double distance,
                                                           const Predicate<boost::shared_ptr<Entity> >* predicate)
{
    MemSect(0x15);
    boost::shared_ptr<Player> nearest;
    double nearestDistance = -1.0;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if ((predicate != NULL && !predicate->apply(*it)) || (*it)->mDimension != entity->mDimension)
        {
            continue;
        }
        double currentDistance = (*it)->distanceToSqr(entity->mX, entity->mY, entity->mZ);
        if ((distance < 0.0 || currentDistance < distance * distance)
            && (nearestDistance == -1.0 || currentDistance < nearestDistance) && (*it)->isAlive())
        {
            nearest = *it;
            nearestDistance = currentDistance;
        }
    }
    MemSect(0);
    return nearest;
}

boost::shared_ptr<Player> Level::getNearestPlayerPredicate(double x, double y, double z, double distance,
                                                           const Predicate<boost::shared_ptr<Entity> >* predicate)
{
    MemSect(0x15);
    boost::shared_ptr<Player> nearest;
    double nearestDistance = -1.0;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if (predicate != NULL && !predicate->apply(*it))
        {
            continue;
        }

        double currentDistance = (*it)->distanceToSqr(x, y, z);
        if ((distance < 0.0 || currentDistance < distance * distance)
            && (nearestDistance == -1.0 || currentDistance < nearestDistance) && (*it)->isAlive())
        {
            nearest = *it;
            nearestDistance = currentDistance;
        }
    }
    MemSect(0);
    return nearest;
}

void Level::setGameTime(long long value)
{
    mLevelData->setGameTime(value);
}

long long Level::getGameTime()
{
    return mLevelData->getGameTime();
}

long long Level::getDayTime()
{
    return mLevelData->getDayTime();
}

void Level::setDayTime(long long value)
{
    if (value != 0)
    {
        long long oldTime = mLevelData->getDayTime();
        unsigned long long delta = static_cast<unsigned long long>(value) - oldTime;
        if (oldTime > 0 && delta < 101)
        {
            for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
            {
                (*it)->awardStat(GenericStats::timePlayed(), GenericStats::param_time(static_cast<int>(delta)));
            }
        }
    }
    mLevelData->setDayTime(value);
}

void Level::forceDayTime(long long value)
{
    mLevelData->setDayTime(value);
    mForceTimeUpdate = true;
}

bool Level::ShouldForceTimeUpdate()
{
    bool force = mForceTimeUpdate;
    mForceTimeUpdate = false;
    return force;
}

BlockPos Level::getSharedSpawnPos()
{
    BlockPos spawn(mLevelData->getXSpawn(), mLevelData->getYSpawn(), mLevelData->getZSpawn());
    if (!getWorldBorder()->isWithinBounds(spawn))
    {
        spawn = getHeightmapPos(BlockPos(static_cast<int>(getWorldBorder()->getCenterX()), 0,
                                         static_cast<int>(getWorldBorder()->getCenterZ())));
    }
    return spawn;
}

ChunkSource* Level::getChunkSource()
{
    return mChunkSource;
}

LevelStorage* Level::getLevelStorage()
{
    return mLevelStorage.get();
}

unsigned char Level::getWeatherState()
{
    if (isThundering())
    {
        return 2;
    }
    return isRaining() ? 1 : 0;
}

bool Level::hasTickNextTick(const BlockPos&, Block*)
{
    return true;
}

void Level::addToTickNextTick(const BlockPos&, Block*, int)
{
    return;
}

void Level::addToTickNextTick(const BlockPos&, Block*, int, int)
{
    return;
}

void Level::forceAddBlockTick(const BlockPos&, Block*, int, int)
{
    return;
}

bool Level::newPrimedTntAllowed()
{
    return true;
}

BlockPos Level::getTopSolidBlockPos(const BlockPos& pos)
{
    LevelChunk* chunk = getChunkAt(pos);
    BlockPos current(pos.x, chunk->getHighestSectionPosition() + 16, pos.z);
    while (current.y >= 1)
    {
        BlockPos below = current.below();
        Material* material = chunk->getBlockState(below)->getMaterial();
        if (material->blocksMotion() && material != Material::LEAVES)
        {
            return current;
        }
        current = below;
    }
    return current;
}

bool Level::shouldFreeze(const BlockPos& pos)
{
    return shouldFreeze(pos, true);
}

bool Level::tickPendingTicks(bool runAll)
{
    return false;
}

std::vector<TickNextTickData>* Level::fetchTicksInChunk(LevelChunk* chunk, bool runAll)
{
    return NULL;
}

std::vector<TickNextTickData>* Level::fetchTicksInArea(BoundingBox* box, bool runAll)
{
    return NULL;
}

void Level::updateSleepingPlayerList()
{
}

LootTables* Level::getLootTables()
{
    return mLootTables;
}

bool Level::newFallingBlockAllowed()
{
    return true;
}

bool Level::explodeNextEnderCrystal()
{
    return true;
}

void Level::disconnect(bool save)
{
}

void Level::checkSession()
{
    mLevelStorage->checkSession();
}

void Level::broadcastEntityEvent(boost::shared_ptr<Entity> entity, unsigned char event, int data)
{
}

void Level::sendPacketToServer(boost::shared_ptr<Packet>)
{
    return;
}

boost::shared_ptr<Villages> Level::getVillages()
{
    return mVillages;
}

BlockPos* Level::findNearestMapFeature(const std::wstring& feature, const BlockPos& pos, bool findUnexplored)
{
    return NULL;
}

bool Level::isHumidAt(const BlockPos& pos)
{
    return getBiome(pos)->isHumid();
}

int Level::getAuxValueForMap(PlayerUID uid, int dimension, const BlockPos& pos, int scale)
{
#ifdef MINECRAFT_PC
    const int size = (1 << scale) << 7;
    const int centreX = (int)Math::round(pos.x / (double)size) * size;
    const int centreZ = (int)Math::round(pos.z / (double)size) * size;
    return getAuxValueForMap(uid, dimension, centreX, centreZ, scale);
#else
    return getAuxValueForMap(uid, dimension, 0, 0, scale);
#endif
}

void Level::blockEntityAddedExtra(boost::shared_ptr<BlockEntity>)
{
    return;
}

void Level::blockEntityRemovedExtra(boost::shared_ptr<BlockEntity>)
{
    return;
}

std::vector<boost::shared_ptr<Player> >* Level::getPlayers(unsigned long long flags,
                                                           const Predicate<boost::shared_ptr<Player> >* predicate)
{
    std::vector<boost::shared_ptr<Player> >* result = new std::vector<boost::shared_ptr<Player> >();
    std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        boost::shared_ptr<Player> player = *it;
        if (player != NULL)
        {
            unsigned long long type = player->GetType();
            unsigned long long required = flags;
            bool matches = (flags & 0x3FULL) == 0
                               ? ((type & (required >> 32)) == (required >> 32)
                                  && (type & (required & 0xFFFFFFFFULL)) == (required & 0xFFFFFFFFULL))
                               : type == required;
            if (matches && (predicate == NULL || predicate->apply(player)))
            {
                result->push_back(player);
            }
        }
        ++it;
    }
    return result;
}

std::vector<boost::shared_ptr<Entity> > Level::getAllEntities()
{
    EnterCriticalSection(&mEntityMutex);
    std::vector<boost::shared_ptr<Entity> > entities(mEntities);
    LeaveCriticalSection(&mEntityMutex);
    return entities;
}

boost::shared_ptr<Entity> Level::getEntity(int id)
{
    boost::unordered_map<int, boost::shared_ptr<Entity>, IntKeyHash2, IntKeyEq>::iterator it = mEntityMap.find(id);
    return it == mEntityMap.end() ? boost::shared_ptr<Entity>() : it->second;
}

bool Level::isFullAABBBlock(const BlockPos& pos)
{
    const AABB* shape = getBlockState(pos)->getClipAABB(this, pos);
    return shape != NULL && shape->getSize() >= 1.0;
}

bool Level::isSolidBlockingBlock(const BlockPos& pos)
{
    return getBlockState(pos)->isSolidBlockingBlock();
}

bool Level::isConsideredSolidBlock(const BlockPos& pos)
{
    return getBlockState(pos)->isSolid();
}

bool Level::isConsideredSolidBlock(int x, int y, int z)
{
    return getBlockState(BlockPos(x, y, z))->isSolid();
}

void Level::updateSkyBrightness()
{
    int skyDarken = getOldSkyDarken(1.0f);
    if (skyDarken != mSkyDarken)
        mSkyDarken = skyDarken;
}

int Level::getAboveTopSolidBlock(const BlockPos& pos, bool, bool)
{
    return getTopSolidBlockPos(pos).y;
}

bool Level::isUnderWater(const BlockPos& pos, const BlockState* state)
{
    Block* block = state->getBlock();
    bool cannotContainLiquid = !block->canContainLiquid(state);
    bool fullBlock = block->mFullBlock;
    bool notSlab = !block->isSlabBlock();
    bool topSlab = false;
    if (block->isSlabBlock())
    {
        topSlab = !HalfSlabBlock::isBottomSlab(block->convertBlockStateToLegacyData(state));
    }

    Material* above = Block::byId(LiquidBlock::getLiquidElement(this, pos.above()) >> 4)->mMaterial;
    Material* below = Block::byId(LiquidBlock::getLiquidElement(this, pos.below()) >> 4)->mMaterial;
    Material* north = Block::byId(LiquidBlock::getLiquidElement(this, pos.north()) >> 4)->mMaterial;
    Material* south = Block::byId(LiquidBlock::getLiquidElement(this, pos.south()) >> 4)->mMaterial;
    Material* east = Block::byId(LiquidBlock::getLiquidElement(this, pos.east()) >> 4)->mMaterial;
    Material* west = Block::byId(LiquidBlock::getLiquidElement(this, pos.west()) >> 4)->mMaterial;
    Material* center = Block::byId(LiquidBlock::getLiquidElement(this, pos) >> 4)->mMaterial;

    if (cannotContainLiquid && notSlab && above == Material::WATER && below != Material::AIR && north != Material::AIR
        && south != Material::AIR && east != Material::AIR && west != Material::AIR)
    {
        return true;
    }
    if (!fullBlock)
    {
        if (notSlab && center == Material::WATER)
        {
            return true;
        }
        if (cannotContainLiquid)
        {
            return above == Material::WATER;
        }
        if (notSlab)
        {
            return false;
        }
    }
    else
    {
        if (cannotContainLiquid)
        {
            return false;
        }
        if (notSlab)
        {
            return north != Material::AIR && south != Material::AIR && east != Material::AIR && west != Material::AIR
                   && below != Material::AIR && above != Material::AIR;
        }
    }
    return center == Material::WATER && (!topSlab || above != Material::AIR) && north != Material::AIR
           && south != Material::AIR && east != Material::AIR && west != Material::AIR && below != Material::AIR;
}

bool Level::isUnderWater(const BlockPos& pos, int blockId, int data)
{
    Block* block = Block::byId(blockId);
    return isUnderWater(pos, block->getBlockState(data));
}

void Level::tick(boost::shared_ptr<Entity> entity)
{
    tick(entity, true);
}

bool Level::addBlockEntity(boost::shared_ptr<BlockEntity> entity)
{
    EnterCriticalSection(&mBlockEntityMutex);
    mBlockEntities.push_back(entity);
    if (eTYPE_DERIVED_FROM(BlockEntityType::TICKABLE, entity->GetType()))
    {
        mTickingBlockEntities.push_back(entity);
    }
    if (mIsClientSide)
    {
        BlockPos pos = entity->getBlockPos();
        const BlockState* state = getBlockState(pos);
        sendBlockUpdated(pos, state, state, 2, true);
    }
    blockEntityAddedExtra(entity);
    LeaveCriticalSection(&mBlockEntityMutex);
    return true;
}

void Level::AddToBlockEntityList(boost::shared_ptr<BlockEntity> entity)
{
    if (std::find(mBlockEntities.begin(), mBlockEntities.end(), entity) == mBlockEntities.end())
    {
        addBlockEntity(entity);
    }
}

void Level::tickServerPlayers()
{
}

void Level::addAllPendingBlockEntities(std::vector<boost::shared_ptr<BlockEntity> >& entities)
{
    EnterCriticalSection(&mBlockEntityMutex);
    if (mProcessingBlockEntities)
    {
        mPendingBlockEntities.insert(mPendingBlockEntities.end(), entities.begin(), entities.end());
    }
    else
    {
        for (std::vector<boost::shared_ptr<BlockEntity> >::iterator it = entities.begin(); it != entities.end(); ++it)
        {
            AddToBlockEntityList(*it);
        }
    }
    LeaveCriticalSection(&mBlockEntityMutex);
}

void Level::tick(boost::shared_ptr<Entity> entity, bool updateChunk)
{
    int blockX = Mth::floor(entity->mX);
    int blockZ = Mth::floor(entity->mZ);
    int range = sEntityTickRange;
    if (updateChunk)
    {
        if (!eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
            && !hasChunksAt(blockX - range, 0, blockZ - range, blockX + range, 0, blockZ + range, true))
        {
            if (!mIsClientSide)
            {
                if (eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
                {
                    boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
                    mob->checkDespawn();
                    if (mob->mRemoved)
                    {
                        removeEntity(mob);
                    }
                }
            }
            return;
        }
    }

    entity->mXOld = entity->mX;
    entity->mYOld = entity->mY;
    entity->mZOld = entity->mZ;
    entity->mYRotO = entity->mYRot;
    entity->mXRotO = entity->mXRot;

    if (updateChunk)
    {
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()) || entity->mInChunk)
        {
            ++entity->mTickCount;
            if (entity->isPassenger())
            {
                entity->rideTick();
            }
            else
            {
                entity->tick();
            }
        }
    }

    if (Mth::isNaN(entity->mX))
    {
        entity->mX = entity->mXOld;
    }
    if (Mth::isNaN(entity->mY))
    {
        entity->mY = entity->mYOld;
    }
    if (Mth::isNaN(entity->mZ))
    {
        entity->mZ = entity->mZOld;
    }
    if (Mth::isNaN(entity->mXRot))
    {
        entity->mXRot = entity->mXRotO;
    }
    if (Mth::isNaN(entity->mYRot))
    {
        entity->mYRot = entity->mYRotO;
    }

    int chunkX = Mth::floor(entity->mX * 0.0625);
    int chunkY = Mth::floor(entity->mY * 0.0625);
    int chunkZ = Mth::floor(entity->mZ * 0.0625);
    if (!entity->mInChunk || entity->mChunkX != chunkX || entity->mChunkY != chunkY || entity->mChunkZ != chunkZ)
    {
        if (entity->mInChunk && hasChunk(entity->mChunkX, entity->mChunkZ, true))
        {
            getChunk(entity->mChunkX, entity->mChunkZ)->removeEntity(entity, entity->mChunkY);
        }
        if (entity->checkAndResetTeleportedFlag() || hasChunk(chunkX, chunkZ, true))
        {
            MemSect(0x27);
            getChunk(chunkX, chunkZ)->addEntity(entity);
            MemSect(0);
        }
        else
        {
            entity->mInChunk = false;
        }
    }

    if (updateChunk && entity->mInChunk)
    {
        const std::vector<boost::weak_ptr<Entity> >* passengers = entity->getPassengers();
        for (unsigned int i = 0; i < passengers->size(); ++i)
        {
            boost::shared_ptr<Entity> passenger = passengers->at(i).lock();
            if (passenger == NULL)
            {
                entity->removePassenger(boost::shared_ptr<Entity>());
            }
            else if (passenger->mRemoved || passenger->getVehicle() != entity)
            {
                passenger->stopRiding();
            }
            else
            {
                tick(passenger);
            }
        }
    }
}

void Level::tickEntities()
{
    PIXBeginNamedEvent(0.0f, "Ticking global entities");
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = mGlobalEntities.begin(); it != mGlobalEntities.end();)
    {
        boost::shared_ptr<Entity> entity = *it;
        ++entity->mTickCount;
        entity->tick();
        if (entity->mRemoved)
            it = mGlobalEntities.erase(it);
        else
            ++it;
    }
    PIXEndNamedEvent();

    EnterCriticalSection(&mEntityMutex);
    PIXBeginNamedEvent(0.0f, "Erasing removed entities");
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin(); it != mEntities.end();)
    {
        if (std::find(mEntitiesToRemove.begin(), mEntitiesToRemove.end(), *it) != mEntitiesToRemove.end())
            it = mEntities.erase(it);
        else
            ++it;
    }
    PIXEndNamedEvent();
    LeaveCriticalSection(&mEntityMutex);

    PIXBeginNamedEvent(0.0f, "Removing entities");
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = mEntitiesToRemove.begin();
         it != mEntitiesToRemove.end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        int chunkX = entity->mChunkX;
        int chunkZ = entity->mChunkZ;
        if (entity->mInChunk && hasChunk(chunkX, chunkZ, true))
            getChunk(chunkX, chunkZ)->removeEntity(entity);
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Entity removed handlers");
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = mEntitiesToRemove.begin();
         it != mEntitiesToRemove.end(); ++it)
        entityRemoved(*it);
    mEntitiesToRemove.clear();
    PIXEndNamedEvent();

    tickServerPlayers();

    EnterCriticalSection(&mEntityMutex);
    if (!mIsClientSide && mDimension->getType() == DimensionType::OVERWORLD)
        PIXAddNamedCounter(static_cast<float>(mEntities.size()), "Entity count");

    for (unsigned int i = 0; i < mEntities.size();)
    {
        boost::shared_ptr<Entity> entity = mEntities.at(i);
        boost::shared_ptr<Entity> vehicle = entity->getVehicle();
        if (vehicle != NULL)
        {
            if (!vehicle->mRemoved && vehicle->hasPassenger(entity))
            {
                ++i;
                continue;
            }
            entity->stopRiding();
        }

        if (!entity->shouldTick())
        {
            if (!mIsClientSide && eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
            {
                boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
                mob->checkDespawn();
                if (mob->mRemoved)
                    removeEntity(mob);
            }
        }
        else if (!entity->mRemoved && entity->GetType() != EntityType::SERVER_PLAYER)
        {
            PIXBeginNamedEvent(0.0f, "Ticking entity %s", entity->getName().c_str());
            tick(entity);
            PIXEndNamedEvent();
        }

        if (!entity->mRemoved)
        {
            ++i;
            continue;
        }

        int chunkX = entity->mChunkX;
        int chunkZ = entity->mChunkZ;
        if (entity->mInChunk && hasChunk(chunkX, chunkZ, true))
            getChunk(chunkX, chunkZ)->removeEntity(entity);
        std::vector<boost::shared_ptr<Entity> >::iterator found = std::find(mEntities.begin(), mEntities.end(), entity);
        if (found != mEntities.end())
            mEntities.erase(found);
        entityRemoved(entity);
    }
    LeaveCriticalSection(&mEntityMutex);

    EnterCriticalSection(&mBlockEntityMutex);
    PIXBeginNamedEvent(0.0f, "Ticking block entities");
    mProcessingBlockEntities = true;
    for (unsigned int i = 0; i < mTickingBlockEntities.size();)
    {
        boost::shared_ptr<BlockEntity> blockEntity = mTickingBlockEntities.at(i);
        if (!blockEntity->isRemoved() && blockEntity->hasLevel())
        {
            BlockPos pos = blockEntity->getBlockPos();
            if (hasChunkAt(pos) && mWorldBorder->isWithinBounds(pos))
            {
                LevelChunk* chunk = getChunk(pos.x >> 4, pos.z >> 4);
                if (!mIsClientSide || !chunk->isUnloaded())
                {
                    PIXBeginNamedEvent(0.0f, "Ticking block entity %I64d", blockEntity->GetType());
                    blockEntity->tick();
                    PIXEndNamedEvent();
                }
            }
        }

        if (!blockEntity->isRemoved())
        {
            ++i;
            continue;
        }

        mTickingBlockEntities.erase(mTickingBlockEntities.begin() + i);
        std::vector<boost::shared_ptr<BlockEntity> >::iterator found = std::find(mBlockEntities.begin(),
                                                                                 mBlockEntities.end(), blockEntity);
        if (found != mBlockEntities.end())
            mBlockEntities.erase(found);
        if (hasChunkAt(blockEntity->getBlockPos()))
            getChunkAt(blockEntity->getBlockPos())->removeBlockEntity(blockEntity->getBlockPos());
    }
    mProcessingBlockEntities = false;
    PIXEndNamedEvent();

    if (!mBlockEntitiesToRemove.empty())
    {
        PIXBeginNamedEvent(0.0f, "Unloading block entities");
        for (std::vector<boost::shared_ptr<BlockEntity> >::iterator it = mBlockEntitiesToRemove.begin();
             it != mBlockEntitiesToRemove.end(); ++it)
        {
            std::vector<boost::shared_ptr<BlockEntity> >::iterator found = std::find(mBlockEntities.begin(),
                                                                                     mBlockEntities.end(), *it);
            if (found != mBlockEntities.end())
                mBlockEntities.erase(found);
            found = std::find(mTickingBlockEntities.begin(), mTickingBlockEntities.end(), *it);
            if (found != mTickingBlockEntities.end())
                mTickingBlockEntities.erase(found);
        }
        mBlockEntitiesToRemove.clear();
        PIXEndNamedEvent();
    }

    if (!mPendingBlockEntities.empty())
    {
        PIXBeginNamedEvent(0.0f, "Sending block entity updates");
        for (std::vector<boost::shared_ptr<BlockEntity> >::iterator it = mPendingBlockEntities.begin();
             it != mPendingBlockEntities.end(); ++it)
        {
            boost::shared_ptr<BlockEntity> blockEntity = *it;
            if (blockEntity->isRemoved())
                continue;

            AddToBlockEntityList(blockEntity);
            if (hasChunkAt(blockEntity->getBlockPos()))
            {
                const BlockState* state =
                    getChunkAt(blockEntity->getBlockPos())->getBlockState(blockEntity->getBlockPos());
                getChunkAt(blockEntity->getBlockPos())->setBlockEntity(blockEntity->getBlockPos(), blockEntity);
                sendBlockUpdated(blockEntity->getBlockPos(), state, state, 3, true);
            }
        }
        mPendingBlockEntities.clear();
        PIXEndNamedEvent();
    }
    LeaveCriticalSection(&mBlockEntityMutex);
}

void Level::addEntities(std::vector<boost::shared_ptr<Entity> >* entities)
{
    EnterCriticalSection(&mEntityMutex);
    mEntities.insert(mEntities.end(), entities->begin(), entities->end());
    bool checkDuplicates = false;
    std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
    while (it != entities->end())
    {
        entityAdded(*it);
        if ((*it)->GetType() == EntityType::ENDER_DRAGON)
        {
            checkDuplicates = true;
        }
        ++it;
    }
    if (checkDuplicates)
    {
        bool found = false;
        for (it = mEntities.begin(); it != mEntities.end(); ++it)
        {
            if ((*it)->GetType() == EntityType::ENDER_DRAGON)
            {
                if (found)
                {
                    (*it)->remove();
                }
                else
                {
                    found = true;
                }
            }
        }
    }
    LeaveCriticalSection(&mEntityMutex);
}

void Level::removeEntities(std::vector<boost::shared_ptr<Entity> >* entities)
{
    mEntitiesToRemove.insert(mEntitiesToRemove.end(), entities->begin(), entities->end());
}

void Level::ensureAdded(boost::shared_ptr<Entity> entity)
{
    int chunkX = Mth::floor(entity->mX * 0.0625);
    int chunkZ = Mth::floor(entity->mZ * 0.0625);

    for (int x = chunkX - 2; x <= chunkX + 2; ++x)
    {
        for (int z = chunkZ - 2; z <= chunkZ + 2; ++z)
        {
            getChunk(x, z);
        }
    }

    EnterCriticalSection(&mEntityMutex);
    if (std::find(mEntities.begin(), mEntities.end(), entity) == mEntities.end())
    {
        mEntities.push_back(entity);
    }
    LeaveCriticalSection(&mEntityMutex);
}

bool Level::mayInteract(boost::shared_ptr<Player>, const BlockPos&, Block*)
{
    return true;
}

void Level::blockEvent(const BlockPos& pos, Block* block, int event, int data)
{
    getBlockState(pos)->triggerEvent(this, pos, event, data);
}

void Level::createFireworks(double, double, double, double, double, double, CompoundTag*)
{
    return;
}

std::wstring Level::gatherChunkSourceStats()
{
    return mChunkSource->gatherStats();
}

int Level::getSaveVersion()
{
    return getLevelStorage()->getSaveFile()->getSaveVersion();
}

int Level::getOriginalSaveVersion()
{
    return getLevelStorage()->getSaveFile()->getOriginalSaveVersion();
}

void Level::tickClientSideBlocks(int, int, LevelChunk*)
{
    return;
}

void Level::tickBlocks()
{
    buildAndPrepareChunksToPoll();
}

void Level::instaTick(const BlockPos& pos, const BlockState* state, Random& random)
{
    setInstaTick(true);
    state->getBlock()->tick(this, pos, state, &random);
    setInstaTick(false);
}

std::vector<boost::shared_ptr<Player> >* Level::getNearbyPlayers(boost::shared_ptr<Entity> entity, double distance)
{
    MemSect(0x15);
    std::vector<boost::shared_ptr<Player> >* result = new std::vector<boost::shared_ptr<Player> >();
    bool unlimited = distance < 0.0;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        double distanceSquared = (*it)->distanceToSqr(entity->mX, entity->mY, entity->mZ);
        if ((unlimited || distanceSquared < distance * distance) && (*it)->isAlive() && distanceSquared > 0.0)
        {
            result->push_back(*it);
        }
    }
    MemSect(0);
    return result;
}

bool Level::hasNearPlayer(double x, double y, double z, double distance)
{
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        double distanceSqr = mPlayers.at(i)->distanceToSqr(x, y, z);
        if (distance < 0.0 || distanceSqr < distance * distance)
            return true;
    }
    return false;
}

boost::shared_ptr<Player> Level::getNearestAttackablePlayer(double x, double y, double z, double distance,
                                                            double maxYDistance, const RangeModifier* modifier,
                                                            const Predicate<boost::shared_ptr<Entity> >* predicate)
{
    boost::shared_ptr<Player> nearest;
    std::vector<boost::shared_ptr<Player> >::iterator end = mPlayers.end();
    double nearestDistance = -1.0;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != end; ++it)
    {
        boost::shared_ptr<Player> player = *it;
        if (player->mAbilities.mInvulnerable || !player->isAlive() || player->hasInvisiblePrivilege()
            || player->isSpectator()
            || (predicate != NULL && !predicate->apply(boost::static_pointer_cast<Entity>(player))))
        {
            continue;
        }
        double distanceSquared = player->distanceToSqr(x, player->mY, z);
        double adjustedRange = distance;
        if (player->isSneaking())
        {
            adjustedRange *= 0.800000011920929;
        }
        if (player->isInvisible())
        {
            float cover = player->getArmorCoverPercentage();
            if (cover < 0.1f)
            {
                cover = 0.1f;
            }
            adjustedRange *= 0.7f * cover;
        }
        if (modifier != NULL)
        {
            adjustedRange *= modifier->apply(player);
        }
        if ((maxYDistance < 0.0 || std::fabs(player->mY - y) < maxYDistance * maxYDistance)
            && (distance < 0.0 || distanceSquared < adjustedRange * adjustedRange)
            && (nearestDistance == -1.0 || distanceSquared < nearestDistance))
        {
            nearest = player;
            nearestDistance = distanceSquared;
        }
    }
    return nearest;
}

boost::shared_ptr<Player> Level::getNearestAttackablePlayer(const BlockPos& pos, double distance, double maxYDistance)
{
    return getNearestAttackablePlayer(pos.x, pos.y, pos.z, distance, maxYDistance, NULL, NULL);
}

boost::shared_ptr<Player> Level::getPlayerByName(const std::wstring& name)
{
    std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        if (name == (*it)->getName())
        {
            return *it;
        }
        ++it;
    }
    return boost::shared_ptr<Player>();
}

boost::shared_ptr<Player> Level::getPlayerByUUID(const std::wstring& uuid)
{
    std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        if (uuid == (*it)->getUUID())
        {
            return *it;
        }
        ++it;
    }
    return boost::shared_ptr<Player>();
}

void Level::blockEntityChanged(const BlockPos& pos, boost::shared_ptr<BlockEntity> entity)
{
    if (hasChunkAt(pos))
    {
        getChunkAt(pos)->markUnsaved();
    }
}

boost::shared_ptr<BlockEntity> Level::getPendingBlockEntityAt(const BlockPos& pos)
{
    EnterCriticalSection(&mBlockEntityMutex);
    for (unsigned int i = 0; i < mPendingBlockEntities.size(); ++i)
    {
        boost::shared_ptr<BlockEntity> entity = mPendingBlockEntities.at(i);
        if (!entity->isRemoved() && entity->getBlockPos().equals(pos))
        {
            LeaveCriticalSection(&mBlockEntityMutex);
            return entity;
        }
    }
    LeaveCriticalSection(&mBlockEntityMutex);
    return boost::shared_ptr<BlockEntity>();
}

boost::shared_ptr<BlockEntity> Level::getBlockEntity(const BlockPos& pos)
{
    if (isOutsideBuildHeight(pos))
    {
        return boost::shared_ptr<BlockEntity>();
    }
    if (mProcessingBlockEntities)
    {
        boost::shared_ptr<BlockEntity> pending = getPendingBlockEntityAt(pos);
        if (pending != NULL)
        {
            return pending;
        }
    }
    LevelChunk* chunk = getChunkAt(pos);
    boost::shared_ptr<BlockEntity> result = chunk->getBlockEntity(pos, LevelChunk::IMMEDIATE);
    if (result == NULL)
    {
        result = getPendingBlockEntityAt(pos);
    }
    return result;
}

void Level::setBlockEntity(const BlockPos& pos, boost::shared_ptr<BlockEntity> entity)
{
    if (isOutsideBuildHeight(pos) || entity == NULL || entity->isRemoved())
    {
        return;
    }
    EnterCriticalSection(&mBlockEntityMutex);
    if (mProcessingBlockEntities)
    {
        entity->setPosition(pos);
        for (std::vector<boost::shared_ptr<BlockEntity> >::iterator it = mPendingBlockEntities.begin();
             it != mPendingBlockEntities.end();)
        {
            boost::shared_ptr<BlockEntity> pending = *it;
            if (pending->getBlockPos().equals(pos))
            {
                pending->setRemoved();
                it = mPendingBlockEntities.erase(it);
            }
            else
            {
                ++it;
            }
        }
        mPendingBlockEntities.push_back(entity);
    }
    else
    {
        LevelChunk* chunk = getChunkAt(pos);
        if (chunk != NULL)
        {
            chunk->setBlockEntity(pos, entity);
        }
        AddToBlockEntityList(entity);
    }
    LeaveCriticalSection(&mBlockEntityMutex);
}

void Level::removeBlockEntity(const BlockPos& pos)
{
    EnterCriticalSection(&mBlockEntityMutex);
    boost::shared_ptr<BlockEntity> entity = getBlockEntity(pos);
    if (entity != NULL && mProcessingBlockEntities)
    {
        entity->setRemoved();
        std::vector<boost::shared_ptr<BlockEntity> >::iterator it = std::find(mPendingBlockEntities.begin(),
                                                                              mPendingBlockEntities.end(), entity);
        if (it != mPendingBlockEntities.end())
        {
            mPendingBlockEntities.erase(it);
        }
        LeaveCriticalSection(&mBlockEntityMutex);
    }
    else
    {
        if (entity != NULL)
        {
            std::vector<boost::shared_ptr<BlockEntity> >::iterator it = std::find(mPendingBlockEntities.begin(),
                                                                                  mPendingBlockEntities.end(), entity);
            if (it != mPendingBlockEntities.end())
            {
                mPendingBlockEntities.erase(it);
            }
            std::vector<boost::shared_ptr<BlockEntity> >::iterator it2 = std::find(mBlockEntities.begin(),
                                                                                   mBlockEntities.end(), entity);
            if (it2 != mBlockEntities.end())
            {
                mBlockEntities.erase(it2);
            }
            if (eTYPE_DERIVED_FROM(BlockEntityType::TICKABLE, entity->GetType()))
            {
                std::vector<boost::shared_ptr<BlockEntity> >::iterator it3 = std::find(
                    mTickingBlockEntities.begin(), mTickingBlockEntities.end(), entity);
                if (it3 != mTickingBlockEntities.end())
                {
                    mTickingBlockEntities.erase(it3);
                }
            }
            blockEntityRemovedExtra(entity);
        }
        LevelChunk* chunk = getChunkAt(pos);
        if (chunk != NULL)
        {
            chunk->removeBlockEntity(pos);
        }
        LeaveCriticalSection(&mBlockEntityMutex);
    }
}

void Level::markForRemoval(boost::shared_ptr<BlockEntity> entity)
{
    EnterCriticalSection(&mBlockEntityMutex);
    mBlockEntitiesToRemove.push_back(entity);
    LeaveCriticalSection(&mBlockEntityMutex);
}

bool Level::isSolidBlockingBlockInLoadedChunk(const BlockPos& pos, bool allowEmpty)
{
    if (isOutsideBuildHeight(pos))
    {
        return false;
    }
    LevelChunk* chunk = mChunkSource->getChunkIfLoaded(pos.x >> 4, pos.z >> 4);
    if (chunk == NULL || chunk->isEmpty())
    {
        return allowEmpty;
    }
    return getBlockState(pos)->isSolidBlockingBlock();
}

void Level::prepareWeather()
{
    if (mLevelData->isRaining())
    {
        mRainLevel = 1.0f;
        if (mLevelData->isThundering())
        {
            mThunderLevel = 1.0f;
        }
    }
}

void Level::tick()
{
    PIXBeginNamedEvent(0.0f, "Weather tick");
    tickWeather(false);
    PIXEndNamedEvent();
}

void Level::tickWeather(bool immediate)
{
    if (!mDimension->isHasSkyLight() || mIsClientSide
        || (!getGameRules()->getBoolean(GameRules::RULE_WEATHER_CYCLE) && !immediate)
        || gApp.GetGameHostOption(eGameHostOption_MiniGame))
    {
        return;
    }
    int clearTime = mLevelData->getClearWeatherTime();
    if (clearTime > 0)
    {
        mLevelData->setClearWeatherTime(clearTime - 1);
        mLevelData->setThunderTime(mLevelData->isThundering() ? 1 : 2);
        mLevelData->setRainTime(mLevelData->isRaining() ? 1 : 2);
    }
    int thunderTime = mLevelData->getThunderTime();
    if (thunderTime <= 0)
    {
        mLevelData->setThunderTime(mLevelData->isThundering() ? mRandom->nextInt(12000) + 3600
                                                              : mRandom->nextInt(168000) + 12000);
    }
    else
    {
        mLevelData->setThunderTime(--thunderTime);
        if (thunderTime <= 0)
        {
            mLevelData->setThundering(!mLevelData->isThundering());
        }
    }
    mOldThunderLevel = mThunderLevel;
    if (immediate)
    {
        mOldThunderLevel = mThunderLevel = mLevelData->isThundering() ? 1.0f : 0.0f;
    }
    mThunderLevel += mLevelData->isThundering() ? 0.01f : -0.01f;
    mThunderLevel = Mth::clamp(mThunderLevel, 0.0f, 1.0f);
    int rainTime = mLevelData->getRainTime();
    if (rainTime <= 0)
    {
        mLevelData->setRainTime(mLevelData->isRaining() ? mRandom->nextInt(12000) + 12000
                                                        : mRandom->nextInt(168000) + 12000);
    }
    else
    {
        mLevelData->setRainTime(--rainTime);
        if (rainTime <= 0)
        {
            mLevelData->setRaining(!mLevelData->isRaining());
        }
    }
    mOldRainLevel = mRainLevel;
    if (immediate)
    {
        mOldRainLevel = mRainLevel = mLevelData->isRaining() ? 1.0f : 0.0f;
    }
    mRainLevel += mLevelData->isRaining() ? 0.01f : -0.01f;
    mRainLevel = Mth::clamp(mRainLevel, 0.0f, 1.0f);
}

void Level::buildAndPrepareChunksToPoll()
{
    int count = std::min(static_cast<int>(mPlayers.size()), 8);
    int chunkX[8];
    int chunkZ[8];
    for (int i = 0; i < count; ++i)
    {
        boost::shared_ptr<Player> player = mPlayers[i];
        chunkX[i] = Mth::floor(player->mX * 0.0625);
        chunkZ[i] = Mth::floor(player->mZ * 0.0625);
        mChunksToPoll.insert(ChunkPos(chunkX[i], chunkZ[i]));
    }
    int maxRadius = 9;
#ifdef MINECRAFT_PC
    if (!mIsClientSide && !Minecraft::InMiniGame(eMiniGameId_Any, true))
        maxRadius = PcSettingsSimulationDistance();
#endif
    for (int radius = 1; radius <= maxRadius; ++radius)
    {
        for (int offset = 0; offset < radius * 2; ++offset)
        {
            for (int i = 0; i < count; ++i)
            {
                mChunksToPoll.insert(ChunkPos(chunkX[i] - radius + offset, chunkZ[i] - radius));
                mChunksToPoll.insert(ChunkPos(chunkX[i] + radius, chunkZ[i] - radius + offset));
                mChunksToPoll.insert(ChunkPos(chunkX[i] + radius - offset, chunkZ[i] + radius));
                mChunksToPoll.insert(ChunkPos(chunkX[i] - radius, chunkZ[i] + radius - offset));
            }
        }
    }
}

std::vector<boost::shared_ptr<Entity> >* Level::getEntities(boost::shared_ptr<Entity> excluded, const AABB* box,
                                                            const Predicate<boost::shared_ptr<Entity> >* predicate,
                                                            std::vector<boost::shared_ptr<Entity> >* entities)
{
    MemSect(0x28);
    std::vector<boost::shared_ptr<Entity> >* result = entities != NULL ? entities : &mEntityQueryResult;
    result->clear();
    int minChunkX = Mth::floor((box->mMinX - 2.0) * 0.0625);
    double upperX = (box->mMaxX + 2.0) * 0.0625;
    int maxChunkX = static_cast<int>(upperX);
    if (static_cast<float>(maxChunkX) < static_cast<float>(upperX))
    {
        ++maxChunkX;
    }
    int minChunkZ = Mth::floor((box->mMinZ - 2.0) * 0.0625);
    double upperZ = (box->mMaxZ + 2.0) * 0.0625;
    int maxChunkZ = static_cast<int>(upperZ);
    if (static_cast<float>(maxChunkZ) < static_cast<float>(upperZ))
    {
        ++maxChunkZ;
    }
    EnterCriticalSection(&LevelChunk::m_csEntities);
    for (int chunkX = minChunkX; chunkX <= maxChunkX; ++chunkX)
    {
        for (int chunkZ = minChunkZ; chunkZ <= maxChunkZ; ++chunkZ)
        {
            LevelChunk* chunk = getChunkIfHasChunk(chunkX, chunkZ, true);
            if (chunk != NULL)
            {
                chunk->getEntities(excluded, box, *result, predicate);
            }
        }
    }
    MemSect(0);
    LeaveCriticalSection(&LevelChunk::m_csEntities);
    return result;
}

std::vector<boost::shared_ptr<Entity> >* Level::getEntities(boost::shared_ptr<Entity> excluded, const AABB* box,
                                                            std::vector<boost::shared_ptr<Entity> >* entities)
{
    return getEntities(excluded, box, NULL, entities);
}

std::vector<boost::shared_ptr<Entity> >* Level::getEntitiesOfClass(
    const std::type_info& type, const AABB* box, const Predicate<boost::shared_ptr<Entity> >* predicate,
    bool excludeInvisiblePlayers)
{
    int minChunkX = Mth::floor((box->mMinX - 2.0) * 0.0625);
    double upperX = (box->mMaxX + 2.0) * 0.0625;
    int maxChunkX = static_cast<int>(upperX);
    if (static_cast<float>(maxChunkX) < static_cast<float>(upperX))
    {
        ++maxChunkX;
    }
    int minChunkZ = Mth::floor((box->mMinZ - 2.0) * 0.0625);
    double upperZ = (box->mMaxZ + 2.0) * 0.0625;
    int maxChunkZ = static_cast<int>(upperZ);
    if (static_cast<float>(maxChunkZ) < static_cast<float>(upperZ))
    {
        ++maxChunkZ;
    }
    std::vector<boost::shared_ptr<Entity> >* result = new std::vector<boost::shared_ptr<Entity> >();
    EnterCriticalSection(&LevelChunk::m_csEntities);
    for (int chunkX = minChunkX; chunkX <= maxChunkX; ++chunkX)
    {
        for (int chunkZ = minChunkZ; chunkZ <= maxChunkZ; ++chunkZ)
        {
            LevelChunk* chunk = getChunkIfHasChunk(chunkX, chunkZ, true);
            if (chunk != NULL)
            {
                chunk->getEntitiesOfClass(type, box, *result, predicate, excludeInvisiblePlayers);
            }
        }
    }
    LeaveCriticalSection(&LevelChunk::m_csEntities);
    return result;
}

std::vector<boost::shared_ptr<Entity> >* Level::getEntitiesOfClass(const std::type_info& type, const AABB* box)
{
    return getEntitiesOfClass(type, box, EntitySelector::NO_SPECTATORS, false);
}

bool Level::hasEntitiesOfClass(const std::type_info& type, const AABB* box,
                               const Predicate<boost::shared_ptr<Entity> >* predicate, bool excludeInvisiblePlayers)
{
    int minChunkX = Mth::floor((box->mMinX - 2.0) * 0.0625);
    double upperX = (box->mMaxX + 2.0) * 0.0625;
    int maxChunkX = static_cast<int>(upperX);
    if (static_cast<float>(maxChunkX) < static_cast<float>(upperX))
    {
        ++maxChunkX;
    }
    int minChunkZ = Mth::floor((box->mMinZ - 2.0) * 0.0625);
    double upperZ = (box->mMaxZ + 2.0) * 0.0625;
    int maxChunkZ = static_cast<int>(upperZ);
    if (static_cast<float>(maxChunkZ) < static_cast<float>(upperZ))
    {
        ++maxChunkZ;
    }
    std::vector<boost::shared_ptr<Entity> >* found = new std::vector<boost::shared_ptr<Entity> >();
    EnterCriticalSection(&LevelChunk::m_csEntities);
    for (int chunkX = minChunkX; chunkX <= maxChunkX; ++chunkX)
    {
        for (int chunkZ = minChunkZ; chunkZ <= maxChunkZ; ++chunkZ)
        {
            if (!hasChunk(chunkX, chunkZ, true))
            {
                continue;
            }
            getChunk(chunkX, chunkZ)->getEntitiesOfClass(type, box, *found, predicate, excludeInvisiblePlayers);
            if (!found->empty())
            {
                delete found;
                LeaveCriticalSection(&LevelChunk::m_csEntities);
                return true;
            }
        }
    }
    delete found;
    LeaveCriticalSection(&LevelChunk::m_csEntities);
    return false;
}

boost::shared_ptr<Entity> Level::getClosestEntityOfClass(const std::type_info& type, const AABB* box,
                                                         boost::shared_ptr<Entity> excluded)
{
    boost::shared_ptr<Entity> closest;
    double closestDistance = DBL_MAX;
    std::vector<boost::shared_ptr<Entity> >* entities = getEntitiesOfClass(type, box);
    std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
    while (it != entities->end())
    {
        if (*it != excluded)
        {
            double distance = excluded->distanceToSqr(*it);
            if (distance <= closestDistance)
            {
                closest = *it;
                closestDistance = distance;
            }
        }
        ++it;
    }
    delete entities;
    return closest;
}

bool Level::isUnobstructed(const AABB* box, boost::shared_ptr<Entity> excluded)
{
    std::vector<boost::shared_ptr<Entity> >* entities = getEntities(boost::shared_ptr<Entity>(), box, NULL);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        if (!(*it)->mRemoved && (*it)->mBlocksBuilding && *it != excluded
            && (excluded == NULL || (*it)->isPassengerOfSameVehicle(excluded)))
        {
            return false;
        }
    }
    return true;
}

bool Level::isUnobstructed(const AABB* box)
{
    return isUnobstructed(box, boost::shared_ptr<Entity>());
}

bool Level::internalGetCollisionAABBs(boost::shared_ptr<Entity> entity, const AABB* box, bool stopOnFirst,
                                      std::vector<AABB*>* collisions, bool, bool includePlayerConstraints,
                                      bool forParticles)
{
    PIXBeginNamedEvent(0.0f, "Level::getCubes");
    PIXBeginNamedEvent(0.0f, "Setup");
    const int expansion = forParticles ? 0 : 1;
    const int minX = Mth::floor(box->mMinX) - expansion;
    const int maxX = Mth::ceil(static_cast<float>(box->mMaxX)) + expansion;
    const int minY = Mth::floor(box->mMinY) - expansion;
    const int maxY = Mth::ceil(static_cast<float>(box->mMaxY)) + expansion;
    const int minZ = Mth::floor(box->mMinZ) - expansion;
    const int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ)) + expansion;

    const bool inMiniGame = Minecraft::InMiniGame(eMiniGameId_Any, true);
    AABB* constraints = entity != NULL ? mDimension->getPlayerConstraints(BlockPos(entity)) : NULL;
    const bool outsideWorldBorder = entity != NULL && entity->mOutsideWorldBorder;
    const bool insideWorldBorder = entity != NULL && isInsideWorldBorder(entity);
    PIXEndNamedEvent();

    if (includePlayerConstraints && constraints != NULL)
    {
        collisions->push_back(getOuterWall(constraints, Direction::eDirection_North));
        collisions->push_back(getOuterWall(constraints, Direction::eDirection_South));
        collisions->push_back(getOuterWall(constraints, Direction::eDirection_East));
        collisions->push_back(getOuterWall(constraints, Direction::eDirection_West));
        collisions->push_back(getOuterWall(constraints, Direction::eDirection_Up));
        collisions->push_back(getOuterWall(constraints, Direction::eDirection_Down));
    }

    const BlockState* stone = Blocks::stone->defaultBlockState();
    LevelChunk* chunk = NULL;
    int chunkX = -99999;
    int chunkZ = -99999;
    MutableBlockPos pos(0, 0, 0);
    std::vector<int> emptySections;
    for (int x = minX; x < maxX; ++x)
    {
        for (int z = minZ; z < maxZ; ++z)
        {
            const bool edgeX = x == minX || x == maxX - 1;
            const bool edgeZ = z == minZ || z == maxZ - 1;
            if (edgeX && edgeZ)
            {
                continue;
            }
            const int newChunkX = x >> 4;
            const int newChunkZ = z >> 4;
            if (newChunkX != chunkX || newChunkZ != chunkZ)
            {
                chunk = getChunk(newChunkX, newChunkZ);
                chunkX = newChunkX;
                chunkZ = newChunkZ;
            }
            pos.set(x, 64, z);
            if (chunk == NULL || chunk->isEmpty())
            {
                continue;
            }
            for (int y = minY - 1; y < maxY; ++y)
            {
                if (((edgeX || edgeZ) && y == maxY - 1) || y < 0 || y >= 256)
                {
                    continue;
                }
                if (entity != NULL && outsideWorldBorder == insideWorldBorder)
                {
                    entity->setOutsideWorldBorder(!insideWorldBorder);
                }
                pos.set(x, y, z);

                const BlockState* state;
                if (stopOnFirst || (constraints != NULL && constraints->contains(pos)) || !insideWorldBorder)
                {
                    if (entity != NULL)
                    {
                        state = getBlockState(chunk, entity->mTickCache, pos);
                    }
                    else
                    {
                        const int section = HierarchicalElementStorage::GetSectionIDFromXYZ(x, y, z);
                        bool knownEmpty = false;
                        for (unsigned int i = 0; i < emptySections.size(); ++i)
                        {
                            if (emptySections[i] == section)
                            {
                                knownEmpty = true;
                                break;
                            }
                        }
                        if (knownEmpty)
                        {
                            continue;
                        }
                        bool sectionEmpty;
                        const unsigned short element = chunk->getElement(x & 15, y, z & 15, &sectionEmpty);
                        if (element == 0)
                        {
                            if (sectionEmpty)
                            {
                                emptySections.push_back(section);
                            }
                            continue;
                        }
                        const unsigned short blockId = element >> 4;
                        if (blockId == BlockId::BUBBLE_COLUMN_SEGMENT || blockId == BlockId::WATER
                            || blockId == BlockId::FLOWING_WATER)
                        {
                            continue;
                        }
                        state = Block::getStateByElementId(element);
                    }
                }
                else
                {
                    state = stone;
                }
                if (state == NULL)
                {
                    continue;
                }

                if (!inMiniGame || gApp.ShouldCollideWith(state->getBlock()))
                {
                    if (!forParticles || !state->getBlock()->ParticlesSurviveWithin())
                    {
                        state->addCollissionAABBs(this, pos, box, collisions, entity, false);
                    }
                }
                if (stopOnFirst && collisions->size() != 0)
                {
                    PIXEndNamedEvent();
                    return true;
                }
            }
        }
    }
    PIXEndNamedEvent();
    return false;
}

std::vector<AABB*>* Level::getCollisionAABBs(boost::shared_ptr<Entity> entity, const AABB* box, bool noEntities,
                                             bool includePlayerConstraints, bool forParticles)
{
    mCollisionBoxes.clear();
    mCollisionBoxes.reserve(8);
    internalGetCollisionAABBs(entity, box, false, &mCollisionBoxes, noEntities, includePlayerConstraints, forParticles);
    PIXBeginNamedEvent(0.0f, "Update level renderer");
    Minecraft::GetInstance()->mLevelRenderer->mDestroyedBlocks->addAABBs(this, box, &mCollisionBoxes);
    PIXEndNamedEvent();
    if (!noEntities)
    {
        PIXBeginNamedEvent(0.0f, "Check entity boxes");
        if (entity != NULL)
        {
            std::vector<boost::shared_ptr<Entity> >* entities = getEntities(entity, box->grow(0.25), NULL);
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
            {
                if (entity->isPassengerOfSameVehicle(*it))
                {
                    continue;
                }
                AABB* collideBox = (*it)->getCollideBox();
                if (collideBox != NULL
                    && collideBox->intersects(box->mMinX, box->mMinY, box->mMinZ, box->mMaxX, box->mMaxY, box->mMaxZ))
                {
                    mCollisionBoxes.push_back(collideBox);
                }
                collideBox = entity->getCollideAgainstBox(*it);
                if (collideBox != NULL
                    && collideBox->intersects(box->mMinX, box->mMinY, box->mMinZ, box->mMaxX, box->mMaxY, box->mMaxZ))
                {
                    mCollisionBoxes.push_back(collideBox);
                }
            }
        }
        PIXEndNamedEvent();
    }
    return &mCollisionBoxes;
}

bool Level::hasBlockCubes(const AABB* box)
{
    std::vector<AABB*> collisions;
    return internalGetCollisionAABBs(boost::shared_ptr<Entity>(), box, true, &collisions, false, false, false);
}

bool Level::containsAnyBlocks(AABB* box)
{
    int minX = Mth::floor(box->mMinX);
    int maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    int minY = Mth::floor(box->mMinY);
    int maxY = Mth::ceil(static_cast<float>(box->mMaxY));
    int minZ = Mth::floor(box->mMinZ);
    int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));
    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                BlockPos pos(x, y, z);
                if (getBlockState(pos)->getMaterial() != Material::AIR)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Level::containsAnyLiquid(AABB* box)
{
    int minX = Mth::floor(box->mMinX);
    int maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    int minY = Mth::floor(box->mMinY);
    int maxY = Mth::ceil(static_cast<float>(box->mMaxY));
    int minZ = Mth::floor(box->mMinZ);
    int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));
    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                BlockPos pos(x, y, z);
                if (getLiquidBlockState(pos)->getMaterial()->isLiquid())
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Level::containsAnysolidBlocking(AABB* box)
{
    int minX = Mth::floor(box->mMinX);
    int maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    int minY = Mth::floor(box->mMinY);
    int maxY = Mth::ceil(static_cast<float>(box->mMaxY));
    int minZ = Mth::floor(box->mMinZ);
    int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));
    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                BlockPos pos(x, y, z);
                if (getBlockState(pos)->getMaterial()->isSolidBlocking())
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Level::containsAnyLiquid_NoLoad(AABB* box)
{
    int minX = Mth::floor(box->mMinX);
    int maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    int minY = Mth::floor(box->mMinY);
    int maxY = Mth::ceil(static_cast<float>(box->mMaxY));
    int minZ = Mth::floor(box->mMinZ);
    int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));
    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                if (!hasChunkAt(BlockPos(x, y, z)) || getLiquidBlockState(BlockPos(x, y, z))->getMaterial()->isLiquid())
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Level::containsFireBlock(AABB* box)
{
    int minX = Mth::floor(box->mMinX);
    int maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    int minY = Mth::floor(box->mMinY);
    int maxY = Mth::ceil(static_cast<float>(box->mMaxY));
    int minZ = Mth::floor(box->mMinZ);
    int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));
    if (!hasChunksAt(minX, minY, minZ, maxX, maxY, maxZ, true))
    {
        return false;
    }
    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                Block* block = getBlock(BlockPos(x, y, z));
                if (block == Blocks::fire || block == Blocks::flowing_lava || block == Blocks::lava)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Level::containsFireBlock(Entity* entity, AABB* box)
{
    int minX = Mth::floor(box->mMinX), maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    int minY = Mth::floor(box->mMinY), maxY = Mth::ceil(static_cast<float>(box->mMaxY));
    int minZ = Mth::floor(box->mMinZ), maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));

    if (!entity->mTickCache.mCacheChanged && entity->mFireCacheMinX == minX && entity->mFireCacheMinY == minY
        && entity->mFireCacheMinZ == minZ && entity->mFireCacheMaxX == maxX && entity->mFireCacheMaxY == maxY
        && entity->mFireCacheMaxZ == maxZ)
    {
        return entity->mFireCacheResult;
    }

    entity->mFireCacheMinX = minX;
    entity->mFireCacheMinY = minY;
    entity->mFireCacheMinZ = minZ;
    entity->mFireCacheMaxX = maxX;
    entity->mFireCacheMaxY = maxY;
    entity->mFireCacheMaxZ = maxZ;
    if (!hasChunksAt(minX, minY, minZ, maxX, maxY, maxZ, true))
    {
        entity->mFireCacheResult = false;
        return false;
    }

    for (int x = minX; x < maxX; ++x)
        for (int y = minY; y < maxY; ++y)
            for (int z = minZ; z < maxZ; ++z)
            {
                int element = entity->mTickCache.getElement(x, y, z);
                if (element == 0xFFFF)
                {
                    element = getElement(x, y, z);
                }
                Block* block = Block::byId(element >> 4);
                if (block == Blocks::fire || block == Blocks::flowing_lava || block == Blocks::lava)
                {
                    entity->mFireCacheResult = true;
                    return true;
                }
            }
    entity->mFireCacheResult = false;
    return false;
}

bool Level::containsMaterial(AABB* box, Material* material)
{
    int minX = Mth::floor(box->mMinX);
    int maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    int minY = Mth::floor(box->mMinY);
    int maxY = Mth::ceil(static_cast<float>(box->mMaxY));
    int minZ = Mth::floor(box->mMinZ);
    int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));
    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                if (getBlock(BlockPos(x, y, z))->mMaterial == material)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Level::isInLava(Entity* entity, AABB* box)
{
    int minX = Mth::floor(box->mMinX), maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    int minY = Mth::floor(box->mMinY), maxY = Mth::ceil(static_cast<float>(box->mMaxY));
    int minZ = Mth::floor(box->mMinZ), maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));

    if (!entity->mTickCache.mCacheChanged && entity->mLavaCacheMinX == minX && entity->mLavaCacheMinY == minY
        && entity->mLavaCacheMinZ == minZ && entity->mLavaCacheMaxX == maxX && entity->mLavaCacheMaxY == maxY
        && entity->mLavaCacheMaxZ == maxZ)
    {
        return entity->mLavaCacheResult;
    }

    entity->mLavaCacheMinX = minX;
    entity->mLavaCacheMinY = minY;
    entity->mLavaCacheMinZ = minZ;
    entity->mLavaCacheMaxX = maxX;
    entity->mLavaCacheMaxY = maxY;
    entity->mLavaCacheMaxZ = maxZ;
    for (int x = minX; x < maxX; ++x)
        for (int y = minY; y < maxY; ++y)
            for (int z = minZ; z < maxZ; ++z)
            {
                Block* block = Block::byId(getElement(entity->mTickCache, x, y, z) >> 4);
                if (block != NULL && block->mMaterial == Material::LAVA)
                {
                    entity->mLavaCacheResult = true;
                    return true;
                }
            }
    entity->mLavaCacheResult = false;
    return false;
}

bool Level::containsLiquid(AABB* box, Material* material)
{
    int minX = Mth::floor(box->mMinX);
    int maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    int minY = Mth::floor(box->mMinY);
    int maxY = Mth::ceil(static_cast<float>(box->mMaxY));
    int minZ = Mth::floor(box->mMinZ);
    int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));
    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                const BlockState* state = getBlockState(BlockPos(x, y, z));
                if (state->getMaterial() == material)
                {
                    int level = state->getValue(LiquidBlock::LEVEL);
                    double surface = y + 1;
                    if (level < 8)
                    {
                        surface -= level * 0.125;
                    }
                    if (box->mMinY <= surface)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Level::canProvideSupport(const BlockPos& pos, const Direction* direction, EBlockSupportType supportType)
{
    const BlockState* state = getBlockState(pos);
    return state->canProvideSupport(direction, supportType);
}

bool Level::isTouchingMaterial(const BlockPos& pos, const Material* material)
{
    return getLiquidBlock(pos)->mMaterial == material || getLiquidBlock(pos.above())->mMaterial == material
           || getLiquidBlock(pos.north())->mMaterial == material || getLiquidBlock(pos.east())->mMaterial == material
           || getLiquidBlock(pos.south())->mMaterial == material || getLiquidBlock(pos.west())->mMaterial == material
           || getLiquidBlock(pos.below())->mMaterial == material;
}

int Level::countInstanceOf(unsigned long long type, bool exact, unsigned int* count, unsigned int* result)
{
    unsigned int matches = 0;
    if (count != NULL)
    {
        *count = 0;
    }
    if (result != NULL)
    {
        *result = 0;
    }
    EnterCriticalSection(&mEntityMutex);
    std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin();
    while (it != mEntities.end())
    {
        unsigned long long entityType = (*it)->GetType();
        bool matchesType = exact || (type & 0x3fULL) != 0 ? entityType == type : (entityType & type) == type;
        if (matchesType)
        {
            ++matches;
            if (exact && count != NULL && (*it)->isDespawnProtected())
            {
                ++*count;
            }
            if (exact && result != NULL && (*it)->couldWander())
            {
                ++*result;
            }
        }
        ++it;
    }
    LeaveCriticalSection(&mEntityMutex);
    return static_cast<int>(matches);
}

int Level::countInstanceOfInRange(unsigned long long type, bool exact, int range, int x, int y, int z)
{
    int matches = 0;
    EnterCriticalSection(&mEntityMutex);
    std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin();
    while (it != mEntities.end())
    {
        int distance = static_cast<int>((*it)->distanceTo(x, y, z));
        if (distance * distance <= range * range)
        {
            unsigned long long entityType = (*it)->GetType();
            if (exact || (type & 0x3fULL) != 0 ? entityType == type : (entityType & type) == type)
            {
                ++matches;
            }
        }
        ++it;
    }
    LeaveCriticalSection(&mEntityMutex);
    return matches;
}

bool Level::mayPlace(Block* block, const BlockPos& pos, bool replaceable, const Direction* direction,
                     boost::shared_ptr<Entity> entity)
{
    const BlockState* oldState = getBlockState(pos);
    const AABB* shape = replaceable ? NULL : block->defaultBlockState()->getClipAABB(this, pos);
    if (shape != NULL && !isUnobstructed(shape->move(pos), entity))
    {
        return false;
    }
    if (oldState->getMaterial() == Material::DECORATION && block == Blocks::anvil)
    {
        return true;
    }
    return oldState->getMaterial()->isReplaceable() && block->mayPlace(this, pos, direction);
}

boost::shared_ptr<Explosion> Level::explode(boost::shared_ptr<Entity> source, double x, double y, double z,
                                            float radius, bool fire, bool breaksBlocks, bool playersOnly,
                                            bool noBlockDrops, bool silent, float damageRadius)
{
    boost::shared_ptr<Explosion> explosion(new Explosion(this, source, x, y, z, radius, fire, breaksBlocks, playersOnly,
                                                         noBlockDrops, silent, damageRadius, false));
    explosion->explode();
    explosion->finalizeExplosion(true, NULL);
    return explosion;
}

double Level::getSeenPercent(Vec3* source, AABB* box)
{
    double xStep = 1.0 / ((box->mMaxX - box->mMinX) * 2.0 + 1.0);
    double yStep = 1.0 / ((box->mMaxY - box->mMinY) * 2.0 + 1.0);
    double zStep = 1.0 / ((box->mMaxZ - box->mMinZ) * 2.0 + 1.0);
    if (xStep < 0.0 || yStep < 0.0 || zStep < 0.0)
        return 0.0f;
    int visible = 0;
    int total = 0;
    for (double x = 0.0; x <= 1.0; x += xStep)
    {
        for (double y = 0.0; y <= 1.0; y += yStep)
        {
            for (double z = 0.0; z <= 1.0; z += zStep)
            {
                Vec3* target = Vec3::newTemp(box->mMinX + (box->mMaxX - box->mMinX) * x,
                                             box->mMinY + (box->mMaxY - box->mMinY) * y,
                                             box->mMinZ + (box->mMaxZ - box->mMinZ) * z);
                HitResult* hit = clip(target, source);
                if (hit == NULL)
                    ++visible;
                else
                    delete hit;
                ++total;
            }
        }
    }
    return (float)visible / (float)total;
}

bool Level::extinguishFire(boost::shared_ptr<Player> player, const BlockPos& pos, const Direction* direction)
{
    BlockPos target = pos.relative(direction);
    if (getBlock(target) == Blocks::fire)
    {
        levelEvent(player, LevelEvent::SOUND_EXTINGUISH_FIRE, target, 0);
        removeBlock(target);
        return true;
    }
    return false;
}

Vec3* Level::getSkyColor(boost::shared_ptr<Entity> entity, float partialTick)
{
    float daylight = Mth::clamp(Mth::cos(getTimeOfDay(partialTick) * 6.2831855f) * 2.0f + 0.5f, 0.0f, 1.0f);
    BlockPos pos(Mth::floor(entity->mX), Mth::floor(entity->mY), Mth::floor(entity->mZ));
    unsigned int colour = BiomeColors::getAverageSkyColor(this, pos);
    float red = static_cast<float>((colour >> 16) & 255) / 255.0f * daylight;
    float green = static_cast<float>((colour >> 8) & 255) / 255.0f * daylight;
    float blue = static_cast<float>(colour & 255) / 255.0f * daylight;
    float rain = getRainLevel(partialTick);
    if (rain > 0.0f)
    {
        float grey = (red * 0.3f + green * 0.59f + blue * 0.11f) * 0.6f;
        float clear = 1.0f - rain * 0.75f;
        red = red * clear + grey * (1.0f - clear);
        green = green * clear + grey * (1.0f - clear);
        blue = blue * clear + grey * (1.0f - clear);
    }
    float thunder = getThunderLevel(partialTick);
    if (thunder > 0.0f)
    {
        float grey = (red * 0.3f + green * 0.59f + blue * 0.11f) * 0.2f;
        float clear = 1.0f - thunder * 0.75f;
        red = red * clear + grey * (1.0f - clear);
        green = green * clear + grey * (1.0f - clear);
        blue = blue * clear + grey * (1.0f - clear);
    }
    if (mSkyFlashTime > 0)
    {
        float flash = std::min(mSkyFlashTime - partialTick, 1.0f) * 0.45f;
        float clear = 1.0f - flash;
        red = red * clear + 0.8f * flash;
        green = green * clear + 0.8f * flash;
        blue = blue * clear + flash;
    }
    return Vec3::newTemp(red, green, blue);
}

Vec3* Level::getFogColor(boost::shared_ptr<Entity> entity, float partialTick)
{
    BlockPos pos(Mth::floor(entity->mX), 0, Mth::floor(entity->mZ));
    return getBiome(pos)->getFogColor(getTimeOfDay(partialTick), partialTick);
}

int Level::calcSnowBlockDepth(const BlockPos& pos, int depth)
{
    BlockPos current = pos;
    int layers = 0;
    int blockCount = depth < 1 ? 1 : depth + 1;
    for (int i = 0; i < blockCount; ++i)
    {
        Block* block = getBlock(current);
        if (block == Blocks::snow_layer)
        {
            layers += block->defaultBlockState()->getValue(SnowLayerBlock::LAYERS) + 1;
        }
        else if (block == Blocks::snow)
        {
            layers += 8;
        }
        else
        {
            return layers;
        }
        current = BlockPos(current.x, current.y - 1, current.z);
    }
    return layers;
}

bool Level::canPlaceSnowLayer(const BlockPos& pos, bool checkNeighbors, bool allowExisting, int* depth)
{
    Biome* biome = getBiome(pos);
    if (biome->getTemperature(pos) >= 0.15f || getBrightness(LightLayer::BLOCK, pos) > 11)
    {
        return false;
    }
    if (checkNeighbors && static_cast<unsigned int>(pos.y) > 255)
    {
        return false;
    }
    Block* block = getBlock(pos);
    if (block == Blocks::snow_layer)
    {
        int layers = block->defaultBlockState()->getValue(SnowLayerBlock::LAYERS);
        if (depth != NULL && layers == 7)
        {
            return false;
        }
        if (allowExisting)
        {
            int maximum = biome->getMaxSnowLayers();
            if (maximum > 0 && calcSnowBlockDepth(pos, maximum) >= maximum)
            {
                return false;
            }
        }
        if (depth != NULL)
        {
            *depth = layers + 2;
        }
        return true;
    }
    if (block != Blocks::air)
    {
        return false;
    }
    Block* below = getBlock(BlockPos(pos.x, pos.y - 1, pos.z));
    Material* material = below->defaultBlockState()->getMaterial();
    if (below == Blocks::snow_layer)
    {
        if (below->defaultBlockState()->getValue(SnowLayerBlock::LAYERS) != 7)
        {
            return false;
        }
        if (allowExisting)
        {
            int maximum = biome->getMaxSnowLayers();
            if (maximum > 0 && calcSnowBlockDepth(pos, maximum) >= maximum)
            {
                return false;
            }
        }
    }
    else
    {
        if (checkNeighbors && (below == Blocks::ice || below == Blocks::packed_ice || below == Blocks::grass_path))
        {
            return false;
        }
        if (!below->defaultBlockState()->isSolid() && material != Material::LEAVES && material != Material::ICE
            && material != Material::DIRT)
        {
            return false;
        }
        if (!material->blocksMotion())
        {
            return false;
        }
    }
    if (depth != NULL)
    {
        *depth = 1;
    }
    return true;
}

bool Level::tryToPlaceTopSnow(const BlockPos& pos, bool checkNeighbors, bool allowExisting)
{
    if (!getGameRules()->getBoolean(GameRules::RULE_WEATHER_CYCLE))
    {
        return false;
    }
    int depth = 0;
    bool canPlace = canPlaceSnowLayer(pos, checkNeighbors, true, &depth);
    if (allowExisting)
    {
        int threshold = depth / 2;
        int bestDepth = depth;
        BlockPos bestPos = pos;
        for (int direction = 2; direction < 6; ++direction)
        {
            BlockPos candidate = pos.relative(Direction::from3DDataValue(direction));
            int candidateDepth = 0;
            if (canPlaceSnowLayer(candidate, checkNeighbors, true, &candidateDepth) && candidateDepth < bestDepth)
            {
                LevelChunk* chunk = getChunkAt(pos);
                if ((chunk != NULL && chunk->getTopRainBlock(candidate) == candidate) || candidateDepth < threshold)
                {
                    bestPos = candidate;
                    bestDepth = candidateDepth;
                }
            }
        }
        if (bestPos != pos)
        {
            setBlock(bestPos, SnowLayerBlock::buildSnowBlock(this, bestPos, 1, 1), Block::UPDATE_CLIENTS, false);
            return true;
        }
    }
    if (!canPlace)
    {
        return false;
    }
    setBlock(pos, SnowLayerBlock::buildSnowBlock(this, pos, 1, 1), Block::UPDATE_CLIENTS, false);
    return true;
}

arrayWithLength<unsigned char> Level::getBlocksAndData(int x, int y, int z, int width, int height, int depth,
                                                       bool includeLight)
{
    arrayWithLength<unsigned char> data(static_cast<unsigned int>(width * height * depth * 5), true);
    int minY = std::max(y, 0);
    int maxY = std::min(y + height, 256);
    int offset = 0;
    for (int chunkX = x >> 4; chunkX <= (x + width - 1) >> 4; ++chunkX)
    {
        int minX = std::max(0, x - (chunkX << 4));
        int maxX = std::min(16, x + width - (chunkX << 4));
        for (int chunkZ = z >> 4; chunkZ <= (z + depth - 1) >> 4; ++chunkZ)
        {
            int minZ = std::max(0, z - (chunkZ << 4));
            int maxZ = std::min(16, z + depth - (chunkZ << 4));
            offset = getChunk(chunkX, chunkZ)
                         ->getBlocksAndData(&data, minX, minY, minZ, maxX, maxY, maxZ, offset, includeLight);
        }
    }
    return data;
}

void Level::setBlocksAndData(int x, int y, int z, int width, int height, int depth, arrayWithLength<unsigned char> data,
                             bool includeLight)
{
    int minY = std::max(y, 0);
    int maxY = std::min(y + height, 256);
    int offset = 0;
    for (int chunkX = x >> 4; chunkX <= (x + width - 1) >> 4; ++chunkX)
    {
        int minX = std::max(0, x - (chunkX << 4));
        int maxX = std::min(16, x + width - (chunkX << 4));
        for (int chunkZ = z >> 4; chunkZ <= (z + depth - 1) >> 4; ++chunkZ)
        {
            int minZ = std::max(0, z - (chunkZ << 4));
            int maxZ = std::min(16, z + depth - (chunkZ << 4));
            LevelChunk* chunk = getChunk(chunkX, chunkZ);
            if (static_cast<unsigned int>(offset) < data.length)
            {
                offset = chunk->setBlocksAndData(data, minX, minY, minZ, maxX, maxY, maxZ, offset, includeLight);
            }
            setBlocksDirty(BlockPos((chunkX << 4) + minX, minY, (chunkZ << 4) + minZ),
                           BlockPos((chunkX << 4) + maxX, maxY, (chunkZ << 4) + maxZ));
        }
    }
}

void Level::setFromRawData(int x, int z, arrayWithLength<unsigned char> data)
{
    LevelChunk* chunk = getChunk(x, z);
    if (chunk != NULL)
    {
        chunk->setFromRawData(data);
        setBlocksDirty(BlockPos(x * 16, 0, z * 16), BlockPos(x * 16 + 16, 256, z * 16 + 16));
    }
}

LevelChunk* Level::getCacheChunkDirect(int x, int z)
{
    int halfSize = mChunkCacheSize / 2;
    return mChunkCache[(x + halfSize) * mChunkCacheSize + z + halfSize];
}

bool Level::isChunkPostPostProcessed(int x, int z)
{
    LevelChunk* chunk = getChunkSource()->getChunkIfLoaded(x, z);
    if (chunk == NULL || chunk->mInvalid)
    {
        return false;
    }
    return chunk->isEmpty() || chunk->isTerrainPopulated();
}

bool Level::isChunkFinalised(int x, int z)
{
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dz = -1; dz <= 1; ++dz)
        {
            if (!isChunkPostPostProcessed(x + dx, z + dz))
            {
                return false;
            }
        }
    }
    return true;
}

void Level::refreshLevelInfoFromLevelGen()
{
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    getLevelData()->setCloudHeight(options->getCloudHeight());
    getLevelData()->setDayTime(options->getDayTime());
    mFogDistance = options->getFogDistance();
}

void Level::refreshFromLevelGen()
{
    ConsoleSaveFile* saveFile = getLevelStorage()->getSaveFile();
    saveFile->refreshFromLevelgen();
    refreshLevelInfoFromLevelGen();
}

bool Level::canCreateMore(unsigned long long type, ESPAWN_TYPE spawnType)
{
    int count = 0;
    int limit = 0;
    if (spawnType == eSpawnType_Egg || spawnType == eSpawnType_Portal)
    {
        MobCategory* category = MobCategory::getCategory(type);
        if (category != NULL)
        {
            count = countInstanceOf(category->getEnumBaseClass(), category->isSingleType(), NULL, NULL);
            limit = category->getMaxInstancesPerLevelWithSpawnEgg();
        }
        else if (eTYPE_DERIVED_FROM(EntityType::MINECART, type))
        {
            count = countInstanceOf(EntityType::MINECART, false, NULL, NULL);
            limit = 250;
        }
        else if (eTYPE_DERIVED_FROM(EntityType::BOAT, type))
        {
            count = countInstanceOf(EntityType::BOAT, true, NULL, NULL);
            limit = 40;
        }
    }
    else if (spawnType == eSpawnType_Breed)
    {
        MobCategory* category = MobCategory::getCategory(type);
        if (category != NULL
            && (category->isSingleType() || category->getEnumBaseClass() == EntityType::CREATURE_CATEGORY))
        {
            count = countInstanceOf(type, category->isSingleType(), NULL, NULL);
            limit = category->getMaxInstancesPerLevelWithBreeding();
        }
    }
    return limit == 0 || count < limit;
}

float Level::getSpecialMultiplier(int)
{
    const Difficulty* difficulty = mLevelData->getDifficulty();
    long long levelTime = mLevelData->getGameTime();
    float inhabitedTime = DifficultyInstance::INHABITED_TIME_RANGE * 0.5f;
    float multiplier = 0.0f;
    float moonBrightness = getMoonBrightness();
    if (difficulty != Difficulty::PEACEFUL)
    {
        bool hard = difficulty == Difficulty::HARD;

        multiplier = 0.75f;
        float timeFactor = Mth::clamp(static_cast<float>(levelTime)
                                          + DifficultyInstance::LEVEL_TIME_OFFSET
                                                / DifficultyInstance::LEVEL_TIME_RANGE,
                                      0.0f, 1.0f)
                           * 0.25f;
        multiplier += timeFactor;

        float bonus = 0.0f;
        bonus += Mth::clamp(inhabitedTime / DifficultyInstance::INHABITED_TIME_RANGE, 0.0f, 1.0f)
                 * (hard ? 1.0f : 0.75f);
        bonus += Mth::clamp(moonBrightness * 0.25f, 0.0f, timeFactor);

        if (difficulty == Difficulty::EASY)
        {
            bonus *= 0.5f;
        }

        multiplier += bonus;
        multiplier = multiplier * difficulty->getId();
        if (multiplier < 2.0f)
        {
            multiplier = 0.0f;
        }
        else if (multiplier > 4.0f)
        {
            multiplier = 1.0f;
        }
        else
        {
            multiplier = (multiplier - 2.0f) * 0.5f;
        }
    }
    return multiplier;
}

void Level::removeAllEntities(bool players, bool mobs, bool items, bool vehicles, int x, int y, int z, int radius)
{
    std::vector<boost::shared_ptr<Entity> > entities = getAllEntities();
    std::vector<boost::shared_ptr<Entity> >::iterator it = entities.begin();
    while (it != entities.end())
    {
        boost::shared_ptr<Entity> entity = *it;
        bool isPlayer = eTYPE_DERIVED_FROM(EntityType::HANGING_ENTITY, entity->GetType())
                        || eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, entity->GetType());
        bool isMob = eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType());
        bool isItem = eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType())
                      || eTYPE_DERIVED_FROM(EntityType::BOAT, entity->GetType());
        bool isOther = !isPlayer && !isMob && !isItem;
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::ENDER_DRAGON, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::ENDER_CRYSTAL, entity->GetType()))
        {
            if (Minecraft::InMiniGame(eMiniGameId_Glide, true)
                && eTYPE_DERIVED_FROM(EntityType::ENDER_CRYSTAL, entity->GetType()))
            {
                ++it;
            }
            else
            {
                it = entities.erase(it);
            }
        }
        else if (entity->mChunkX >= x && entity->mChunkX <= z && entity->mChunkZ >= y && entity->mChunkZ <= radius)
        {
            it = entities.erase(it);
        }
        else if ((!players && isPlayer) || (!mobs && isMob) || (!items && isItem) || (!vehicles && isOther))
        {
            it = entities.erase(it);
        }
        else
        {
            ++it;
        }
    }
    removeEntities(&entities);
}

void Level::updateNeighbourForOutputSignal(const BlockPos& pos, Block* block)
{
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const Direction* direction = directions.data[i];
        BlockPos neighbour = pos.relative(direction);
        if (!hasChunkAt(neighbour))
        {
            continue;
        }
        const BlockState* state = getBlockState(neighbour);
        if (static_cast<DiodeBlock*>(Blocks::unpowered_comparator)->isSameDiode(state))
        {
            state->neighborChanged(this, neighbour, block, pos);
        }
        else if (state->isSolidBlockingCubeAndNotSignalSource())
        {
            neighbour = neighbour.relative(direction);
            state = getBlockState(neighbour);
            if (static_cast<DiodeBlock*>(Blocks::unpowered_comparator)->isSameDiode(state))
            {
                state->neighborChanged(this, neighbour, block, pos);
            }
        }
    }
}
