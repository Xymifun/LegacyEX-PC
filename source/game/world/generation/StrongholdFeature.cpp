#include "game/world/generation/StrongholdFeature.hpp"

#include <cmath>
#include <stdio.h>
#include <limits>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/StrongholdPieces.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"

StrongholdFeature::StrongholdFeature() : StructureFeature()
{
    _init();
}

StrongholdFeature::StrongholdFeature(SuperflatConfig* config) : StructureFeature()
{
    _init();
    FlatGeneratorInfo info(config);
    mDistance = static_cast<unsigned int>(info.getStrongholdDistance());
    int count = info.getStrongholdCount();
    mPositions.clear();
    for (int i = 0; i < count; ++i)
    {
        mPositions.push_back(NULL);
    }
    mVariant = 1;
}

void StrongholdFeature::_init()
{
    mAllowedBiomes = std::vector<Biome*>();
    for (int i = 0; i < 40; ++i)
    {
        Biome* biome = Biome::getBiomes()[i];
        if (biome != NULL && biome->getDepth() > 0.0f)
        {
            mAllowedBiomes.push_back(biome);
        }
    }
    mDistance = 32.0;
    mVariant = 3;
    mPositions.push_back(NULL);
    mPositionsGenerated = false;
}

StrongholdFeature::~StrongholdFeature()
{
    for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
    {
        delete *it;
    }
}

std::wstring StrongholdFeature::getFeatureName()
{
    return STRONGHOLD;
}

#ifdef MINECRAFT_PC
bool StrongholdFeature::GenerateRingPositions()
{
    const int massiveWidth = 640;
    const int ringCounts[] = {3, 6, 9};
    const int ringNearest[] = {40, 120, 220};
    const int ringFarthest[] = {72, 170, 270};
    if (mVariant != 3 || level->getLevelData()->getXZSize() < massiveWidth)
        return false;

    for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
        delete *it;
    mPositions.clear();

    Random positionRandom;
    positionRandom.setSeed(level->getSeed());
    for (int ring = 0; ring < 3; ++ring)
    {
        double angle = positionRandom.nextDouble() * 3.1415927f * 2.0;
        for (int index = 0; index < ringCounts[ring]; ++index)
        {
            const double distance = ringNearest[ring]
                                    + positionRandom.nextDouble() * (ringFarthest[ring] - ringNearest[ring]);
            int x = Math::round(std::cos(angle) * distance);
            int z = Math::round(std::sin(angle) * distance);
            BlockPos* biomePosition = level->getBiomeSource()->findBiome(x * 16 + 8, z * 16 + 8, 112, mAllowedBiomes,
                                                                         &positionRandom);
            if (biomePosition != NULL)
            {
                x = biomePosition->x >> 4;
                z = biomePosition->z >> 4;
                delete biomePosition;
            }
            gApp.AddTerrainFeaturePosition(eTerrainFeatureType_Stronghold, x, z);
            mPositions.push_back(new ChunkPos(x, z));
            angle += 3.1415927f * 2.0 / static_cast<double>(ringCounts[ring]);
        }
    }
    printf("[pc] world: %u strongholds at (block x, z):", static_cast<unsigned int>(mPositions.size()));
    for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
        printf(" (%d, %d)", (*it)->x * 16 + 8, (*it)->z * 16 + 8);
    printf("\n");
    fflush(stdout);
    return true;
}
#endif

void StrongholdFeature::generatePositions()
{
    restoreSavedData(level);
#ifdef MINECRAFT_PC
    if (GenerateRingPositions())
        return;
#endif
    Random positionRandom;
    positionRandom.setSeed(level->getSeed());
    double angle = positionRandom.nextDouble() * 3.1415927f * 2.0;
    bool foundPosition = false;

    for (int attempt = 0; !foundPosition && attempt < 10; ++attempt)
    {
        for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
        {
            double distance = 0.0;
            if (level->getOriginalSaveVersion() >= 5)
            {
                distance = (positionRandom.nextDouble() + 1.25) * static_cast<double>(positionRandom.nextInt(4) + 3);
            }
            else
            {
                distance = (positionRandom.nextDouble() + 1.25)
                           * (static_cast<double>(positionRandom.nextInt(7)) + 5.0);
            }
            int x = Math::round(std::cos(angle) * distance);
            int z = Math::round(std::sin(angle) * distance);
            BlockPos* biomePosition = level->getBiomeSource()->findBiome(x * 16 + 8, z * 16 + 8, 112, mAllowedBiomes,
                                                                         &positionRandom);
            if (biomePosition != NULL)
            {
                x = biomePosition->x >> 4;
                z = biomePosition->z >> 4;
                gApp.AddTerrainFeaturePosition(eTerrainFeatureType_Stronghold, x, z);
                foundPosition = true;
                delete biomePosition;
            }

            delete *it;
            *it = new ChunkPos(x, z);
            angle += 3.1415927f * 2.0 / static_cast<double>(mPositions.size());
        }
    }

    if (!foundPosition)
    {
        gApp.AddTerrainFeaturePosition(eTerrainFeatureType_Stronghold, mPositions[0]->x, mPositions[0]->z);
    }
}

