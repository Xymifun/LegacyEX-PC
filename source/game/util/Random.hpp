#ifndef GAME_UTIL_RANDOM_HPP
#define GAME_UTIL_RANDOM_HPP

class Random
{
public:
    Random();
    Random(long long seed);

    void setSeed(long long seed);

    int nextInt();
    int nextInt(int bound);
    int nextInt(int min, int max);
    long long nextLong();
    bool nextBoolean();
    float nextFloat();
    float nextFloat(float max);
    float nextFloat(float min, float max);
    double nextDouble();
    double nextGaussian();
    void nextBytes(unsigned char* bytes, unsigned int length);

    float nextGaussianFloat();
    int nextGaussianInt(int bound);
    bool rollPercentageChance(int percentage);
    bool rollPercentageChance(float percentage);

protected:
    int next(int bits);

private:
    long long seed;
    bool haveNextNextGaussian;
    double nextNextGaussian;
};

#endif
