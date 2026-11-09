#ifndef GAME_WORLD_BIOME_SAVANNABIOME_HPP
#define GAME_WORLD_BIOME_SAVANNABIOME_HPP

#include "game/world/biome/Biome.hpp"

class SavannaBiome : public Biome
{
public:
    SavannaBiome(EBiomeIDs biomeId, BiomeProperties* properties);
    virtual ~SavannaBiome() {}

    virtual AbstractTreeFeature* getTreeFeature(Random& random);
    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual EBiomeBaseClass getBaseClass();
};

#endif
