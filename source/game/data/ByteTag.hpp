#ifndef GAME_DATA_BYTETAG_HPP
#define GAME_DATA_BYTETAG_HPP

#include <string>

#include "game/data/NumericTag.hpp"

class DataInput;
class DataOutput;

class ByteTag : public NumericTag
{
public:
    ByteTag();
    ByteTag(unsigned char value);

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual ~ByteTag();
    virtual bool equals(Tag* other);
    virtual ByteTag* copy();

    virtual long long getAsLong();
    virtual int getAsInt();
    virtual short getAsShort();
    virtual unsigned char getAsByte();
    virtual double getAsDouble();
    virtual float getAsFloat();

private:
    unsigned char mValue;
};

#endif
