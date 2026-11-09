#ifndef GAME_CORE_IMPROVEDNOISE_HPP
#define GAME_CORE_IMPROVEDNOISE_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include "game/core/Synth.hpp"

class Random;

class ImprovedNoise : public Synth
{
public:
    ImprovedNoise();
    ImprovedNoise(Random* random);
    virtual ~ImprovedNoise();

    void init(Random* random);

    double noise(double x, double y, double z);

    virtual double getValue(double x, double y);
    double getValue(double x, double y, double z);

    void add(arrayWithLength<double> values, double x, double y, double z, int xSize, int ySize, int zSize,
             double xScale, double yScale, double zScale, double noiseScale);

    int* p;
    int pLength;
    int pMask;
    double xo;
    double yo;
    double zo;
};

#endif
