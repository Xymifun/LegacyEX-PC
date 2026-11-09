#ifndef GAME_DATA_STRINGTAG_HPP
#define GAME_DATA_STRINGTAG_HPP

#include <string>

#include "game/data/Tag.hpp"

class DataInput;
class DataOutput;

class StringTag : public Tag
{
public:
    StringTag();
    StringTag(const std::wstring& data);

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual bool equals(Tag* other);
    virtual StringTag* copy();
    virtual bool isEmpty() const;
    virtual std::wstring getAsString();

    std::wstring mData;
};

#endif