BlockPos* StrongholdFeature::getNearestGeneratedFeature(Level*, const BlockPos& pos, bool)
{
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL && canPlaceFeatureInChunk(false, eFeatureType_Stronghold, pos.x >> 4, pos.z >> 4))
    {
        return options->getClosestFeature(eFeatureType_Stronghold, pos);
    }

    if (!mPositionsGenerated)
    {
        generatePositions();
        mPositionsGenerated = true;
    }

    BlockPos* nearest = NULL;
    BlockPos candidate(0, 0, 0);
    double nearestDistance = std::numeric_limits<double>::max();
    for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); it++)
    {
        candidate = BlockPos((*it)->x * 16 + 8, 32, (*it)->z * 16 + 8);
        double distance = candidate.distSqr(pos);
        if (nearest == NULL)
        {
            nearest = new BlockPos(candidate);
            nearestDistance = distance;
        }
        else if (distance < nearestDistance)
        {
            *nearest = candidate;
            nearestDistance = distance;
        }
    }
    return nearest;
}

bool StrongholdFeature::isFeatureChunk(int chunkX, int chunkZ, bool)
{
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL && canPlaceFeatureInChunk(false, eFeatureType_Stronghold, chunkX, chunkZ))
    {
        return options->isFeatureChunk(chunkX, chunkZ, eFeatureType_Stronghold, NULL);
    }

    if (!mPositionsGenerated)
    {
        generatePositions();
        mPositionsGenerated = true;
    }
    for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
    {
        int inOptions = 0;
        LevelGenerationOptions* currentOptions = gApp.getLevelGenerationOptions();
        if (currentOptions != NULL)
        {
            inOptions = currentOptions->isFeatureChunk(chunkX, chunkZ, eFeatureType_Stronghold, NULL);
        }
        ChunkPos* position = *it;
        if (inOptions != 0 || (position != NULL && chunkX == position->x && chunkZ == position->z))
        {
            return true;
        }
    }
    return false;
}

std::vector<BlockPos>* StrongholdFeature::getGuesstimatedFeaturePositions()
{
    std::vector<BlockPos>* positions = new std::vector<BlockPos>;
    for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
    {
        if (*it != NULL)
        {
            positions->push_back((*it)->getMiddleBlockPosition(64));
        }
    }
    return positions;
}

StructureStart* StrongholdFeature::createStructureStart(int chunkX, int chunkZ)
{
    StrongholdStart* start = NULL;
    do
    {
        delete start;
        start = new StrongholdStart(level, random, chunkX, chunkZ);
    } while (start->getPieces()->empty()
             || static_cast<StrongholdPieces::StartPiece*>(start->getPieces()->front())->mPortalRoom == NULL);
    return start;
}

StrongholdFeature::StrongholdStart::StrongholdStart(Level* sourceLevel, Random& sourceRandom, int chunkX, int chunkZ)
    : StructureStart(chunkX, chunkZ)
{
    StrongholdPieces::resetPieces();
    StrongholdPieces::StartPiece* piece = new StrongholdPieces::StartPiece(0, sourceRandom, chunkX * 16 + 2,
                                                                           chunkZ * 16 + 2, sourceLevel);
    mPieces.push_back(piece);
    piece->addChildren(piece, &mPieces, sourceRandom);
    std::vector<StructurePiece*>& pendingChildren = piece->mPendingChildren;
    while (!pendingChildren.empty())
    {
        int index = sourceRandom.nextInt(pendingChildren.size());
        std::vector<StructurePiece*>::iterator it = pendingChildren.begin() + index;
        StructurePiece* child = *it;
        pendingChildren.erase(it);
        child->addChildren(piece, &mPieces, sourceRandom);
    }
    calculateBoundingBox();
    moveBelowSeaLevel(sourceLevel, sourceRandom, 10);
}

StrongholdFeature::StrongholdStart::StrongholdStart() : StructureStart()
{
}

EStructureStart StrongholdFeature::StrongholdStart::GetType()
{
    return eStructureStart_Stronghold;
}

StructureStart* StrongholdFeature::StrongholdStart::Create()
{
    return new StrongholdStart();
}
