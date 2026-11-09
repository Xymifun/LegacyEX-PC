#include "game/world/TheEndLevelSource.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChorusFlowerBlock.hpp"
#include "game/world/block/FallingBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/EndCityFeature.hpp"
#include "game/world/generation/EndIslandFeature.hpp"
#include "game/world/generation/EndPodiumFeature.hpp"
#include "game/world/generation/LargeFeature.hpp"
#include "game/world/TheEndDimension.hpp"

TheEndLevelSource::TheEndLevelSource(Level* level, bool generateStructures, long long seed, const BlockPos& origin)
{
    mLevelSize = 128;
    mEndCityFeature = new EndCityFeature(this);
    mRandom = new Random(seed);
    mPopulationRandom = new Random(seed);
    mNoise1 = new PerlinNoise(mRandom, 16);
    mNoise2 = new PerlinNoise(mRandom, 16);
    mNoise3 = new PerlinNoise(mRandom, 8);
    mNoise4 = new PerlinNoise(mRandom, 10);
    mNoise5 = new PerlinNoise(mRandom, 16);
    mLevel = level;
    mGenerateStructures = generateStructures;
    mOrigin = origin;
    mIslandNoise = new SimplexNoise(mRandom);
    mEndIslandFeature = new EndIslandFeature();
}

TheEndLevelSource::~TheEndLevelSource()
{
    delete mEndCityFeature;
    delete mRandom;
    delete mPopulationRandom;
    delete mNoise1;
    delete mNoise2;
    delete mNoise3;
    delete mNoise4;
    delete mNoise5;
    delete mIslandNoise;
    delete mEndIslandFeature;
    delete[] mBiomes.data;
}

double TheEndLevelSource::fjIslandProximity(int x, int z)
{
    ChunkPos* area = TheEndDimension::closestAreaToPoint(x, z);
    if (area == &TheEndDimension::AREAS[0])
    {
        return 12.0;
    }

    double xDistance = x + 0.5 - area->x;
    double zDistance = z + 0.5 - area->z;
    return std::max(std::fabs(xDistance), std::fabs(zDistance));
}

bool TheEndLevelSource::fjIslandCheck(int x, int z)
{
    return fjIslandProximity(x, z) < 3.0;
}

float TheEndLevelSource::getIslandHeightValue(int x, int z, int xOffset, int zOffset)
{
    ChunkPos* area = TheEndDimension::closestAreaToPoint(x, z);

    if (area == &TheEndDimension::AREAS[0])
    {
        const float sampleZ = z * 2.0f + zOffset;
        const float sampleX = x * 2.0f + xOffset;
        return 100.0f - std::sqrt(sampleX * sampleX + sampleZ * sampleZ) * 9.0f;
    }

    const int sampleZ = z * 2 + zOffset;
    const int sampleX = x * 2 + xOffset;
    float height = 100.0f - std::sqrt(static_cast<float>(sampleX * sampleX + sampleZ * sampleZ)) * 8.0f;
    if (height > 80.0f)
    {
        height = 80.0f;
    }
    else if (height < -100.0f)
    {
        height = -100.0f;
    }

    for (int offsetX = -4; offsetX <= 4; ++offsetX)
    {
        for (int offsetZ = -4; offsetZ <= 4; ++offsetZ)
        {
            const double islandX = x + offsetX;
            const double islandZ = z + offsetZ;
            if (fjIslandCheck(islandX, islandZ))
            {
                const float scale = static_cast<int>(std::fabs(islandX) * 3439.0 + std::fabs(islandZ) * 147.0) % 6 + 18;
                const int distanceX = xOffset - offsetX * 2;
                const int distanceZ = zOffset - offsetZ * 2;
                float islandHeight =
                    100.0f - std::sqrt(static_cast<float>(distanceX * distanceX + distanceZ * distanceZ)) * scale;
                if (islandHeight > 80.0f)
                {
                    islandHeight = 80.0f;
                }
                else if (islandHeight < -100.0f)
                {
                    islandHeight = -100.0f;
                }
                if (islandHeight > height)
                {
                    height = islandHeight;
                }
            }
        }
    }

    return height;
}

