#include "game/world/biome/RiverBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/world/biome/BiomeDecorator.hpp"

static const int RIVER_SEAGRASS_PER_CHUNK = 12;

RiverBiome::RiverBiome(EBiomeIDs biomeId, BiomeProperties* properties) : Biome(biomeId, eBiomeType_River, properties)
{
    mCreatures.clear();
    mChickens.clear();
    mWolves.clear();

    mWaterCreatures.push_back(new MobSpawnerData(16, EntityType::SALMON, 3, 5));
    mMonsters.push_back(new MobSpawnerData(100, EntityType::DROWNED, 1, 1));

    mDecorator->seagrassPerChunk = RIVER_SEAGRASS_PER_CHUNK;
}

Biome::EBiomeBaseClass RiverBiome::getBaseClass()
{
    return eBiomeBaseClass_River;
}
