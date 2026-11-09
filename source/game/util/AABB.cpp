#include "game/util/AABB.hpp"


#include "game/core/Direction.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/HitResult.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

#include <sstream>

static const int TEMP_POOL_SIZE = 1024;

static unsigned int sThreadStorageTlsIndex;
static AABB::ThreadStorage* sDefaultThreadStorage;

AABB::ThreadStorage::ThreadStorage()
{
    mPool = new AABB[TEMP_POOL_SIZE];
    mNextIndex = 0;
}

AABB::ThreadStorage::~ThreadStorage()
{
    delete[] mPool;
}

void AABB::CreateNewThreadStorage()
{
    ThreadStorage* storage = new ThreadStorage();

    if (sDefaultThreadStorage == NULL)
    {
        sThreadStorageTlsIndex = TlsAlloc();
        sDefaultThreadStorage = storage;
    }

    TlsSetValue(sThreadStorageTlsIndex, storage);
}

void AABB::UseDefaultThreadStorage()
{
    TlsSetValue(sThreadStorageTlsIndex, sDefaultThreadStorage);
}

void AABB::ReleaseThreadStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sThreadStorageTlsIndex));

    if (storage != sDefaultThreadStorage)
    {
        delete storage;
    }
}

AABB::AABB(double x0, double y0, double z0, double x1, double y1, double z1)
{
    mMinX = x1 < x0 ? x1 : x0;
    mMinY = y1 < y0 ? y1 : y0;
    mMinZ = z1 < z0 ? z1 : z0;
    mMaxX = x0 < x1 ? x1 : x0;
    mMaxY = y0 < y1 ? y1 : y0;
    mMaxZ = z0 < z1 ? z1 : z0;
}

AABB* AABB::newPermanent(double x0, double y0, double z0, double x1, double y1, double z1)
{
    return new AABB(x0, y0, z0, x1, y1, z1);
}

AABB* AABB::newPermanent(const BlockPos& pos)
{
    return newPermanent((double)pos.x, (double)pos.y, (double)pos.z, (double)(pos.x + 1), (double)(pos.y + 1),
                        (double)(pos.z + 1));
}

AABB* AABB::newPermanent(const BlockPos& from, const BlockPos& to)
{
    return newPermanent((double)from.x, (double)from.y, (double)from.z, (double)to.x, (double)to.y, (double)to.z);
}

AABB* AABB::newPermanent(const AABB* box)
{
    return newPermanent(box->mMinX, box->mMinY, box->mMinZ, box->mMaxX, box->mMaxY, box->mMaxZ);
}

void AABB::clearPool()
{
}

void AABB::resetPool()
{
}

AABB* AABB::set(double x0, double y0, double z0, double x1, double y1, double z1)
{
    mMinX = x1 < x0 ? x1 : x0;
    mMinY = y1 < y0 ? y1 : y0;
    mMinZ = z1 < z0 ? z1 : z0;
    mMaxX = x0 < x1 ? x1 : x0;
    mMaxY = y0 < y1 ? y1 : y0;
    mMaxZ = z0 < z1 ? z1 : z0;
    return this;
}

AABB* AABB::newTemp(double x0, double y0, double z0, double x1, double y1, double z1)
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sThreadStorageTlsIndex));

    AABB* box = &storage->mPool[storage->mNextIndex];
    box->set(x0, y0, z0, x1, y1, z1);
    storage->mNextIndex = (storage->mNextIndex + 1) & (TEMP_POOL_SIZE - 1);

    return box;
}

AABB* AABB::newTemp(const BlockPos& pos)
{
    return newTemp((double)pos.x, (double)pos.y, (double)pos.z, (double)(pos.x + 1), (double)(pos.y + 1),
                   (double)(pos.z + 1));
}

AABB* AABB::newTemp(const BlockPos& from, const BlockPos& to)
{
    return newTemp((double)from.x, (double)from.y, (double)from.z, (double)to.x, (double)to.y, (double)to.z);
}

AABB* AABB::newTemp(const AABB* box)
{
    return newTemp(box->mMinX, box->mMinY, box->mMinZ, box->mMaxX, box->mMaxY, box->mMaxZ);
}

AABB* AABB::setMaxY(double maxY)
{
    return new AABB(mMinX, mMinY, mMinZ, mMaxX, maxY, mMaxZ);
}

