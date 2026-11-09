#ifndef GAME_WORLD_BIOME_THEENDBIOME_HPP
#define GAME_WORLD_BIOME_THEENDBIOME_HPP

#include "game/world/biome/Biome.hpp"

class TheEndBiome : public Biome
{
public:
    TheEndBiome(EBiomeIDs biomeId, BiomeProperties* properties);

    virtual unsigned int getSkyColor(float temperature);
    virtual Vec3* getFogColor(float celestialAngle, float partialTicks) const;
    virtual EBiomeBaseClass getBaseClass();
    virtual bool isNatural();
};

#endif
