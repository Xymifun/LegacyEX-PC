#include "game/util/phys__Vec2.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

#include <stddef.h>

#include <math.h>

unsigned int phys::Vec2::sThreadStorageTlsIndex = 0;
phys::Vec2::ThreadStorage* phys::Vec2::sDefaultThreadStorage = NULL;

phys::Vec2::ThreadStorage::ThreadStorage()
{
    mPool = new Vec2[TEMP_POOL_SIZE];
    mNextIndex = 0;
}

phys::Vec2::ThreadStorage::~ThreadStorage()
{
    delete[] mPool;
}

void phys::Vec2::CreateNewThreadStorage()
{
    ThreadStorage* storage = new ThreadStorage();

    if (sDefaultThreadStorage == NULL)
    {
        sThreadStorageTlsIndex = TlsAlloc();
        sDefaultThreadStorage = storage;
    }

    TlsSetValue(sThreadStorageTlsIndex, storage);
}

void phys::Vec2::UseDefaultThreadStorage()
{
    TlsSetValue(sThreadStorageTlsIndex, sDefaultThreadStorage);
}

void phys::Vec2::ReleaseThreadStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sThreadStorageTlsIndex));

    if (storage != sDefaultThreadStorage)
    {
        delete storage;
    }
}

phys::Vec2::Vec2(float x, float y) : x(x), y(y)
{
}

phys::Vec2* phys::Vec2::newPermanent(float x, float y)
{
    return new Vec2(x, y);
}

void phys::Vec2::clearPool()
{
}

void phys::Vec2::resetPool()
{
}

phys::Vec2* phys::Vec2::newTemp(float x, float y)
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sThreadStorageTlsIndex));

    Vec2* vec = &storage->mPool[storage->mNextIndex];
    vec->x = x;
    vec->y = y;
    storage->mNextIndex = (storage->mNextIndex + 1) & (TEMP_POOL_SIZE - 1);

    return vec;
}

phys::Vec2* phys::Vec2::scale(float factor)
{
    return newTemp(x * factor, y * factor);
}

double phys::Vec2::dot(Vec2* other)
{
    return (double)(x * other->x + y * other->y);
}

phys::Vec2* phys::Vec2::add(Vec2* other)
{
    return newTemp(x + other->x, y + other->y);
}

phys::Vec2* phys::Vec2::add(float value)
{
    return newTemp(x + value, y + value);
}

bool phys::Vec2::equals(Vec2* other)
{
    return other->x == x && other->y == y;
}

phys::Vec2* phys::Vec2::normalized()
{
    double len = length();

    if (len < 1.0E-4)
    {
        return newTemp(0.0f, 0.0f);
    }

    return newTemp((float)(x / len), (float)(y / len));
}

double phys::Vec2::length()
{
    return sqrt((double)(x * x + y * y));
}

float phys::Vec2::lengthSquared()
{
    return x * x + y * y;
}

phys::Vec2* phys::Vec2::negated()
{
    return newTemp(-x, -y);
}

float phys::Vec2::distanceToSqr(Vec2* other)
{
    float dx = other->x - x;
    float dy = other->y - y;
    return dx * dx + dy * dy;
}
