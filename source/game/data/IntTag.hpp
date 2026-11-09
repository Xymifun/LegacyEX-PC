#ifndef GAME_DATA_INTTAG_HPP
#define GAME_DATA_INTTAG_HPP

#include <string>

#include "game/data/NumericTag.hpp"

class DataInput;
class DataOutput;

class IntTag : public NumericTag
{
public:
    IntTag();
    IntTag(int value);

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual ~IntTag();
    virtual bool equals(Tag* other);
    virtual IntTag* copy();

    virtual long long getAsLong();
    virtual int getAsInt();
    virtual short getAsShort();
    virtual unsigned char getAsByte();
    virtual double getAsDouble();
    virtual float getAsFloat();

    int mValue;
};

#endif
