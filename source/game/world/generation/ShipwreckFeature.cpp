#include "game/world/generation/ShipwreckFeature.hpp"

#include <algorithm>
#include <vector>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/ShipwreckPiece.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/OceanMonumentFeature.hpp"

static std::vector<Biome*>* gShipwreckSpawnBiomes = NULL;
static std::vector<Biome*>* gShipwreckAllowedBiomes = NULL;
static FjFeatureBiomeRequirements* gShipwreckBiomeRequirements = NULL;

std::vector<Biome*>* ShipwreckFeature::GetSpawnBiomes()
{
    if (gShipwreckSpawnBiomes == NULL)
    {
        gShipwreckSpawnBiomes = new std::vector<Biome*>;
        gShipwreckSpawnBiomes->reserve(12);
        gShipwreckSpawnBiomes->push_back(Biome::BEACH);
        gShipwreckSpawnBiomes->push_back(Biome::COLD_BEACH);
        gShipwreckSpawnBiomes->push_back(Biome::MUSHROOM_ISLAND_SHORE);
        gShipwreckSpawnBiomes->push_back(Biome::OCEAN);
        gShipwreckSpawnBiomes->push_back(Biome::DEEP_OCEAN);
        gShipwreckSpawnBiomes->push_back(Biome::COLD_OCEAN);
        gShipwreckSpawnBiomes->push_back(Biome::DEEP_COLD_OCEAN);
        gShipwreckSpawnBiomes->push_back(Biome::LUKEWARM_OCEAN);
        gShipwreckSpawnBiomes->push_back(Biome::DEEP_LUKEWARM_OCEAN);
        gShipwreckSpawnBiomes->push_back(Biome::FROZEN_OCEAN);
        gShipwreckSpawnBiomes->push_back(Biome::DEEP_FROZEN_OCEAN);
        gShipwreckSpawnBiomes->push_back(Biome::WARM_OCEAN);
    }
    return gShipwreckSpawnBiomes;
}

std::vector<Biome*>* ShipwreckFeature::GetAllowedBiomes()
{
    if (gShipwreckAllowedBiomes == NULL)
    {
        gShipwreckAllowedBiomes = new std::vector<Biome*>;
        gShipwreckAllowedBiomes->reserve(12);
        gShipwreckAllowedBiomes->push_back(Biome::BEACH);
        gShipwreckAllowedBiomes->push_back(Biome::COLD_BEACH);
        gShipwreckAllowedBiomes->push_back(Biome::MUSHROOM_ISLAND_SHORE);
        gShipwreckAllowedBiomes->push_back(Biome::OCEAN);
        gShipwreckAllowedBiomes->push_back(Biome::DEEP_OCEAN);
        gShipwreckAllowedBiomes->push_back(Biome::COLD_OCEAN);
        gShipwreckAllowedBiomes->push_back(Biome::DEEP_COLD_OCEAN);
        gShipwreckAllowedBiomes->push_back(Biome::LUKEWARM_OCEAN);
        gShipwreckAllowedBiomes->push_back(Biome::DEEP_LUKEWARM_OCEAN);
        gShipwreckAllowedBiomes->push_back(Biome::FROZEN_OCEAN);
        gShipwreckAllowedBiomes->push_back(Biome::DEEP_FROZEN_OCEAN);
        gShipwreckAllowedBiomes->push_back(Biome::WARM_OCEAN);
    }
    return gShipwreckAllowedBiomes;
}

FjFeatureBiomeRequirements* ShipwreckFeature::GetFJBiomeRequirements()
{
    if (gShipwreckBiomeRequirements == NULL)
    {
        gShipwreckBiomeRequirements = new FjFeatureBiomeRequirements(GetSpawnBiomes(), 5, GetAllowedBiomes(), 10);
    }
    return gShipwreckBiomeRequirements;
}

ShipwreckFeature::ShipwreckFeature(GenericOverworldLevelSource* source, OceanMonumentFeature* monument)
    : StructureFeature(),
      mSource(source),
      mSpacing(10),
      mSeparation(5),
      mOceanMonument(monument)
{
    mFeatureChunk = FjIsFeatureChunk(GetFJBiomeRequirements(), 20, mSpacing, mSeparation);
    mReducedFeatureChunk = FjIsFeatureChunk::CreateReduced(mFeatureChunk);
}

