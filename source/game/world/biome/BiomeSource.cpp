#include "game/world/biome/BiomeSource.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "game/core/ColourTable.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/System.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeCache.hpp"
#include "game/world/biome/FjFeatureBiomeRequirements.hpp"
#include "game/world/generation/Layer.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

BiomeSource::BiomeSource()
{
    _init();
}

BiomeSource::BiomeSource(long long seed, LevelType* levelType, SuperflatConfig* levelTypeOptions,
                         LayerOverrideSettings* overrideSettings)
{
    _init(seed, levelType, levelTypeOptions, NULL, overrideSettings);
}

BiomeSource::BiomeSource(LevelData* levelData)
{
    _init(levelData->getSeed(), levelData->getGeneratorType(), levelData->getGeneratorOptions(), levelData, NULL);
}

BiomeSource::~BiomeSource()
{
    DeleteCriticalSection(&mCriticalSection);

    delete mBiomeCache;
}

void BiomeSource::_init()
{
    InitializeCriticalSection(&mCriticalSection);

    mRawBiomeLayer.reset();
    mBiomeLayer.reset();

    mBiomeCache = new BiomeCache(this);

    mPlayerSpawnBiomes.push_back(Biome::FOREST);
    mPlayerSpawnBiomes.push_back(Biome::PLAINS);
    mPlayerSpawnBiomes.push_back(Biome::TAIGA);
    mPlayerSpawnBiomes.push_back(Biome::TAIGA_HILLS);
    mPlayerSpawnBiomes.push_back(Biome::FOREST_HILLS);
    mPlayerSpawnBiomes.push_back(Biome::JUNGLE);
    mPlayerSpawnBiomes.push_back(Biome::JUNGLE_HILLS);
}

void BiomeSource::_init(long long seed, LevelType* levelType, SuperflatConfig* levelTypeOptions, LevelData* levelData,
                        LayerOverrideSettings* overrideSettings)
{
    _init();

    arrayWithLength<boost::shared_ptr<Layer> > layers = Layer::getDefaultLayers(seed, levelType, levelTypeOptions,
                                                                                levelData, overrideSettings);

    mRawBiomeLayer = layers.data[0];
    mBiomeLayer = layers.data[1];

    delete[] layers.data;
}

unsigned char BiomeSource::getBiomeGroup(int biomeId)
{
    switch (biomeId)
    {
    case 12:
    case 13:
    case 26:
    case 30:
    case 31:
    case 140:
    case 158:
        return 0;

    case 3:
    case 5:
    case 9:
    case 19:
    case 20:
    case 25:
    case 32:
    case 33:
    case 34:
    case 131:
    case 133:
    case 160:
    case 161:
    case 162:
        return 1;

    case 1:
    case 4:
    case 16:
    case 18:
    case 27:
    case 28:
    case 29:
    case 128:
    case 129:
    case 132:
    case 155:
    case 156:
    case 157:
        return 2;

    case 6:
    case 21:
    case 22:
    case 23:
    case 134:
    case 149:
    case 151:
        return 3;

    case 2:
    case 17:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 130:
    case 163:
    case 164:
    case 165:
    case 166:
    case 167:
        return 4;

    case 0:
    case 10:
    case 24:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
        return 5;

    case 7:
    case 11:
        return 6;

    case 14:
    case 15:
        return 7;

    default:
        return 8;
    }
}

void BiomeSource::getFracs(arrayWithLength<int> biomeIndices, float* biomeFracs, float* groupFracs)
{
    int i;

    for (i = 0; i < NUM_BIOMES; i++)
    {
        biomeFracs[i] = 0.0f;
    }

    for (i = 0; i < NUM_BIOME_GROUPS; i++)
    {
        groupFracs[i] = 0.0f;
    }

    for (unsigned int index = 0; index < biomeIndices.length; index++)
    {
        int biomeId = biomeIndices.data[index];

        biomeFracs[biomeId] += 1.0f;
        groupFracs[getBiomeGroup(biomeId)] += 1.0f;
    }

    float total = (float)biomeIndices.length;

    for (i = 0; i < NUM_BIOMES; i++)
    {
        biomeFracs[i] /= total;
    }

    for (i = 0; i < NUM_BIOME_GROUPS; i++)
    {
        groupFracs[i] /= total;
    }
}

bool BiomeSource::getIsMatch(arrayWithLength<int> biomeIndices, float* biomeFracs, float* groupFracs)
{
    getFracs(biomeIndices, biomeFracs, groupFracs);

    for (int i = 0; i < NUM_BIOME_GROUPS; i++)
    {
        if (i == 7)
        {
            if (groupFracs[i] < 0.001f)
            {
                return false;
            }
        }
        else if (i == 5)
        {
            if (groupFracs[i] > 0.25f)
            {
                return false;
            }

            if (groupFracs[i] < 0.01f)
            {
                return false;
            }
        }
        else if (i == 6)
        {
            if (groupFracs[i] > 0.1f)
            {
                return false;
            }
        }
        else
        {
            if (groupFracs[i] < 0.01f)
            {
                return false;
            }
        }
    }

    return true;
}

