#ifndef GAME_CORE_EMBOSS_HPP
#define GAME_CORE_EMBOSS_HPP

#include "game/core/Synth.hpp"

class Emboss : public Synth
{
public:
    Emboss(Synth* source);
    virtual ~Emboss();

    virtual double getValue(double x, double y);

private:
    Synth* mSource;
};

#endif
