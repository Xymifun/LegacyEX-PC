#ifndef GAME_DATA_DOUBLETAG_HPP
#define GAME_DATA_DOUBLETAG_HPP

#include <string>

#include "game/data/NumericTag.hpp"

class DataInput;
class DataOutput;

class DoubleTag : public NumericTag
{
public:
    DoubleTag();
    DoubleTag(double value);

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual ~DoubleTag();
    virtual bool equals(Tag* other);
    virtual DoubleTag* copy();

    virtual long long getAsLong();
    virtual int getAsInt();
    virtual short getAsShort();
    virtual unsigned char getAsByte();
    virtual double getAsDouble();
    virtual float getAsFloat();

    double mValue;
};

#endif
