#include "game/data/ByteArrayTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"
#include "game/core/System.hpp"
#include <cstring>
#include <cwchar>

ByteArrayTag::ByteArrayTag()
{
    mOwnsData = false;
}

ByteArrayTag::ByteArrayTag(arrayWithLength<unsigned char> data, bool ownsData)
{
    mData = data;
    mOwnsData = ownsData;
}

ByteArrayTag::~ByteArrayTag()
{
    if (mOwnsData)
    {
        delete[] mData.data;
    }
}

void ByteArrayTag::write(DataOutput* output)
{
    output->writeInt(mData.length);
    output->write(mData);
}

void ByteArrayTag::load(DataInput* input, int depth)
{
    int length = input->readInt();

    if (mOwnsData)
    {
        delete[] mData.data;
    }
    else
    {
        mOwnsData = true;
    }

    mData = arrayWithLength<unsigned char>(length, true);
    input->readFully(mData);
}

unsigned char ByteArrayTag::getId()
{
    return 7;
}

std::wstring ByteArrayTag::toString()
{
    static wchar_t description[32];
    swprintf(description, 32, L"[%d bytes]", mData.length);
    return description;
}

bool ByteArrayTag::equals(Tag* other)
{
    if (Tag::equals(other))
    {
        ByteArrayTag* otherArray = static_cast<ByteArrayTag*>(other);
        return (mData.data == NULL && otherArray->mData.data == NULL)
               || (mData.data != NULL && mData.length == otherArray->mData.length
                   && std::memcmp(mData.data, otherArray->mData.data, mData.length) == 0);
    }
    return false;
}

ByteArrayTag* ByteArrayTag::copy()
{
    arrayWithLength<unsigned char> copiedData(mData.length, true);
    System::arraycopy(mData, 0, &copiedData, 0, mData.length);
    return new ByteArrayTag(copiedData, true);
}

arrayWithLength<unsigned char> ByteArrayTag::getAsByteArray()
{
    return mData;
}
