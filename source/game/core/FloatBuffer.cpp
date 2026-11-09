#include "game/core/FloatBuffer.hpp"

#include <cstring>

FloatBuffer::FloatBuffer(unsigned int capacity) : Buffer(capacity)
{
    mBuffer = new float[capacity];
    memset(mBuffer, 0, capacity * sizeof(float));
}

FloatBuffer::FloatBuffer(unsigned int capacity, float* data) : Buffer(capacity)
{
    mBuffer = data;
    mIsWrapped = true;
}

FloatBuffer::~FloatBuffer()
{
    if (!mIsWrapped)
    {
        delete[] mBuffer;
    }
}

FloatBuffer* FloatBuffer::flip()
{
    mLimit = mPosition;
    mPosition = 0;
    return this;
}

FloatBuffer* FloatBuffer::put(float value)
{
    mBuffer[mPosition++] = value;
    return this;
}

void FloatBuffer::put(int index, float value)
{
    mBuffer[index] = value;
}

FloatBuffer* FloatBuffer::put(arrayWithLength<float> values)
{
    unsigned int i;
    for (i = 0; i < values.length; ++i)
    {
        put(values.data[i]);
    }
    return this;
}

void FloatBuffer::get(arrayWithLength<float>* values)
{
    unsigned int length = values->length;
    for (unsigned int i = 0; i < length; i++)
    {
        values->data[i] = mBuffer[i];
    }
}

float FloatBuffer::get(int index)
{
    return mBuffer[index];
}
