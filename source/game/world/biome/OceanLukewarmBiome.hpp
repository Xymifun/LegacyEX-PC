#ifndef GAME_WORLD_BIOME_OCEANLUKEWARMBIOME_HPP
#define GAME_WORLD_BIOME_OCEANLUKEWARMBIOME_HPP

#include "game/world/biome/Biome.hpp"

class OceanLukewarmBiome : public Biome
{
public:
    OceanLukewarmBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties);
    virtual float getCreatureProbability();
    virtual EBiomeBaseClass getBaseClass();
    virtual TemperatureCategory getTemperatureCategory() const;
};

#endif
