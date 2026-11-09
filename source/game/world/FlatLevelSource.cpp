#include "game/world/FlatLevelSource.hpp"

#include <algorithm>
#include <cstring>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/Feature.hpp"
#include "game/world/generation/CanyonFeature.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"
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
#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/VillageFeature.hpp"
#include "game/world/generation/WoodlandMansionFeature.hpp"
#include "game/core/XMem.hpp"


FlatLevelSource::FlatLevelSource(Level* level, long long seed, bool generateStructures, SuperflatConfig* config)
{
    mOceanMonument = NULL;
    mLevelSize = level->getLevelData()->getXZSize();
    mLevel = level;
    mRandom = Random(seed);
    mPopulationRandom = Random(seed);
    FlatGeneratorInfo generatorInfo(config);
    mCanyons = NULL;
    mCaves = NULL;

    if (generateStructures)
    {
        mCaves = generatorInfo.getCaves();
        mCanyons = generatorInfo.getCanyons();

        if (generatorInfo.hasVillage())
        {
            mStructureFeatures[LargeFeature::VILLAGE] = generatorInfo.getVillage(mLevelSize);
        }
        if (generatorInfo.hasBiomeSpecific())
        {
            mStructureFeatures[LargeFeature::TEMPLE] = generatorInfo.getBiomeSpecific();
            mStructureFeatures[LargeFeature::RUIN] = new OceanRuinFeature(config);
        }
        if (generatorInfo.hasMineshaft())
        {
            mStructureFeatures[LargeFeature::MINESHAFT] = generatorInfo.getMineshaft();
        }
        if (generatorInfo.hasStronghold())
        {
            mStructureFeatures[LargeFeature::STRONGHOLD] = generatorInfo.getStronghold();
        }
        if (generatorInfo.hasOceanMonument())
        {
            mOceanMonument = generatorInfo.getOceanMonument();
            mStructureFeatures[LargeFeature::MONUMENT] = mOceanMonument;
        }
        if (generatorInfo.hasWoodlandMansion())
        {
            mStructureFeatures[LargeFeature::MANSION] = new WoodlandMansionFeature(this, config);
        }
        if (generatorInfo.hasShipwrecks())
        {
            mStructureFeatures[LargeFeature::SHIPWRECK] = generatorInfo.getShipwreck(this, mOceanMonument);
        }
    }

    mDecorations = generatorInfo.hasDecorations();
    mDungeons = generatorInfo.hasDungeons();
    mLakes = generatorInfo.getLakes();
    mLavaLakes = generatorInfo.getLavaLakes();

    std::memset(mLayers, 0, sizeof(mLayers));
    generatorInfo.getLayers(mLayers);

    std::vector<FlatLayerInfo> layers;
    generatorInfo.getLayers(layers);
    int seaLevel = 0;
    int airHeight = 0;
    for (std::vector<FlatLayerInfo>::iterator it = layers.begin(); it != layers.end(); ++it)
    {
        FlatLayerInfo layer = *it;
        for (int y = layer.getStart(); y < static_cast<int>(layer.getStart() + layer.getHeight()); ++y)
        {
            layer.getBlockState()->getBlock();
        }

        if (layer.getBlockState()->getBlock() == Blocks::air)
        {
            airHeight += layer.getHeight();
        }
        else
        {
            seaLevel += layer.getHeight() + airHeight;
            airHeight = 0;
        }
    }
    level->setSeaLevel(seaLevel);

    arrayWithLength<unsigned short> elements(65536, true);
    mElementIds = elements;
    ChunkPrimer primer(false, elements);
    for (int y = 0; y < 128; ++y)
    {
        const BlockState* state = mLayers[y];
        if (state != NULL)
        {
            for (int x = 0; x < 16; ++x)
            {
                for (int z = 0; z < 16; ++z)
                {
                    primer.setState(x, y, z, state);
                }
            }
        }
    }
}

FlatLevelSource::~FlatLevelSource()
{
    delete mCaves;
    delete mCanyons;

    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        delete feature->second;
    }

    delete mLakes;
    delete mLavaLakes;
    delete[] mElementIds.data;
}

void FlatLevelSource::prepareHeights(int, int, ChunkPrimer* primer)
{
    std::memcpy(primer->getElementIds().data, mElementIds.data, mElementIds.length * sizeof(unsigned short));
}

bool FlatLevelSource::isFlat()
{
    return true;
}