bool AABB::equals(const AABB* other) const
{
    if (!(other != NULL && other->mMinX == mMinX && other->mMinY == mMinY && other->mMinZ == mMinZ
          && other->mMaxX == mMaxX && other->mMaxY == mMaxY))
    {
        return false;
    }

    return other->mMaxZ == mMaxZ;
}

void AABB::set(const AABB* box)
{
    mMinX = box->mMinX;
    mMinY = box->mMinY;
    mMinZ = box->mMinZ;
    mMaxX = box->mMaxX;
    mMaxY = box->mMaxY;
    mMaxZ = box->mMaxZ;
}

AABB* AABB::contract(double x, double y, double z)
{
    double minX = mMinX;
    double minY = mMinY;
    double minZ = mMinZ;
    double maxX = mMaxX;
    double maxY = mMaxY;
    double maxZ = mMaxZ;

    if (x < 0.0)
    {
        minX -= x;
    }
    else if (x > 0.0)
    {
        maxX -= x;
    }

    if (y < 0.0)
    {
        minY -= y;
    }
    else if (y > 0.0)
    {
        maxY -= y;
    }

    if (z < 0.0)
    {
        minZ -= z;
    }
    else if (z > 0.0)
    {
        maxZ -= z;
    }

    return newTemp(minX, minY, minZ, maxX, maxY, maxZ);
}

AABB* AABB::expand(double x, double y, double z) const
{
    double minX = mMinX;
    double minY = mMinY;
    double minZ = mMinZ;
    double maxX = mMaxX;
    double maxY = mMaxY;
    double maxZ = mMaxZ;

    if (x < 0.0)
    {
        minX += x;
    }

    if (x > 0.0)
    {
        maxX += x;
    }

    if (y < 0.0)
    {
        minY += y;
    }

    if (y > 0.0)
    {
        maxY += y;
    }

    if (z < 0.0)
    {
        minZ += z;
    }

    if (z > 0.0)
    {
        maxZ += z;
    }

    return newTemp(minX, minY, minZ, maxX, maxY, maxZ);
}

AABB* AABB::grow(double x, double y, double z) const
{
    return newTemp(mMinX - x, mMinY - y, mMinZ - z, mMaxX + x, mMaxY + y, mMaxZ + z);
}

AABB* AABB::grow(double size) const
{
    return grow(size, size, size);
}

AABB* AABB::intersect(const AABB* other) const
{
    double minX = mMinX < other->mMinX ? other->mMinX : mMinX;
    double minY = mMinY < other->mMinY ? other->mMinY : mMinY;
    double minZ = mMinZ < other->mMinZ ? other->mMinZ : mMinZ;
    double maxX = other->mMaxX < mMaxX ? other->mMaxX : mMaxX;
    double maxY = other->mMaxY < mMaxY ? other->mMaxY : mMaxY;
    double maxZ = other->mMaxZ < mMaxZ ? other->mMaxZ : mMaxZ;

    return newTemp(minX, minY, minZ, maxX, maxY, maxZ);
}

AABB* AABB::minmax(const AABB* other)
{
    double minX = other->mMinX < mMinX ? other->mMinX : mMinX;
    double minY = other->mMinY < mMinY ? other->mMinY : mMinY;
    double minZ = other->mMinZ < mMinZ ? other->mMinZ : mMinZ;
    double maxX = other->mMaxX > mMaxX ? other->mMaxX : mMaxX;
    double maxY = other->mMaxY > mMaxY ? other->mMaxY : mMaxY;
    double maxZ = other->mMaxZ > mMaxZ ? other->mMaxZ : mMaxZ;

    return newTemp(minX, minY, minZ, maxX, maxY, maxZ);
}

AABB* AABB::minmax(double x0, double y0, double z0, double x1, double y1, double z1)
{
    return newTemp(x1 < x0 ? x1 : x0, y1 < y0 ? y1 : y0, z1 < z0 ? z1 : z0, x0 < x1 ? x1 : x0, y0 < y1 ? y1 : y0,
                   z0 < z1 ? z1 : z0);
}

AABB* AABB::move(double x, double y, double z) const
{
    return newTemp(mMinX + x, mMinY + y, mMinZ + z, mMaxX + x, mMaxY + y, mMaxZ + z);
}

AABB* AABB::move(const BlockPos& pos) const
{
    double x = (double)pos.x;
    double y = (double)pos.y;
    double z = (double)pos.z;

    return newTemp(mMinX + x, mMinY + y, mMinZ + z, mMaxX + x, mMaxY + y, mMaxZ + z);
}

