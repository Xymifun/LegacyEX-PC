#ifndef GAME_WORLD_BIOME_JUNGLEBIOME_HPP
#define GAME_WORLD_BIOME_JUNGLEBIOME_HPP

#include "game/world/biome/Biome.hpp"

class JungleBiome : public Biome
{
public:
    JungleBiome(EBiomeIDs biomeId, bool isEdge, BiomeProperties* properties);

    static void staticCtor();
    virtual AbstractTreeFeature* getTreeFeature(Random& random);
    virtual Feature* getGrassFeature(Random& random);
    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual EBiomeBaseClass getBaseClass();

private:
    bool mIsEdge;
};

#endif
