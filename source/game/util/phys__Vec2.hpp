#ifndef GAME_UTIL_PHYS_VEC2_HPP
#define GAME_UTIL_PHYS_VEC2_HPP

namespace phys
{

class Vec2
{
public:
    class ThreadStorage
    {
    public:
        ThreadStorage();
        ~ThreadStorage();

        Vec2* mPool;
        int mNextIndex;
    };

    float x;
    float y;

    Vec2() {}
    Vec2(float x, float y);

    static void CreateNewThreadStorage();
    static void UseDefaultThreadStorage();
    static void ReleaseThreadStorage();

    static Vec2* newPermanent(float x, float y);

    static void clearPool();
    static void resetPool();

    static Vec2* newTemp(float x, float y);

    Vec2* scale(float factor);
    double dot(Vec2* other);
    Vec2* add(Vec2* other);
    Vec2* add(float value);
    bool equals(Vec2* other);
    Vec2* normalized();
    double length();
    float lengthSquared();
    Vec2* negated();
    float distanceToSqr(Vec2* other);

private:
    static const int TEMP_POOL_SIZE = 1024;

    static unsigned int sThreadStorageTlsIndex;
    static ThreadStorage* sDefaultThreadStorage;
};

}

#endif