AABB* AABB::move(const Vec3& vec) const
{
    return move(vec.x, vec.y, vec.z);
}

double AABB::clipXCollide(const AABB* other, double xa) const
{
    if (other->mMaxY > mMinY && other->mMinY < mMaxY && other->mMaxZ > mMinZ && other->mMinZ < mMaxZ)
    {
        if (xa > 0.0 && other->mMaxX <= mMinX)
        {
            double clipped = mMinX - other->mMaxX;

            if (clipped < xa)
            {
                xa = clipped;
            }
        }
        else if (xa < 0.0 && other->mMinX >= mMaxX)
        {
            double clipped = mMaxX - other->mMinX;

            if (clipped > xa)
            {
                xa = clipped;
            }
        }
    }

    return xa;
}

double AABB::clipYCollide(const AABB* other, double ya) const
{
    if (other->mMaxX > mMinX && other->mMinX < mMaxX && other->mMaxZ > mMinZ && other->mMinZ < mMaxZ)
    {
        if (ya > 0.0 && other->mMaxY <= mMinY)
        {
            double clipped = mMinY - other->mMaxY;

            if (clipped < ya)
            {
                ya = clipped;
            }
        }
        else if (ya < 0.0 && other->mMinY >= mMaxY)
        {
            double clipped = mMaxY - other->mMinY;

            if (clipped > ya)
            {
                ya = clipped;
            }
        }
    }

    return ya;
}

double AABB::clipZCollide(const AABB* other, double za) const
{
    if (other->mMaxX > mMinX && other->mMinX < mMaxX && other->mMaxY > mMinY && other->mMinY < mMaxY)
    {
        if (za > 0.0 && other->mMaxZ <= mMinZ)
        {
            double clipped = mMinZ - other->mMaxZ;

            if (clipped < za)
            {
                za = clipped;
            }
        }
        else if (za < 0.0 && other->mMinZ >= mMaxZ)
        {
            double clipped = mMaxZ - other->mMinZ;

            if (clipped > za)
            {
                za = clipped;
            }
        }
    }

    return za;
}

bool AABB::intersectsInner(const AABB* other) const
{
    if (!(other->mMaxX >= mMinX && other->mMinX <= mMaxX && other->mMaxY >= mMinY && other->mMinY <= mMaxY
          && other->mMaxZ >= mMinZ && other->mMinZ <= mMaxZ))
    {
        return false;
    }

    return true;
}

bool AABB::intersects(Vec3* from, Vec3* to) const
{
    return intersects(to->x < from->x ? to->x : from->x, to->y < from->y ? to->y : from->y,
                      to->z < from->z ? to->z : from->z, from->x < to->x ? to->x : from->x,
                      from->y < to->y ? to->y : from->y, from->z < to->z ? to->z : from->z);
}

bool AABB::contains(double x, double y, double z) const
{
    if (!(x > mMinX && x < mMaxX && y > mMinY && y < mMaxY && z > mMinZ && z < mMaxZ))
    {
        return false;
    }

    return true;
}

bool AABB::contains(Vec3* pos) const
{
    return contains(pos->x, pos->y, pos->z);
}

bool AABB::contains(const BlockPos& pos) const
{
    return contains((double)pos.x, (double)pos.y, (double)pos.z);
}

bool AABB::containsIncludingLowerBound(Vec3* pos) const
{
    if (!(pos->x >= mMinX && pos->x < mMaxX && pos->y >= mMinY && pos->y < mMaxY && pos->z >= mMinZ && pos->z < mMaxZ))
    {
        return false;
    }

    return true;
}

double AABB::getSize() const
{
    return ((mMaxX - mMinX) + (mMaxY - mMinY) + (mMaxZ - mMinZ)) / 3.0;
}

AABB* AABB::shrink(double x, double y, double z) const
{
    return grow(-x, -y, -z);
}

AABB* AABB::shrink(double size) const
{
    return grow(-size);
}

bool AABB::insideYZ(Vec3* pos) const
{
    if (pos == NULL)
    {
        return false;
    }

    return mMinY <= pos->y && pos->y <= mMaxY && mMinZ <= pos->z && pos->z <= mMaxZ;
}

