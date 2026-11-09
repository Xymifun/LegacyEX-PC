#include "game/world/biome/MutatedForestBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/generation/AbstractTreeFeature.hpp"
#include "game/world/generation/BirchFeature.hpp"

MutatedForestBiome::MutatedForestBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : ForestBiome(biomeId, eType_Birch, properties)
{
}

AbstractTreeFeature* MutatedForestBiome::getTreeFeature(Random& random)
{
    if (random.nextBoolean())
    {
        return new BirchFeature(false, true);
    }
    return new BirchFeature(false, false);
}
