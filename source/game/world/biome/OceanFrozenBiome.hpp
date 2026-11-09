#ifndef GAME_WORLD_BIOME_OCEANFROZENBIOME_HPP
#define GAME_WORLD_BIOME_OCEANFROZENBIOME_HPP

#include "game/world/biome/Biome.hpp"

class PerlinSimplexNoise;
class Random;

class OceanFrozenBiome : public Biome
{
public:
    OceanFrozenBiome(EBiomeIDs id, bool deep, BiomeProperties* properties);

    virtual float getCreatureProbability();
    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual void buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise);
    virtual float getTemperature(const BlockPos& pos);
    virtual EBiomeBaseClass getBaseClass();
    virtual TemperatureCategory getTemperatureCategory() const;

private:
    bool mDeep;
    long long mSeed;
    Random* mRandom;
    PerlinSimplexNoise* mIcebergsNoise;
    PerlinSimplexNoise* mIcebergsNoise2;
    PerlinSimplexNoise* mIcebergNoise;
};

#endif
