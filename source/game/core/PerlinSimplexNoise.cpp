#include "game/core/PerlinSimplexNoise.hpp"

#include "game/core/SimplexNoise.hpp"
#include "game/util/Random.hpp"
#include "game/core/MemSect.hpp"

PerlinSimplexNoise::PerlinSimplexNoise(int levels)
{
    Random random;
    init(&random, levels);
}

PerlinSimplexNoise::PerlinSimplexNoise(long long seed, int levels)
{
    Random random(seed);

    init(&random, levels);
}

PerlinSimplexNoise::PerlinSimplexNoise(Random* random, int levels)
{
    init(random, levels);
}

PerlinSimplexNoise::~PerlinSimplexNoise()
{
    for (int i = 0; i < levels; i++)
    {
        delete noiseLevels[i];
    }

    delete[] noiseLevels;
}

void PerlinSimplexNoise::init(Random* random, int levels)
{
    MemSect(3);

    this->levels = levels;
    noiseLevels = new SimplexNoise*[levels];

    for (int i = 0; i < levels; i++)
    {
        noiseLevels[i] = new SimplexNoise(random);
    }

    MemSect(0);
}

double PerlinSimplexNoise::getValue(double x, double y)
{
    double value = 0.0;
    double frequency = 1.0;

    for (int i = 0; i < levels; i++)
    {
        value += noiseLevels[i]->getValue(x * frequency, y * frequency) / frequency;

        frequency *= 0.5;
    }

    return value;
}

double PerlinSimplexNoise::getValue(double x, double y, double z)
{
    double value = 1.0;
    double frequency = 0.5;
    for (int i = 0; i < levels; i++)
    {
        value += noiseLevels[i]->getValue(x * frequency, y * frequency, z * frequency) / frequency;
        frequency *= 0.5;
    }
    return value;
}

arrayWithLength<double> PerlinSimplexNoise::getRegion(arrayWithLength<double> values, double x, double y, int xSize,
                                                      int ySize, double xScale, double yScale, double frequencyScale)
{
    return getRegion(values, x, y, xSize, ySize, xScale, yScale, frequencyScale, 0.5);
}

arrayWithLength<double> PerlinSimplexNoise::getRegion(arrayWithLength<double> values, double x, double y, int xSize,
                                                      int ySize, double xScale, double yScale, double frequencyScale,
                                                      double amplitudeScale)
{
    xScale /= 1.5;
    yScale /= 1.5;

    if (values.data == NULL || (int)values.length < xSize * ySize)
    {
        if (values.data != NULL)
        {
            delete[] values.data;
        }

        values = arrayWithLength<double>(xSize * ySize, true);
    }
    else
    {
        for (unsigned int i = 0; i < values.length; i++)
        {
            values.data[i] = 0.0;
        }
    }

    double amplitude = 1.0;
    double frequency = 1.0;

    for (int i = 0; i < levels; i++)
    {
        noiseLevels[i]->add(values, x, y, xSize, ySize, xScale * frequency, yScale * frequency, 0.55 / amplitude);

        frequency *= frequencyScale;
        amplitude *= amplitudeScale;
    }

    return values;
}

arrayWithLength<double> PerlinSimplexNoise::getRegion(arrayWithLength<double> values, double x, double y, double z,
                                                      int xSize, int ySize, int zSize, double xScale, double yScale,
                                                      double zScale)
{
    unsigned int required = (unsigned int)(xSize * ySize * zSize);
    if (values.data == NULL || values.length < required)
    {
        if (values.data != NULL)
        {
            delete[] values.data;
        }
        values = arrayWithLength<double>(required, true);
    }
    else
    {
        for (unsigned int i = 0; i < values.length; i++)
        {
            values.data[i] = 0.0;
        }
    }
    double frequency = 0.5;
    for (int i = 0; i < levels; i++)
    {
        noiseLevels[i]->add(values, x, y, z, xSize, ySize, zSize, xScale * frequency, yScale * frequency,
                            zScale * frequency, frequency);
        frequency *= 0.5;
    }
    return values;
}
