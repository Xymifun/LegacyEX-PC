#ifndef GAME_WORLD_BIOME_HELLBIOME_HPP
#define GAME_WORLD_BIOME_HELLBIOME_HPP

#include "game/world/biome/Biome.hpp"

class HellBiome : public Biome
{
public:
    HellBiome(EBiomeIDs biomeId, BiomeProperties* properties);

    virtual Vec3* getFogColor(float celestialAngle, float partialTicks) const;
    virtual bool isFoggy();
    virtual bool isNatural();
    virtual unsigned int getSkyColor(float temperature);
    virtual EBiomeBaseClass getBaseClass();
};

#endif
