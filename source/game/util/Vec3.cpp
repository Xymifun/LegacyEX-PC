#include "game/util/Vec3.hpp"

#include "game/core/Mth.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3i.hpp"
#include "game/util/phys__Vec2.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

#include <math.h>
#include <stddef.h>

unsigned int Vec3::sThreadStorageTlsIndex = 0;
Vec3::ThreadStorage* Vec3::sDefaultThreadStorage = NULL;

Vec3* Vec3::ZERO = new Vec3(0.0, 0.0, 0.0);

Vec3::ThreadStorage::ThreadStorage()
{
    mPool = new Vec3[TEMP_POOL_SIZE];
    mNextIndex = 0;
}

Vec3::ThreadStorage::~ThreadStorage()
{
    delete[] mPool;
}

void Vec3::CreateNewThreadStorage()
{
    ThreadStorage* storage = new ThreadStorage();

    if (sDefaultThreadStorage == NULL)
    {
        sThreadStorageTlsIndex = TlsAlloc();
        sDefaultThreadStorage = storage;
    }

    TlsSetValue(sThreadStorageTlsIndex, storage);
}

void Vec3::UseDefaultThreadStorage()
{
    TlsSetValue(sThreadStorageTlsIndex, sDefaultThreadStorage);
}

void Vec3::ReleaseThreadStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sThreadStorageTlsIndex));

    if (storage != sDefaultThreadStorage)
    {
        delete storage;
    }
}

void Vec3::_init(double x, double y, double z)
{
    this->x = x == -0.0 ? 0.0 : x;
    this->y = y == -0.0 ? 0.0 : y;
    this->z = z == -0.0 ? 0.0 : z;
}

Vec3::Vec3(double x, double y, double z)
{
    _init(x, y, z);
}

Vec3* Vec3::newPermanent(double x, double y, double z)
{
    return new Vec3(x, y, z);
}

Vec3* Vec3::newPermanent(const Vec3* other)
{
    return new Vec3(other->x, other->y, other->z);
}

void Vec3::clearPool()
{
}

void Vec3::resetPool()
{
}

void Vec3::set(double x, double y, double z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

Vec3* Vec3::newTemp(double x, double y, double z)
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sThreadStorageTlsIndex));

    Vec3* vec = &storage->mPool[storage->mNextIndex];
    vec->set(x, y, z);
    storage->mNextIndex = (storage->mNextIndex + 1) & (TEMP_POOL_SIZE - 1);

    return vec;
}

Vec3* Vec3::newTemp(const Vec3i* pos)
{
    return newTemp((double)pos->x, (double)pos->y, (double)pos->z);
}

Vec3* Vec3::newTemp(const Vec3* other)
{
    return newTemp(other->x, other->y, other->z);
}

void Vec3::set(const Vec3* other)
{
    x = other->x;
    y = other->y;
    z = other->z;
}

Vec3* Vec3::interpolateTo(Vec3* to, double part)
{
    return newTemp(x + (to->x - x) * part, y + (to->y - y) * part, z + (to->z - z) * part);
}

Vec3* Vec3::vectorTo(Vec3* to)
{
    return newTemp(to->x - x, to->y - y, to->z - z);
}

Vec3* Vec3::normalize()
{
    double len = sqrt(x * x + y * y + z * z);

    if (len < 1.0E-4)
    {
        return newTemp(ZERO);
    }

    return newTemp(x / len, y / len, z / len);
}

double Vec3::dot(Vec3* other)
{
    return x * other->x + y * other->y + z * other->z;
}

Vec3* Vec3::cross(Vec3* other)
{
    return newTemp(y * other->z - z * other->y, z * other->x - x * other->z, x * other->y - y * other->x);
}

Vec3* Vec3::add(double x, double y, double z)
{
    return newTemp(this->x + x, this->y + y, this->z + z);
}

Vec3* Vec3::subtract(double x, double y, double z)
{
    return add(-x, -y, -z);
}

Vec3* Vec3::subtract(Vec3* other)
{
    return subtract(other->x, other->y, other->z);
}

Vec3* Vec3::add(Vec3* other)
{
    return add(other->x, other->y, other->z);
}

void Vec3::move(double x, double y, double z)
{
    this->x += x;
    this->y += y;
    this->z += z;
}

double Vec3::distanceTo(Vec3* other)
{
    double dx = other->x - x;
    double dy = other->y - y;
    double dz = other->z - z;

    return sqrt(dx * dx + dy * dy + dz * dz);
}

