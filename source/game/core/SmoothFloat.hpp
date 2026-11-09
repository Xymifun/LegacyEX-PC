#ifndef GAME_CORE_SMOOTHFLOAT_HPP
#define GAME_CORE_SMOOTHFLOAT_HPP

class SmoothFloat
{
public:
    SmoothFloat();

    float getNewDeltaValue(float amount, float scale);
    float getTargetValue();
    void reset();

private:
    float targetValue;
    float remainingValue;
    float lastAmount;
};

#endif
