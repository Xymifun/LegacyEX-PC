#ifndef GAME_UTIL_AABB_HPP
#define GAME_UTIL_AABB_HPP

#include <string>

#include "game/core/Direction.hpp"

class BlockPos;
class HitResult;
class Vec3;

class AABB
{
public:
    class ThreadStorage
    {
    public:
        ThreadStorage();
        ~ThreadStorage();

        AABB* mPool;
        int mNextIndex;
    };

    AABB() {}

    AABB(double x0, double y0, double z0, double x1, double y1, double z1);

    AABB(const AABB& other)
        : mMinX(other.mMinX),
          mMinY(other.mMinY),
          mMinZ(other.mMinZ),
          mMaxX(other.mMaxX),
          mMaxY(other.mMaxY),
          mMaxZ(other.mMaxZ)
    {
    }

    static void CreateNewThreadStorage();
    static void UseDefaultThreadStorage();
    static void ReleaseThreadStorage();

    static AABB* newPermanent(double x0, double y0, double z0, double x1, double y1, double z1);
    static AABB* newPermanent(const BlockPos& pos);
    static AABB* newPermanent(const BlockPos& from, const BlockPos& to);
    static AABB* newPermanent(const AABB* box);

    static AABB* newTemp(double x0, double y0, double z0, double x1, double y1, double z1);
    static AABB* newTemp(const BlockPos& pos);
    static AABB* newTemp(const BlockPos& from, const BlockPos& to);
    static AABB* newTemp(const AABB* box);

    static void clearPool();
    static void resetPool();

    AABB* set(double x0, double y0, double z0, double x1, double y1, double z1);
    void set(const AABB* box);

    AABB* setMinX(double minX);
    AABB* setMinY(double minY);
    AABB* setMinZ(double minZ);
    AABB* setMaxX(double maxX);
    AABB* setMaxY(double maxY);
    AABB* setMaxZ(double maxZ);

    bool equals(const AABB* other) const;

    AABB* contract(double x, double y, double z);
    AABB* contract(double x, double y, double z) const;
    AABB* expand(double x, double y, double z) const;
    AABB* grow(double x, double y, double z) const;
    AABB* grow(double size) const;
    AABB* growMax(double x, double y, double z) const;
    AABB* shrink(double x, double y, double z) const;
    AABB* shrink(double size) const;

    AABB* intersect(const AABB* other) const;
    AABB* getOverlap(const AABB* other) const;
    AABB* minmax(const AABB* other);
    static AABB* minmax(double x0, double y0, double z0, double x1, double y1, double z1);

    AABB* move(double x, double y, double z) const;
    AABB* move(const BlockPos& pos) const;
    AABB* move(const Vec3& vec) const;

    AABB* scale(double x, double y, double z) const;
    AABB* scaleAndShift(double x, double y, double z) const;

    double clipXCollide(const AABB* other, double xa) const;
    double clipYCollide(const AABB* other, double ya) const;
    double clipZCollide(const AABB* other, double za) const;

    bool intersects(double minX, double minY, double minZ, double maxX, double maxY, double maxZ) const
    {
        return mMinX < maxX && mMaxX > minX && mMinY < maxY && mMaxY > minY && mMinZ < maxZ && mMaxZ > minZ;
    }

    bool intersects(const AABB* other) const
    {
        return intersects(other->mMinX, other->mMinY, other->mMinZ, other->mMaxX, other->mMaxY, other->mMaxZ);
    }

    bool intersectsInner(const AABB* other) const;
    bool intersects(Vec3* from, Vec3* to) const;
    bool intersectsIgnoringDimensions(const AABB* other, bool ignoreX, bool ignoreY, bool ignoreZ) const;

    bool contains(double x, double y, double z) const;
    bool contains(Vec3* pos) const;
    bool contains(const BlockPos& pos) const;
    bool containsIncludingLowerBound(Vec3* pos) const;

    bool insideXY(Vec3* pos) const;
    bool insideXZ(Vec3* pos) const;
    bool insideYZ(Vec3* pos) const;
    bool isClosest(Vec3* from, Vec3* current, Vec3* candidate) const;
    bool hasNaN() const;

    Vec3* collideWithXPlane(double x, Vec3* from, Vec3* to) const;
    Vec3* collideWithYPlane(double y, Vec3* from, Vec3* to) const;
    Vec3* collideWithZPlane(double z, Vec3* from, Vec3* to) const;
    HitResult* clip(Vec3* from, Vec3* to) const;

    double getSize() const;
    Vec3* getCenter() const;

    AABB* divideInternalsBy(double divisor);
    void correctMinMax();

    std::wstring toString() const;

    double mMinX;
    double mMinY;
    double mMinZ;
    double mMaxX;
    double mMaxY;
    double mMaxZ;
};

bool clip(AABB* box, float* plane);
bool clip(float* matrix, float* plane);

AABB* getOuterWall(const AABB* box, Direction::EDirection direction);

#endif
