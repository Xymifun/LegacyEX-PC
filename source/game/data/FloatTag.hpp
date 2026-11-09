#ifndef GAME_DATA_FLOATTAG_HPP
#define GAME_DATA_FLOATTAG_HPP

#include <string>

#include "game/data/NumericTag.hpp"

class DataInput;
class DataOutput;

class FloatTag : public NumericTag
{
public:
    FloatTag();
    FloatTag(float value);

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual ~FloatTag();
    virtual bool equals(Tag* other);
    virtual FloatTag* copy();

    virtual long long getAsLong();
    virtual int getAsInt();
    virtual short getAsShort();
    virtual unsigned char getAsByte();
    virtual double getAsDouble();
    virtual float getAsFloat();

private:
    float mValue;
};

#endif
