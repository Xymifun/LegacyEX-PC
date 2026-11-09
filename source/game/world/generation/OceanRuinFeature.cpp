#include "game/world/generation/OceanRuinFeature.hpp"

#include <algorithm>

#include "game/core/BoundingBox.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/StructurePiece.hpp"

OceanRuinFeature::OceanRuinFeature() : StructureFeature(), mSpacing(8), mSeparation(2)
{
}

OceanRuinFeature::OceanRuinFeature(SuperflatConfig* config) : StructureFeature()
{
    FlatGeneratorInfo info(config);
    mSeparation = info.getShipwreckSeparation();
    mSpacing = info.getShipwreckSpacing();
}

bool OceanRuinFeature::IsValidSpawnArea(Level* level, int x, int z, int radius)
{
    std::vector<Biome*> biomes(10);
    biomes.push_back(Biome::OCEAN);
    biomes.push_back(Biome::DEEP_OCEAN);
    biomes.push_back(Biome::COLD_OCEAN);
    biomes.push_back(Biome::DEEP_COLD_OCEAN);
    biomes.push_back(Biome::FROZEN_OCEAN);
    biomes.push_back(Biome::DEEP_FROZEN_OCEAN);
    biomes.push_back(Biome::LUKEWARM_OCEAN);
    biomes.push_back(Biome::DEEP_LUKEWARM_OCEAN);
    biomes.push_back(Biome::WARM_OCEAN);
    biomes.push_back(Biome::DEEP_WARM_OCEAN);
    return level->getBiomeSource()->containsOnly(x, z, radius, biomes);
}

std::wstring OceanRuinFeature::getFeatureName()
{
    return L"Ocean Ruin";
}

BlockPos* OceanRuinFeature::getNearestGeneratedFeature(Level* sourceLevel, const BlockPos& pos, bool skipExistingChunks)
{
    level = sourceLevel;
    return StructureFeature::findNearestFeaturePositionBySpacing(sourceLevel, this, pos, mSpacing, mSeparation,
                                                                 0xdb1471, false, 100, skipExistingChunks);
}

StructureStart* OceanRuinFeature::createStructureStart(int chunkX, int chunkZ)
{
    OceanRuinConfiguration configuration =
        level->getBiome(BlockPos(chunkX << 4, 64, chunkZ << 4))->getOceanRuinConfig();
    return new OceanRuinStart(level, random, chunkX, chunkZ, configuration);
}

bool OceanRuinFeature::isFeatureChunk(int chunkX, int chunkZ, bool)
{
    int originalX = chunkX;
    int originalZ = chunkZ;
    if (chunkX < 0)
    {
        chunkX -= mSpacing - 1;
    }
    if (chunkZ < 0)
    {
        chunkZ -= mSpacing - 1;
    }

    int regionX = chunkX / mSpacing;
    int regionZ = chunkZ / mSpacing;
    Random* chunkRandom = level->getRandomFor(regionX, regionZ, 0xdb1471);
    regionX *= mSpacing;
    regionZ *= mSpacing;
    regionX += chunkRandom->nextInt(mSpacing - mSeparation);
    regionZ += chunkRandom->nextInt(mSpacing - mSeparation);
    bool featureChunk = originalX == regionX && originalZ == regionZ;

    if (canPlaceFeatureInChunk(featureChunk, eFeatureType_OceanRuin, originalX, originalZ))
    {
        return IsValidSpawnArea(level, originalX * 16 + 8, originalZ * 16 + 8, 8);
    }
    return false;
}

OceanRuinFeature::OceanRuinStart::OceanRuinStart(Level* level, Random& random, int chunkX, int chunkZ,
                                                 const OceanRuinConfiguration& configuration)
    : StructureStart(chunkX, chunkZ),
      mLevel(level)
{
    create(random, chunkX, chunkZ, configuration);
}

