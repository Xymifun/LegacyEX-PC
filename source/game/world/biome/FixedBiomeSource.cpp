#include "game/world/biome/FixedBiomeSource.hpp"

#include <algorithm>

#include "game/core/MemSect.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"

FixedBiomeSource::FixedBiomeSource(Biome* biome, float rainfall) : BiomeSource(), mBiome(biome)
{
}

Biome* FixedBiomeSource::getBiome(const BlockPos& pos)
{
    return mBiome;
}

void FixedBiomeSource::getBiomeBlock(arrayWithLength<Biome*>& biomes, int x, int z, int width, int height,
                                     bool useCache) const
{
    int count = width * height;

    MemSect(36);
    biomes.resize(count);
    MemSect(0);

    for (Biome** it = biomes.data; it != biomes.data + count; it++)
    {
        *it = mBiome;
    }
}

void FixedBiomeSource::getBiomeIndexBlock(arrayWithLength<unsigned char>& biomeIndices, int x, int z, int width,
                                          int height, bool useCache) const
{
    int count = width * height;

    MemSect(36);
    biomeIndices.resize(count);
    MemSect(0);

    unsigned char* it = biomeIndices.data;
    unsigned char* end = it + count;

    for (; it != end; it++)
    {
        *it = (unsigned char)mBiome->id;
    }
}

void FixedBiomeSource::getRawBiomeBlock(arrayWithLength<Biome*>& biomes, int x, int z, int width, int height) const
{
    int count = width * height;

    MemSect(36);
    biomes.resize(count);
    MemSect(0);

    for (Biome** it = biomes.data; it != biomes.data + count; it++)
    {
        *it = mBiome;
    }
}

BlockPos* FixedBiomeSource::findBiome(int x, int z, int radius, std::vector<Biome*> biomes, Random* random)
{
    BlockPos* result = NULL;

    if (std::find(biomes.begin(), biomes.end(), mBiome) != biomes.end())
    {
        int offsetX = random->nextInt(radius * 2 + 1);
        int offsetZ = random->nextInt(radius * 2 + 1);

        result = new BlockPos((x - radius) + offsetX, 0, (z - radius) + offsetZ);
    }

    return result;
}

bool FixedBiomeSource::containsOnly(int x, int z, int radius, const std::vector<Biome*>& biomes)
{
    return std::find(biomes.begin(), biomes.end(), mBiome) != biomes.end();
}

bool FixedBiomeSource::isFixedBiome()
{
    return true;
}

Biome* FixedBiomeSource::getFixedBiome()
{
    return mBiome;
}
