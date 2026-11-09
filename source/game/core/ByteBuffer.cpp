#include "game/core/ByteBuffer.hpp"

#include <cstring>

#include "game/core/FloatBuffer.hpp"
#include "game/core/IntBuffer.hpp"

ByteBuffer::ByteBuffer(unsigned int capacity) : Buffer(capacity)
{
    mIsWrapped = false;
    mBuffer = new unsigned char[capacity];
    memset(mBuffer, 0, capacity);
    mOrder = eByteOrder_BigEndian;
}

ByteBuffer::~ByteBuffer()
{
    if (!mIsWrapped)
    {
        delete[] mBuffer;
    }
}

ByteBuffer* ByteBuffer::flip()
{
    mLimit = mPosition;
    mPosition = 0;
    return this;
}

ByteBuffer* ByteBuffer::allocate(unsigned int capacity)
{
    return new ByteBuffer(capacity);
}

ByteBuffer* ByteBuffer::allocateDirect(int capacity)
{
    return new ByteBuffer(capacity);
}

unsigned char* ByteBuffer::getBuffer()
{
    return mBuffer;
}

unsigned int ByteBuffer::getInt(unsigned int index)
{
    unsigned int result = 0;
    unsigned int first = mBuffer[index];
    unsigned int second = mBuffer[index + 1];
    unsigned int third = mBuffer[index + 2];
    unsigned int fourth = mBuffer[index + 3];

    if (mOrder == eByteOrder_BigEndian)
    {
        result = (first << 24) | (second << 16) | (third << 8) | fourth;
    }
    else if (mOrder == eByteOrder_LittleEndian)
    {
        result = first | (second << 8) | (third << 16) | (fourth << 24);
    }

    return result;
}

void ByteBuffer::putInt(unsigned int index, int value)
{
    if (mOrder == eByteOrder_BigEndian)
    {
        mBuffer[index] = (unsigned char)((value >> 24) & 0xFF);
        mBuffer[index + 1] = (unsigned char)((value >> 16) & 0xFF);
        mBuffer[index + 2] = (unsigned char)((value >> 8) & 0xFF);
        mBuffer[index + 3] = (unsigned char)(value & 0xFF);
    }
    else if (mOrder == eByteOrder_LittleEndian)
    {
        mBuffer[index] = (unsigned char)(value & 0xFF);
        mBuffer[index + 1] = (unsigned char)((value >> 8) & 0xFF);
        mBuffer[index + 2] = (unsigned char)((value >> 16) & 0xFF);
        mBuffer[index + 3] = (unsigned char)((value >> 24) & 0xFF);
    }
}

ByteBuffer* ByteBuffer::put(arrayWithLength<unsigned char> bytes)
{
    memcpy(mBuffer + mPosition, bytes.data, bytes.length);
    mPosition += bytes.length;

    return this;
}

IntBuffer* ByteBuffer::asIntBuffer()
{
    return new IntBuffer((mLimit - mPosition) >> 2, (int*)(mBuffer + mPosition));
}

FloatBuffer* ByteBuffer::asFloatBuffer()
{
    return new FloatBuffer((mLimit - mPosition) >> 2, (float*)(mBuffer + mPosition));
}
