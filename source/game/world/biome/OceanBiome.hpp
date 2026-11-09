#ifndef GAME_WORLD_BIOME_OCEANBIOME_HPP
#define GAME_WORLD_BIOME_OCEANBIOME_HPP

#include "game/world/biome/Biome.hpp"

class OceanBiome : public Biome
{
public:
    OceanBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties);

    virtual float getCreatureProbability();
    virtual EBiomeBaseClass getBaseClass();
    virtual TemperatureCategory getTemperatureCategory() const;

private:
    bool mDeep;
};

#endif
