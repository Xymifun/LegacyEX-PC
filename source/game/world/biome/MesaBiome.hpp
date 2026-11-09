#ifndef GAME_WORLD_BIOME_MESABIOME_HPP
#define GAME_WORLD_BIOME_MESABIOME_HPP

#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeDecorator.hpp"

class BlockState;
class Level;
class PerlinSimplexNoise;
class Random;

class MesaBiome : public Biome
{
public:
    class MesaDecorator : public BiomeDecorator
    {
    public:
        virtual void decorateOres(Level* level, Random* random);
    };
    MesaBiome(EBiomeIDs id, bool brycePillars, bool hasForest, BiomeProperties* properties);
    virtual ~MesaBiome();
    static void staticCtor();
    virtual AbstractTreeFeature* getTreeFeature(Random& random);
    virtual unsigned int getFoliageColor(const BlockPos& pos);
    virtual unsigned int getGrassColor(const BlockPos& pos);
    virtual void buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise);
    virtual EBiomeBaseClass getBaseClass();
    BlockPair getBand(int x, int y, int z);
    void generateBands(long long seed);

private:
    BlockPair* mClayBands;
    long long mSeed;
    PerlinSimplexNoise* mPillarNoise;
    PerlinSimplexNoise* mPillarRoofNoise;
    PerlinSimplexNoise* mNoise;
    bool mBrycePillars;
    bool mHasForest;
};

#endif