double Vec3::distanceToSqr(Vec3* other)
{
    double dx = other->x - x;
    double dy = other->y - y;
    double dz = other->z - z;

    return dx * dx + dy * dy + dz * dz;
}

Vec3* Vec3::scale(double factor)
{
    return newTemp(x * factor, y * factor, z * factor);
}

double Vec3::length()
{
    return sqrt(x * x + y * y + z * z);
}

double Vec3::lengthSqr()
{
    return x * x + y * y + z * z;
}

Vec3* Vec3::clipX(Vec3* to, double x)
{
    double dx = to->x - this->x;
    double dy = to->y - this->y;
    double dz = to->z - this->z;

    if (dx * dx < 1.0E-7f)
    {
        return NULL;
    }

    double part = (x - this->x) / dx;

    if (part < 0.0 || part > 1.0)
    {
        return NULL;
    }

    return newTemp(this->x + dx * part, this->y + dy * part, this->z + dz * part);
}

Vec3* Vec3::clipY(Vec3* to, double y)
{
    double dx = to->x - this->x;
    double dy = to->y - this->y;
    double dz = to->z - this->z;

    if (dy * dy < 1.0E-7f)
    {
        return NULL;
    }

    double part = (y - this->y) / dy;

    if (part < 0.0 || part > 1.0)
    {
        return NULL;
    }

    return newTemp(this->x + dx * part, this->y + dy * part, this->z + dz * part);
}

Vec3* Vec3::clipZ(Vec3* to, double z)
{
    double dx = to->x - this->x;
    double dy = to->y - this->y;
    double dz = to->z - this->z;

    if (dz * dz < 1.0E-7f)
    {
        return NULL;
    }

    double part = (z - this->z) / dz;

    if (part < 0.0 || part > 1.0)
    {
        return NULL;
    }

    return newTemp(this->x + dx * part, this->y + dy * part, this->z + dz * part);
}

bool Vec3::equals(Vec3* other)
{
    if (other->x == x && other->y == y)
    {
        return other->z == z;
    }
    return false;
}

Vec3* Vec3::xRot(float angle)
{
    float cosAngle = cosf(angle);
    float sinAngle = sinf(angle);

    return newTemp(x, y * cosAngle + z * sinAngle, z * cosAngle - y * sinAngle);
}

Vec3* Vec3::yRot(float angle)
{
    float cosAngle = cosf(angle);
    float sinAngle = sinf(angle);

    return newTemp(x * cosAngle + z * sinAngle, y, z * cosAngle - x * sinAngle);
}

Vec3* Vec3::zRot(float angle)
{
    float cosAngle = cosf(angle);
    float sinAngle = sinf(angle);

    return newTemp(x * cosAngle + y * sinAngle, y * cosAngle - x * sinAngle, z);
}

Vec3* Vec3::multiply(double x, double y, double z)
{
    this->x *= x;
    this->y *= y;
    this->z *= z;
    return this;
}

Vec3* Vec3::multiply(double factor)
{
    return multiply(factor, factor, factor);
}

Vec3* Vec3::multiply(float factor)
{
    return multiply((double)factor);
}

double Vec3::distanceTo(AABB* box)
{
    if (box->contains(this))
    {
        return 0.0;
    }

    double dx = 0.0;

    if (x < box->mMinX)
    {
        dx = box->mMinX - x;
    }
    else if (x > box->mMaxX)
    {
        dx = x - box->mMaxX;
    }

    double dy = 0.0;

    if (y < box->mMinY)
    {
        dy = box->mMinY - y;
    }
    else if (y > box->mMaxY)
    {
        dy = y - box->mMaxY;
    }

    double dz = 0.0;

    if (z < box->mMinZ)
    {
        dz = box->mMinZ - z;
    }
    else if (z > box->mMaxZ)
    {
        dz = z - box->mMaxZ;
    }

    return sqrt(dx * dx + dy * dy + dz * dz);
}

Vec3* Vec3::directionFromRotation(float xRot, float yRot)
{
    float yaw = -yRot * Mth::DEG_TO_RAD - 3.1415927f;
    float cosYaw = cosf(yaw);
    float sinYaw = sinf(yaw);

    float pitch = -xRot * Mth::DEG_TO_RAD;
    float cosPitch = -cosf(pitch);
    float sinPitch = sinf(pitch);

    return new Vec3((double)(sinYaw * cosPitch), (double)sinPitch, (double)(cosYaw * cosPitch));
}

Vec3* Vec3::directionFromRotation(phys::Vec2* rotation)
{
    return directionFromRotation(rotation->x, rotation->y);
}
