#include "game/core/Rotate.hpp"
#include "game/core/Mth.hpp"

Rotate::Rotate(Synth* source, float angle) : mSource(source), mSin(Mth::sin(angle)), mCos(Mth::cos(angle))
{
}

Rotate::~Rotate()
{
}

double Rotate::getValue(double x, double y)
{
    return mSource->getValue(x * mCos + y * mSin, y * mCos - x * mSin);
}
