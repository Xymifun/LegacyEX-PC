#ifndef GAME_WORLD_BIOME_MUTATEDSAVANNABIOME_HPP
#define GAME_WORLD_BIOME_MUTATEDSAVANNABIOME_HPP

#include "game/world/biome/SavannaBiome.hpp"

class MutatedSavannaBiome : public SavannaBiome
{
public:
    MutatedSavannaBiome(EBiomeIDs biomeId, BiomeProperties* properties);

    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual void buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise);
};

#endif
