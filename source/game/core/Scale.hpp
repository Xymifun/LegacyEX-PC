#ifndef GAME_CORE_SCALE_HPP
#define GAME_CORE_SCALE_HPP

#include "game/core/Synth.hpp"

class Scale : public Synth
{
public:
    Scale(Synth* source, double scaleX, double scaleY);
    virtual ~Scale();

    virtual double getValue(double x, double y);

private:
    Synth* mSource;
    double mScaleX;
    double mScaleY;
};

#endif
