#include "game/core/Emboss.hpp"

Emboss::Emboss(Synth* source) : mSource(source)
{
}

Emboss::~Emboss()
{
}

double Emboss::getValue(double x, double y)
{
    return mSource->getValue(x, y) - mSource->getValue(x + 1.0, y + 1.0);
}
