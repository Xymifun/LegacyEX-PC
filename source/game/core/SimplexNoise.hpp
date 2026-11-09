#ifndef GAME_CORE_SIMPLEXNOISE_HPP
#define GAME_CORE_SIMPLEXNOISE_HPP

#include "game/world/biome/arrayWithLength.hpp"

class Random;

class SimplexNoise
{
public:
    SimplexNoise();
    SimplexNoise(Random* random);
    ~SimplexNoise();
    void init(Random* random);
    static int fastfloor(double value);
    static double dot(int* grad, double x, double y);
    static double dot(int* grad, double x, double y, double z);
    double getValue(double x, double y);
    double getValue(double x, double y, double z);
    void add(arrayWithLength<double> values, double x, double y, int xSize, int ySize, double xScale, double yScale,
             double noiseScale);
    void add(arrayWithLength<double> values, double x, double y, double z, int xSize, int ySize, int zSize,
             double xScale, double yScale, double zScale, double noiseScale);

    static int grad3[12][3];
    static double F2;
    static double G2;
    static const double F3;
    static const double G3;

    int* p;
    int pLength;
    int pMask;
    double xo;
    double yo;
    double zo;
};

#endif
