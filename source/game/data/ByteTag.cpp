#include "game/data/ByteTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"

#include <wchar.h>

ByteTag::ByteTag()
{
}

ByteTag::ByteTag(unsigned char value)
{
    mValue = value;
}

ByteTag::~ByteTag()
{
}

void ByteTag::write(DataOutput* output)
{
    output->writeByte(mValue);
}

void ByteTag::load(DataInput* input, int depth)
{
    mValue = input->readByte();
}

std::wstring ByteTag::toString()
{
    static wchar_t description[32];

    swprintf(description, 32, L"%d", mValue);

    return description;
}

unsigned char ByteTag::getId()
{
    return 1;
}

bool ByteTag::equals(Tag* other)
{
    if (Tag::equals(other))
    {
        ByteTag* tag = (ByteTag*)other;
        return mValue == tag->mValue;
    }
    return false;
}

ByteTag* ByteTag::copy()
{
    return new ByteTag(mValue);
}

long long ByteTag::getAsLong()
{
    return mValue;
}

int ByteTag::getAsInt()
{
    return mValue;
}

short ByteTag::getAsShort()
{
    return mValue;
}

unsigned char ByteTag::getAsByte()
{
    return mValue;
}

double ByteTag::getAsDouble()
{
    return (double)mValue;
}

float ByteTag::getAsFloat()
{
    return (float)mValue;
}
