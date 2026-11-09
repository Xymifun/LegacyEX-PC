#include "game/core/IntBuffer.hpp"

#include <cstring>

IntBuffer::IntBuffer(unsigned int capacity) : Buffer(capacity)
{
    mBuffer = new int[capacity];
    std::memset(mBuffer, 0, capacity * sizeof(int));
}

IntBuffer::IntBuffer(unsigned int capacity, int* data) : Buffer(capacity)
{
    mBuffer = data;
    mIsWrapped = true;
}

IntBuffer::~IntBuffer()
{
    if (!mIsWrapped)
    {
        delete[] mBuffer;
    }
}

IntBuffer* IntBuffer::flip()
{
    mLimit = mPosition;
    mPosition = 0;
    return this;
}

unsigned int IntBuffer::get(unsigned int index)
{
    return (unsigned int)mBuffer[index];
}

int* IntBuffer::getBuffer()
{
    return mBuffer;
}

IntBuffer* IntBuffer::put(arrayWithLength<int>* values, unsigned int offset, unsigned int length)
{
    std::memcpy(mBuffer + mPosition, values->data + offset, length * sizeof(int));
    mPosition += length;
    return this;
}

IntBuffer* IntBuffer::put(arrayWithLength<int> values)
{
    return put(&values, 0, values.length);
}

void IntBuffer::put(int value)
{
    mBuffer[mPosition++] = value;
}

void IntBuffer::put(int index, int value)
{
    mBuffer[index] = value;
}
