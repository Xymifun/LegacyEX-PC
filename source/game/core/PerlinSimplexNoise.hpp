#ifndef GAME_CORE_PERLINSIMPLEXNOISE_HPP
#define GAME_CORE_PERLINSIMPLEXNOISE_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include "game/core/Synth.hpp"

class Random;
class SimplexNoise;

class PerlinSimplexNoise : public Synth
{
public:
    PerlinSimplexNoise(int levels);
    PerlinSimplexNoise(long long seed, int levels);
    PerlinSimplexNoise(Random* random, int levels);
    virtual ~PerlinSimplexNoise();

    void init(Random* random, int levels);

    virtual double getValue(double x, double y);
    double getValue(double x, double y, double z);

    arrayWithLength<double> getRegion(arrayWithLength<double> values, double x, double y, int xSize, int ySize,
                                      double xScale, double yScale, double frequencyScale);
    arrayWithLength<double> getRegion(arrayWithLength<double> values, double x, double y, int xSize, int ySize,
                                      double xScale, double yScale, double frequencyScale, double amplitudeScale);
    arrayWithLength<double> getRegion(arrayWithLength<double> values, double x, double y, double z, int xSize,
                                      int ySize, int zSize, double xScale, double yScale, double zScale);

    SimplexNoise** noiseLevels;
    int levels;
};

#endif
