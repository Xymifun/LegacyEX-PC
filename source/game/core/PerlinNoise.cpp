#include "game/core/PerlinNoise.hpp"
#include "game/core/ImprovedNoise.hpp"
#include "game/util/Random.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Mth.hpp"
PerlinNoise::PerlinNoise(int levels)
{
    Random random(0);
    init(&random, levels);
}
PerlinNoise::PerlinNoise(Random* random, int levels)
{
    init(random, levels);
}
PerlinNoise::~PerlinNoise()
{
    for (int i = 0; i < levels; ++i)
        delete noiseLevels[i];
    delete[] noiseLevels;
}
void PerlinNoise::init(Random* random, int levels)
{
    MemSect(2);
    this->levels = levels;
    noiseLevels = new ImprovedNoise*[levels];
    for (int i = 0; i < levels; ++i)
        noiseLevels[i] = new ImprovedNoise(random);
    MemSect(0);
}
double PerlinNoise::getValue(double x, double y)
{
    double value = 0.0;
    double frequency = 1.0;
    for (int i = 0; i < levels; ++i)
    {
        value += noiseLevels[i]->getValue(x * frequency, y * frequency) / frequency;
        frequency *= 0.5;
    }
    return value;
}
double PerlinNoise::getValue(double x, double y, double z)
{
    double value = 0.0;
    double frequency = 1.0;
    for (int i = 0; i < levels; ++i)
    {
        value += noiseLevels[i]->getValue(x * frequency, y * frequency, z * frequency) / frequency;
        frequency *= 0.5;
    }
    return value;
}
arrayWithLength<double> PerlinNoise::getRegion(arrayWithLength<double> values, int x, int z, int xSize, int zSize,
                                               double xScale, double zScale, double yScale)
{
    return getRegion(values, x, 10, z, xSize, 1, zSize, xScale, 1.0, zScale);
}
arrayWithLength<double> PerlinNoise::getRegion(arrayWithLength<double> values, int x, int y, int z, int xSize,
                                               int ySize, int zSize, double xScale, double yScale, double zScale)
{
    if (values.data == NULL)
        values = arrayWithLength<double>((unsigned int)(xSize * ySize * zSize), true);
    else
    {
        for (unsigned int i = 0; i < values.length; ++i)
            values.data[i] = 0.0;
    }
    double frequency = 1.0;
    for (int i = 0; i < levels; ++i)
    {
        double xin = (double)x * frequency * xScale;
        double yin = (double)y * frequency * yScale;
        double zin = (double)z * frequency * zScale;
        long long xFloor = Mth::lfloor(xin);
        long long zFloor = Mth::lfloor(zin);
        xin -= (double)xFloor;
        zin -= (double)zFloor;
        xFloor %= 16777216LL;
        zFloor %= 16777216LL;
        xin += (double)xFloor;
        zin += (double)zFloor;
        noiseLevels[i]->add(values, xin, yin, zin, xSize, ySize, zSize, xScale * frequency, yScale * frequency,
                            zScale * frequency, frequency);
        frequency *= 0.5;
    }
    return values;
}
