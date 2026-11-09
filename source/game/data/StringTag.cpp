#include "game/data/StringTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"
#include "game/data/Tag.hpp"

StringTag::StringTag()
{
    mData = L"";
}

StringTag::StringTag(const std::wstring& data)
{
    mData = data;
}

void StringTag::write(DataOutput* output)
{
    output->writeUTF(mData);
}

void StringTag::load(DataInput* input, int depth)
{
    mData = input->readUTF();
}

std::wstring StringTag::toString()
{
    return mData;
}

unsigned char StringTag::getId()
{
    return 8;
}

bool StringTag::equals(Tag* other)
{
    if (Tag::equals(other))
    {
        StringTag* tag = (StringTag*)other;
        return (mData.empty() && tag->mData.empty()) || (!mData.empty() && mData.compare(tag->mData) == 0);
    }

    return false;
}

StringTag* StringTag::copy()
{
    return new StringTag(mData);
}

bool StringTag::isEmpty() const
{
    return mData.empty();
}

std::wstring StringTag::getAsString()
{
    return mData;
}
