#ifndef GAME_WORLD_BIOME_VOIDBIOME_HPP
#define GAME_WORLD_BIOME_VOIDBIOME_HPP

#include "game/world/biome/Biome.hpp"

class VoidBiome : public Biome
{
public:
    VoidBiome(EBiomeIDs biomeId, BiomeProperties* properties);

    virtual EBiomeBaseClass getBaseClass();
    virtual bool isAlwaysValidSpawn();
};

#endif
