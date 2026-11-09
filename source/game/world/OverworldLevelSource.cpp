#include "game/world/OverworldLevelSource.hpp"
#include "game/core/PIXNamedEvents.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/CustomizableSourceSettings.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PerlinNoise.hpp"
#include "game/core/PerlinSimplexNoise.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/entity/mob/NaturalMobSpawner.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/FallingBlock.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/BuriedTreasureFeature.hpp"
#include "game/world/generation/CanyonFeature.hpp"
#include "game/world/generation/LakeFeature.hpp"
#include "game/world/generation/LargeCaveFeature.hpp"
#include "game/world/generation/LargeFeature.hpp"
#include "game/world/generation/MineShaftFeature.hpp"
#include "game/world/generation/MonsterRoomFeature.hpp"
#include "game/world/generation/OceanMonumentFeature.hpp"
#include "game/world/generation/OceanRuinFeature.hpp"
#include "game/world/generation/RandomScatteredLargeFeature.hpp"
#include "game/world/generation/ShipwreckFeature.hpp"
#include "game/world/generation/StrongholdFeature.hpp"
#include "game/world/generation/UnderwaterCanyonFeature.hpp"
#include "game/world/generation/UnderwaterLargeCaveFeature.hpp"
#include "game/world/generation/VillageFeature.hpp"
#include "game/world/generation/WoodlandMansionFeature.hpp"
#include "game/core/XMem.hpp"


OverworldLevelSource::OverworldLevelSource(Level* level, long long seed, bool generateStructures,
                                           SuperflatConfig* config)
    : mRandom(),
      mPopulationRandom()
{
    mLevelSize = level->getLevelData()->getXZSize();
    mLevel = level;
    mGenerateStructures = generateStructures;
    mGeneratorType = level->getLevelData()->getGeneratorType();
    mCaves = new LargeCaveFeature();
    mStronghold = new StrongholdFeature();
    mVillage = new VillageFeature(level->getLevelData()->getXZSize());
    mMineshaft = new MineShaftFeature();
    mTemple = new RandomScatteredLargeFeature();
    mRavines = new CanyonFeature();
    mUnderwaterCaves = new UnderwaterLargeCaveFeature();
    mUnderwaterRavines = new UnderwaterCanyonFeature();
    mMonument = new OceanMonumentFeature();
    mMansion = new WoodlandMansionFeature(this);
    mBuriedTreasure = new BuriedTreasureFeature(level->getLevelData()->getXZSize());
    mShipwreck = new ShipwreckFeature(this, mMonument);
    mOceanRuin = new OceanRuinFeature();
    mRandom = Random(seed);
    mPopulationRandom = Random(seed);
    mMinLimitNoise = new PerlinNoise(&mRandom, 16);
    mMaxLimitNoise = new PerlinNoise(&mRandom, 16);
    mMainNoise = new PerlinNoise(&mRandom, 8);
    mSurfaceNoise = new PerlinSimplexNoise(&mRandom, 4);
    mScaleNoise = new PerlinNoise(&mRandom, 10);
    mDepthNoise = new PerlinNoise(&mRandom, 16);
    mUnusedNoiseA = NULL;
    mUnusedNoiseB = NULL;
    mForestNoise = new PerlinNoise(&mRandom, 8);
    for (int x = -2; x <= 2; ++x)
    {
        for (int z = -2; z <= 2; ++z)
        {
            float weight = 10.0f / std::sqrt(static_cast<float>(x * x + z * z) + 0.2f);
            mBiomeWeights[x + 2 + (z + 2) * 5] = weight;
        }
    }

    mOceanBlock = Blocks::water->defaultBlockState();

    CustomizableSourceSettings::Builder* builder;
    if (config != NULL)
    {
        builder = CustomizableSourceSettings::Builder::fromString(config);
        mSettings = builder->build();
        if (mSettings->useLavaOceans)
        {
            mOceanBlock = Blocks::lava->defaultBlockState();
        }
        else
        {
            mOceanBlock = Blocks::water->defaultBlockState();
        }
        level->setSeaLevel(mSettings->seaLevel);
    }
    else
    {
        builder = new CustomizableSourceSettings::Builder();
        mSettings = new CustomizableSourceSettings(builder);
    }
    delete builder;
}