long long BiomeSource::findSeed(LevelType* levelType)
{
    long long bestSeed = 0;
    float bestScore = 0.0f;

    ProgressRenderer* progressListener = Minecraft::GetInstance()->mProgressRenderer;
    progressListener->progressStage(0x6C457A87);

    Random random(System::processTimeInNanoSecs());

    arrayWithLength<int> biomeIndices(40000, true);

    int attempts = 0;
    unsigned int startTime = GetTickCount();
    bool matched;

    do
    {
        long long seed = random.nextLong();

        if (levelType == LevelType::DEFAULT)
        {
            float biomeFracs[NUM_BIOMES];
            float groupFracs[NUM_BIOME_GROUPS];

            {
                BiomeSource biomeSource(seed, levelType, NULL, NULL);
                biomeSource.getRawBiomeIndices(biomeIndices, -100, -100, 200, 200);

                matched = getIsMatch(biomeIndices, biomeFracs, groupFracs);
            }

            if (matched)
            {
                bestSeed = seed;
            }
            else
            {
                float score = 0.0f;

                for (int i = 0; i < NUM_BIOME_GROUPS; i++)
                {
                    if (i == 5)
                    {
                        score += groupFracs[i];
                    }
                    else
                    {
                        score += groupFracs[i] * 4.0f;
                    }
                }

                score *= 0.125f;

                if (score > bestScore)
                {
                    bestScore = score;
                    bestSeed = seed;
                }
            }
        }
        else
        {
            matched = true;
            bestSeed = seed;
        }

        attempts++;
        progressListener->progressStagePercentage(attempts % 100);
    } while (GetTickCount() - startTime <= 10000 && !matched);

    delete[] biomeIndices.data;

    return bestSeed;
}

Biome* BiomeSource::getBiome(const BlockPos& pos)
{
    return getBiome(pos, NULL);
}

Biome* BiomeSource::getBiome(const BlockPos& pos, Biome* fallback)
{
    return mBiomeCache->getBiome(pos.x, pos.z, fallback);
}

float BiomeSource::scaleTemp(float temperature, int height) const
{
    return temperature;
}

void BiomeSource::getRawBiomeBlock(arrayWithLength<Biome*>& biomes, int x, int z, int width, int height) const
{
    PIXBeginNamedEvent(0.0f, "Setup");
    IntCache::releaseAll();

    int count = width * height;
    biomes.resize(count);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Getting area");
    arrayWithLength<int> area = mRawBiomeLayer->getArea(x, z, width, height);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Copying values");
    for (int i = 0; i < count; i++)
    {
        biomes.data[i] = Biome::getBiome(area.data[i], Biome::DEFAULT);
    }
    PIXEndNamedEvent();
}

void BiomeSource::getRawBiomeIndices(arrayWithLength<int>& biomeIndices, int x, int z, int width, int height) const
{
    IntCache::releaseAll();

    arrayWithLength<int> area = mRawBiomeLayer->getArea(x, z, width, height);

    int count = width * height;
    int* destination = biomeIndices.data;

    for (int i = 0; i < count; i++)
    {
        destination[i] = area[i];
    }
}

void BiomeSource::getBiomeBlock(arrayWithLength<Biome*>& biomes, int x, int z, int width, int height,
                                bool useCache) const
{
    PIXBeginNamedEvent(0.0f, "Setup");
    IntCache::releaseAll();

    int count = width * height;
    biomes.resize(count);
    PIXEndNamedEvent();

    if (useCache && width == 16 && height == 16 && ((x | z) & 15) == 0)
    {
        PIXBeginNamedEvent(0.0f, "Copying cache");
        arrayWithLength<Biome*> cached = mBiomeCache->getBiomeBlockAt(x, z);
        System::arraycopy(cached, 0, &biomes, 0, 256);
        delete cached.data;
        PIXEndNamedEvent();

        return;
    }

    PIXBeginNamedEvent(0.0f, "Getting area");
    arrayWithLength<int> area = mBiomeLayer->getArea(x, z, width, height);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Copying values");
    for (int i = 0; i < count; i++)
    {
        biomes.data[i] = Biome::getBiome(area.data[i], Biome::DEFAULT);
    }
    PIXEndNamedEvent();
}

unsigned char* BiomeSource::getBiomeIndexBlock(int x, int z, int width, int height) const
{
    if (width == 16 && height == 16 && ((x | z) & 15) == 0)
    {
        return mBiomeCache->getBiomeIndexBlockAt(x, z).data;
    }

    arrayWithLength<unsigned char> biomeIndices;

    getBiomeIndexBlock(biomeIndices, x, z, width, height, true);

    return biomeIndices.data;
}

void BiomeSource::getBiomeIndexBlock(arrayWithLength<unsigned char>& biomeIndices, int x, int z, int width, int height,
                                     bool useCache) const
{
    IntCache::releaseAll();

    int count = width * height;
    biomeIndices.resize(count);

    if (useCache && width == 16 && height == 16 && ((x | z) & 15) == 0)
    {
        PIXBeginNamedEvent(0.0f, "Copying cached block");
        arrayWithLength<unsigned char> cached = mBiomeCache->getBiomeIndexBlockAt(x, z);
        System::arraycopy(cached, 0, &biomeIndices, 0, 256);
        PIXEndNamedEvent();

        return;
    }

    PIXBeginNamedEvent(0.0f, "Getting area");
    arrayWithLength<int> area = mBiomeLayer->getArea(x, z, width, height);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Copying values");
    for (int i = 0; i < count; i++)
    {
        biomeIndices.data[i] = (unsigned char)area.data[i];
    }
    PIXEndNamedEvent();
}

