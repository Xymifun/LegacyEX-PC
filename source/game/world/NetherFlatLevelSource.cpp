#include "game/world/NetherFlatLevelSource.hpp"

#include <cmath>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FallingBlock.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/HellFireFeature.hpp"
#include "game/world/generation/LightGemFeature.hpp"
#include "game/core/XMem.hpp"


NetherFlatLevelSource::NetherFlatLevelSource(Level* level, bool generateStructures, long long seed)
{
    int xzSize = level->getLevelData()->getXZSize();
    int hellScale = level->getLevelData()->getHellScale();
    mLevelSize = static_cast<int>(std::ceil(static_cast<float>(xzSize) / static_cast<float>(hellScale)));
    mLevel = level;
    mGenerateStructures = generateStructures;
    mRandom = new Random(seed);
    mPopulationRandom = new Random(seed);
}

NetherFlatLevelSource::~NetherFlatLevelSource()
{
    delete mRandom;
    delete mPopulationRandom;
}

void NetherFlatLevelSource::prepareHeights(int, int, ChunkPrimer* primer)
{
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            for (int y = 0; y < 16; ++y)
            {
                const BlockState* state = Blocks::air->defaultBlockState();
                if (y < 7 || y > 120)
                {
                    state = Blocks::netherrack->defaultBlockState();
                }
                primer->setState((x << 11) | (z << 7) | y, state);
            }
        }
    }
}

void NetherFlatLevelSource::buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    for (int z = 0; z < 16; ++z)
    {
        for (int x = 0; x < 16; ++x)
        {
            for (int y = 127; y >= 0; --y)
            {
                int blockIndex = (x * 16 + z) * 128 + y;
                bool placed = false;

                if (chunkX <= -(mLevelSize / 2))
                {
                    if (x - mRandom->nextInt(4) <= 0 || chunkX < -(mLevelSize / 2))
                    {
                        primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                        placed = true;
                    }
                }

                if (chunkZ <= -(mLevelSize / 2))
                {
                    if (z - mRandom->nextInt(4) <= 0 || chunkZ < -(mLevelSize / 2))
                    {
                        primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                        placed = true;
                    }
                }

                if (chunkX >= mLevelSize / 2 - 1)
                {
                    if (x + mRandom->nextInt(4) >= 15 || chunkX > mLevelSize / 2)
                    {
                        primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                        placed = true;
                    }
                }

                if (chunkZ < mLevelSize / 2 - 1)
                {
                    if (!placed)
                    {
                        if (y >= 127 - mRandom->nextInt(5) || y <= mRandom->nextInt(5))
                        {
                            primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                        }
                    }
                }
                else if (z + mRandom->nextInt(4) >= 15 || chunkZ > mLevelSize / 2)
                {
                    primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                }
                else if (!placed)
                {
                    if (y >= 127 - mRandom->nextInt(5) || y <= mRandom->nextInt(5))
                    {
                        primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                    }
                }
            }
        }
    }
}

LevelChunk* NetherFlatLevelSource::createChunk(int chunkX, int chunkZ)
{
    mRandom->setSeed(static_cast<long long>(chunkX) * 341873128712LL + static_cast<long long>(chunkZ) * 132897987541LL);

    unsigned int elementCount = 32768;
    unsigned short* elementIds = static_cast<unsigned short*>(
        XPhysicalAlloc(elementCount * sizeof(unsigned short), 0xFFFFFFFF, 0x1000, 4));
    XMemSet128(elementIds, 0, elementCount * sizeof(unsigned short));
    ChunkPrimer primer(false, arrayWithLength<unsigned short>(elementIds, elementCount));

    prepareHeights(chunkX, chunkZ, &primer);
    buildSurfaces(chunkX, chunkZ, &primer);
    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    XPhysicalFree(elementIds);
    return chunk;
}

void NetherFlatLevelSource::lightChunk(LevelChunk* chunk)
{
    chunk->recalcHeightmap();
}

void NetherFlatLevelSource::postProcess(int chunkX, int chunkZ)
{
    FallingBlock::fallInstantly = true;
    const int originX = chunkX * 16;
    const int originZ = chunkZ * 16;
    mPopulationRandom->setSeed(mLevel->getSeed());
    const long long xSeed = mPopulationRandom->nextLong() / 2LL * 2LL + 1LL;
    const long long zSeed = mPopulationRandom->nextLong() / 2LL * 2LL + 1LL;
    mPopulationRandom->setSeed((static_cast<long long>(chunkX) * xSeed + static_cast<long long>(chunkZ) * zSeed)
                               ^ mLevel->getSeed());

    int count = mPopulationRandom->nextInt(mPopulationRandom->nextInt(10) + 1) + 1;
    for (int i = 0; i < count; ++i)
    {
        int x = originX + mPopulationRandom->nextInt(16) + 8;
        int y = mPopulationRandom->nextInt(120) + 4;
        int z = originZ + mPopulationRandom->nextInt(16) + 8;
        HellFireFeature().place(mLevel, *mPopulationRandom, BlockPos(x, y, z));
    }

    count = mPopulationRandom->nextInt(mPopulationRandom->nextInt(10) + 1);
    for (int i = 0; i < count; ++i)
    {
        int x = originX + mPopulationRandom->nextInt(16) + 8;
        int y = mPopulationRandom->nextInt(120) + 4;
        int z = originZ + mPopulationRandom->nextInt(16) + 8;
        LightGemFeature().place(mLevel, *mPopulationRandom, BlockPos(x, y, z));
    }

    FallingBlock::fallInstantly = false;
    gApp.processSchematics(mLevel->getChunk(chunkX, chunkZ));
}

bool NetherFlatLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
    return false;
}

std::vector<Biome::MobSpawnerData*>* NetherFlatLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    Biome* biome = mLevel->getBiome(pos);
    if (biome == NULL)
    {
        return NULL;
    }
    return biome->getMobs(category);
}

BlockPos* NetherFlatLevelSource::findNearestMapFeature(Level*, const std::wstring&, const BlockPos&, bool)
{
    return NULL;
}

bool NetherFlatLevelSource::isPosInFeature(Level*, const std::wstring&, const BlockPos&)
{
    return false;
}

void NetherFlatLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int, int)
{
}
