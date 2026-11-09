#ifndef GAME_CORE_PERLINNOISE_HPP
#define GAME_CORE_PERLINNOISE_HPP
#include "game/world/biome/arrayWithLength.hpp"
#include "game/core/Synth.hpp"
class ImprovedNoise;
class Random;
class PerlinNoise : public Synth
{
public:
    PerlinNoise(int levels);
    PerlinNoise(Random* random, int levels);
    virtual ~PerlinNoise();
    void init(Random* random, int levels);
    virtual double getValue(double x, double y);
    double getValue(double x, double y, double z);
    arrayWithLength<double> getRegion(arrayWithLength<double> values, int x, int z, int xSize, int zSize, double xScale,
                                      double zScale, double yScale);
    arrayWithLength<double> getRegion(arrayWithLength<double> values, int x, int y, int z, int xSize, int ySize,
                                      int zSize, double xScale, double yScale, double zScale);
    ImprovedNoise** noiseLevels;
    int levels;
};
#endif