bool BiomeSource::containsOnly(int x, int z, int radius, const std::vector<Biome*>& biomes)
{
    IntCache::releaseAll();

    int minX = (x - radius) >> 2;
    int minZ = (z - radius) >> 2;
    int width = (((x + radius) >> 2) - minX) + 1;
    int height = (((z + radius) >> 2) - minZ) + 1;

    arrayWithLength<int> area = mRawBiomeLayer->getArea(minX, minZ, width, height);

    for (int i = 0; i < width * height; i++)
    {
        Biome* biome = Biome::getBiome(area.data[i]);

        if (std::find(biomes.begin(), biomes.end(), biome) == biomes.end())
        {
            return false;
        }
    }

    return true;
}

BlockPos* BiomeSource::findBiome(int x, int z, int radius, std::vector<Biome*> biomes, Random* random)
{
    IntCache::releaseAll();

    int minX = (x - radius) >> 2;
    int minZ = (z - radius) >> 2;
    unsigned int width = (unsigned int)((((x + radius) >> 2) - minX) + 1);
    unsigned int height = (unsigned int)((((z + radius) >> 2) - minZ) + 1);

    MemSect(50);
    arrayWithLength<int> area = mRawBiomeLayer->getArea(minX, minZ, (int)width, (int)height);

    BlockPos* found = NULL;
    int foundCount = 0;
    unsigned int count = width * height;

    for (unsigned int i = 0; i < count; i++)
    {
        Biome* biome = Biome::getBiome(area.data[i]);

        if (std::find(biomes.begin(), biomes.end(), biome) != biomes.end()
            && (found == NULL || random->nextInt(foundCount + 1) == 0))
        {
            delete found;

            found = new BlockPos((minX + (int)(i % width)) * 4, 0, (minZ + (int)(i / width)) * 4);

            foundCount++;
        }
    }

    MemSect(0);

    return found;
}

void BiomeSource::update()
{
    mBiomeCache->update();
}

bool BiomeSource::isFixedBiome()
{
    return false;
}

Biome* BiomeSource::getFixedBiome()
{
    return NULL;
}

bool BiomeSource::fjCouldSpawn(const FjFeatureBiomeRequirements* requirements, int chunkX, int chunkZ)
{
    for (std::vector<CouldSpawnCache>::iterator it = mCouldSpawnCaches.begin(); it != mCouldSpawnCaches.end(); ++it)
    {
        if (it->mRequirements == requirements)
        {
            EnterCriticalSection(&mCriticalSection);

            bool couldSpawn = it->couldSpawn(chunkX, chunkZ);

            LeaveCriticalSection(&mCriticalSection);

            return couldSpawn;
        }
    }

    mCouldSpawnCaches.push_back(CouldSpawnCache(this, requirements));

    return fjCouldSpawn(requirements, chunkX, chunkZ);
}

BiomeSource::CouldSpawnCache::CouldSpawnCache(BiomeSource* biomeSource, const FjFeatureBiomeRequirements* requirements)
{
    mBiomeSource = biomeSource;
    mRequirements = requirements;
}

bool BiomeSource::CouldSpawnCache::couldSpawn(int chunkX, int chunkZ)
{
    ChunkPos chunkPos(chunkX, chunkZ);

    const_iterator cached = find(chunkPos);

    if (cached != end())
    {
        return cached->second;
    }

    int x = chunkX * 16 + 8;
    int z = chunkZ * 16 + 8;

    bool couldSpawn = mBiomeSource->containsOnly(x, z, mRequirements->spawnRadius, *mRequirements->spawnBiomes);

    if (couldSpawn && mRequirements->allowedBiomes != NULL)
    {
        couldSpawn = mBiomeSource->containsOnly(x, z, mRequirements->allowedRadius, *mRequirements->allowedBiomes);
    }

    (*this)[chunkPos] = couldSpawn;

    return couldSpawn;
}

BiomeSource::LayerOverrideSettings::LayerOverrideSettings()
{
    mFullyGenerated = false;
    mLastGeneratedSection = -1;

    mRandom = new Random(System::processTimeInNanoSecs());
    mSeed = mRandom->nextLong();

    mWidthInChunksToDisplay = 54;
    mImageWidth = 864;
    mHalfImageWidth = 432;
    mBiomeSectionCount = 12;
    mBiomeSectionWidth = 72;
    mHalfBiomeSectionWidth = 36;
    mChunksPerScroll = 6;
    mGenerationScale = eBiomeGenerationScale_Small;
    mCentreXChunk = 0;
    mCentreZChunk = 0;
    mZoomLevel = 0;
    mZoomedCentreX = 0;
    mZoomedCentreZ = 0;

    InitializeCriticalSection(&mBiomesCriticalSection);

    EnterCriticalSection(&mBiomesCriticalSection);

    mBiomes = arrayWithLength<Biome*>(mImageWidth * mImageWidth, true);

    LeaveCriticalSection(&mBiomesCriticalSection);
}

BiomeSource::LayerOverrideSettings::~LayerOverrideSettings()
{
    delete mRandom;

    EnterCriticalSection(&mBiomesCriticalSection);

    delete[] mBiomes.data;

    LeaveCriticalSection(&mBiomesCriticalSection);

    DeleteCriticalSection(&mBiomesCriticalSection);
}

int BiomeSource::LayerOverrideSettings::getBiomeIndex(int x, int z)
{
    return z * mImageWidth + x;
}

void BiomeSource::LayerOverrideSettings::SetWidthInChunksToDisplay(int widthInChunks, int extraSections)
{
    mWidthInChunksToDisplay = widthInChunks;
    mImageWidth = widthInChunks * 16;
    mBiomeSectionCount = mBiomeSectionCount + extraSections;
    mHalfImageWidth = mImageWidth / 2;
    mBiomeSectionWidth = mImageWidth / mBiomeSectionCount;
    mChunksPerScroll = widthInChunks / 8;
    mHalfBiomeSectionWidth = mBiomeSectionWidth / 2;

    EnterCriticalSection(&mBiomesCriticalSection);

    delete[] mBiomes.data;

    mBiomes = arrayWithLength<Biome*>(mImageWidth * mImageWidth, true);

    LeaveCriticalSection(&mBiomesCriticalSection);
}

