#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/generation/NetherBridgeFeature.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/core/NetherBridgePieces.hpp"

#include <stdio.h>

NetherBridgeFeature::NetherBridgeFeature() : StructureFeature()
{
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(10, EntityType::BLAZE, 2, 3));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(5, EntityType::PIG_ZOMBIE, 4, 4));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(8, EntityType::WITHER_SKELETON, 5, 5));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(2, EntityType::SKELETON, 5, 5));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(3, EntityType::MAGMA_CUBE, 4, 4));
    mCachedBuildings = false;
    mCachedChunk = NULL;
}

NetherBridgeFeature::~NetherBridgeFeature()
{
    std::vector<Biome::MobSpawnerData*>::iterator end = mBridgeEnemies.end();
    for (std::vector<Biome::MobSpawnerData*>::iterator it = mBridgeEnemies.begin(); it != end; ++it)
    {
        delete *it;
    }
    delete mCachedChunk;
#ifdef MINECRAFT_PC
    for (std::vector<ChunkPos*>::iterator it = mGridChunks.begin(); it != mGridChunks.end(); ++it)
        delete *it;
#endif
}

std::wstring NetherBridgeFeature::getFeatureName()
{
    return std::wstring(L"Fortress");
}

std::vector<Biome::MobSpawnerData*>* NetherBridgeFeature::getBridgeEnemies()
{
    return &mBridgeEnemies;
}

#ifdef MINECRAFT_PC
void NetherBridgeFeature::PlaceGridFortresses()
{
    const int massiveWidth = 640;
    const int cellChunks = 20;
    LevelData* levelData = level->getLevelData();
    if (levelData->getXZSize() < massiveWidth || levelData->getHellScale() <= 0)
        return;
    const int half = levelData->getXZSize() / levelData->getHellScale() / 2;
    const int cells = half * 2 / cellChunks;
    printf("[pc] world: nether fortresses at (nether block x, z): (%d, %d)", mCachedChunk->x * 16 + 8,
           mCachedChunk->z * 16 + 8);
    for (int cellX = 0; cellX < cells; ++cellX)
    {
        for (int cellZ = 0; cellZ < cells; ++cellZ)
        {
            const int originX = cellX * cellChunks - half;
            const int originZ = cellZ * cellChunks - half;
            if (mCachedChunk->x >= originX && mCachedChunk->x < originX + cellChunks && mCachedChunk->z >= originZ
                && mCachedChunk->z < originZ + cellChunks)
                continue;
            Random cellRandom;
            cellRandom.setSeed(level->getSeed() ^ (static_cast<long long>(cellX) * 341873128712LL)
                               ^ (static_cast<long long>(cellZ) * 132897987541LL) ^ 0x4E657468LL);
            const int x = originX + 4 + cellRandom.nextInt(cellChunks - 8);
            const int z = originZ + 4 + cellRandom.nextInt(cellChunks - 8);
            mGridChunks.push_back(new ChunkPos(x, z));
            printf(" (%d, %d)", x * 16 + 8, z * 16 + 8);
        }
    }
    printf("\n");
    fflush(stdout);
}
#endif

bool NetherBridgeFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    if (!mCachedBuildings)
    {
        long long seed = level->getSeed();
        random.setSeed(seed);
        random.nextInt();
        int index = random.nextInt(49);
        int x = index % 7;
        int z = index / 7;
        mCachedChunk = new ChunkPos(x, z);
        mCachedBuildings = true;
#ifdef MINECRAFT_PC
        PlaceGridFortresses();
#endif
    }
#ifdef MINECRAFT_PC
    for (std::vector<ChunkPos*>::iterator it = mGridChunks.begin(); it != mGridChunks.end(); ++it)
        if ((*it)->x == chunkX && (*it)->z == chunkZ)
            return true;
#endif

    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL && options->isFeatureChunk(chunkX, chunkZ, eFeatureType_NetherBridge, NULL))
        return true;
    if (chunkX == mCachedChunk->x && chunkZ == mCachedChunk->z)
        return true;
    return false;
}

StructureStart* NetherBridgeFeature::createStructureStart(int chunkX, int chunkZ)
{
    return new NetherBridgeStart(level, random, chunkX, chunkZ);
}

BlockPos* NetherBridgeFeature::getNearestGeneratedFeature(Level* levelIn, const BlockPos& pos, bool skipExistingChunks)
{
    int chunkX = pos.x >> 4;
    int chunkZ = pos.z >> 4;
    for (int radius = 0; radius < 1001; ++radius)
    {
        for (int offsetX = -radius; offsetX <= radius; ++offsetX)
        {
            bool edgeX = offsetX == -radius || offsetX == radius;
            for (int offsetZ = -radius; offsetZ <= radius; ++offsetZ)
            {
                bool edgeZ = offsetZ == -radius || offsetZ == radius;
                if (edgeX || edgeZ)
                {
                    int candidateX = chunkX + offsetX;
                    int candidateZ = chunkZ + offsetZ;
                    if (isFeatureChunk(candidateX, candidateZ, false)
                        && (!skipExistingChunks || !levelIn->isChunkGeneratedAt(candidateX, candidateZ)))
                        return new BlockPos(candidateX * 16 + 8, 64, candidateZ * 16 + 8);
                }
            }
        }
    }

    return NULL;
}

NetherBridgeFeature::NetherBridgeStart::NetherBridgeStart(Level* levelIn, Random& random, int chunkX, int chunkZ)
    : StructureStart(chunkX, chunkZ)
{
    NetherBridgePieces::StartPiece* startPiece = new NetherBridgePieces::StartPiece(random, chunkX * 16 + 2,
                                                                                    chunkZ * 16 + 2, levelIn);
    mPieces.push_back(startPiece);
    startPiece->addChildren(startPiece, &mPieces, random);

    std::vector<StructurePiece*>& pendingChildren = startPiece->mPendingChildren;
    while (!pendingChildren.empty())
    {
        int index = random.nextInt((int)pendingChildren.size());
        std::vector<StructurePiece*>::iterator it = pendingChildren.begin() + index;
        StructurePiece* piece = *it;
        pendingChildren.erase(it);
        piece->addChildren(startPiece, &mPieces, random);
    }

    calculateBoundingBox();
    moveInsideHeights(levelIn, random, 0x30, 0x46);
}

NetherBridgeFeature::NetherBridgeStart::NetherBridgeStart() : StructureStart()
{
}

EStructureStart NetherBridgeFeature::NetherBridgeStart::GetType()
{
    return eStructureStart_NetherBridge;
}

StructureStart* NetherBridgeFeature::NetherBridgeStart::Create()
{
    return new NetherBridgeStart();
}
