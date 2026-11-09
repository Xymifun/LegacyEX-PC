#ifndef GAME_DATA_INTARRAYTAG_HPP
#define GAME_DATA_INTARRAYTAG_HPP

#include <string>

#include "game/data/Tag.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInput;
class DataOutput;

class IntArrayTag : public Tag
{
public:
    IntArrayTag();
    IntArrayTag(arrayWithLength<int> data);

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual ~IntArrayTag();
    virtual bool equals(Tag* other);
    virtual IntArrayTag* copy();

    arrayWithLength<int> getAsIntArray();

private:
    arrayWithLength<int> mData;
};

#endif
