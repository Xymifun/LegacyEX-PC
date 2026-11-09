#include "game/world/biome/OceanColdBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/world/biome/BiomeDecorator.hpp"

OceanColdBiome::OceanColdBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Ocean, properties),
      mDeep(deep)
{
    clearDefaultFriendlies();

    mWaterCreatures.push_back(new MobSpawnerData(26, EntityType::SALMON, 3, 5));
    mWaterCreatures.push_back(new MobSpawnerData(75, EntityType::FISH, 4, 7));
    mSquid.push_back(new MobSpawnerData(10, EntityType::SQUID, 2, 4));
    mWaterCreatures.push_back(new MobSpawnerData(7, EntityType::DOLPHIN, 3, 5));
    mMonsters.push_back(new MobSpawnerData(10, EntityType::DROWNED, 2, 4));

    mDecorator->kelpPerChunk = 20;
    mDecorator->seagrassPerChunk = 12;

    mOceanRuinConfig.enabled = true;
    mOceanRuinConfig.biomeType = OceanRuin::eRuinType_Cold;

    if (mDeep)
    {
        mOceanRuinConfig.smallProbability = 0.5F;
        mOceanRuinConfig.clusterProbability = 0.95F;
    }
    else
    {
        mOceanRuinConfig.smallProbability = 0.3F;
        mOceanRuinConfig.clusterProbability = 0.9F;
    }
}

float OceanColdBiome::getCreatureProbability()
{
    return 0.1f;
}

Biome::EBiomeBaseClass OceanColdBiome::getBaseClass()
{
    return eBiomeBaseClass_OceanCold;
}

Biome::TemperatureCategory OceanColdBiome::getTemperatureCategory() const
{
    return eTemperatureCategory_Ocean;
}
