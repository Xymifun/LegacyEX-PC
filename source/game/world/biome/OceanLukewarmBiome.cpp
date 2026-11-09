#include "game/world/biome/OceanLukewarmBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/world/biome/BiomeDecorator.hpp"

OceanLukewarmBiome::OceanLukewarmBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Ocean, properties)
{
    clearDefaultFriendlies();

    mWaterCreatures.push_back(new MobSpawnerData(26, EntityType::SALMON, 3, 5));
    mWaterCreatures.push_back(new MobSpawnerData(75, EntityType::FISH, 4, 7));

    if (!deep)
    {
        mWaterCreatures.push_back(new MobSpawnerData(25, EntityType::PUFFERFISH, 3, 5));
        mWaterCreatures.push_back(new MobSpawnerData(75, EntityType::TROPICAL_FISH, 6, 10));
        mWaterCreatures.push_back(new MobSpawnerData(25, EntityType::TROPICAL_FISH, 3, 5));
    }

    mSquid.push_back(new MobSpawnerData(10, EntityType::SQUID, 2, 4));
    mWaterCreatures.push_back(new MobSpawnerData(7, EntityType::DOLPHIN, 3, 5));
    mMonsters.push_back(new MobSpawnerData(10, EntityType::DROWNED, 2, 4));

    mDecorator->kelpPerChunk = 20;
    mDecorator->seagrassPerChunk = 12;
    mOceanRuinConfig.enabled = true;
    mOceanRuinConfig.smallProbability = 0.3F;
    mOceanRuinConfig.clusterProbability = 0.9F;
    mOceanRuinConfig.biomeType = OceanRuin::eRuinType_Warm;
}

float OceanLukewarmBiome::getCreatureProbability()
{
    return 0.1F;
}

Biome::EBiomeBaseClass OceanLukewarmBiome::getBaseClass()
{
    return eBiomeBaseClass_OceanLukewarm;
}

Biome::TemperatureCategory OceanLukewarmBiome::getTemperatureCategory() const
{
    return eTemperatureCategory_Ocean;
}