OverworldLevelSource::~OverworldLevelSource()
{
    delete mCaves;
    delete mStronghold;
    delete mVillage;
    delete mMineshaft;
    delete mTemple;
    delete mRavines;
    delete mUnderwaterRavines;
    delete mMonument;
    delete mMansion;
    delete mBuriedTreasure;
    delete mShipwreck;
    delete mOceanRuin;
    delete mMinLimitNoise;
    delete mMaxLimitNoise;
    delete mMainNoise;
    delete mSurfaceNoise;
    delete mScaleNoise;
    delete mDepthNoise;
    delete mUnusedNoiseA;
    delete mUnusedNoiseB;
    delete mForestNoise;
    delete mSettings;
    delete[] mSurfaceDepths.data;
}

float OverworldLevelSource::getHeightFalloff(int x, int z, int* distance)
{
    int radius = (mLevelSize << 4) / 2;
    int west = x + radius;
    if (west < 0)
        west = 0;
    int east = radius - x - 1;
    if (east < 0)
        east = 0;
    int north = z + radius;
    if (north < 0)
        north = 0;
    int south = radius - z - 1;
    if (south < 0)
        south = 0;
    int edgeDistance = west;
    if (east < edgeDistance)
        edgeDistance = east;
    if (north < edgeDistance)
        edgeDistance = north;
    if (south < edgeDistance)
        edgeDistance = south;
    float falloff = 0.0f;
    if (edgeDistance < 32)
    {
        float edgeRatio = (32 - edgeDistance) / 32.0f;
        falloff = edgeRatio * 128.0f;
    }
    *distance = edgeDistance;
    return falloff;
}

void OverworldLevelSource::getHeights(int x, int y, int z, arrayWithLength<Biome*>& biomes,
                                      arrayWithLength<double>& heights)
{
    arrayWithLength<double> pnr;
    arrayWithLength<double> ar;
    arrayWithLength<double> br;
    arrayWithLength<double> dr;
    arrayWithLength<double> fi;
    arrayWithLength<double> fis;

    float coordinateScale = mSettings->coordinateScale;
    float heightScale = mSettings->heightScale;
    dr = mDepthNoise->getRegion(dr, x, z, 5, 5, mSettings->depthNoiseScaleX, mSettings->depthNoiseScaleZ,
                                mSettings->depthNoiseScaleExponent);
    pnr = mMainNoise->getRegion(pnr, x, y, z, 5, 17, 5, coordinateScale / mSettings->mainNoiseScaleX,
                                heightScale / mSettings->mainNoiseScaleY, coordinateScale / mSettings->mainNoiseScaleZ);
    ar = mMinLimitNoise->getRegion(ar, x, y, z, 5, 17, 5, coordinateScale, heightScale, coordinateScale);
    br = mMaxLimitNoise->getRegion(br, x, y, z, 5, 17, 5, coordinateScale, heightScale, coordinateScale);

    int noiseIndex = 0;
    int depthIndex = 0;
    for (int sampleX = 0; sampleX < 5; ++sampleX)
    {
        for (int sampleZ = 0; sampleZ < 5; ++sampleZ)
        {
            float biomeScale = 0.0f;
            float biomeDepth = 0.0f;
            float weightSum = 0.0f;
            Biome* center = biomes.data[sampleX + 2 + (sampleZ + 2) * 10];

            for (int offsetX = -2; offsetX <= 2; ++offsetX)
            {
                for (int offsetZ = -2; offsetZ <= 2; ++offsetZ)
                {
                    Biome* biome = biomes.data[sampleX + offsetX + 2 + (sampleZ + offsetZ + 2) * 10];
                    float depth = biome->getDepth() * mSettings->biomeDepthWeight + mSettings->biomeDepthOffset;
                    float scale = biome->getScale() * mSettings->biomeScaleWeight + mSettings->biomeScaleOffset;
                    if (mGeneratorType == LevelType::AMPLIFIED && depth > 0.0f)
                    {
                        depth = depth * 2.0f + 1.0f;
                        scale = scale * 4.0f + 1.0f;
                    }
                    float weight = mBiomeWeights[offsetX + 2 + (offsetZ + 2) * 5] / (depth + 2.0f);
                    if (biome->getDepth() > center->getDepth())
                    {
                        weight *= 0.5f;
                    }
                    biomeScale += scale * weight;
                    biomeDepth += depth * weight;
                    weightSum += weight;
                }
            }

            biomeScale /= weightSum;
            biomeDepth /= weightSum;
            biomeScale = biomeScale * 0.9f + 0.1f;
            biomeDepth = (biomeDepth * 4.0f - 1.0f) * 0.125f;

            double depthValue = dr.data[depthIndex++] / 8000.0;
            if (depthValue < 0.0)
            {
                depthValue = -depthValue * 0.3;
            }
            depthValue = depthValue * 3.0 - 2.0;
            if (depthValue < 0.0)
            {
                depthValue /= 2.0;
                if (depthValue < -1.0)
                {
                    depthValue = -1.0;
                }
                depthValue /= 1.4;
                depthValue /= 2.0;
            }
            else
            {
                if (depthValue > 1.0)
                {
                    depthValue = 1.0;
                }
                depthValue /= 8.0;
            }

            double centerHeight = (depthValue * 0.2 + biomeDepth) * mSettings->baseSize * 0.125 * 4.0
                                  + mSettings->baseSize;

            for (int sampleY = 0; sampleY < 17; ++sampleY)
            {
                double vertical = (sampleY - centerHeight) * mSettings->stretchY * 128.0 / 256.0 / biomeScale;
                if (vertical < 0.0)
                {
                    vertical *= 4.0;
                }
                double low = ar.data[noiseIndex] / mSettings->upperLimitScale;
                double high = br.data[noiseIndex] / mSettings->lowerLimitScale;
                double blend = (pnr.data[noiseIndex] / 10.0 + 1.0) / 2.0;
                double density = Mth::clampedLerp(low, high, blend) - vertical;
                if (sampleY > 13)
                {
                    double topBlend = (float)(sampleY - 13) / 3.0f;
                    density = density * (1.0 - topBlend) + -10.0 * topBlend;
                }
                heights.data[noiseIndex++] = density;
            }
        }
    }

    delete[] pnr.data;
    delete[] ar.data;
    delete[] br.data;
    delete[] dr.data;
    delete[] fi.data;
    delete[] fis.data;
}