void BiomeSource::LayerOverrideSettings::SetSeed(const std::wstring& seedText)
{
    if (seedText.length() == 0)
    {
        return;
    }

    std::wstring seed = seedText;
    unsigned int length = seed.length();
    bool isNumber = length != 0;
    unsigned int i;

    for (i = 0; i < length; i++)
    {
        if ((seed.at(i) < L'0' || seed.at(i) > L'9') && (i != 0 || seed.at(0) != L'-'))
        {
            isNumber = false;
            break;
        }
    }

    if (isNumber)
    {
        mSeed = _fromString<long long>(seed);
    }
    else
    {
        int hash = 0;

        for (i = 0; i < length; i++)
        {
            hash = hash * 31 + seed.at(i);
        }

        mSeed = hash;
    }
}

void BiomeSource::LayerOverrideSettings::GenerateNewSeed()
{
    mSeed = mRandom->nextLong();

    mCentreXChunk = 0;
    mCentreZChunk = 0;
    mZoomLevel = 0;
    mZoomedCentreX = 0;
    mZoomedCentreZ = 0;
    mGenerationScale = eBiomeGenerationScale_Small;

    int biomeCount = mImageWidth * mImageWidth;

    EnterCriticalSection(&mBiomesCriticalSection);

    memset(mBiomes.data, 0, biomeCount << 2);

    LeaveCriticalSection(&mBiomesCriticalSection);

    mFullyGenerated = false;
    mLastGeneratedSection = -1;
}

long long BiomeSource::LayerOverrideSettings::GetSeed()
{
    return mSeed;
}

bool BiomeSource::LayerOverrideSettings::FullyGenerated()
{
    return mFullyGenerated;
}

void BiomeSource::LayerOverrideSettings::SetGenerationScale(BiomeSource::eBiomeGenerationScale scale)
{
    mGenerationScale = scale;
}

BiomeSource::eBiomeGenerationScale BiomeSource::LayerOverrideSettings::GetGenerationScale()
{
    return mGenerationScale;
}

unsigned char BiomeSource::LayerOverrideSettings::GetBiomeScale()
{
    return (unsigned char)mGenerationScale;
}

void BiomeSource::LayerOverrideSettings::SetScaledCentrePosition()
{
    int halfChunksToDisplay = mWidthInChunksToDisplay / 2;
    int zoomScale = (int)pow(2.0, (double)mZoomLevel + 1.0);

    double xFraction = Mth::abs((double)mZoomedCentreX / zoomScale);
    double zFraction = Mth::abs((double)mZoomedCentreZ / zoomScale);

    int offsetX = (int)Mth::clampedLerp(0.0, (double)halfChunksToDisplay, xFraction);
    int offsetZ = (int)Mth::clampedLerp(0.0, (double)halfChunksToDisplay, zFraction);

    if (mZoomedCentreX < 0)
    {
        offsetX = -offsetX;
    }

    if (mZoomedCentreZ < 0)
    {
        offsetZ = -offsetZ;
    }

    int scale = (int)pow(2.0, (double)mGenerationScale);

    mCentreXChunk += offsetX / scale;
    mCentreZChunk += offsetZ / scale;
}

bool BiomeSource::LayerOverrideSettings::IncreaseScale()
{
    if (mGenerationScale == eBiomeGenerationScale_Small)
    {
        mGenerationScale = eBiomeGenerationScale_Medium;
    }
    else if (mGenerationScale == eBiomeGenerationScale_Medium)
    {
        mGenerationScale = eBiomeGenerationScale_Large;
    }
    else
    {
        return false;
    }

    SetScaledCentrePosition();

    mZoomedCentreZ = 0;
    mZoomedCentreX = 0;
    mZoomLevel = 0;

    EnterCriticalSection(&mBiomesCriticalSection);

    memset(mBiomes.data, 0, mImageWidth * mImageWidth * sizeof(Biome*));

    LeaveCriticalSection(&mBiomesCriticalSection);

    mFullyGenerated = false;
    mLastGeneratedSection = -1;

    return true;
}

bool BiomeSource::LayerOverrideSettings::DecreaseScale()
{
    if (mGenerationScale == eBiomeGenerationScale_Medium)
    {
        mGenerationScale = eBiomeGenerationScale_Small;
    }
    else if (mGenerationScale == eBiomeGenerationScale_Large)
    {
        mGenerationScale = eBiomeGenerationScale_Medium;
    }
    else
    {
        return false;
    }

    SetScaledCentrePosition();

    mZoomedCentreZ = 0;
    mZoomedCentreX = 0;
    mZoomLevel = 0;

    EnterCriticalSection(&mBiomesCriticalSection);

    memset(mBiomes.data, 0, mImageWidth * mImageWidth * sizeof(Biome*));

    LeaveCriticalSection(&mBiomesCriticalSection);

    mFullyGenerated = false;
    mLastGeneratedSection = -1;

    return true;
}

void BiomeSource::LayerOverrideSettings::SetCentrePosition(int centreXChunk, int centreZChunk)
{
    mCentreXChunk = centreXChunk;
    mCentreZChunk = centreZChunk;
}

int BiomeSource::LayerOverrideSettings::GetCentreXChunk()
{
    return mCentreXChunk;
}

