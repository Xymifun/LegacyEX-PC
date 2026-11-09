#ifndef GAME_CORE_SYNTH_HPP
#define GAME_CORE_SYNTH_HPP

#include "game/world/biome/arrayWithLength.hpp"

class Synth
{
public:
    virtual ~Synth() {}

    virtual double getValue(double x, double y) = 0;

    arrayWithLength<double> create(int width, int height);
};

#endif