void OverworldLevelSource::prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    int seaLevel = mLevel->mSeaLevel;
    arrayWithLength<Biome*> biomes;
    PIXBeginNamedEvent(0.0f, "Getting raw biome block");
    mLevel->getBiomeSource()->getRawBiomeBlock(biomes, chunkX * 4 - 2, chunkZ * 4 - 2, 10, 10);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Get heights");
    arrayWithLength<double> heights(425, true);
    getHeights(chunkX * 4, 0, chunkZ * 4, biomes, heights);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Generating");
    for (int cellX = 0; cellX < 4; ++cellX)
    {
        int offset0 = cellX * 5;
        int offset1 = (cellX + 1) * 5;
        for (int cellZ = 0; cellZ < 4; ++cellZ)
        {
            int offset00 = (offset0 + cellZ) * 17;
            int offset01 = (offset0 + cellZ + 1) * 17;
            int offset10 = (offset1 + cellZ) * 17;
            int offset11 = (offset1 + cellZ + 1) * 17;
            for (int cellY = 0; cellY < 16; ++cellY)
            {
                double a = heights[offset00 + cellY];
                double b = heights[offset01 + cellY];
                double c = heights[offset10 + cellY];
                double d = heights[offset11 + cellY];
                double da = heights[offset00 + cellY + 1] - a;
                double db = heights[offset01 + cellY + 1] - b;
                double dc = heights[offset10 + cellY + 1] - c;
                double dd = heights[offset11 + cellY + 1] - d;

                for (int subY = 0; subY < 8; ++subY)
                {
                    double left = a;
                    double right = b;
                    double leftStep = (c - a) * 0.25;
                    double rightStep = (d - b) * 0.25;
                    int blockY = cellY * 8 + subY;

                    for (int subX = 0; subX < 4; ++subX)
                    {
                        double densityStep = (right - left) * 0.25;
                        double density = left - densityStep;
                        int index = ((subX + (cellX << 2)) << 11) | ((cellZ << 2) << 7) | blockY;
                        int step = 1 << 7;
                        index -= step;
                        for (int subZ = 0; subZ < 4; ++subZ)
                        {
                            int blockZ = cellZ * 4 + subZ;
                            int edgeDistance;
                            double falloff = getHeightFalloff(chunkX * 16 + (subX + (cellX << 2)), chunkZ * 16 + blockZ,
                                                              &edgeDistance);
                            int blockId = 0;
                            density += densityStep;
                            if (density > falloff)
                            {
                                blockId = 1;
                            }
                            else if (blockY < seaLevel)
                            {
                                blockId = 9;
                            }
                            if (edgeDistance == 0)
                            {
                                if (blockY <= mLevel->getSeaLevel() - 10)
                                {
                                    blockId = 1;
                                }
                                else if (blockY < mLevel->getSeaLevel())
                                {
                                    blockId = 9;
                                }
                            }
                            index += step;
                            primer->setBlockAndData(index, blockId, 0);
                        }
                        left += leftStep;
                        right += rightStep;
                    }
                    a += da * 0.125;
                    b += db * 0.125;
                    c += dc * 0.125;
                    d += dd * 0.125;
                }
            }
        }
    }
    PIXEndNamedEvent();
    delete[] heights.data;
    delete[] biomes.data;
}