LevelChunk* FlatLevelSource::createChunk(int chunkX, int chunkZ)
{
    unsigned short* elementIds = static_cast<unsigned short*>(
        XPhysicalAlloc(mElementIds.length * sizeof(unsigned short), 0xFFFFFFFF, 0x1000, 4));
    arrayWithLength<unsigned short> elements(elementIds, mElementIds.length);
    XMemCpy(elementIds, mElementIds.data, elements.length * sizeof(unsigned short));
    ChunkPrimer primer(false, elements);

    if (mCaves != NULL)
    {
        mCaves->apply(mLevel, chunkX, chunkZ, &primer);
    }
    if (mCanyons != NULL)
    {
        mCanyons->apply(mLevel, chunkX, chunkZ, &primer);
    }

    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        if (feature->first.compare(LargeFeature::SHIPWRECK) != 0)
        {
            feature->second->apply(mLevel, chunkX, chunkZ, &primer);
        }
    }
    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        if (feature->first == LargeFeature::SHIPWRECK)
        {
            feature->second->apply(mLevel, chunkX, chunkZ, &primer);
        }
    }

    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    XPhysicalFree(elementIds);

    arrayWithLength<Biome*> biomes;
    mLevel->getBiomeSource()->getBiomeBlock(biomes, chunkX << 4, chunkZ << 4, 16, 16, true);
    arrayWithLength<unsigned char> biomeIds = chunk->getBiomes();
    for (unsigned int index = 0; index < biomeIds.length; ++index)
    {
        biomeIds.data[index] = static_cast<unsigned char>(biomes.data[index]->id);
    }
    delete[] biomes.data;

    chunk->recalcHeightmap();
    return chunk;
}

void FlatLevelSource::postProcess(int chunkX, int chunkZ)
{
    int x = chunkX * 16;
    int z = chunkZ * 16;
    BlockPos origin(x, 0, z);
    Biome* biome = mLevel->getBiome(BlockPos(x + 16, 0, z + 16));
    bool villageGenerated = false;

    mPopulationRandom.setSeed(mLevel->getSeed());
    long long xSeed = mPopulationRandom.nextLong() / 2 * 2 + 1;
    long long zSeed = mPopulationRandom.nextLong() / 2 * 2 + 1;
    mPopulationRandom.setSeed((static_cast<long long>(chunkX) * xSeed + static_cast<long long>(chunkZ) * zSeed)
                              ^ mLevel->getSeed());

    ChunkPos chunkPos(chunkX, chunkZ);
    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        bool generated = feature->second->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (dynamic_cast<VillageFeature*>(feature->second) != NULL)
        {
            bool allowed = std::find(VillageFeature::allowedBiomes.begin(), VillageFeature::allowedBiomes.end(), biome)
                           != VillageFeature::allowedBiomes.end();
            villageGenerated = villageGenerated || (generated && allowed);
        }
    }

    if (mLakes != NULL && !villageGenerated && mPopulationRandom.nextInt(4) == 0)
    {
        mLakes->place(mLevel, mPopulationRandom,
                      origin.offset(mPopulationRandom.nextInt(16) + 8, mPopulationRandom.nextInt(128),
                                    mPopulationRandom.nextInt(16) + 8));
    }

    if (mLavaLakes != NULL && !villageGenerated && mRandom.nextInt(8) == 0)
    {
        BlockPos pos = origin.offset(mPopulationRandom.nextInt(16) + 8,
                                     mPopulationRandom.nextInt(mPopulationRandom.nextInt(120) + 8),
                                     mPopulationRandom.nextInt(16) + 8);
        if (pos.y < mLevel->getSeaLevel() || mPopulationRandom.nextInt(10) == 0)
        {
            mLavaLakes->place(mLevel, mPopulationRandom, pos);
        }
    }

    if (mDungeons)
    {
        for (int index = 0; index < 8; ++index)
        {
            MonsterRoomFeature feature = MonsterRoomFeature();
            BlockPos pos = origin.offset(mPopulationRandom.nextInt(16) + 8, mPopulationRandom.nextInt(128),
                                         mPopulationRandom.nextInt(16) + 8);
            feature.place(mLevel, mPopulationRandom, pos);
        }
    }

    if (mDecorations)
    {
        biome->decorate(mLevel, mPopulationRandom, origin);
    }

    gApp.processSchematics(mLevel->getChunk(chunkX, chunkZ));
}

bool FlatLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
    return false;
}

