#ifndef GAME_UTIL_MATH_HPP
#define GAME_UTIL_MATH_HPP

class Math
{
public:
    static double random();
    static int randomInt(int bound);

    static long long round(double value);
    static int round(float value);

    static int _max(int a, int b);
    static float _max(float a, float b);
    static int _min(int a, int b);
    static float _min(float a, float b);

    static float wrapDegrees(float degrees);
    static double wrapDegrees(double degrees);

    static int roundUpToPowerOf2(int value);
};

#endif
