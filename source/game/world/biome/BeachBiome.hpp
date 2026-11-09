#ifndef GAME_WORLD_BIOME_BEACHBIOME_HPP
#define GAME_WORLD_BIOME_BEACHBIOME_HPP

#include "game/world/biome/Biome.hpp"

class BeachBiome : public Biome
{
public:
    BeachBiome(EBiomeIDs biomeId, BiomeProperties* properties);

    virtual EBiomeBaseClass getBaseClass();
};

#endif
