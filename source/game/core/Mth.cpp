#include "game/core/Mth.hpp"

#include <cmath>
#include <stdint.h>
#include <math.h>
#include <utility>

#include "game/util/Random.hpp"

const float Mth::PI = 3.1415927f;
const float Mth::DEG_TO_RAD = 0.017453292f;
const float Mth::RAD_TO_DEG = 57.295776f;

static const int ATAN_CACHE_SIZE = 40;

static const int MULTIPLY_DE_BRUIJN_BIT_POSITION[32] = {0,  1,  28, 2,  29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
                                                        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9};

static float* sSinTable = NULL;
static float* sAtanTable = NULL;
static Random sRandom;

int Mth::cached_atan_index(int y, int x)
{
    return y + x * ATAN_CACHE_SIZE;
}

void Mth::init()
{
    sSinTable = new float[65536];

    for (int i = 0; i < 65536; ++i)
    {
        float sine = ::sinf((float)i / 10430.378f);
        sSinTable[i] = sine;
    }

    sAtanTable = new float[cached_atan_index(ATAN_CACHE_SIZE, ATAN_CACHE_SIZE) + 1];

    for (int y = 0; y < ATAN_CACHE_SIZE; ++y)
    {
        for (int x = 0; x < ATAN_CACHE_SIZE; ++x)
        {
            sAtanTable[cached_atan_index(y, x)] = ::atanf((float)y / (float)x);
        }
    }
}

int Mth::abs(int value)
{
    return value < 0 ? -value : value;
}

float Mth::cached_atan(int y, int x)
{
    if (sAtanTable == NULL)
    {
        init();
    }

    const float* table = sAtanTable;
    bool flip = (y < 0) ^ (x < 0);
    int absY = abs(y);
    int absX = abs(x);
    int index = cached_atan_index(absY, absX);
    static const int tableSize = cached_atan_index(ATAN_CACHE_SIZE, ATAN_CACHE_SIZE) + 1;
    float result = table[index];

    if (flip)
    {
        result = -result;
    }

    return result;
}

float Mth::sin(float radians)
{
    if (sSinTable == NULL)
    {
        init();
    }

    return sSinTable[(int)(radians * 10430.378f) & 65535];
}

float Mth::cos(float radians)
{
    if (sSinTable == NULL)
    {
        init();
    }

    return sSinTable[(int)(radians * 10430.378f + 16384.0f) & 65535];
}

float Mth::atan2(int y, int x)
{
    int absY = abs(y);
    int absX = abs(x);

    if (absY > ATAN_CACHE_SIZE || absX > ATAN_CACHE_SIZE)
    {
        return ::atan2f((float)y, (float)x);
    }

    if (x > 0)
    {
        return cached_atan(y, x);
    }

    if (y > 0)
    {
        return 1.5707964f - cached_atan(x, y);
    }

    if (y < 0)
    {
        return -1.5707964f - cached_atan(x, y);
    }

    if (x < 0)
    {
        return cached_atan(0, x) + 3.1415927f;
    }

    return 0.0f;
}

double Mth::fastInvSqrt(double value)
{
    union
    {
        double asDouble;
        long long asBits;
    } converted;

    converted.asDouble = value;
    converted.asBits = 0x5FE6EB50C7B537AALL - (converted.asBits >> 1);

    return converted.asDouble * (1.5 - 0.5 * value * converted.asDouble * converted.asDouble);
}

float Mth::sqrt(float value)
{
    return ::sqrtf(value);
}

float Mth::sqrt(double value)
{
    return ::sqrt(value);
}

long long Mth::lfloor(double value)
{
    long long truncated = (long long)value;

    if (value < (double)truncated)
    {
        return truncated - 1;
    }

    return truncated;
}

int Mth::fastFloor(double value)
{
#ifdef MINECRAFT_PC
    int truncated = (int)value;
    return value < (double)truncated ? truncated - 1 : truncated;
#else
    return (int)(value + 1024.0) - 1024;
#endif
}

float Mth::abs(float value)
{
    return ::fabsf(value);
}

double Mth::abs(double value)
{
    return ::fabs(value);
}

int Mth::clamp(int value, int min, int max)
{
    if (value < min)
    {
        return min;
    }

    if (value > max)
    {
        return max;
    }

    return value;
}

float Mth::clamp(float value, float min, float max)
{
    if (value < min)
    {
        return min;
    }

    if (value > max)
    {
        return max;
    }

    return value;
}

