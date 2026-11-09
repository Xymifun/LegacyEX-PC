#ifndef GAME_CORE_ROTATE_HPP
#define GAME_CORE_ROTATE_HPP

#include "game/core/Synth.hpp"

class Rotate : public Synth
{
public:
    Rotate(Synth* source, float angle);
    virtual ~Rotate();

    virtual double getValue(double x, double y);

private:
    Synth* mSource;
    double mSin;
    double mCos;
};

#endif