arrayWithLength<double> TheEndLevelSource::getHeights(arrayWithLength<double> values, int x, int y, int z, int xSize,
                                                      int ySize, int zSize)
{
    if (values.data == NULL)
    {
        values = arrayWithLength<double>(xSize * ySize * zSize, true);
    }

    arrayWithLength<double> pnr;
    arrayWithLength<double> ar;
    arrayWithLength<double> br;
    arrayWithLength<double> sr;
    arrayWithLength<double> dr;
    arrayWithLength<double> fi;
    arrayWithLength<double> fis;

    sr = mNoise4->getRegion(sr, x, z, xSize, zSize, 1.121, 1.121, 0.5);
    dr = mNoise5->getRegion(dr, x, z, xSize, zSize, 200.0, 200.0, 0.5);
    pnr = mNoise3->getRegion(pnr, x, y, z, xSize, ySize, zSize, 17.110300000000002, 4.277575000000001,
                             17.110300000000002);
    ar = mNoise1->getRegion(ar, x, y, z, xSize, ySize, zSize, 1368.824, 684.412, 1368.824);
    br = mNoise2->getRegion(br, x, y, z, xSize, ySize, zSize, 1368.824, 684.412, 1368.824);

    int index = 0;
    const int middle = ySize / 2;
    for (int localX = 0; localX < xSize; ++localX)
    {
        for (int localZ = 0; localZ < zSize; ++localZ)
        {
            float islandHeight = getIslandHeightValue(x / 2, z / 2, localX, localZ);
            if (islandHeight > 80.0f)
            {
                islandHeight = 80.0f;
            }
            else if (islandHeight < -100.0f)
            {
                islandHeight = -100.0f;
            }

            for (int localY = 0; localY < ySize; ++localY)
            {
                const double selector = (pnr.data[index] / 10.0 + 1.0) / 2.0;
                const double low = ar.data[index] / 512.0;
                const double high = br.data[index] / 512.0;
                double density;
                if (selector < 0.0)
                {
                    density = low;
                }
                else if (selector > 1.0)
                {
                    density = high;
                }
                else
                {
                    density = low + (high - low) * selector;
                }

                density = density - 8.0 + islandHeight;
                if (localY > middle - 2)
                {
                    const double fade = Mth::clamp(
                        static_cast<double>(static_cast<float>(localY + 2 - middle) * 0.015625f), 0.0, 1.0);
                    density = density * (1.0 - fade) + -3000.0 * fade;
                }
                if (localY < 8)
                {
                    const double fade = static_cast<float>(8 - localY) / 7.0f;
                    density = density * (1.0 - fade) + -30.0 * fade;
                }

                const int worldY = (y + localY) * 4;
                const double originFade = Mth::clamp(
                    (19.0 - EndPodiumFeature::END_PODIUM_LOCATION.dist((x + localX) * 8, worldY, (z + localZ) * 8))
                        / 16.0,
                    0.0, 1.0);
                if (EndPodiumFeature::END_PODIUM_LOCATION.y < worldY + 1)
                {
                    values.data[index] = originFade * -50.0 + (1.0 - originFade) * density;
                }
                else
                {
                    values.data[index] = originFade * 50.0 + (1.0 - originFade) * density;
                }
                ++index;
            }
        }
    }

    delete[] pnr.data;
    delete[] ar.data;
    delete[] br.data;
    delete[] sr.data;
    delete[] dr.data;
    delete[] fi.data;
    delete[] fis.data;
    return values;
}

