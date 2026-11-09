#ifndef GAME_CORE_MTH_HPP
#define GAME_CORE_MTH_HPP

#include <string>
#include <vector>

#include "game/util/Random.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/Vec3i.hpp"

class Mth
{
public:
    static const float PI;
    static const float DEG_TO_RAD;
    static const float RAD_TO_DEG;

    static int cached_atan_index(int y, int x);
    static void init();

    static int abs(int value);
    static float cached_atan(int y, int x);
    static float sin(float radians);
    static float cos(float radians);
    static float atan(float value);
    static float atan2(int y, int x);
    static double fastInvSqrt(double value);
    static float sqrt(float value);
    static float sqrt(double value);
    static int floor(float value);
    static int floor(double value);
    static int ceil(float value);
    static long long lfloor(double value);
    static int absFloor(double value);
    static int fastFloor(double value);
    static float abs(float value);
    static double abs(double value);
    static bool isNaN(double value);

    static char clamp(char value, char min, char max);
    static int clamp(int value, int min, int max);
    static float clamp(float value, float min, float max);
    static double clamp(double value, double min, double max);

    static float Lerp(float from, float to, float t);
    static double Lerp(double from, double to, double t);
    static double clampedLerp(double from, double to, double t);
    static double asbMax(double a, double b);
    static double average(const std::vector<long>& values);
    static int intFloorDiv(int value, int divisor);

    static int nextInt(Random* random, int min, int max);
    static float nextFloat(Random* random, float min, float max);
    static double nextDouble(Random* random, double min, double max);

    static bool equal(float a, float b);
    static int positiveModulo(int value, int divisor);
    static float positiveModulo(float value, float divisor);
    static double positiveModulo(double value, double divisor);

    static int smallestEncompassingPowerOfTwo(int value);
    static int countTrailingZeros(int value);

    static float wrapDegrees(float degrees);
    static double wrapDegrees(double degrees);
    static int wrapDegrees(int degrees);
    static float clampRotateDegrees(float from, float to, float maxDelta);
    static float lerpRotateDegrees(float from, float to, float t);

    static std::wstring createInsecureUUID(Random* random);
    static std::wstring createInsecureUUID();

    static int getInt(const std::wstring& text, int defaultValue);
    static int getInt(const std::wstring& text, int defaultValue, int min);
    static double getDouble(const std::wstring& text, double defaultValue);
    static double getDouble(const std::wstring& text, double defaultValue, double min);

    static int color(int red, int green, int blue);
    static int color(float red, float green, float blue);
    static int colorMultiply(int colour, int other);
    static int colorMultiply(int colour, float red, float green, float blue);

    static double pct(double value, double min, double max);
    static bool isPowerOfTwo(int value);
    static int ceillog2(int value);
    static int log2(int value);
    static int roundUp(int value, int multiple);

    static float frac(float value);
    static double frac(double value);

    static Vec3 catmullRomSplinePos(Vec3 a, Vec3 b, Vec3 c, Vec3 d, double t);

    static long long getSeed(int x, int y, int z);
    static long long getSeed(Vec3i pos);
    static float ceil(double value, double step);
    static bool rayIntersectsAABB(Vec3 from, Vec3 to, AABB box);

    static float subtractFromLarger(float a, float b);
    static float degreesDifference(float a, float b);
    static float approach(float value, float target, float step);
    static float approachDegrees(float value, float target, float step);
    static float degreesDifferenceAbs(float a, float b);
    static bool almostEquals(double a, double b, double epsilon);

    static int murmurHash3Mixer(int value);
    static long long murmurHash3Mixer(long long value);

    static int hsvToRgb(float hue, float saturation, float value);

    template <typename T>
    static void shuffle(std::vector<T>* values, Random& random);
};

inline int Mth::floor(float value)
{
    int truncated = (int)value;
    return value < (float)truncated ? truncated - 1 : truncated;
}

inline int Mth::floor(double value)
{
    int truncated = (int)value;
    return value < (double)truncated ? truncated - 1 : truncated;
}

inline int Mth::absFloor(double value)
{
    int truncated = static_cast<int>(value);
    return value < static_cast<double>(truncated) ? truncated - 1 : truncated;
}

inline bool Mth::isNaN(double value)
{
    return value != value;
}

inline int Mth::ceil(float value)
{
    int truncated = (int)value;
    return value > (float)truncated ? truncated + 1 : truncated;
}

template <typename T>
void Mth::shuffle(std::vector<T>* values, Random& random)
{
    if (values->empty())
        return;

    for (int i = (int)values->size() - 1; i > 0; --i)
    {
        int j = random.nextInt(i);

        if (i != j)
        {
            T swapped = values->at(j);
            values->at(j) = values->at(i);
            values->at(i) = swapped;
        }
    }
}

#endif
