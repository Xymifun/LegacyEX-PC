#ifndef GAME_DATA_TAG_HPP
#define GAME_DATA_TAG_HPP

#include <iosfwd>
#include <string>

class DataInput;
class DataOutput;

class Tag
{
public:
    enum
    {
        TAG_END = 0,
        TAG_BYTE = 1,
        TAG_SHORT = 2,
        TAG_INT = 3,
        TAG_LONG = 4,
        TAG_FLOAT = 5,
        TAG_DOUBLE = 6,
        TAG_BYTE_ARRAY = 7,
        TAG_STRING = 8,
        TAG_LIST = 9,
        TAG_COMPOUND = 10,
        TAG_INT_ARRAY = 11,
        TAG_LONG_ARRAY = 12,
        TAG_ANY_NUMERIC = 99
    };

    Tag();

    virtual void write(DataOutput* output) = 0;
    virtual void load(DataInput* input, int depth) = 0;
    virtual std::wstring toString() = 0;
    virtual unsigned char getId() = 0;
    virtual void print(const std::wstring& name, char* indent, std::wostream& out);
    virtual ~Tag() {}
    virtual bool equals(Tag* other);
    virtual Tag* copy() = 0;
    virtual void stripEmptyChildren();
    virtual bool isEmpty() const;
    virtual std::wstring getAsString();

    static const wchar_t* getTagTypeName(unsigned char id);
    static Tag* newTag(unsigned char id);
};

#endif