double Mth::clamp(double value, double min, double max)
{
    if (value < min)
    {
        return min;
    }

    if (value > max)
    {
        return max;
    }

    return value;
}

float Mth::Lerp(float from, float to, float t)
{
    return (1.0f - t) * from + t * to;
}

double Mth::Lerp(double from, double to, double t)
{
    return (1.0 - t) * from + t * to;
}

double Mth::clampedLerp(double from, double to, double t)
{
    if (t < 0.0)
    {
        return from;
    }

    if (t > 1.0)
    {
        return to;
    }

    return (to - from) * t + from;
}

double Mth::asbMax(double a, double b)
{
    if (a < 0.0)
    {
        a = -a;
    }

    if (b < 0.0)
    {
        b = -b;
    }

    return a > b ? a : b;
}

int Mth::intFloorDiv(int value, int divisor)
{
    if (value < 0)
    {
        return -((-value - 1) / divisor) - 1;
    }

    return value / divisor;
}

int Mth::nextInt(Random* random, int min, int max)
{
    if (min < max)
    {
        min = random->nextInt(max - min + 1) + min;
    }

    return min;
}

float Mth::nextFloat(Random* random, float min, float max)
{
    if (min < max)
    {
        min = random->nextFloat() * (max - min) + min;
    }

    return min;
}

double Mth::nextDouble(Random* random, double min, double max)
{
    if (min < max)
    {
        min = random->nextDouble() * (max - min) + min;
    }

    return min;
}

bool Mth::equal(float a, float b)
{
    return abs(b - a) < 1.0E-5f;
}

float Mth::positiveModulo(float value, float divisor)
{
    return ::fmodf(::fmodf(value, divisor) + divisor, divisor);
}

double Mth::positiveModulo(double value, double divisor)
{
    return ::fmod(::fmod(value, divisor) + divisor, divisor);
}

int Mth::smallestEncompassingPowerOfTwo(int value)
{
    int result = value - 1;
    result |= result >> 1;
    result |= result >> 2;
    result |= result >> 4;
    result |= result >> 8;
    result |= result >> 16;

    return result + 1;
}

int Mth::countTrailingZeros(int value)
{
    return MULTIPLY_DE_BRUIJN_BIT_POSITION[((value & -value) * 0x077CB531 >> 27) & 31];
}

float Mth::wrapDegrees(float degrees)
{
    degrees = ::fmodf(degrees, 360.0f);

    if (degrees >= 180.0f)
    {
        degrees -= 360.0f;
    }

    if (degrees < -180.0f)
    {
        degrees += 360.0f;
    }

    return degrees;
}

double Mth::wrapDegrees(double degrees)
{
    degrees = ::fmod(degrees, 360.0);

    if (degrees >= 180.0)
    {
        degrees -= 360.0;
    }

    if (degrees < -180.0)
    {
        degrees += 360.0;
    }

    return degrees;
}

int Mth::wrapDegrees(int degrees)
{
    degrees = degrees % 360;

    if (degrees >= 180)
    {
        degrees -= 360;
    }

    if (degrees < -180)
    {
        degrees += 360;
    }

    return degrees;
}

float Mth::clampRotateDegrees(float from, float to, float maxDelta)
{
    float difference = wrapDegrees(to - from);

    if (difference > maxDelta)
    {
        difference = maxDelta;
    }

    if (difference < -maxDelta)
    {
        difference = -maxDelta;
    }

    float result = from + difference;

    if (result >= 180.0f)
    {
        result -= 360.0f;
    }

    if (result < -180.0f)
    {
        result += 360.0f;
    }

    return result;
}

float Mth::lerpRotateDegrees(float from, float to, float t)
{
    return from + t * wrapDegrees(to - from);
}

std::wstring Mth::createInsecureUUID(Random* random)
{
    wchar_t digits[33];
    digits[32] = 0;

    long long mostSignificantBits = (random->nextLong() & 0xFFFFFFFFFFFF0FFFLL) | 0x4000LL;
    long long leastSignificantBits = (random->nextLong() & 0x3FFFFFFFFFFFFFFFLL) | (long long)0x8000000000000000ULL;

    for (int i = 0; i < 16; ++i)
    {
        int high = (int)(mostSignificantBits & 0xF);
        int low = (int)(leastSignificantBits & 0xF);

        digits[31 - i] = (wchar_t)(low + (low > 9 ? 'a' - 10 : '0'));
        digits[15 - i] = (wchar_t)(high + (high > 9 ? 'a' - 10 : '0'));

        mostSignificantBits >>= 4;
        leastSignificantBits >>= 4;
    }

    return std::wstring(digits);
}