void TheEndLevelSource::prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    arrayWithLength<double> heights = getHeights(arrayWithLength<double>(), chunkX * 2, 0, chunkZ * 2, 3, 33, 3);
    for (int cellX = 0; cellX < 2; ++cellX)
    {
        for (int cellZ = 0; cellZ < 2; ++cellZ)
        {
            for (int cellY = 0; cellY < 32; ++cellY)
            {
                double lowerNear = heights.data[(cellX * 3 + cellZ) * 33 + cellY];
                double lowerFar = heights.data[(cellX * 3 + cellZ + 1) * 33 + cellY];
                double upperNear = heights.data[((cellX + 1) * 3 + cellZ) * 33 + cellY];
                double upperFar = heights.data[((cellX + 1) * 3 + cellZ + 1) * 33 + cellY];
                const double lowerNearStep = (heights.data[(cellX * 3 + cellZ) * 33 + cellY + 1] - lowerNear) * 0.25;
                const double lowerFarStep = (heights.data[(cellX * 3 + cellZ + 1) * 33 + cellY + 1] - lowerFar) * 0.25;
                const double upperNearStep = (heights.data[((cellX + 1) * 3 + cellZ) * 33 + cellY + 1] - upperNear)
                                             * 0.25;
                const double upperFarStep = (heights.data[((cellX + 1) * 3 + cellZ + 1) * 33 + cellY + 1] - upperFar)
                                            * 0.25;

                for (int subY = 0; subY < 4; ++subY)
                {
                    double near = lowerNear;
                    double far = lowerFar;
                    const double nearStep = (upperNear - lowerNear) * 0.125;
                    const double farStep = (upperFar - lowerFar) * 0.125;

                    for (int subX = 0; subX < 8; ++subX)
                    {
                        double density = near;
                        const double densityStep = (far - near) * 0.125;
                        for (int subZ = 0; subZ < 8; ++subZ)
                        {
                            const BlockState* state = Blocks::air->defaultBlockState();
                            if (density > 0.0)
                            {
                                state = Blocks::end_stone->defaultBlockState();
                            }
                            primer->setState(cellX * 8 + subX, cellY * 4 + subY, cellZ * 8 + subZ, state);
                            density += densityStep;
                        }
                        near += nearStep;
                        far += farStep;
                    }

                    lowerNear += lowerNearStep;
                    lowerFar += lowerFarStep;
                    upperNear += upperNearStep;
                    upperFar += upperFarStep;
                }
            }
        }
    }

    delete[] heights.data;
}

void TheEndLevelSource::buildSurfaces(ChunkPrimer* primer)
{
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            int depth = -1;
            const BlockState* top = Blocks::end_stone->defaultBlockState();
            const BlockState* filler = Blocks::end_stone->defaultBlockState();
            for (int y = 127; y >= 0; --y)
            {
                const BlockState* state = primer->getState(x, y, z);
                if (state->getBlock()->mMaterial == Material::AIR)
                {
                    depth = -1;
                }
                else if (state->getBlock() == Blocks::stone)
                {
                    if (depth == -1)
                    {
                        depth = 1;
                        primer->setState(x, y, z, top);
                    }
                    else if (depth > 0)
                    {
                        --depth;
                        primer->setState(x, y, z, filler);
                    }
                }
            }
        }
    }
}

LevelChunk* TheEndLevelSource::createChunk(int chunkX, int chunkZ)
{
    mRandom->setSeed(static_cast<long long>(chunkX) * 341873128712LL + static_cast<long long>(chunkZ) * 132897987541LL);
    arrayWithLength<unsigned short> elements(65536, true);
    ChunkPrimer primer(true, elements);
    mLevel->getBiomeSource()->getBiomeBlock(mBiomes, chunkX << 4, chunkZ << 4, 16, 16, true);
    prepareHeights(chunkX, chunkZ, &primer);
    buildSurfaces(&primer);
    if (mGenerateStructures)
    {
        mEndCityFeature->apply(mLevel, chunkX, chunkZ, &primer);
    }

    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    arrayWithLength<unsigned char> biomeIds = chunk->getBiomes();
    for (unsigned int index = 0; index < biomeIds.length; ++index)
    {
        biomeIds.data[index] = static_cast<unsigned char>(Biome::getId(mBiomes.data[index]));
    }
    chunk->recalcHeightmap();
    return chunk;
}

