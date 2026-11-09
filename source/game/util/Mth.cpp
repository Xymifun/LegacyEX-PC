#include "game/util/Mth.hpp"

float fclamp(float value, float min, float max)
{
    if (value < min)
    {
        return min;
    }

    return value > max ? max : value;
}