void OverworldLevelSource::buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer, arrayWithLength<Biome*> biomes)
{
    arrayWithLength<double> surfaceDepths(256, true);
    PIXBeginNamedEvent(0.0f, "Getting noise region");
    const double noiseScale = 0.0625;
    surfaceDepths = mSurfaceNoise->getRegion(surfaceDepths, chunkX * 16, chunkZ * 16, 16, 16, noiseScale, noiseScale,
                                             1.0);
    PIXEndNamedEvent();
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            Biome* biome = biomes.data[z + x * 16];
            PIXBeginNamedEvent(0.0f, "Building surface for biome %ls", biome->getName(false).c_str());
            biome->buildSurfaceAt(mLevel, mRandom, primer, chunkX * 16 + x, chunkZ * 16 + z,
                                  surfaceDepths.data[z + x * 16]);
            PIXEndNamedEvent();
        }
    }
    delete[] surfaceDepths.data;
}

LevelChunk* OverworldLevelSource::createChunk(int chunkX, int chunkZ)
{
    mRandom.setSeed(static_cast<long long>(chunkX) * 341873128712LL + static_cast<long long>(chunkZ) * 132897987541LL);
    PIXBeginNamedEvent(0.0f, "Setting up primer");
    unsigned short* elements = static_cast<unsigned short*>(XPhysicalAlloc(0x10000, 0xFFFFFFFF, 0x1000, 4));
    XMemSet128(elements, 0, 0x10000);
    arrayWithLength<unsigned short> elementData;
    elementData.data = elements;
    elementData.length = 0x8000;
    ChunkPrimer primer(false, elementData);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Preparing heights");
    prepareHeights(chunkX, chunkZ, &primer);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Getting biome block");
    arrayWithLength<Biome*> biomes;
    mLevel->getBiomeSource()->getBiomeBlock(biomes, chunkX << 4, chunkZ << 4, 16, 16, true);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Building surfaces");
    buildSurfaces(chunkX, chunkZ, &primer, biomes);
    PIXEndNamedEvent();
    delete[] biomes.data;

    if (mSettings->useCaves)
    {
        PIXBeginNamedEvent(0.0f, "Applying underwater caves");
        mUnderwaterCaves->apply(mLevel, chunkX, chunkZ, &primer);
        PIXEndNamedEvent();
    }
    if (mSettings->useRavines)
    {
        PIXBeginNamedEvent(0.0f, "Applying underwater ravines");
        mUnderwaterRavines->apply(mLevel, chunkX, chunkZ, &primer);
        PIXEndNamedEvent();
    }
    if (mSettings->useCaves)
    {
        PIXBeginNamedEvent(0.0f, "Applying caves");
        mCaves->apply(mLevel, chunkX, chunkZ, &primer);
        PIXEndNamedEvent();
    }
    if (mSettings->useRavines)
    {
        PIXBeginNamedEvent(0.0f, "Applying ravines");
        mRavines->apply(mLevel, chunkX, chunkZ, &primer);
        PIXEndNamedEvent();
    }
    if (mGenerateStructures)
    {
        if (mSettings->useMineShafts)
        {
            PIXBeginNamedEvent(0.0f, "Applying mineshafts");
            mMineshaft->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useVillages)
        {
            PIXBeginNamedEvent(0.0f, "Applying villages");
            mVillage->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useStrongholds)
        {
            PIXBeginNamedEvent(0.0f, "Applying strongholds");
            mStronghold->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useTemples)
        {
            PIXBeginNamedEvent(0.0f, "Applying scatterd features");
            mTemple->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useOceanRuins)
        {
            PIXBeginNamedEvent(0.0f, "Applying ruin feature");
            mOceanRuin->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useMonuments)
        {
            PIXBeginNamedEvent(0.0f, "Applying ocean monuments");
            mMonument->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useMansions)
        {
            PIXBeginNamedEvent(0.0f, "Applying woodland mansions");
            mMansion->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useShipwrecks)
        {
            PIXBeginNamedEvent(0.0f, "Applying shipwreck feature");
            mShipwreck->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useBuriedTreasure)
        {
            PIXBeginNamedEvent(0.0f, "Applying buried treasure");
            mBuriedTreasure->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
    }
    PIXBeginNamedEvent(0.0f, "Creating chunk");
    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    PIXEndNamedEvent();
    XPhysicalFree(elements);
    return chunk;
}

void OverworldLevelSource::lightChunk(LevelChunk* chunk)
{
    chunk->recalcHeightmap();
}

bool OverworldLevelSource::isWater(const BlockPos& pos)
{
    return mLevel->getBlockState(pos)->getBlock() == mOceanBlock->getBlock()
           && mLevel->getBlockState(pos)->getValue(LiquidBlock::LEVEL) < 7;
}

void OverworldLevelSource::calcWaterDepths(int chunkX, int chunkZ)
{
    BlockPos chunkOrigin(chunkX << 4, 0, chunkZ << 4);
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            BlockPos center = chunkOrigin.offset(x + 7, mLevel->getSeaLevel(), z + 7);
            if (mLevel->getHeightmapPos(center).y <= 0)
            {
                BlockPos west = center.west();
                BlockPos east = center.east();
                BlockPos north = center.north();
                BlockPos south = center.south();
                if ((mLevel->getHeightmapPos(west).y > 0 || mLevel->getHeightmapPos(east).y > 0
                     || mLevel->getHeightmapPos(north).y > 0 || mLevel->getHeightmapPos(south).y > 0)
                    && (isWater(west) || isWater(east) || isWater(north) || isWater(south)))
                {
                    for (int dx = -5; dx <= 5; ++dx)
                    {
                        for (int dz = -5; dz <= 5; ++dz)
                        {
                            int distance = std::abs(dx) + std::abs(dz);
                            if (distance > 5)
                                continue;

                            int depth = 6 - distance;
                            if (depth > 7)
                                depth = 7;
                            BlockPos pos = center.offset(dx, 0, dz);
                            const BlockState* state = mLevel->getBlockState(pos);
                            if (state->getBlock() == mOceanBlock->getBlock()
                                && state->getValue(LiquidBlock::LEVEL) < depth)
                            {
                                mLevel->setBlock(pos, state->setValue(LiquidBlock::LEVEL, depth), Block::UPDATE_ALL,
                                                 false);
                            }
                        }
                    }

                    mLevel->setBlock(center, mOceanBlock->setValue(LiquidBlock::LEVEL, 7), Block::UPDATE_CLIENTS,
                                     false);
                    for (BlockPos pos = center.below(); pos.y >= 0; pos = pos.below())
                    {
                        mLevel->setBlock(pos, mOceanBlock->setValue(LiquidBlock::LEVEL, 8), Block::UPDATE_CLIENTS,
                                         false);
                    }
                }
            }
        }
    }
}

