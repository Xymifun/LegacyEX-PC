#ifndef GAME_WORLD_BIOME_FJFEATUREBIOMEREQUIREMENTS_HPP
#define GAME_WORLD_BIOME_FJFEATUREBIOMEREQUIREMENTS_HPP

#include <cstddef>
#include <vector>

class Biome;

class FjFeatureBiomeRequirements
{
public:
    FjFeatureBiomeRequirements();
    FjFeatureBiomeRequirements(const std::vector<Biome*>* spawnBiomes, unsigned int spawnRadius,
                               const std::vector<Biome*>* allowedBiomes, unsigned int allowedRadius);

    const std::vector<Biome*>* spawnBiomes;
    unsigned int spawnRadius;
    const std::vector<Biome*>* allowedBiomes;
    unsigned int allowedRadius;
};

#endif
