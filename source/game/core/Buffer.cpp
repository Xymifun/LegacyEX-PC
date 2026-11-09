#include "game/core/Buffer.hpp"

Buffer::Buffer(unsigned int capacity)
{
    mCapacity = capacity;
    mPosition = 0;
    mLimit = capacity;
    mIsWrapped = false;
}

void Buffer::clear()
{
    mPosition = 0;
    mLimit = mCapacity;
}

void Buffer::limit(unsigned int newLimit)
{
    mLimit = newLimit;

    if (mPosition > newLimit)
    {
        mPosition = newLimit;
    }
}

unsigned int Buffer::limit()
{
    return mLimit;
}

Buffer* Buffer::position(unsigned int newPosition)
{
    mPosition = newPosition;
    return this;
}

unsigned int Buffer::position()
{
    return mPosition;
}

int Buffer::remaining()
{
    return mLimit - mPosition;
}
