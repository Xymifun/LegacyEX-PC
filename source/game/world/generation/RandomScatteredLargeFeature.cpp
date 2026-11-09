#include "game/world/generation/RandomScatteredLargeFeature.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/ScatteredFeaturePieces.hpp"

static std::vector<Biome*> gRandomScatteredLargeFeatureBiomes;

RandomScatteredLargeFeature::RandomScatteredLargeFeature() : StructureFeature()
{
    _init();
}

RandomScatteredLargeFeature::RandomScatteredLargeFeature(SuperflatConfig* config)
    : StructureFeature(),
      mSwamphutEnemies()
{
    _init();
    int spacing = (int)config->maybeOption(0x21, 0, (unsigned char)mSpacing);
    int minimum = mSeparation + 1;
    if (spacing > minimum)
    {
        minimum = spacing;
    }
    mSpacing = minimum;
}

RandomScatteredLargeFeature::~RandomScatteredLargeFeature()
{
    std::vector<Biome::MobSpawnerData*>::iterator last = mSwamphutEnemies.end();
    for (std::vector<Biome::MobSpawnerData*>::iterator it = mSwamphutEnemies.begin(); it != last; ++it)
    {
        delete *it;
    }
}

void RandomScatteredLargeFeature::staticCtor()
{
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::DESERT);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::DESERT_HILLS);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::JUNGLE);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::JUNGLE_HILLS);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::SWAMPLAND);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::ICE_PLAINS);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::COLD_TAIGA);
}

void RandomScatteredLargeFeature::_init()
{
    mSpacing = 0x20;
    mSeparation = 8;
    mSwamphutEnemies.push_back(new Biome::MobSpawnerData(1, EntityType::WITCH, 1, 1));
}

std::wstring RandomScatteredLargeFeature::getFeatureName()
{
    return std::wstring(L"Temple");
}

bool RandomScatteredLargeFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    int spacing;
    if (flatWorld)
    {
        spacing = mSpacing;
    }
    else
    {
        mSpacing = 0x10;
        spacing = 0x10;
    }
    int originalX = chunkX;
    int originalZ = chunkZ;
    if (chunkX < 0)
    {
        chunkX -= spacing - 1;
    }
    if (chunkZ < 0)
    {
        chunkZ -= spacing - 1;
    }
    int regionX = chunkX / spacing;
    int regionZ = chunkZ / spacing;
    Random* random = level->getRandomFor(regionX, regionZ, 0xDB1471);
    regionX *= mSpacing;
    regionZ *= mSpacing;
    regionX += random->nextInt(mSpacing - mSeparation);
    regionZ += random->nextInt(mSpacing - mSeparation);
    chunkX = originalX;
    chunkZ = originalZ;
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if ((options != NULL && options->isFeatureChunk(chunkX, chunkZ, eFeatureType_ScatteredFeature, NULL))
        || (chunkX == regionX && chunkZ == regionZ))
    {
        Biome* biome = level->getBiomeSource()->getBiome(BlockPos(chunkX * 16 + 8, 0, chunkZ * 16 + 8));
        for (std::vector<Biome*>::const_iterator it = gRandomScatteredLargeFeatureBiomes.begin();
             it != gRandomScatteredLargeFeatureBiomes.end(); ++it)
        {
            if (biome == *it)
            {
                return true;
            }
        }
    }
    return false;
}

BlockPos* RandomScatteredLargeFeature::getNearestGeneratedFeature(Level* levelIn, const BlockPos& pos,
                                                                  bool skipExistingChunks)
{
    level = levelIn;
    return StructureFeature::findNearestFeaturePositionBySpacing(levelIn, this, pos, mSpacing, mSeparation, 0xDB1471,
                                                                 false, 100, skipExistingChunks);
}

StructureStart* RandomScatteredLargeFeature::createStructureStart(int chunkX, int chunkZ)
{
    return new ScatteredFeatureStart(level, random, chunkX, chunkZ);
}

bool RandomScatteredLargeFeature::isSwamphut(const BlockPos& pos)
{
    StructureStart* start = getStructureAt(pos);
    if (start == NULL || dynamic_cast<ScatteredFeatureStart*>(start) == NULL || start->mPieces.empty())
    {
        return false;
    }
    std::vector<StructurePiece*>::iterator first = start->mPieces.begin();
    ScatteredFeaturePieces::SwamplandHut* hut = first != start->mPieces.end() && *first != NULL
                                                    ? dynamic_cast<ScatteredFeaturePieces::SwamplandHut*>(*first)
                                                    : NULL;
    return hut != NULL;
}

std::vector<Biome::MobSpawnerData*>* RandomScatteredLargeFeature::getSwamphutEnemies()
{
    return &mSwamphutEnemies;
}

RandomScatteredLargeFeature::ScatteredFeatureStart::ScatteredFeatureStart(Level* levelIn, Random& randomIn, int chunkX,
                                                                          int chunkZ)
    : StructureStart(chunkX, chunkZ)
{
    Biome* biome = levelIn->getBiome(BlockPos(chunkX * 16 + 8, 0, chunkZ * 16 + 8));
    int x = chunkX * 16;
    int z = chunkZ * 16;
    if (biome == Biome::JUNGLE || biome == Biome::JUNGLE_HILLS)
    {
        mPieces.push_back(new ScatteredFeaturePieces::JunglePyramidPiece(levelIn, randomIn, x, z));
    }
    else if (biome == Biome::SWAMPLAND)
    {
        mPieces.push_back(new ScatteredFeaturePieces::SwamplandHut(levelIn, randomIn, x, z));
    }
    else if (biome == Biome::DESERT || biome == Biome::DESERT_HILLS)
    {
        mPieces.push_back(new ScatteredFeaturePieces::DesertPyramidPiece(levelIn, randomIn, x, z));
    }
    else if (biome == Biome::ICE_PLAINS || biome == Biome::COLD_TAIGA)
    {
        mPieces.push_back(new ScatteredFeaturePieces::Igloo(levelIn, randomIn, x, z));
    }
    calculateBoundingBox();
}

RandomScatteredLargeFeature::ScatteredFeatureStart::ScatteredFeatureStart() : StructureStart()
{
}

EStructureStart RandomScatteredLargeFeature::ScatteredFeatureStart::GetType()
{
    return eStructureStart_ScatteredFeature;
}

StructureStart* RandomScatteredLargeFeature::ScatteredFeatureStart::Create()
{
    return new ScatteredFeatureStart();
}