void OverworldLevelSource::postProcess(int chunkX, int chunkZ)
{
    FallingBlock::fallInstantly = true;
    BlockPos origin(chunkX * 16, 0, chunkZ * 16);
    Biome* biome = mLevel->getBiome(origin.offset(16, 0, 16));
    mPopulationRandom.setSeed(mLevel->getSeed());
    long long oddX = mPopulationRandom.nextLong() / 2LL * 2LL + 1LL;
    long long oddZ = mPopulationRandom.nextLong() / 2LL * 2LL + 1LL;
    mPopulationRandom.setSeed((static_cast<long long>(chunkX) * oddX + static_cast<long long>(chunkZ) * oddZ)
                              ^ mLevel->getSeed());
    ChunkPos chunkPos(chunkX, chunkZ);
    bool villageGenerated = false;
    PIXBeginNamedEvent(0.0f, "Structure postprocessing");
    if (mGenerateStructures)
    {
        if (mSettings->useMineShafts)
            mMineshaft->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useVillages)
            villageGenerated = mVillage->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useStrongholds)
            mStronghold->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useTemples)
            mTemple->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useMonuments)
            mMonument->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useOceanRuins)
            mOceanRuin->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useMansions)
            mMansion->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useBuriedTreasure)
            mBuriedTreasure->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useShipwrecks)
            mShipwreck->postProcess(mLevel, mPopulationRandom, &chunkPos);
    }
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Lakes");
    if (biome != Biome::DESERT && biome != Biome::DESERT_HILLS && mSettings->useWaterLakes && !villageGenerated
        && mPopulationRandom.nextInt(mSettings->waterLakeChance) == 0)
    {
        const int x = mPopulationRandom.nextInt(16) + 8;
        const int y = mPopulationRandom.nextInt(128);
        const int z = mPopulationRandom.nextInt(16) + 8;
        LakeFeature(Blocks::water).place(mLevel, mPopulationRandom, origin.offset(x, y, z));
    }
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Lava");
    if (!villageGenerated && mPopulationRandom.nextInt(mSettings->lavaLakeChance / 10) == 0 && mSettings->useLavaLakes)
    {
        const int x = mPopulationRandom.nextInt(16) + 8;
        const int y = mPopulationRandom.nextInt(mPopulationRandom.nextInt(120) + 8);
        const int z = mPopulationRandom.nextInt(16) + 8;
        BlockPos pos = origin.offset(x, y, z);
        if (pos.y < mLevel->getSeaLevel() || mPopulationRandom.nextInt(mSettings->lavaLakeChance / 8) == 0)
        {
            LakeFeature(Blocks::lava).place(mLevel, mPopulationRandom, pos);
        }
    }
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Monster rooms");
    if (mSettings->useDungeons)
    {
        MonsterRoomFeature monsterRoom;
        for (int i = 0; i < mSettings->dungeonChance; ++i)
        {
            const int x = mPopulationRandom.nextInt(16) + 8;
            const int y = mPopulationRandom.nextInt(128);
            const int z = mPopulationRandom.nextInt(16) + 8;
            monsterRoom.place(mLevel, mPopulationRandom, origin.offset(x, y, z));
        }
    }
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Biome decorate");
    biome->decorate(mLevel, mPopulationRandom, BlockPos(chunkX * 16, 0, chunkZ * 16));
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Post process mobs");
    NaturalMobSpawner::spawnMobsForChunkGeneration(mLevel, biome, chunkX * 16 + 8, chunkZ * 16 + 8, 16, 16,
                                                   &mPopulationRandom);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Update ice and snow");
    origin = origin.offset(8, 0, 8);
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            BlockPos top = mLevel->getTopRainBlockPos(origin.offset(x, 0, z));
            BlockPos below = top.below();
            if (mLevel->shouldFreezeIgnoreNeighbors(below))
            {
                mLevel->setBlock(below, Blocks::ice->defaultBlockState(), Block::UPDATE_CLIENTS, false);
            }
            mLevel->tryToPlaceTopSnow(top, true, true);
        }
    }
    PIXEndNamedEvent();
    FallingBlock::fallInstantly = false;
}

