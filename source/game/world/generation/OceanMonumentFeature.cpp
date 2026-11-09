#include "game/world/generation/OceanMonumentFeature.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>

#include "game/core/BoundingBox.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/core/OceanMonumentPieces.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/biome/FjFeatureBiomeRequirements.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"

static FjFeatureBiomeRequirements gOceanMonumentBiomeRequirements;
static std::vector<Biome*> gOceanMonumentAllowedBiomes;
static std::vector<Biome*> gOceanMonumentSpawnBiomes;
static std::vector<Biome::MobSpawnerData*> gOceanMonumentEnemies;

void OceanMonumentFeature::staticCtor()
{
    gOceanMonumentSpawnBiomes.push_back(Biome::DEEP_OCEAN);
    gOceanMonumentSpawnBiomes.push_back(Biome::DEEP_COLD_OCEAN);
    gOceanMonumentSpawnBiomes.push_back(Biome::DEEP_LUKEWARM_OCEAN);
    gOceanMonumentSpawnBiomes.push_back(Biome::DEEP_WARM_OCEAN);
    gOceanMonumentSpawnBiomes.push_back(Biome::DEEP_FROZEN_OCEAN);

    gOceanMonumentAllowedBiomes.push_back(Biome::OCEAN);
    gOceanMonumentAllowedBiomes.push_back(Biome::DEEP_OCEAN);
    gOceanMonumentAllowedBiomes.push_back(Biome::COLD_OCEAN);
    gOceanMonumentAllowedBiomes.push_back(Biome::DEEP_COLD_OCEAN);
    gOceanMonumentAllowedBiomes.push_back(Biome::FROZEN_OCEAN);
    gOceanMonumentAllowedBiomes.push_back(Biome::DEEP_FROZEN_OCEAN);
    gOceanMonumentAllowedBiomes.push_back(Biome::LUKEWARM_OCEAN);
    gOceanMonumentAllowedBiomes.push_back(Biome::DEEP_LUKEWARM_OCEAN);
    gOceanMonumentAllowedBiomes.push_back(Biome::WARM_OCEAN);
    gOceanMonumentAllowedBiomes.push_back(Biome::DEEP_WARM_OCEAN);
    gOceanMonumentAllowedBiomes.push_back(Biome::RIVER);
    gOceanMonumentAllowedBiomes.push_back(Biome::FROZEN_RIVER);

    gOceanMonumentBiomeRequirements = FjFeatureBiomeRequirements(&gOceanMonumentSpawnBiomes, 8,
                                                                 &gOceanMonumentAllowedBiomes, 29);
    gOceanMonumentEnemies.push_back(new Biome::MobSpawnerData(1, EntityType::GUARDIAN, 2, 4));
}

OceanMonumentFeature::OceanMonumentFeature() : StructureFeature()
{
    mSpacing = 32;
    mSeparation = 5;
    mFeatureChunk = FjIsFeatureChunk(&gOceanMonumentBiomeRequirements, 40, mSpacing, mSeparation);
    mReducedFeatureChunk = FjIsFeatureChunk::CreateReduced(mFeatureChunk);
}

OceanMonumentFeature::OceanMonumentFeature(SuperflatConfig* config) : StructureFeature()
{
    FlatGeneratorInfo generatorInfo(config);
    int separation = generatorInfo.getOceanMonuemntSeparation();
    if (separation < 5)
        separation = 5;
    mSeparation = separation;
    separation = separation + 1;
    int spacing = generatorInfo.getOceanMonumentSpacing();
    mSpacing = spacing > separation ? spacing : separation;
    mFeatureChunk = FjIsFeatureChunk(&gOceanMonumentBiomeRequirements, 40, mSpacing, mSeparation);
    mReducedFeatureChunk = FjIsFeatureChunk::CreateReduced(mFeatureChunk);
}

OceanMonumentFeature::~OceanMonumentFeature()
{
}

std::wstring OceanMonumentFeature::getFeatureName()
{
    return std::wstring(L"Monument");
}

bool OceanMonumentFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    int sizeInChunks = level->getLevelData()->getXZSize();
    const FjIsFeatureChunk* featureChunk = FjIsFeatureChunk::UseReducedSpacing(sizeInChunks, flatWorld)
                                               ? &mReducedFeatureChunk
                                               : &mFeatureChunk;
    ChunkPos centerChunk = featureChunk->getCenterChunk(chunkX, chunkZ);
    bool result = featureChunk->isFeatureChunk(*level->getRandomFor(centerChunk.x, centerChunk.z, 0x9E7F71),
                                               level->getBiomeSource(), sizeInChunks, chunkX, chunkZ);
    result = canPlaceFeatureInChunk(result, eFeatureType_OceanMonument, chunkX, chunkZ);
    if (result)
    {
        gApp.AddTerrainFeaturePosition((_eTerrainFeatureType)7, chunkX, chunkZ);
    }
    return result;
}

