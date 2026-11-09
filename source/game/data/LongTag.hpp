#ifndef GAME_DATA_LONGTAG_HPP
#define GAME_DATA_LONGTAG_HPP

#include <string>

#include "game/data/NumericTag.hpp"

class DataInput;
class DataOutput;

class LongTag : public NumericTag
{
public:
    LongTag();
    LongTag(long long value);

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual ~LongTag();
    virtual bool equals(Tag* other);
    virtual LongTag* copy();

    virtual long long getAsLong();
    virtual int getAsInt();
    virtual short getAsShort();
    virtual unsigned char getAsByte();
    virtual double getAsDouble();
    virtual float getAsFloat();

private:
    long long mValue;
};

#endif
