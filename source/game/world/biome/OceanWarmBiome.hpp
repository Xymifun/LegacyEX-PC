#ifndef GAME_WORLD_BIOME_OCEANWARMBIOME_HPP
#define GAME_WORLD_BIOME_OCEANWARMBIOME_HPP

#include "game/world/biome/Biome.hpp"

class OceanWarmBiome : public Biome
{
public:
    OceanWarmBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties);
    virtual float getCreatureProbability();
    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual EBiomeBaseClass getBaseClass();
    virtual TemperatureCategory getTemperatureCategory() const;

private:
    bool mDeep;
};

#endif