ShipwreckFeature::ShipwreckFeature(GenericOverworldLevelSource* source, OceanMonumentFeature* monument,
                                   SuperflatConfig* config)
    : StructureFeature(),
      mSource(source),
      mOceanMonument(monument)
{
    FlatGeneratorInfo info(config);
    int separation = info.getShipwreckSeparation();
    if (separation < 5)
        separation = 5;
    mSeparation = separation;
    int spacing = std::max(info.getShipwreckSpacing(), separation + 1);
    mSpacing = spacing;
    mFeatureChunk = FjIsFeatureChunk(GetFJBiomeRequirements(), 20, mSpacing, mSeparation);
    mReducedFeatureChunk = FjIsFeatureChunk::CreateReduced(mFeatureChunk);
}

std::wstring ShipwreckFeature::getFeatureName()
{
    return L"Shipwreck";
}

BlockPos* ShipwreckFeature::getNearestGeneratedFeature(Level* sourceLevel, const BlockPos& pos, bool skipExistingChunks)
{
    level = sourceLevel;
    return StructureFeature::findNearestFeaturePositionBySpacing(sourceLevel, this, pos, 10, 5, 0xdb1471, true, 100,
                                                                 skipExistingChunks);
}

char ShipwreckFeature::fjIsFeatureChunk(BiomeSource*, long long, int, int, int)
{
    return false;
}

StructureStart* ShipwreckFeature::createStructureStart(int chunkX, int chunkZ)
{
    long long seed = level->getSeed();
    Random featureRandom = Level::fjGetRandomFor(seed, chunkX, chunkZ, 0xdb1471);
    return new ShipwreckStart(level, mSource, featureRandom, chunkX, chunkZ);
}

bool ShipwreckFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    int sizeInChunks = level->getLevelData()->getXZSize();
    FjIsFeatureChunk* featureChunk = FjIsFeatureChunk::UseReducedSpacing(sizeInChunks, flatWorld)
                                         ? &mReducedFeatureChunk
                                         : &mFeatureChunk;
    ChunkPos centerChunk = featureChunk->getCenterChunk(chunkX, chunkZ);
    bool result = featureChunk->isFeatureChunk(*level->getRandomFor(centerChunk.x, centerChunk.z, 0xdb1471),
                                               level->getBiomeSource(), sizeInChunks, chunkX, chunkZ);
    result = canPlaceFeatureInChunk(result, eFeatureType_Shipwreck, chunkX, chunkZ);
    if (result && mOceanMonument != NULL)
    {
        ChunkPos chunk(chunkX, chunkZ);
        for (int x = chunk.x - 5; x < chunk.x + 5; ++x)
            for (int z = chunk.z - 5; z < chunk.z + 5; ++z)
                if (mOceanMonument->isFeatureChunk(x, z, flatWorld))
                    return false;
    }
    if (result)
        gApp.AddTerrainFeaturePosition((_eTerrainFeatureType)11, chunkX, chunkZ);
    return result;
}

void ShipwreckFeature::ShipwreckStart::create(Level* level, GenericOverworldLevelSource* source, Random& random,
                                              int chunkX, int chunkZ)
{
    int x = chunkX * 16 - 2;
    int z = chunkZ * 16 - 2;
    const Direction* direction = Direction::SOUTH;
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL)
    {
        const Direction* featureDirection = NULL;
        if (options->isFeatureChunk(chunkX, chunkZ, eFeatureType_Shipwreck, &featureDirection)
            && featureDirection != NULL)
            direction = featureDirection;
    }
    mPieces.push_back(new ShipwreckPiece(level, source, x, z, random, direction));
    calculateBoundingBox();
}

ShipwreckFeature::ShipwreckStart::ShipwreckStart(Level* level, GenericOverworldLevelSource* source, Random& random,
                                                 int chunkX, int chunkZ)
    : StructureStart(chunkX, chunkZ)
{
    create(level, source, random, chunkX, chunkZ);
}

ShipwreckFeature::ShipwreckStart::ShipwreckStart() : StructureStart(0, 0)
{
}

EStructureStart ShipwreckFeature::ShipwreckStart::GetType()
{
    return eStructureStart_Shipwreck;
}

StructureStart* ShipwreckFeature::ShipwreckStart::Create()
{
    return new ShipwreckStart();
}
