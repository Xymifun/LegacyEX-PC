#ifndef GAME_WORLD_BIOME_DESERTBIOME_HPP
#define GAME_WORLD_BIOME_DESERTBIOME_HPP

#include "game/world/biome/Biome.hpp"

class BlockPos;
class Level;
class Random;

class DesertBiome : public Biome
{
public:
    DesertBiome(EBiomeIDs biomeId, BiomeProperties* properties);

    virtual void decorate(Level* level, Random& random, const BlockPos& origin);
    virtual EBiomeBaseClass getBaseClass();
};

#endif
