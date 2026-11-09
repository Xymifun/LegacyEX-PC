#ifndef GAME_WORLD_BIOME_STONEBEACHBIOME_HPP
#define GAME_WORLD_BIOME_STONEBEACHBIOME_HPP

#include "game/world/biome/Biome.hpp"

class StoneBeachBiome : public Biome
{
public:
    StoneBeachBiome(EBiomeIDs biomeId, BiomeProperties* properties);

    virtual EBiomeBaseClass getBaseClass();
};

#endif