void TheEndLevelSource::postProcess(int chunkX, int chunkZ)
{
    FallingBlock::fallInstantly = true;
    const BlockPos& origin = BlockPos(chunkX << 4, 0, chunkZ << 4);
    mPopulationRandom->setSeed(mLevel->getSeed());
    const long long xSeed = mPopulationRandom->nextLong() / 2LL * 2LL + 1LL;
    const long long zSeed = mPopulationRandom->nextLong() / 2LL * 2LL + 1LL;
    mPopulationRandom->setSeed((static_cast<long long>(chunkX) * xSeed + static_cast<long long>(chunkZ) * zSeed)
                               ^ mLevel->getSeed());

    if (mGenerateStructures)
    {
        ChunkPos chunkPos(chunkX, chunkZ);
        mEndCityFeature->postProcess(mLevel, *mPopulationRandom, &chunkPos);
    }
    mLevel->getBiome(origin.offset(16, 0, 16))->decorate(mLevel, *mPopulationRandom, origin);

    if (fjIslandCheck(chunkX, chunkZ))
    {
        if (getIslandHeightValue(chunkX, chunkZ, 1, 1) < -20.0f && mPopulationRandom->nextInt(14) == 0)
        {
            mEndIslandFeature->place(mLevel, *mPopulationRandom,
                                     origin.offset(mPopulationRandom->nextInt(16) + 8,
                                                   mPopulationRandom->nextInt(16) + 55,
                                                   mPopulationRandom->nextInt(16) + 8));
            if (mPopulationRandom->nextInt(4) == 0)
            {
                mEndIslandFeature->place(mLevel, *mPopulationRandom,
                                         origin.offset(mPopulationRandom->nextInt(16) + 8,
                                                       mPopulationRandom->nextInt(16) + 55,
                                                       mPopulationRandom->nextInt(16) + 8));
            }
        }

        if (getIslandHeightValue(chunkX, chunkZ, 1, 1) > 40.0f)
        {
            const int plants = mPopulationRandom->nextInt(5);
            for (int index = 0; index < plants; ++index)
            {
                const int x = mPopulationRandom->nextInt(16) + 8;
                const int z = mPopulationRandom->nextInt(16) + 8;
                const BlockPos column = origin.offset(x, 0, z);
                const BlockPos top = mLevel->getHeightmapPos(column);
                if (top.y > 0)
                {
                    const int y = top.y - 1;
                    if (mLevel->isEmptyBlock(origin.offset(x, y + 1, z))
                        && mLevel->getBlockState(origin.offset(x, y, z))->getBlock() == Blocks::end_stone)
                    {
                        ChorusFlowerBlock::generatePlant(mLevel, origin.offset(x, y + 1, z), mPopulationRandom, 8);
                    }
                }
            }
        }
    }
    FallingBlock::fallInstantly = false;
}

bool TheEndLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
    return false;
}

std::vector<Biome::MobSpawnerData*>* TheEndLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    return mLevel->getBiome(pos)->getMobs(category);
}

BlockPos* TheEndLevelSource::findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                                   bool findUnexplored)
{
    if (LargeFeature::END_CITY == feature && mEndCityFeature != NULL)
    {
        return mEndCityFeature->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    return NULL;
}

void TheEndLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int, int)
{
}

bool TheEndLevelSource::isPosInFeature(Level*, const std::wstring& feature, const BlockPos& pos)
{
    if (LargeFeature::END_CITY == feature && mEndCityFeature != NULL)
    {
        return mEndCityFeature->isInsideFeature(pos);
    }
    return false;
}
