#include "game/core/Distort.hpp"

Distort::Distort(Synth* source, Synth* distortion) : mSource(source), mDistortion(distortion)
{
}

Distort::~Distort()
{
}

double Distort::getValue(double x, double y)
{
    return mSource->getValue(x + mDistortion->getValue(x, y), y);
}
