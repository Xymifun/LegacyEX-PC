#ifndef GAME_DATA_ENDTAG_HPP
#define GAME_DATA_ENDTAG_HPP

#include <string>

#include "game/data/Tag.hpp"

class DataInput;
class DataOutput;

class EndTag : public Tag
{
public:
    EndTag();

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual ~EndTag();
    virtual bool equals(Tag* other);
    virtual EndTag* copy();
};

#endif