int BiomeSource::LayerOverrideSettings::GetCentreZChunk()
{
    return mCentreZChunk;
}

void BiomeSource::LayerOverrideSettings::RegenerateUp(arrayWithLength<unsigned char> pixels)
{
    int scale = (int)pow(2.0, (double)mGenerationScale);

    int rows = mChunksPerScroll / scale;

    if (rows < 1)
    {
        rows = 1;
    }

    int scaledRows = scale * rows;
    int shiftRows = scaledRows * 16;
    int shiftCount = mImageWidth * shiftRows;
    int keptCount = mImageWidth * (mImageWidth - shiftRows);

    mCentreZChunk -= rows;

    EnterCriticalSection(&mBiomesCriticalSection);

    memmove(mBiomes.data + shiftCount, mBiomes.data, keptCount * sizeof(Biome*));
    memset(mBiomes.data, 0, shiftCount << 2);

    LeaveCriticalSection(&mBiomesCriticalSection);

    mLastGeneratedSection = -1;

    int pixelWidth = mImageWidth;
    int pixelShift = pixelWidth * shiftRows;

    memmove(pixels.data + shiftCount * 4, pixels.data, pixelWidth * (pixelWidth - shiftRows) * 4);
    memset(pixels.data, 0, pixelShift << 2);

    mLastGeneratedSection = -1;
    mFullyGenerated = false;
}

void BiomeSource::LayerOverrideSettings::RegenerateDown(arrayWithLength<unsigned char> pixels)
{
    int scale = (int)pow(2.0, (double)mGenerationScale);

    int rows = mChunksPerScroll / scale;

    if (rows < 1)
    {
        rows = 1;
    }

    int scaledRows = scale * rows;
    int shiftRows = scaledRows * 16;
    int shiftCount = mImageWidth * shiftRows;
    int keptCount = mImageWidth * (mImageWidth - shiftRows);
    int clearedStart = mImageWidth * mImageWidth - shiftCount;

    mCentreZChunk += rows;

    EnterCriticalSection(&mBiomesCriticalSection);

    memmove(mBiomes.data, mBiomes.data + shiftCount, keptCount * sizeof(Biome*));
    memset(mBiomes.data + clearedStart, 0, shiftCount << 2);

    LeaveCriticalSection(&mBiomesCriticalSection);

    mLastGeneratedSection = -1;

    int pixelWidth = mImageWidth;
    int pixelShift = pixelWidth * shiftRows;

    memmove(pixels.data, pixels.data + shiftCount * 4, pixelWidth * (pixelWidth - shiftRows) * 4);
    memset(pixels.data + clearedStart * 4, 0, pixelShift << 2);

    mLastGeneratedSection = -1;
    mFullyGenerated = false;
}

void BiomeSource::LayerOverrideSettings::RegenerateLeft(arrayWithLength<unsigned char> pixels)
{
    int scale = (int)pow(2.0, (double)mGenerationScale);

    int columns = mChunksPerScroll / scale;

    if (columns < 1)
    {
        columns = 1;
    }

    int scaledColumns = scale * columns;
    int shiftCount = scaledColumns * 16;
    int keptCount = mImageWidth - shiftCount;

    mCentreXChunk -= columns;

    mLastGeneratedSection = -1;

    for (int z = 0; z < mImageWidth; z++)
    {
        int destination = getBiomeIndex(shiftCount, z);
        int source = destination - shiftCount;

        EnterCriticalSection(&mBiomesCriticalSection);

        memmove(mBiomes.data + destination, mBiomes.data + source, keptCount * sizeof(Biome*));
        memset(mBiomes.data + source, 0, scaledColumns << 6);

        LeaveCriticalSection(&mBiomesCriticalSection);

        memmove(pixels.data + destination * 4, pixels.data + source * 4, keptCount * 4);
        memset(pixels.data + source * 4, 0, scaledColumns << 6);
    }

    mLastGeneratedSection = -1;
    mFullyGenerated = false;
}

void BiomeSource::LayerOverrideSettings::RegenerateRight(arrayWithLength<unsigned char> pixels)
{
    int scale = (int)pow(2.0, (double)mGenerationScale);

    int columns = mChunksPerScroll / scale;

    if (columns < 1)
    {
        columns = 1;
    }

    int scaledColumns = scale * columns;
    int shiftCount = scaledColumns * 16;
    int keptCount = mImageWidth - shiftCount;

    mCentreXChunk += columns;

    mLastGeneratedSection = -1;

    for (int z = 0; z < mImageWidth; z++)
    {
        int destination = z * mImageWidth;
        int source = destination + shiftCount;
        int cleared = destination + keptCount;

        EnterCriticalSection(&mBiomesCriticalSection);

        memmove(mBiomes.data + destination, mBiomes.data + source, keptCount * sizeof(Biome*));
        memset(mBiomes.data + cleared, 0, scaledColumns << 6);

        LeaveCriticalSection(&mBiomesCriticalSection);

        memmove(pixels.data + destination * 4, pixels.data + source * 4, keptCount * 4);
        memset(pixels.data + cleared * 4, 0, scaledColumns << 6);
    }

    mLastGeneratedSection = -1;
    mFullyGenerated = false;
}

bool BiomeSource::LayerOverrideSettings::isZoomed()
{
    return mZoomLevel != 0;
}

int BiomeSource::LayerOverrideSettings::GetZoomedCentreX()
{
    if (!isZoomed())
    {
        return 0;
    }

    int zoomScale = (int)pow(2.0, (double)mZoomLevel + 1.0);

    return (mHalfImageWidth * mZoomedCentreX) / zoomScale;
}