bool OverworldLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
    return false;
}

std::vector<Biome::MobSpawnerData*>* OverworldLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    Biome* biome = mLevel->getBiome(pos);
    if (mGenerateStructures)
    {
        if ((category == MobCategory::MONSTER || category == MobCategory::SLIME) && mTemple->isSwamphut(pos))
        {
            return mTemple->getSwamphutEnemies();
        }
        if (category == MobCategory::GUARDIAN && mSettings->useMonuments
            && mMonument->isInsideBoundingFeature(mLevel, pos))
        {
            return mMonument->getEnemies();
        }
    }
    return biome->getMobs(category);
}

bool OverworldLevelSource::isPosInFeature(Level*, const std::wstring& feature, const BlockPos& pos)
{
    if (!mGenerateStructures)
        return false;
    if (LargeFeature::STRONGHOLD == feature && mStronghold != NULL)
        return mStronghold->isInsideFeature(pos);
    if (LargeFeature::SHIPWRECK == feature && mShipwreck != NULL)
        return mShipwreck->isInsideFeature(pos);
    if (LargeFeature::MANSION == feature && mMansion != NULL)
        return mMansion->isInsideFeature(pos);
    if (LargeFeature::MONUMENT == feature && mMonument != NULL)
        return mMonument->isInsideFeature(pos);
    if (LargeFeature::VILLAGE == feature && mVillage != NULL)
        return mVillage->isInsideFeature(pos);
    if (LargeFeature::MINESHAFT == feature && mMineshaft != NULL)
        return mMineshaft->isInsideFeature(pos);
    if (LargeFeature::TEMPLE == feature && mTemple != NULL)
        return mTemple->isInsideFeature(pos);
    if (LargeFeature::RUIN.compare(feature) == 0 && mOceanRuin != NULL)
        return mOceanRuin->isInsideFeature(pos);
    return false;
}

