#ifndef GAME_WORLD_BIOME_SWAMPBIOME_HPP
#define GAME_WORLD_BIOME_SWAMPBIOME_HPP

#include "game/world/biome/Biome.hpp"

class BlockPos;
class ChunkPrimer;
class Level;
class Random;

class SwampBiome : public Biome
{
public:
    SwampBiome(EBiomeIDs biomeId, BiomeProperties* properties);
    virtual AbstractTreeFeature* getTreeFeature(Random& random);
    virtual FlowerBlock::Flower* getRandomFlower(Random& random, const BlockPos& pos);
    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual unsigned int getGrassColor(const BlockPos& pos);
    virtual unsigned int getFoliageColor(const BlockPos& pos);
    virtual void buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise);
    virtual EBiomeBaseClass getBaseClass();

private:
    unsigned int mUnreferenced[3];
};

#endif
