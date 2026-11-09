#ifndef GAME_WORLD_BIOME_PLAINSBIOME_HPP
#define GAME_WORLD_BIOME_PLAINSBIOME_HPP

#include "game/world/biome/Biome.hpp"

class PlainsBiome : public Biome
{
public:
    PlainsBiome(EBiomeIDs biomeId, bool sunflower, BiomeProperties* properties);

    virtual AbstractTreeFeature* getTreeFeature(Random& random);
    virtual FlowerBlock::Flower* getRandomFlower(Random& random, const BlockPos& pos);
    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual EBiomeBaseClass getBaseClass();

private:
    bool mIsSunflower;
};

#endif
