#include "game/util/Math.hpp"

#include "game/util/Random.hpp"

#include <math.h>

static Random sRandom;

double Math::random()
{
    return sRandom.nextDouble();
}

int Math::randomInt(int bound)
{
    return sRandom.nextInt(bound);
}

long long Math::round(double value)
{
    return (long long)floor(value + 0.5);
}

int Math::round(float value)
{
    return (int)floor(value + 0.5);
}

int Math::_max(int a, int b)
{
    return (a >= b) ? a : b;
}

float Math::_max(float a, float b)
{
    return (a >= b) ? a : b;
}

int Math::_min(int a, int b)
{
    return (a <= b) ? a : b;
}

float Math::_min(float a, float b)
{
    return (a <= b) ? a : b;
}

float Math::wrapDegrees(float degrees)
{
    while (degrees >= 360.0f)
    {
        degrees -= 360.0f;
    }

    if (degrees >= 180.0f)
    {
        degrees -= 360.0f;
    }

    if (degrees < -180.0f)
    {
        degrees += 360.0f;
    }

    return degrees;
}

double Math::wrapDegrees(double degrees)
{
    while (degrees >= 360.0)
    {
        degrees -= 360.0;
    }

    if (degrees >= 180.0)
    {
        degrees -= 360.0;
    }

    if (degrees < -180.0)
    {
        degrees += 360.0;
    }

    return degrees;
}

int Math::roundUpToPowerOf2(int value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return value + 1;
}
