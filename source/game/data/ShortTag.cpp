#include "game/data/ShortTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"

#include <wchar.h>

ShortTag::ShortTag()
{
}

ShortTag::ShortTag(int value)
{
    mValue = (short)value;
}

ShortTag::~ShortTag()
{
}

void ShortTag::write(DataOutput* output)
{
    output->writeShort(mValue);
}

void ShortTag::load(DataInput* input, int depth)
{
    mValue = input->readShort();
}

std::wstring ShortTag::toString()
{
    static wchar_t description[32];

    swprintf(description, 32, L"%d", mValue);

    return description;
}

unsigned char ShortTag::getId()
{
    return 2;
}

bool ShortTag::equals(Tag* other)
{
    if (Tag::equals(other))
    {
        ShortTag* tag = (ShortTag*)other;
        return mValue == tag->mValue;
    }
    return false;
}

ShortTag* ShortTag::copy()
{
    return new ShortTag(mValue);
}

long long ShortTag::getAsLong()
{
    return mValue;
}

int ShortTag::getAsInt()
{
    return mValue;
}

short ShortTag::getAsShort()
{
    return mValue;
}

unsigned char ShortTag::getAsByte()
{
    return (char)(mValue & 255);
}

double ShortTag::getAsDouble()
{
    return (double)mValue;
}

float ShortTag::getAsFloat()
{
    return (float)mValue;
}
