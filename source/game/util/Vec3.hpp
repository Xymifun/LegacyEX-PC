#ifndef GAME_UTIL_VEC3_HPP
#define GAME_UTIL_VEC3_HPP

#include <string>

class AABB;
class Vec3i;

namespace phys
{

class Vec2;

}

class Vec3
{
public:
    class ThreadStorage
    {
    public:
        ThreadStorage();
        ~ThreadStorage();

        Vec3* mPool;
        int mNextIndex;
    };

    double x;
    double y;
    double z;

    static Vec3* ZERO;

    Vec3() {}

    Vec3(double x, double y, double z);
    Vec3(const Vec3i* pos);

    static void CreateNewThreadStorage();
    static void UseDefaultThreadStorage();
    static void ReleaseThreadStorage();

    static Vec3* newPermanent(double x, double y, double z);
    static Vec3* newPermanent(const Vec3* other);
    static Vec3* newPermanent(const Vec3i* pos);

    static void clearPool();
    static void resetPool();

    static Vec3* newTemp(double x, double y, double z);
    static Vec3* newTemp(const Vec3i* pos);
    static Vec3* newTemp(const Vec3* other);

    void set(double x, double y, double z);
    void set(const Vec3* other);

    Vec3* interpolateTo(Vec3* to, double part);
    Vec3* lerp(Vec3* to, double part);
    Vec3* vectorTo(Vec3* to);
    Vec3* normalize();
    double dot(Vec3* other);
    Vec3* cross(Vec3* other);

    Vec3* add(double x, double y, double z);
    Vec3* add(Vec3* other);
    Vec3* subtract(double x, double y, double z);
    Vec3* subtract(Vec3* other);
    void move(double x, double y, double z);

    double distanceTo(Vec3* other);
    double distanceToSqr(Vec3* other);
    double distanceToSqr(double x, double y, double z);
    double distanceTo(AABB* box);
    Vec3* scale(double factor);
    double length();
    double lengthSqr();

    Vec3* clipX(Vec3* to, double x);
    Vec3* clipY(Vec3* to, double y);
    Vec3* clipZ(Vec3* to, double z);

    bool equals(Vec3* other);

    Vec3* xRot(float angle);
    Vec3* yRot(float angle);
    Vec3* zRot(float angle);

    Vec3* multiply(double x, double y, double z);
    Vec3* multiply(double factor);
    Vec3* multiply(float factor);

    Vec3* closestPointOnLine(Vec3* from, Vec3* to);
    double distanceFromLine(Vec3* from, Vec3* to);

    std::wstring toString();

    static Vec3* directionFromRotation(float xRot, float yRot);
    static Vec3* directionFromRotation(phys::Vec2* rotation);

private:
    void _init(double x, double y, double z);

    static const int TEMP_POOL_SIZE = 1024;

    static unsigned int sThreadStorageTlsIndex;
    static ThreadStorage* sDefaultThreadStorage;
};

#endif