int BiomeSource::LayerOverrideSettings::GetZoomedCentreZ()
{
    if (!isZoomed())
    {
        return 0;
    }

    int zoomScale = (int)pow(2.0, (double)mZoomLevel + 1.0);

    return (mHalfImageWidth * mZoomedCentreZ) / zoomScale;
}

bool BiomeSource::LayerOverrideSettings::CheckWithinBounds(int& value, int delta)
{
    if (!isZoomed())
    {
        value = 0;
        return false;
    }

    int limit = (int)pow(2.0, (double)mZoomLevel + 1.0) - 2;
    int moved = value + delta;

    bool withinBounds = true;

    if (moved > limit || moved < -limit)
    {
        withinBounds = false;
    }
    else
    {
        value = moved;
    }

    return withinBounds;
}

void BiomeSource::LayerOverrideSettings::SetZoomedDisplay(int x, int z)
{
    if (!isZoomed())
    {
        return;
    }

    int zoomScale = (int)pow(2.0, (double)mZoomLevel + 1.0);

    mZoomedCentreX = (x * zoomScale) / mHalfImageWidth;
    mZoomedCentreZ = (z * zoomScale) / mHalfImageWidth;

    int deltaX = 0;
    int deltaZ = 0;

    while (!CheckWithinBounds(mZoomedCentreX, deltaX))
    {
        if (mZoomedCentreX < 0)
        {
            deltaX++;
        }
        else
        {
            deltaX--;
        }
    }

    while (!CheckWithinBounds(mZoomedCentreZ, deltaZ))
    {
        if (mZoomedCentreZ < 0)
        {
            deltaZ++;
        }
        else
        {
            deltaZ--;
        }
    }
}

bool BiomeSource::LayerOverrideSettings::ZoomIn()
{
    if (mZoomLevel >= 2)
    {
        return false;
    }

    int centreX = GetZoomedCentreX();
    int centreZ = GetZoomedCentreZ();

    mZoomLevel++;

    SetZoomedDisplay(centreX, centreZ);

    return true;
}

bool BiomeSource::LayerOverrideSettings::ZoomOut()
{
    if (mZoomLevel == 0)
    {
        return false;
    }

    int centreX = GetZoomedCentreX();
    int centreZ = GetZoomedCentreZ();

    mZoomLevel--;

    SetZoomedDisplay(centreX, centreZ);

    return true;
}

bool BiomeSource::LayerOverrideSettings::ScrollUp(arrayWithLength<unsigned char> pixels)
{
    if (CheckWithinBounds(mZoomedCentreZ, -1))
    {
        return true;
    }

    RegenerateUp(pixels);

    return false;
}

bool BiomeSource::LayerOverrideSettings::ScrollDown(arrayWithLength<unsigned char> pixels)
{
    if (CheckWithinBounds(mZoomedCentreZ, 1))
    {
        return true;
    }

    RegenerateDown(pixels);

    return false;
}

bool BiomeSource::LayerOverrideSettings::ScrollLeft(arrayWithLength<unsigned char> pixels)
{
    if (CheckWithinBounds(mZoomedCentreX, -1))
    {
        return true;
    }

    RegenerateLeft(pixels);

    return false;
}

bool BiomeSource::LayerOverrideSettings::ScrollRight(arrayWithLength<unsigned char> pixels)
{
    if (CheckWithinBounds(mZoomedCentreX, 1))
    {
        return true;
    }

    RegenerateRight(pixels);

    return false;
}

unsigned char BiomeSource::LayerOverrideSettings::GetZoomLevel()
{
    return mZoomLevel;
}

int BiomeSource::LayerOverrideSettings::GetZoomedCentreXPercent()
{
    if (!isZoomed())
    {
        return 0;
    }

    int zoomScale = (int)pow(2.0, (double)mZoomLevel + 1.0);

    return (mZoomedCentreX * 100) / zoomScale;
}

int BiomeSource::LayerOverrideSettings::GetZoomedCentreZPercent()
{
    if (!isZoomed())
    {
        return 0;
    }

    int zoomScale = (int)pow(2.0, (double)mZoomLevel + 1.0);

    return (mZoomedCentreZ * 100) / zoomScale;
}

void BiomeSource::LayerOverrideSettings::NullBiomeSection(int x, int width, int z, int height)
{
    size_t byteCount = width * sizeof(Biome*);
    int endRow = z + height;

    for (int row = z; row < endRow; row++)
    {
        memset(mBiomes.data + (x + row * mImageWidth), 0, byteCount);
    }
}

void BiomeSource::LayerOverrideSettings::RegenerateAllBiomes()
{
    BiomeSource biomeSource(mSeed, LevelType::DEFAULT, NULL, this);

    int scale = (int)pow(2.0, (double)mGenerationScale);
    int centreXChunk = mCentreXChunk;
    int centreZChunk = mCentreZChunk;

    EnterCriticalSection(&mBiomesCriticalSection);

    if (mBiomes.data != NULL)
    {
        delete[] mBiomes.data;

        mBiomes.data = NULL;
        mBiomes.length = 0;
    }

    biomeSource.getBiomeBlock(mBiomes, scale * centreXChunk * 16 - mHalfImageWidth,
                              scale * centreZChunk * 16 - mHalfImageWidth, mImageWidth, mImageWidth, true);

    LeaveCriticalSection(&mBiomesCriticalSection);
}

