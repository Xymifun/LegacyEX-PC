#include "platform/wiiu/PlatformTiming.hpp"
#include "game/util/Random.hpp"

#include <math.h>


Random::Random()
{
    _LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    counter.QuadPart += 8682522807148012LL;
    setSeed(counter.QuadPart);
}

Random::Random(long long seed)
{
    setSeed(seed);
}

void Random::setSeed(long long seed)
{
    this->seed = (seed ^ 0x5DEECE66DLL) & 0xFFFFFFFFFFFFLL;
    haveNextNextGaussian = false;
}

int Random::next(int bits)
{
    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xFFFFFFFFFFFFLL;
    return (int)(seed >> (48 - bits));
}

int Random::nextInt()
{
    return next(32);
}

int Random::nextInt(int bound)
{
    if ((bound & -bound) == bound)
    {
        return (int)(((long long)bound * next(31)) >> 31);
    }

    int bits;
    int val;
    do
    {
        bits = next(31);
        val = bits % bound;
    } while (bits - val + (bound - 1) < 0);

    return val;
}

int Random::nextInt(int min, int max)
{
    if (min < max)
    {
        min += nextInt(max - min);
    }

    return min;
}

long long Random::nextLong()
{
    long long high = next(32);
    return (high << 32) + next(32);
}

bool Random::nextBoolean()
{
    return next(1) != 0;
}

float Random::nextFloat()
{
    return next(24) / (float)(1 << 24);
}

float Random::nextFloat(float max)
{
    return nextFloat() * max;
}

float Random::nextFloat(float min, float max)
{
    return nextFloat() * (max - min) + min;
}

double Random::nextDouble()
{
    return (((long long)next(26) << 27) + next(27)) / (double)(1LL << 53);
}

double Random::nextGaussian()
{
    if (haveNextNextGaussian)
    {
        haveNextNextGaussian = false;
        return nextNextGaussian;
    }

    double v1;
    double v2;
    double s;
    do
    {
        v1 = 2.0 * nextDouble() - 1.0;
        v2 = 2.0 * nextDouble() - 1.0;
        s = v1 * v1 + v2 * v2;
    } while (s >= 1.0 || s == 0.0);

    double multiplier = sqrt(-2.0 * log(s) / s);
    nextNextGaussian = v2 * multiplier;
    haveNextNextGaussian = true;
    return v1 * multiplier;
}

float Random::nextGaussianFloat()
{
    return nextFloat() - nextFloat();
}

int Random::nextGaussianInt(int bound)
{
    return nextInt(bound) - nextInt(bound);
}

bool Random::rollPercentageChance(int percentage)
{
    return nextInt(100) < percentage;
}

bool Random::rollPercentageChance(float percentage)
{
    return nextFloat() * 100.0f < percentage;
}

void Random::nextBytes(unsigned char* bytes, unsigned int length)
{
    unsigned int index = 0;
    while (index < length)
    {
        int randomValue = nextInt();
        unsigned int count = length - index;
        if (count > 4)
        {
            count = 4;
        }
        unsigned int i = 0;
        while (i < count)
        {
            bytes[index++] = (unsigned char)randomValue;
            randomValue >>= 8;
            ++i;
        }
    }
}

int randomBetween(Random& random, int min, int max)
{
    int upperBound = max;
    if (max < min)
    {
        upperBound = min;
        min = max;
    }

    return random.nextInt(upperBound - min) + min;
}
