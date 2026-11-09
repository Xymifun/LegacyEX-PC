#ifndef GAME_CORE_RANGEDATTRIBUTE_HPP
#define GAME_CORE_RANGEDATTRIBUTE_HPP

#include "game/core/BaseAttribute.hpp"

class RangedAttribute : public BaseAttribute
{
public:
    RangedAttribute(Attribute* parentAttribute, eATTRIBUTE_ID id, double defaultValue, double minValue,
                    double maxValue);
    virtual ~RangedAttribute();

    virtual double sanitizeValue(double value);

    double getMinValue();
    double getMaxValue();

protected:
    double minValue;
    double maxValue;
};

#endif
