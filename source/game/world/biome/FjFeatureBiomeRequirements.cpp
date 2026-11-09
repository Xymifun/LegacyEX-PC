#include "game/world/biome/FjFeatureBiomeRequirements.hpp"

FjFeatureBiomeRequirements::FjFeatureBiomeRequirements()
    : spawnBiomes(NULL),
      spawnRadius(0),
      allowedBiomes(NULL),
      allowedRadius(0)
{
}

FjFeatureBiomeRequirements::FjFeatureBiomeRequirements(const std::vector<Biome*>* spawnBiomes, unsigned int spawnRadius,
                                                       const std::vector<Biome*>* allowedBiomes,
                                                       unsigned int allowedRadius)
    : spawnBiomes(spawnBiomes),
      spawnRadius(spawnRadius),
      allowedBiomes(allowedBiomes),
      allowedRadius(allowedRadius)
{
}