char OceanMonumentFeature::fjIsFeatureChunk(BiomeSource*, long long, int, int, int)
{
    return false;
}

BlockPos* OceanMonumentFeature::getNearestGeneratedFeature(Level* levelIn, const BlockPos& pos, bool skipExistingChunks)
{
    level = levelIn;
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL)
    {
        int chunkX = pos.x >> 4;
        int chunkZ = pos.z >> 4;
        if (canPlaceFeatureInChunk(false, eFeatureType_OceanMonument, chunkX, chunkZ))
        {
            return options->getClosestFeature(eFeatureType_OceanMonument, pos);
        }
    }
    return StructureFeature::findNearestFeaturePositionBySpacing(levelIn, this, pos, mSpacing, mSeparation, 0x9E7F71,
                                                                 true, 100, skipExistingChunks);
}

StructureStart* OceanMonumentFeature::createStructureStart(int chunkX, int chunkZ)
{
    return new OceanMonumentStart(level, random, chunkX, chunkZ);
}

std::vector<Biome::MobSpawnerData*>* OceanMonumentFeature::getEnemies()
{
    return &gOceanMonumentEnemies;
}

void OceanMonumentFeature::OceanMonumentStart::Init()
{
    mCreated = false;
}

void OceanMonumentFeature::OceanMonumentStart::create(Level* level, Random& random, int chunkX, int chunkZ)
{
    random.setSeed(level->getSeed());
    long long xSeed = random.nextLong();
    long long zSeed = random.nextLong();
    long long seedX = (long long)chunkX * xSeed;
    long long seedZ = (long long)chunkZ * zSeed;
    random.setSeed(seedX ^ seedZ ^ level->getSeed());

    int originX = chunkX * 16 - 21;
    int originZ = chunkZ * 16 - 21;
    const Direction* direction = Direction::Plane::HORIZONTAL->getRandomDirection(random);
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL)
    {
        const Direction* featureDirection = NULL;
        if (options->isFeatureChunk(chunkX, chunkZ, eFeatureType_OceanMonument, &featureDirection)
            && featureDirection != NULL)
        {
            direction = featureDirection;
        }
    }

    mPieces.push_back(new OceanMonumentPieces::MonumentBuilding(random, originX, originZ, direction));
    calculateBoundingBox();
    mCreated = true;
}

OceanMonumentFeature::OceanMonumentStart::OceanMonumentStart(Level* level, Random& random, int chunkX, int chunkZ)
    : StructureStart(chunkX, chunkZ)
{
    Init();
    create(level, random, chunkX, chunkZ);
}

OceanMonumentFeature::OceanMonumentStart::OceanMonumentStart() : StructureStart()
{
    Init();
}

void OceanMonumentFeature::OceanMonumentStart::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (!mCreated)
    {
        mPieces.clear();
        create(level, random, getChunkX(), getChunkZ());
    }
    StructureStart::postProcess(level, random, box);
}

bool OceanMonumentFeature::OceanMonumentStart::isValidForPostProcess(ChunkPos* chunkPos)
{
    if (std::find(mProcessedChunks.begin(), mProcessedChunks.end(), *chunkPos) == mProcessedChunks.end())
    {
        return StructureStart::isValidForPostProcess(chunkPos);
    }
    return false;
}

void OceanMonumentFeature::OceanMonumentStart::notifyPostProcessAt(ChunkPos* chunkPos)
{
    StructureStart::notifyPostProcessAt(chunkPos);
    mProcessedChunks.push_back(*chunkPos);
    setDirty();
}

void OceanMonumentFeature::OceanMonumentStart::addAdditionalSaveData(DataOutputStream* output)
{
    StructureStart::addAdditionalSaveData(output);
    output->writeInt(mProcessedChunks.size());
    for (std::vector<ChunkPos>::iterator it = mProcessedChunks.begin(); it != mProcessedChunks.end(); it++)
    {
        output->writeInt(it->x);
        output->writeInt(it->z);
    }
}

void OceanMonumentFeature::OceanMonumentStart::readAdditionalSaveData(DataInputStream* input)
{
    StructureStart::readAdditionalSaveData(input);
    int count = input->readInt();
    for (int i = 0; i < count; ++i)
    {
        int x = input->readInt();
        int z = input->readInt();
        mProcessedChunks.push_back(ChunkPos(x, z));
    }
}

EStructureStart OceanMonumentFeature::OceanMonumentStart::GetType()
{
    return eStructureStart_OceanMonument;
}

StructureStart* OceanMonumentFeature::OceanMonumentStart::Create()
{
    return new OceanMonumentStart();
}