std::vector<Biome::MobSpawnerData*>* FlatLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    Biome* biome = mLevel->getBiome(pos);
    if (mOceanMonument != NULL && category == MobCategory::MONSTER
        && mOceanMonument->isInsideBoundingFeature(mLevel, pos))
    {
        return mOceanMonument->getEnemies();
    }
    return biome->getMobs(category);
}

BlockPos* FlatLevelSource::findNearestMapFeature(Level* level, const std::wstring& featureName, const BlockPos& pos,
                                                 bool findUnexplored)
{
    if (LargeFeature::STRONGHOLD == featureName)
    {
        StructureMap::iterator feature = mStructureFeatures.find(featureName);
        if (feature != mStructureFeatures.end())
        {
            return feature->second->getNearestGeneratedFeature(level, pos, findUnexplored);
        }
    }
    if (LargeFeature::STRONGHOLD == featureName)
    {
        if (mStructureFeatures.find(LargeFeature::STRONGHOLD) != mStructureFeatures.end())
        {
            return mStructureFeatures.find(LargeFeature::STRONGHOLD)
                ->second->getNearestGeneratedFeature(level, pos, findUnexplored);
        }
    }
    if (LargeFeature::SHIPWRECK == featureName)
    {
        if (mStructureFeatures.find(LargeFeature::SHIPWRECK) != mStructureFeatures.end())
        {
            return mStructureFeatures.find(LargeFeature::SHIPWRECK)
                ->second->getNearestGeneratedFeature(level, pos, findUnexplored);
        }
    }
    if (LargeFeature::MANSION == featureName)
    {
        if (mStructureFeatures.find(LargeFeature::MANSION) != mStructureFeatures.end())
        {
            return mStructureFeatures.find(LargeFeature::MANSION)
                ->second->getNearestGeneratedFeature(level, pos, findUnexplored);
        }
    }
    if (LargeFeature::MONUMENT == featureName)
    {
        if (mStructureFeatures.find(LargeFeature::MONUMENT) != mStructureFeatures.end())
        {
            return mStructureFeatures.find(LargeFeature::MONUMENT)
                ->second->getNearestGeneratedFeature(level, pos, findUnexplored);
        }
    }
    if (LargeFeature::VILLAGE == featureName)
    {
        if (mStructureFeatures.find(LargeFeature::VILLAGE) != mStructureFeatures.end())
        {
            return mStructureFeatures.find(LargeFeature::VILLAGE)
                ->second->getNearestGeneratedFeature(level, pos, findUnexplored);
        }
    }
    if (LargeFeature::MINESHAFT == featureName)
    {
        if (mStructureFeatures.find(LargeFeature::MINESHAFT) != mStructureFeatures.end())
        {
            return mStructureFeatures.find(LargeFeature::MINESHAFT)
                ->second->getNearestGeneratedFeature(level, pos, findUnexplored);
        }
    }
    if (LargeFeature::TEMPLE == featureName)
    {
        if (mStructureFeatures.find(LargeFeature::TEMPLE) != mStructureFeatures.end())
        {
            return mStructureFeatures.find(LargeFeature::TEMPLE)
                ->second->getNearestGeneratedFeature(level, pos, findUnexplored);
        }
    }
    if (LargeFeature::BURIED_TREASURE == featureName)
    {
        if (mStructureFeatures.find(LargeFeature::BURIED_TREASURE) != mStructureFeatures.end())
        {
            return mStructureFeatures.find(LargeFeature::BURIED_TREASURE)
                ->second->getNearestGeneratedFeature(level, pos, findUnexplored);
        }
    }
    if (LargeFeature::RUIN == featureName)
    {
        if (mStructureFeatures.find(LargeFeature::RUIN) != mStructureFeatures.end())
        {
            return mStructureFeatures.find(LargeFeature::RUIN)
                ->second->getNearestGeneratedFeature(level, pos, findUnexplored);
        }
    }
    return NULL;
}

bool FlatLevelSource::isPosInFeature(Level*, const std::wstring& featureName, const BlockPos& pos)
{
    StructureMap::iterator feature = mStructureFeatures.find(featureName);
    if (feature != mStructureFeatures.end())
    {
        return feature->second->isInsideFeature(pos);
    }
    return false;
}

void FlatLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int chunkX, int chunkZ)
{
    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        feature->second->apply(mLevel, chunkX, chunkZ, NULL);
    }
}
