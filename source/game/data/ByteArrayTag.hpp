#ifndef GAME_DATA_BYTEARRAYTAG_HPP
#define GAME_DATA_BYTEARRAYTAG_HPP

#include <string>

#include "game/data/Tag.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInput;
class DataOutput;

class ByteArrayTag : public Tag
{
public:
    ByteArrayTag();
    ByteArrayTag(arrayWithLength<unsigned char> data, bool ownsData);

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual ~ByteArrayTag();
    virtual bool equals(Tag* other);
    virtual ByteArrayTag* copy();

    arrayWithLength<unsigned char> getAsByteArray();

private:
    arrayWithLength<unsigned char> mData;
    bool mOwnsData;
};

#endif
