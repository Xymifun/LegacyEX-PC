#include "game/core/Scale.hpp"

Scale::Scale(Synth* source, double scaleX, double scaleY) : mSource(source), mScaleX(scaleX), mScaleY(scaleY)
{
}

Scale::~Scale()
{
}

double Scale::getValue(double x, double y)
{
    return mSource->getValue(x * mScaleX, y * mScaleY);
}
