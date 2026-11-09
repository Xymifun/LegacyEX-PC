#include "game/core/ByteArrayOutputStream.hpp"

#include "game/core/XMem.hpp"

ByteArrayOutputStream::ByteArrayOutputStream()
{
    mCount = 0;
    mBuffer = arrayWithLength<unsigned char>(1024, true);
}

ByteArrayOutputStream::ByteArrayOutputStream(unsigned int size)
{
    mCount = 0;
    mBuffer = arrayWithLength<unsigned char>(size, true);
}

ByteArrayOutputStream::~ByteArrayOutputStream()
{
    delete[] mBuffer.data;
}

void ByteArrayOutputStream::write(unsigned int value)
{
    if (mCount + 1 >= mBuffer.length)
    {
        void* oldData = NULL;

        mBuffer.resizeNoDelete(mBuffer.length * 2, &oldData);

        delete[] (unsigned char*)oldData;
    }

    mBuffer.data[mCount] = (unsigned char)value;
    mCount++;
}

void ByteArrayOutputStream::write(arrayWithLength<unsigned char> data)
{
    write(data, 0, data.length);
}

void ByteArrayOutputStream::write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length)
{
    if (mCount + length >= mBuffer.length)
    {
        unsigned int requiredLength = mCount + length + 1;
        unsigned int newLength = mBuffer.length * 2;

        if (newLength < requiredLength)
        {
            newLength = requiredLength;
        }

        void* oldData = NULL;

        mBuffer.resizeNoDelete(newLength, &oldData);

        delete[] (unsigned char*)oldData;
    }

    XMemCpy(mBuffer.data + mCount, data.data + offset, length);
    mCount += length;
}

void ByteArrayOutputStream::close()
{
}

void ByteArrayOutputStream::flush()
{
}

arrayWithLength<unsigned char> ByteArrayOutputStream::toByteArray()
{
    unsigned int length = mBuffer.length;
    unsigned char* source = mBuffer.data;
    arrayWithLength<unsigned char> bytes(length, true);

    for (unsigned int i = 0; i < length; i++)
    {
        bytes.data[i] = source[i];
    }

    return bytes;
}
