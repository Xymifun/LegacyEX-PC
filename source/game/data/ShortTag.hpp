#ifndef GAME_DATA_SHORTTAG_HPP
#define GAME_DATA_SHORTTAG_HPP

#include <string>

#include "game/data/NumericTag.hpp"

class DataInput;
class DataOutput;

class ShortTag : public NumericTag
{
public:
    ShortTag();
    ShortTag(int value);

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual ~ShortTag();
    virtual bool equals(Tag* other);
    virtual ShortTag* copy();

    virtual long long getAsLong();
    virtual int getAsInt();
    virtual short getAsShort();
    virtual unsigned char getAsByte();
    virtual double getAsDouble();
    virtual float getAsFloat();

    short mValue;
};

#endif
