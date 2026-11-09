#ifndef GAME_WORLD_BIOME_MUTATEDFORESTBIOME_HPP
#define GAME_WORLD_BIOME_MUTATEDFORESTBIOME_HPP

#include "game/world/biome/ForestBiome.hpp"

class AbstractTreeFeature;
class BlockPos;
class Level;
class Random;

class MutatedForestBiome : public ForestBiome
{
public:
    MutatedForestBiome(EBiomeIDs biomeId, BiomeProperties* properties);

    virtual AbstractTreeFeature* getTreeFeature(Random& random);
};

#endif
