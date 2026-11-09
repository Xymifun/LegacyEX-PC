#include "game/core/SmoothFloat.hpp"

SmoothFloat::SmoothFloat()
{
    lastAmount = remainingValue = targetValue = 0.0f;
}

float SmoothFloat::getNewDeltaValue(float amount, float scale)
{
    float newTargetValue = targetValue + amount;
    float delta = (newTargetValue - remainingValue) * scale;
    float smoothedDelta = lastAmount + (delta - lastAmount) * 0.5f;
    targetValue = newTargetValue;
    lastAmount = smoothedDelta;
    if ((delta > 0.0f && delta > smoothedDelta) || (delta < 0.0f && delta < smoothedDelta))
    {
        delta = smoothedDelta;
    }
    remainingValue += delta;
    return delta;
}

float SmoothFloat::getTargetValue()
{
    return targetValue;
}

void SmoothFloat::reset()
{
    targetValue = 0.0f;
    remainingValue = 0.0f;
    lastAmount = 0.0f;
}