std::vector<BlockPos>* OceanRuinFeature::OceanRuinStart::allPositions(Random& random, int x, int z)
{
    std::vector<BlockPos>* positions = new std::vector<BlockPos>;
    positions->push_back(BlockPos(x - 16 + Mth::nextInt(&random, 1, 8), 0, z + 16 + Mth::nextInt(&random, 1, 7)));
    positions->push_back(BlockPos(x - 16 + Mth::nextInt(&random, 1, 8), 0, z + Mth::nextInt(&random, 1, 7)));
    positions->push_back(BlockPos(x - 16 + Mth::nextInt(&random, 1, 8), 0, z - 16 + Mth::nextInt(&random, 4, 8)));
    positions->push_back(BlockPos(x + Mth::nextInt(&random, 1, 7), 0, z + 16 + Mth::nextInt(&random, 1, 7)));
    positions->push_back(BlockPos(x + Mth::nextInt(&random, 1, 7), 0, z - 16 + Mth::nextInt(&random, 4, 6)));
    positions->push_back(BlockPos(x + 16 + Mth::nextInt(&random, 1, 7), 0, z + 16 + Mth::nextInt(&random, 3, 8)));
    positions->push_back(BlockPos(x + 16 + Mth::nextInt(&random, 1, 7), 0, z + Mth::nextInt(&random, 1, 7)));
    positions->push_back(BlockPos(x + 16 + Mth::nextInt(&random, 1, 7), 0, z - 16 + Mth::nextInt(&random, 4, 8)));
    return positions;
}

void OceanRuinFeature::OceanRuinStart::addClusterRuins(Random& random, const Direction* direction, int x, int z,
                                                       bool warm, std::vector<StructurePiece*>& pieces)
{
    BoundingBox* structureBox = pieces[0]->getBoundingBox();
    int startX = structureBox->x0 < structureBox->x1 ? structureBox->x0 : structureBox->x1;
    int startZ = structureBox->z0 < structureBox->z1 ? structureBox->z0 : structureBox->z1;
    std::vector<BlockPos>* positions = allPositions(random, startX, startZ);
    int count = Mth::nextInt(&random, 4, 8);

    for (int i = 0; i < count; ++i)
    {
        if (positions->empty())
            continue;

        int index = random.nextInt((int)positions->size());
        BlockPos position = positions->at(index);
        std::swap(positions->at(index), positions->back());
        positions->pop_back();

        BoundingBox* candidate = BoundingBox::createProper(position.x, structureBox->y0, position.z, position.x + 6,
                                                           structureBox->y0 + 6, position.z + 6);
        int xSpan = candidate->getXSpan();
        int zSpan = candidate->getZSpan();
        int radius = xSpan < zSpan ? zSpan : xSpan;
        if (!candidate->intersects(structureBox)
            && IsValidSpawnArea(mLevel, position.x + candidate->getXSpan() / 2, position.z + candidate->getZSpan() / 2,
                                radius))
        {
            delete candidate;
            pieces.push_back(new OceanRuin(mLevel, random, position.x, position.z, 7, 7, 7, warm, false));
            continue;
        }
        delete candidate;
    }

    delete positions;
}

void OceanRuinFeature::OceanRuinStart::create(Random& random, int chunkX, int chunkZ,
                                              const OceanRuinConfiguration& configuration)
{
    int x = chunkX << 4;
    int z = chunkZ << 4;
    bool warm = configuration.biomeType == OceanRuin::eRuinType_Warm;
    bool large = random.nextFloat() <= configuration.smallProbability;

    StructurePiece* ruin;
    if (!large)
    {
        ruin = new OceanRuin(mLevel, random, x, z, 7, 7, 7, warm, large);
    }
    else
    {
        ruin = new OceanRuin(mLevel, random, x, z, 16, 16, 16, warm, large);
    }
    mPieces.push_back(ruin);

    if (large && random.nextFloat() <= configuration.clusterProbability)
    {
        addClusterRuins(random, Direction::Plane::HORIZONTAL->getRandomDirection(random), x, z, warm, mPieces);
    }

    calculateBoundingBox();
}

EStructureStart OceanRuinFeature::OceanRuinStart::GetType()
{
    return eStructureStart_OceanRuin;
}
