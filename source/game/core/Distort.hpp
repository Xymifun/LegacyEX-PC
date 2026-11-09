#ifndef GAME_CORE_DISTORT_HPP
#define GAME_CORE_DISTORT_HPP

#include "game/core/Synth.hpp"

class Distort : public Synth
{
public:
    Distort(Synth* source, Synth* distortion);
    virtual ~Distort();

    virtual double getValue(double x, double y);

private:
    Synth* mSource;
    Synth* mDistortion;
};

#endif
