#include "game/data/Tag.hpp"

#include "game/data/ByteArrayTag.hpp"
#include "game/data/ByteTag.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DoubleTag.hpp"
#include "game/data/EndTag.hpp"
#include "game/data/FloatTag.hpp"
#include "game/data/IntArrayTag.hpp"
#include "game/data/IntTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/LongTag.hpp"
#include "game/data/ShortTag.hpp"
#include "game/data/StringTag.hpp"

#include <new>
#include <ostream>
#include <string>

static const int NUM_TAG_TYPES = 12;

static const wchar_t* TAG_TYPE_NAMES[NUM_TAG_TYPES + 1] = {
    L"TAG_End",        L"TAG_Byte",   L"TAG_Short", L"TAG_Int",      L"TAG_Long",      L"TAG_Float", L"TAG_Double",
    L"TAG_Byte_Array", L"TAG_String", L"TAG_List",  L"TAG_Compound", L"TAG_Int_Array", L"UNKNOWN"};

Tag::Tag()
{
    Tag* obj = this;
    if (obj != NULL || (obj = static_cast<Tag*>(operator new(4)), obj != NULL))
    {
    }
}

const wchar_t* Tag::getTagTypeName(unsigned char id)
{
    switch (id)
    {
    case TAG_END:
    case TAG_BYTE:
    case TAG_SHORT:
    case TAG_INT:
    case TAG_LONG:
    case TAG_FLOAT:
    case TAG_DOUBLE:
    case TAG_BYTE_ARRAY:
    case TAG_STRING:
    case TAG_LIST:
    case TAG_COMPOUND:
    case TAG_INT_ARRAY:
        return TAG_TYPE_NAMES[id];
    case TAG_ANY_NUMERIC:
        return L"Any Numeric Tag";
    default:
        return TAG_TYPE_NAMES[NUM_TAG_TYPES];
    }
}

void Tag::print(const std::wstring& name, char* indent, std::wostream& out)
{
    out << indent;
    out << getTagTypeName(getId());

    if (name.length() > 0)
    {
        out << L"(\"" << name << L"\")";
    }

    out << L": ";
    out << toString() << std::endl;
}

Tag* Tag::newTag(unsigned char id)
{
    switch (id)
    {
    case TAG_END:
        return new EndTag();
    case TAG_BYTE:
        return new ByteTag();
    case TAG_SHORT:
        return new ShortTag();
    case TAG_INT:
        return new IntTag();
    case TAG_LONG:
        return new LongTag();
    case TAG_FLOAT:
        return new FloatTag();
    case TAG_DOUBLE:
        return new DoubleTag();
    case TAG_BYTE_ARRAY:
        return new ByteArrayTag();
    case TAG_INT_ARRAY:
        return new IntArrayTag();
    case TAG_STRING:
        return new StringTag();
    case TAG_LIST:
        return new ListTag();
    case TAG_COMPOUND:
        return new CompoundTag();
    }

    return NULL;
}

bool Tag::equals(Tag* other)
{
    if (other == NULL || getId() != other->getId())
    {
        return false;
    }

    return true;
}

void Tag::stripEmptyChildren()
{
}

bool Tag::isEmpty() const
{
    return false;
}

std::wstring Tag::getAsString()
{
    return toString();
}
