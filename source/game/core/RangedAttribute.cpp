#include "game/core/RangedAttribute.hpp"
#include "game/core/Mth.hpp"

RangedAttribute::RangedAttribute(Attribute* parentAttribute, eATTRIBUTE_ID id, double defaultValue, double minValue,
                                 double maxValue)
    : BaseAttribute(parentAttribute, id, defaultValue),
      minValue(minValue),
      maxValue(maxValue)
{
}

RangedAttribute::~RangedAttribute()
{
}

double RangedAttribute::sanitizeValue(double value)
{
    return Mth::clamp(value, this->minValue, this->maxValue);
}

double RangedAttribute::getMinValue()
{
    return this->minValue;
}

double RangedAttribute::getMaxValue()
{
    return this->maxValue;
}