void BiomeSource::LayerOverrideSettings::RegenerateBiomeSection(int x, int width, int z, int height)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }

    if (mBiomes.data == NULL)
    {
        RegenerateAllBiomes();
        return;
    }

    BiomeSource biomeSource(mSeed, LevelType::DEFAULT, NULL, this);

    arrayWithLength<Biome*> sectionBiomes(width * height, true);

    int scale = (int)pow(2.0, (double)mGenerationScale);

    biomeSource.getBiomeBlock(sectionBiomes, (scale * mCentreXChunk * 16 - mHalfImageWidth) + x,
                              (scale * mCentreZChunk * 16 - mHalfImageWidth) + z, width, height, true);

    int source = 0;

    for (int row = z; row < z + height; row++)
    {
        for (int column = x; column < x + width; column++)
        {
            unsigned int index = (unsigned int)getBiomeIndex(column, row);

            if (index < mBiomes.length)
            {
                EnterCriticalSection(&mBiomesCriticalSection);

                mBiomes[index] = sectionBiomes[source];

                LeaveCriticalSection(&mBiomesCriticalSection);
            }

            source++;
        }
    }

    delete[] sectionBiomes.data;
}

bool BiomeSource::LayerOverrideSettings::SectionNeedsRegen(int x, int width, int z, int height)
{
    for (int row = z; row < z + height; row++)
    {
        for (int column = x; column < x + width; column++)
        {
            if (mBiomes[getBiomeIndex(column, row)] == NULL)
            {
                return true;
            }
        }
    }

    return false;
}

bool BiomeSource::LayerOverrideSettings::CheckAndRegenerate(int x, int z, bool nullOnly)
{
    int width = mBiomeSectionWidth;
    int height = mBiomeSectionWidth;

    x -= mHalfBiomeSectionWidth;
    z -= mHalfBiomeSectionWidth;

    if (x < 0)
    {
        width = mBiomeSectionWidth + x;
        x = 0;
    }
    else if (x + mBiomeSectionWidth > mImageWidth)
    {
        width = mBiomeSectionWidth - ((x + mBiomeSectionWidth) - mImageWidth);
    }

    if (z < 0)
    {
        height = mBiomeSectionWidth + z;
        z = 0;
    }
    else if (z + mBiomeSectionWidth > mImageWidth)
    {
        height = mBiomeSectionWidth - ((z + mBiomeSectionWidth) - mImageWidth);
    }

    if (nullOnly)
    {
        NullBiomeSection(x, width, z, height);

        return false;
    }

    if (!SectionNeedsRegen(x, width, z, height))
    {
        return false;
    }

    RegenerateBiomeSection(x, width, z, height);

    return true;
}

bool BiomeSource::LayerOverrideSettings::RegenerateNearestBiomeSection(int& sectionX, int& sectionZ,
                                                                       bool& atVerticalEdge, bool& atHorizontalEdge)
{
    int x = mHalfImageWidth;
    int z = mHalfImageWidth;

    if (mLastGeneratedSection < 0)
    {
        if (CheckAndRegenerate(x, z, false))
        {
            sectionX = x - mHalfBiomeSectionWidth;
            sectionZ = z - mHalfBiomeSectionWidth;
            mLastGeneratedSection = 0;

            return true;
        }

        if (mLastGeneratedSection == -1)
        {
            mLastGeneratedSection = 0;
        }
    }

    int section = 1;
    int direction = -1;

    int ring = 1;

    while (ring <= mBiomeSectionCount)
    {
        for (int axis = 0; axis < 2; axis++)
        {
            for (int step = 0; step < ring; step++)
            {
                if (axis == 1)
                {
                    x += mBiomeSectionWidth * direction;
                }
                else
                {
                    z += mBiomeSectionWidth * direction;
                }

                if (section > mLastGeneratedSection && CheckAndRegenerate(x, z, false))
                {
                    if (ring == mBiomeSectionCount - 1 && (axis == 1 || step == ring - 1))
                    {
                        sectionZ = z;
                        atHorizontalEdge = true;

                        if (axis == 1 && step == ring - 1)
                        {
                            atVerticalEdge = true;
                        }
                    }
                    else
                    {
                        sectionZ = z - mHalfBiomeSectionWidth;
                    }

                    sectionX = x - mHalfBiomeSectionWidth;

                    if (mLastGeneratedSection == -1)
                    {
                        CheckAndRegenerate(x, z, true);

                        return false;
                    }

                    mLastGeneratedSection = section;

                    if (ring == mBiomeSectionCount)
                    {
                        if (step == ring - 1)
                        {
                            atVerticalEdge = true;
                            atHorizontalEdge = true;
                        }
                        else if (axis == 1)
                        {
                            atHorizontalEdge = true;
                        }
                        else
                        {
                            atVerticalEdge = true;
                        }
                    }

                    return true;
                }

                section++;
            }

            if (axis == 1)
            {
                ring++;
            }
            else
            {
                direction = -direction;
            }
        }
    }

    for (int row = 0; row < mBiomeSectionCount; row++)
    {
        z -= mBiomeSectionWidth;

        if (section > mLastGeneratedSection && CheckAndRegenerate(x, z, false))
        {
            sectionX = x;
            sectionZ = z;

            if (mLastGeneratedSection == -1)
            {
                CheckAndRegenerate(x, z, true);

                return false;
            }

            mLastGeneratedSection = section;
            atVerticalEdge = true;

            return true;
        }

        section++;
    }

    mFullyGenerated = true;

    return false;
}

int BiomeSource::LayerOverrideSettings::GetBiomeSectionWidth()
{
    return mBiomeSectionWidth;
}

