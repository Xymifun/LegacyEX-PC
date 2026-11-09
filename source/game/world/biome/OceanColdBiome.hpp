#ifndef GAME_WORLD_BIOME_OCEANCOLDBIOME_HPP
#define GAME_WORLD_BIOME_OCEANCOLDBIOME_HPP

#include "game/world/biome/Biome.hpp"

class OceanColdBiome : public Biome
{
public:
    OceanColdBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties);
    virtual float getCreatureProbability();
    virtual EBiomeBaseClass getBaseClass();
    virtual TemperatureCategory getTemperatureCategory() const;

private:
    bool mDeep;
};

#endif
