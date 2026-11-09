#ifndef GAME_CORE_FASTNOISE_HPP
#define GAME_CORE_FASTNOISE_HPP

#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class FastNoise
{
public:
    FastNoise(int seed);
    FastNoise(Random* random, int seed);
    ~FastNoise();

    void init(Random* random, int seed);
    arrayWithLength<double> getRegion(arrayWithLength<double> values, double x, double y, double z, int xSize,
                                      int ySize, int zSize, double xScale, double yScale, double zScale);
};

#endif
