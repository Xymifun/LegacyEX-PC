#include "game/core/Double.hpp"

union DoubleBits
{
    double value;
    long long bits;
};

long long Double::doubleToLongBits(double value)
{
    if (value != value)
    {
        DoubleBits nan;
        nan.value = value;
        return nan.bits;
    }
    else
    {
        DoubleBits converter;
        converter.value = value;
        return converter.bits;
    }
}
