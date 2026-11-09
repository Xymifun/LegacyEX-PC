#ifndef GAME_DATA_NUMERICTAG_HPP
#define GAME_DATA_NUMERICTAG_HPP

#include "game/data/Tag.hpp"

class NumericTag : public Tag
{
public:
    virtual ~NumericTag() {}

    virtual long long getAsLong() = 0;
    virtual int getAsInt() = 0;
    virtual short getAsShort() = 0;
    virtual unsigned char getAsByte() = 0;
    virtual double getAsDouble() = 0;
    virtual float getAsFloat() = 0;
};

#endif
