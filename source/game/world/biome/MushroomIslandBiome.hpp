#ifndef GAME_WORLD_BIOME_MUSHROOMISLANDBIOME_HPP
#define GAME_WORLD_BIOME_MUSHROOMISLANDBIOME_HPP

#include "game/world/biome/Biome.hpp"

class MushroomIslandBiome : public Biome
{
public:
    MushroomIslandBiome(EBiomeIDs biomeId, BiomeProperties* properties);

    virtual EBiomeBaseClass getBaseClass();
};

#endif