Vec3* AABB::collideWithXPlane(double x, Vec3* from, Vec3* to) const
{
    Vec3* hit = from->clipX(to, x);

    if (hit != NULL && insideYZ(hit))
    {
        return hit;
    }

    return NULL;
}

bool AABB::isClosest(Vec3* from, Vec3* current, Vec3* candidate) const
{
    bool closest = true;

    if (current != NULL)
    {
        double candidateDistance = from->distanceToSqr(candidate);
        double currentDistance = from->distanceToSqr(current);

        if (!(candidateDistance < currentDistance))
        {
            closest = false;
        }
    }

    return closest;
}

bool AABB::insideXZ(Vec3* pos) const
{
    if (pos == NULL)
    {
        return false;
    }

    return mMinX <= pos->x && pos->x <= mMaxX && mMinZ <= pos->z && pos->z <= mMaxZ;
}

Vec3* AABB::collideWithYPlane(double y, Vec3* from, Vec3* to) const
{
    Vec3* hit = from->clipY(to, y);

    if (hit != NULL && insideXZ(hit))
    {
        return hit;
    }

    return NULL;
}

bool AABB::insideXY(Vec3* pos) const
{
    if (pos == NULL)
    {
        return false;
    }

    return mMinX <= pos->x && pos->x <= mMaxX && mMinY <= pos->y && pos->y <= mMaxY;
}

Vec3* AABB::collideWithZPlane(double z, Vec3* from, Vec3* to) const
{
    Vec3* hit = from->clipZ(to, z);

    if (hit != NULL && insideXY(hit))
    {
        return hit;
    }

    return NULL;
}

HitResult* AABB::clip(Vec3* from, Vec3* to) const
{
    Vec3* hit = collideWithXPlane(mMinX, from, to);
    const Direction* face = Direction::WEST;

    Vec3* candidate = collideWithXPlane(mMaxX, from, to);

    if (candidate != NULL && isClosest(from, hit, candidate))
    {
        hit = candidate;
        face = Direction::EAST;
    }

    candidate = collideWithYPlane(mMinY, from, to);

    if (candidate != NULL && isClosest(from, hit, candidate))
    {
        hit = candidate;
        face = Direction::DOWN;
    }

    candidate = collideWithYPlane(mMaxY, from, to);

    if (candidate != NULL && isClosest(from, hit, candidate))
    {
        hit = candidate;
        face = Direction::UP;
    }

    candidate = collideWithZPlane(mMinZ, from, to);

    if (candidate != NULL && isClosest(from, hit, candidate))
    {
        hit = candidate;
        face = Direction::NORTH;
    }

    candidate = collideWithZPlane(mMaxZ, from, to);

    if (candidate != NULL && isClosest(from, hit, candidate))
    {
        hit = candidate;
        face = Direction::SOUTH;
    }

    if (hit == NULL)
    {
        return NULL;
    }

    return new HitResult(hit, face);
}

bool AABB::hasNaN() const
{
    return mMinX != mMinX || mMinY != mMinY || mMinZ != mMinZ || mMaxX != mMaxX || mMaxY != mMaxY || mMaxZ != mMaxZ;
}

bool clip(AABB* box, float* plane)
{
    int remaining = 6;

    do
    {
        double xMin = (double)plane[0] * box->mMinX;
        double yMin = (double)plane[1] * box->mMinY;
        double zMin = (double)plane[2] * box->mMinZ;
        double offset = (double)plane[3];

        if (xMin + yMin + zMin + offset <= 0.0)
        {
            double xMax = (double)plane[0] * box->mMaxX;
            double yMax = (double)plane[1] * box->mMaxY;
            double zMax = (double)plane[2] * box->mMaxZ;

            if (xMax + yMin + zMin + offset <= 0.0 && xMin + yMax + zMin + offset <= 0.0
                && xMax + yMax + zMin + offset <= 0.0 && xMin + yMin + zMax + offset <= 0.0
                && xMax + yMin + zMax + offset <= 0.0 && xMin + yMax + zMax + offset <= 0.0
                && xMax + yMax + zMax + offset <= 0.0)
            {
                return false;
            }
        }

        plane += 4;
        --remaining;
    } while (remaining != 0);

    return true;
}