BlockPos* OverworldLevelSource::findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                                      bool takeBlock)
{
    if (!mGenerateStructures)
        return NULL;
    if (LargeFeature::STRONGHOLD == feature && mStronghold != NULL)
        return mStronghold->getNearestGeneratedFeature(level, pos, takeBlock);
    if (LargeFeature::SHIPWRECK == feature && mShipwreck != NULL)
        return mShipwreck->getNearestGeneratedFeature(level, pos, takeBlock);
    if (LargeFeature::MANSION == feature && mMansion != NULL)
        return mMansion->getNearestGeneratedFeature(level, pos, takeBlock);
    if (LargeFeature::MONUMENT == feature && mMonument != NULL)
        return mMonument->getNearestGeneratedFeature(level, pos, takeBlock);
    if (LargeFeature::VILLAGE == feature && mVillage != NULL)
        return mVillage->getNearestGeneratedFeature(level, pos, takeBlock);
    if (LargeFeature::MINESHAFT == feature && mMineshaft != NULL)
        return mMineshaft->getNearestGeneratedFeature(level, pos, takeBlock);
    if (LargeFeature::TEMPLE == feature && mTemple != NULL)
        return mTemple->getNearestGeneratedFeature(level, pos, takeBlock);
    if (LargeFeature::BURIED_TREASURE == feature && mBuriedTreasure != NULL)
        return mBuriedTreasure->getNearestGeneratedFeature(level, pos, takeBlock);
    if (LargeFeature::RUIN == feature && mOceanRuin != NULL)
        return mOceanRuin->getNearestGeneratedFeature(level, pos, takeBlock);
    return NULL;
}

void OverworldLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int chunkX, int chunkZ)
{
    if (!mGenerateStructures)
        return;
    if (mSettings->useMineShafts)
        mMineshaft->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useVillages)
        mVillage->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useStrongholds)
        mStronghold->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useTemples)
        mTemple->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useMonuments)
        mMonument->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useMansions)
        mMansion->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useShipwrecks)
        mShipwreck->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useBuriedTreasure)
        mBuriedTreasure->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useOceanRuins)
        mOceanRuin->apply(mLevel, chunkX, chunkZ, NULL);
}
