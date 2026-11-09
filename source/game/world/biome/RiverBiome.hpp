#ifndef GAME_WORLD_BIOME_RIVERBIOME_HPP
#define GAME_WORLD_BIOME_RIVERBIOME_HPP

#include "game/world/biome/Biome.hpp"

class RiverBiome : public Biome
{
public:
    RiverBiome(EBiomeIDs biomeId, BiomeProperties* properties);

    virtual EBiomeBaseClass getBaseClass();
};
#endif