bool clip(float* matrix, float* plane)
{
    int remaining = 6;

    do
    {
        float xMin = plane[0] * matrix[0];
        float yMin = plane[1] * matrix[1];
        float zMin = plane[2] * matrix[2];
        float offset = plane[3];

        if (xMin + yMin + zMin + offset <= 0.0f)
        {
            float xMax = plane[0] * matrix[3];
            float yMax = plane[1] * matrix[4];
            float zMax = plane[2] * matrix[5];

            if (xMax + yMin + zMin + offset <= 0.0f && xMin + yMax + zMin + offset <= 0.0f
                && xMax + yMax + zMin + offset <= 0.0f && xMin + yMin + zMax + offset <= 0.0f
                && xMax + yMin + zMax + offset <= 0.0f && xMin + yMax + zMax + offset <= 0.0f
                && xMax + yMax + zMax + offset <= 0.0f)
            {
                return false;
            }
        }

        plane += 4;
        --remaining;
    } while (remaining != 0);

    return true;
}

AABB* getOuterWall(const AABB* box, Direction::EDirection direction)
{
    AABB* wall = box->grow(10.0);

    switch (direction)
    {
    case Direction::eDirection_Down:
        wall->mMaxY = box->mMinY;
        break;
    case Direction::eDirection_Up:
        wall->mMinY = box->mMaxY;
        break;
    case Direction::eDirection_North:
        wall->mMaxZ = box->mMinZ + 1.0;
        break;
    case Direction::eDirection_South:
        wall->mMinZ = box->mMaxZ;
        break;
    case Direction::eDirection_West:
        wall->mMaxX = box->mMinX + 1.0;
        break;
    case Direction::eDirection_East:
        wall->mMinX = box->mMaxX;
        break;
    }

    return wall;
}

Vec3* AABB::getCenter() const
{
    return Vec3::newTemp(mMinX + (mMaxX - mMinX) * 0.5, mMinY + (mMaxY - mMinY) * 0.5, mMinZ + (mMaxZ - mMinZ) * 0.5);
}

AABB* AABB::scale(double x, double y, double z) const
{
    double centerX = mMinX + (mMaxX - mMinX) * 0.5;
    double centerY = mMinY + (mMaxY - mMinY) * 0.5;
    double centerZ = mMinZ + (mMaxZ - mMinZ) * 0.5;

    AABB* scaled = newTemp((mMinX - centerX) * x, (mMinY - centerY) * y, (mMinZ - centerZ) * z, (mMaxX - centerX) * x,
                           (mMaxY - centerY) * y, (mMaxZ - centerZ) * z);

    scaled->mMinX += centerX;
    scaled->mMinY += centerY;
    scaled->mMinZ += centerZ;
    scaled->mMaxX += centerX;
    scaled->mMaxY += centerY;
    scaled->mMaxZ += centerZ;

    return scaled;
}

AABB* AABB::scaleAndShift(double x, double y, double z) const
{
    double centerX = mMinX + (mMaxX - mMinX) * 0.5;
    double centerY = mMinY + (mMaxY - mMinY) * 0.5;
    double centerZ = mMinZ + (mMaxZ - mMinZ) * 0.5;

    AABB* scaled = newTemp((mMinX - centerX) * x, (mMinY - centerY) * y, (mMinZ - centerZ) * z, (mMaxX - centerX) * x,
                           (mMaxY - centerY) * y, (mMaxZ - centerZ) * z);

    double shiftY = mMaxY - scaled->mMaxY;

    scaled->mMinX += centerX;
    scaled->mMinY += shiftY;
    scaled->mMinZ += centerZ;
    scaled->mMaxX += centerX;
    scaled->mMaxY += shiftY;
    scaled->mMaxZ += centerZ;

    return scaled;
}

AABB* AABB::divideInternalsBy(double divisor)
{
    mMinX /= divisor;
    mMinY /= divisor;
    mMinZ /= divisor;
    mMaxX /= divisor;
    mMaxY /= divisor;
    mMaxZ /= divisor;
    return this;
}

void AABB::correctMinMax()
{
    double minX = mMinX;
    double minY = mMinY;
    double minZ = mMinZ;
    double maxX = mMaxX;
    double maxY = mMaxY;
    double maxZ = mMaxZ;

    mMinX = maxX < minX ? maxX : minX;
    mMinY = maxY < minY ? maxY : minY;
    mMinZ = maxZ < minZ ? maxZ : minZ;
    mMaxX = maxX < minX ? minX : maxX;
    mMaxY = maxY < minY ? minY : maxY;
    mMaxZ = maxZ < minZ ? minZ : maxZ;
}
