#ifndef GAME_UTIL_RANDOMVALUEBOUNDS_HPP
#define GAME_UTIL_RANDOMVALUEBOUNDS_HPP

#include "game/core/Mth.hpp"

class Random;

class RandomValueBounds
{
public:
    float min;
    float max;

    RandomValueBounds();
    RandomValueBounds(int min, int max);
    RandomValueBounds(double min, double max);
    RandomValueBounds(int min, double max);
    RandomValueBounds(int value);
    RandomValueBounds(float min, float max);
    RandomValueBounds(float value);
    RandomValueBounds(double value);

    int getInt(Random* random) const { return Mth::nextInt(random, Mth::floor(min), Mth::floor(max)); }

    float getFloat(Random* random) const { return Mth::nextFloat(random, min, max); }
};

#endif