std::wstring Mth::createInsecureUUID()
{
    return createInsecureUUID(&sRandom);
}

double Mth::pct(double value, double min, double max)
{
    return (value - min) / (max - min);
}

bool Mth::isPowerOfTwo(int value)
{
    return value != 0 && (value & (value - 1)) == 0;
}

int Mth::ceillog2(int value)
{
    value = isPowerOfTwo(value) ? value : smallestEncompassingPowerOfTwo(value);

    return MULTIPLY_DE_BRUIJN_BIT_POSITION[(value * 0x077CB531 >> 27) & 31];
}

int Mth::log2(int value)
{
    return ceillog2(value) - (isPowerOfTwo(value) ? 0 : 1);
}

int Mth::roundUp(int value, int multiple)
{
    if (multiple == 0)
    {
        return 0;
    }

    if (value == 0)
    {
        return multiple;
    }

    if (value < 0)
    {
        multiple = -multiple;
    }

    int remainder = value % multiple;

    if (remainder == 0)
    {
        return value;
    }

    return value + multiple - remainder;
}

double Mth::frac(double value)
{
    int whole = (int)value;

    if (value < (double)whole)
    {
        --whole;
    }

    return value - (double)whole;
}

long long Mth::getSeed(int x, int y, int z)
{
    long long seed = (long long)(x * 3129871 ^ z * 116129781 ^ y);

    return seed * seed * 42317861LL + seed * 11LL;
}

float Mth::ceil(double value, double step)
{
    double scale = 1.0 / step;

    return ::ceil(value * scale) / scale;
}

float Mth::subtractFromLarger(float a, float b)
{
    float difference = a - b;
    float larger = difference >= 0.0f ? a : b;
    float smaller = difference >= 0.0f ? b : a;

    return larger - smaller;
}

float Mth::degreesDifference(float a, float b)
{
    float difference = wrapDegrees(a - b);

    if (difference >= 180.0f)
    {
        difference -= 360.0f;
    }

    return difference;
}

float Mth::approach(float value, float target, float step)
{
    step = abs(step);

    if (value < target)
    {
        return clamp(value + step, value, target);
    }

    return clamp(value - step, target, value);
}

float Mth::approachDegrees(float value, float target, float step)
{
    float difference = degreesDifference(target, value);

    return approach(value, value + difference, step);
}

float Mth::degreesDifferenceAbs(float a, float b)
{
    float difference = wrapDegrees(a - b);

    if (difference < 180.0f)
    {
        return abs(difference);
    }

    return abs(difference - 360.0f);
}

bool Mth::almostEquals(double a, double b, double epsilon)
{
    return ::fabs(a - b) <= epsilon;
}

int Mth::hsvToRgb(float hue, float saturation, float value)
{
    unsigned int sector = (int)(hue * 6.0f) % 6;
    float offset = hue * 6.0f - (float)(int)sector;
    float shaded = value * (1.0f - saturation);
    float falling = value * (1.0f - offset * saturation);
    float rising = value * (1.0f - (1.0f - offset) * saturation);

    float red;
    float green;
    float blue;

    switch (sector)
    {
    case 0:
        red = value;
        green = rising;
        blue = shaded;
        break;
    case 1:
        red = falling;
        green = value;
        blue = shaded;
        break;
    case 2:
        red = shaded;
        green = value;
        blue = rising;
        break;
    case 3:
        red = shaded;
        green = falling;
        blue = value;
        break;
    case 4:
        red = rising;
        green = shaded;
        blue = value;
        break;
    case 5:
        red = value;
        green = shaded;
        blue = falling;
        break;
    default:
        red = 0.0f;
        green = 0.0f;
        blue = 0.0f;
        break;
    }

    return clamp((int)(red * 255.0f), 0, 255) << 16 | clamp((int)(green * 255.0f), 0, 255) << 8
           | clamp((int)(blue * 255.0f), 0, 255);
}

template void Mth::shuffle<int>(std::vector<int>*, Random&);
template void Mth::shuffle<std::pair<int, int> >(std::vector<std::pair<int, int> >*, Random&);