int BiomeSource::LayerOverrideSettings::GetImageWidth()
{
    return mImageWidth;
}

BlockPos BiomeSource::LayerOverrideSettings::GetCursorBlockPos(float xFraction, float zFraction)
{
    int scale = (int)pow(2.0, (double)mZoomLevel);
    int halfVisibleWidth = mHalfImageWidth / scale;

    int centreX = 0;
    int centreZ = 0;

    if (isZoomed())
    {
        int zoomScale = (int)pow(2.0, (double)mZoomLevel + 1.0);

        centreZ = (mHalfImageWidth * mZoomedCentreZ) / zoomScale;
        centreX = (mHalfImageWidth * mZoomedCentreX) / zoomScale;
    }

    return BlockPos(
        Mth::clampedLerp((double)(centreX - halfVisibleWidth), (double)(centreX + halfVisibleWidth), xFraction), 64.0,
        Mth::clampedLerp((double)(centreZ - halfVisibleWidth), (double)(centreZ + halfVisibleWidth), zFraction));
}

Biome* BiomeSource::LayerOverrideSettings::GetBiomeAt(float xFraction, float zFraction)
{
    BlockPos cursor = GetCursorBlockPos(xFraction, zFraction);

    int maximum = mImageWidth - 1;

    int x = cursor.x + mHalfImageWidth;

    if (x > maximum)
    {
        x = maximum;
    }

    int z = cursor.z + mHalfImageWidth;

    if (z > maximum)
    {
        z = maximum;
    }

    return mBiomes.data[getBiomeIndex(x, z)];
}

arrayWithLength<unsigned char> BiomeSource::LayerOverrideSettings::CreateZoomedImage(unsigned int* biomeMapPixels,
                                                                                     int& zoomLevel)
{
    if (!isZoomed())
    {
        return arrayWithLength<unsigned char>();
    }

    zoomLevel = mZoomLevel;

    int scale = (int)pow(2.0, (double)mZoomLevel);
    int zoomedWidth = mImageWidth / scale;

    arrayWithLength<unsigned char> zoomedImage(zoomedWidth * zoomedWidth * 4, false);

    int halfZoomedWidth = mHalfImageWidth / scale;
    int zoomScale = (int)pow(2.0, (double)zoomLevel + 1.0);

    int centreX = (mHalfImageWidth * mZoomedCentreX) / zoomScale;
    int centreZ = (mHalfImageWidth * mZoomedCentreZ) / zoomScale;

    int minZ = (centreZ - halfZoomedWidth) + mHalfImageWidth;
    int maxZ = centreZ + halfZoomedWidth + mHalfImageWidth;
    int minX = (centreX - halfZoomedWidth) + mHalfImageWidth;
    int maxX = centreX + halfZoomedWidth + mHalfImageWidth;

    unsigned int* destination = (unsigned int*)zoomedImage.data;

    for (int z = minZ; z < maxZ; z++)
    {
        for (int x = minX; x < maxX; x++)
        {
            *destination = biomeMapPixels[getBiomeIndex(x, z)];

            destination++;
        }
    }

    return zoomedImage;
}

void BiomeSource::LayerOverrideSettings::UpdateBiomeMapPixelsInRegion(unsigned int* biomeMapPixels, unsigned int x,
                                                                      unsigned int z, unsigned int width,
                                                                      unsigned int height, bool& missingBiomes,
                                                                      bool wholeMap)
{
    int scale = 1;

    if (!wholeMap)
    {
        scale = (int)pow(2.0, (double)mZoomLevel);
    }

    int centreX = 0;
    int centreZ = 0;

    if (isZoomed() && !wholeMap)
    {
        int zoomScale = (int)pow(2.0, (double)mZoomLevel + 1.0);

        centreZ = (mHalfImageWidth * mZoomedCentreZ) / zoomScale;
        centreX = (mHalfImageWidth * mZoomedCentreX) / zoomScale;
    }

    int border = mHalfImageWidth - mHalfImageWidth / scale;

    int startZ = (int)(z / scale) + border + centreZ;
    int startX = (int)(x / scale) + border + centreX;
    int endX = startX + (int)(width / scale);
    int endZ = startZ + (int)(height / scale);

    ColourTable* colourTable = Minecraft::GetInstance()->getColourTable();

    unsigned int pixelZ = z;

    for (int biomeZ = startZ; biomeZ < endZ; biomeZ++)
    {
        for (int row = 0; row < scale; row++)
        {
            unsigned int* destination = biomeMapPixels + (mImageWidth * (pixelZ + row) + x);

            for (int biomeX = startX; biomeX < endX; biomeX++)
            {
                unsigned int index = (unsigned int)getBiomeIndex(biomeX, biomeZ);

                if (index < mBiomes.length)
                {
                    unsigned int colour;

                    EnterCriticalSection(&mBiomesCriticalSection);

                    if (mBiomes.data[index] == NULL)
                    {
                        missingBiomes = true;

                        LeaveCriticalSection(&mBiomesCriticalSection);

                        colour = 0;
                    }
                    else
                    {
                        colour = colourTable->getColour(mBiomes.data[index]->getPreviewColor());

                        LeaveCriticalSection(&mBiomesCriticalSection);

                        colour = (colour >> 16 & 0xFF) | (colour >> 8 & 0xFF) << 8 | (colour & 0xFF) << 16;
                    }

                    for (int column = 0; column < scale; column++)
                    {
                        *destination = colour | 0xFF000000;

                        destination++;
                    }
                }
            }
        }

        pixelZ += scale;
    }
}
