#ifndef GAME_WORLD_BIOME_FIXEDBIOMESOURCE_HPP
#define GAME_WORLD_BIOME_FIXEDBIOMESOURCE_HPP

#include <vector>

#include "game/world/biome/BiomeSource.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Biome;
class BlockPos;
class Random;

class FixedBiomeSource : public BiomeSource
{
public:
    FixedBiomeSource(Biome* biome, float rainfall);


    virtual Biome* getBiome(const BlockPos& pos);
    virtual void getRawBiomeBlock(arrayWithLength<Biome*>& biomes, int x, int z, int width, int height) const;
    virtual void getBiomeBlock(arrayWithLength<Biome*>& biomes, int x, int z, int width, int height,
                               bool useCache) const;
    virtual void getBiomeIndexBlock(arrayWithLength<unsigned char>& biomeIndices, int x, int z, int width, int height,
                                    bool useCache) const;
    virtual bool containsOnly(int x, int z, int radius, const std::vector<Biome*>& biomes);
    virtual BlockPos* findBiome(int x, int z, int radius, std::vector<Biome*> biomes, Random* random);
    virtual bool isFixedBiome();
    virtual Biome* getFixedBiome();

    Biome* mBiome;
};

#endif
