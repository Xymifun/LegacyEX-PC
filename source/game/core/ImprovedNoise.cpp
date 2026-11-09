#include "game/core/ImprovedNoise.hpp"

#include "game/util/Random.hpp"

static double gradX[16] = {1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, -1.0, 0.0};

static double gradY[16] = {1.0, 1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0};

static double gradZ[16] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 0.0, 1.0, 0.0, -1.0};

static double grad2X[16] = {1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, -1.0, 0.0};

static double grad2Z[16] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 0.0, 1.0, 0.0, -1.0};

struct ImprovedNoiseYSample
{
    double frac;
    double fade;
    int index;
};

static inline double lerp(double t, double from, double to)
{
    return from + t * (to - from);
}

static inline double grad2(int hash, double x, double z)
{
    int h = hash & 15;
    return grad2X[h] * x + grad2Z[h] * z;
}

static inline double grad(int hash, double x, double y, double z)
{
    int h = hash & 15;
    return gradX[h] * x + gradY[h] * y + gradZ[h] * z;
}

ImprovedNoise::ImprovedNoise(Random* random)
{
    init(random);
}

ImprovedNoise::~ImprovedNoise()
{
    delete[] p;
}

void ImprovedNoise::init(Random* random)
{
    p = new int[512];

    xo = random->nextDouble() * 256.0;
    yo = random->nextDouble() * 256.0;
    zo = random->nextDouble() * 256.0;

    int* table = p;
    for (int i = 0; i < 256; ++i)
    {
        table[i] = i;
    }

    for (int i = 0; i < 256; ++i)
    {
        int j = random->nextInt(256 - i) + i;
        int value = p[i];

        p[i] = p[j];
        p[j] = value;
        p[i + 256] = p[i];
    }
}

double ImprovedNoise::noise(double x, double y, double z)
{
    double xin = x + xo;
    double yin = y + yo;
    double zin = z + zo;

    int xFloor = (int)xin;
    int yFloor = (int)yin;
    int zFloor = (int)zin;

    if (xin < xFloor)
    {
        xFloor--;
    }

    if (yin < yFloor)
    {
        yFloor--;
    }

    if (zin < zFloor)
    {
        zFloor--;
    }

    xin -= xFloor;
    yin -= yFloor;
    zin -= zFloor;

    int X = xFloor & 255;
    int Y = yFloor & 255;
    int Z = zFloor & 255;

    double u = xin * xin * xin * (xin * (xin * 6.0 - 15.0) + 10.0);
    double v = yin * yin * yin * (yin * (yin * 6.0 - 15.0) + 10.0);
    double w = zin * zin * zin * (zin * (zin * 6.0 - 15.0) + 10.0);

    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;

    double lerpY0Z0 = lerp(u, grad(p[AA], xin, yin, zin), grad(p[BA], xin - 1.0, yin, zin));
    double lerpY1Z0 = lerp(u, grad(p[AB], xin, yin - 1.0, zin), grad(p[BB], xin - 1.0, yin - 1.0, zin));
    double lerpY0Z1 = lerp(u, grad(p[AA + 1], xin, yin, zin - 1.0), grad(p[BA + 1], xin - 1.0, yin, zin - 1.0));
    double lerpY1Z1 = lerp(u, grad(p[AB + 1], xin, yin - 1.0, zin - 1.0),
                           grad(p[BB + 1], xin - 1.0, yin - 1.0, zin - 1.0));

    return lerp(w, lerp(v, lerpY0Z0, lerpY1Z0), lerp(v, lerpY0Z1, lerpY1Z1));
}

double ImprovedNoise::getValue(double x, double y)
{
    return noise(x, y, 0.0);
}

double ImprovedNoise::getValue(double x, double y, double z)
{
    return noise(x, y, z);
}

void ImprovedNoise::add(arrayWithLength<double> values, double x, double y, double z, int xSize, int ySize, int zSize,
                        double xScale, double yScale, double zScale, double noiseScale)
{
    double xin = x + xo;
    double invNoiseScale = 1.0 / noiseScale;

    if (ySize == 1)
    {
        int index = 0;

        for (int xi = 0; xi < xSize; ++xi)
        {
            int xFloor = (int)xin;

            if (xin < xFloor)
            {
                xFloor--;
            }

            int X = xFloor & 255;
            double xFade = xin * xin * xin * (xin * (xin * 6.0 - 15.0) + 10.0);
            double zin = z + zo;

            for (int zi = 0; zi < zSize; ++zi)
            {
                int zFloor = (int)zin;

                if (zin < zFloor)
                {
                    zFloor--;
                }

                int Z = zFloor & 255;
                double zFade = zin * zin * zin * (zin * (zin * 6.0 - 15.0) + 10.0);

                int A = p[X];
                int AA = p[A] + Z;
                int B = p[X + 1];
                int BA = p[B] + Z;

                double lerpX0 = lerp(xFade, grad2(p[AA], xin, zin), grad(p[BA], xin - 1.0, 0.0, zin));
                double lerpX1 = lerp(xFade, grad(p[AA + 1], xin, 0.0, zin - 1.0),
                                     grad(p[BA + 1], xin - 1.0, 0.0, zin - 1.0));

                values.data[index++] += lerp(zFade, lerpX0, lerpX1) * invNoiseScale;

                zin = zin + zFloor + zScale;
            }

            xin = xin + xFloor + xScale;
        }
    }
    else
    {
        int index = 0;
        ImprovedNoiseYSample ySamples[64];

        int lastY = -1;
        bool fillYSamples = true;

        double lerpY0Z0 = 0.0;
        double lerpY1Z0 = 0.0;
        double lerpY0Z1 = 0.0;
        double lerpY1Z1 = 0.0;

        for (int xi = 0; xi < xSize; ++xi)
        {
            int xFloor = (int)xin;

            if (xin < xFloor)
            {
                xFloor--;
            }

            xin -= xFloor;
            int X = xFloor & 255;
            double xFade = xin * xin * xin * (xin * (xin * 6.0 - 15.0) + 10.0);
            double zin = z + zo;

            for (int zi = 0; zi < zSize; ++zi)
            {
                int zFloor = (int)zin;

                if (zin < zFloor)
                {
                    zFloor--;
                }

                zin -= zFloor;
                int Z = zFloor & 255;
                double zFade = zin * zin * zin * (zin * (zin * 6.0 - 15.0) + 10.0);

                if (fillYSamples)
                {
                    fillYSamples = false;

                    double yin = y + yo;

                    for (int yi = 0; yi < ySize; ++yi)
                    {
                        int yFloor = (int)yin;

                        if (yin < yFloor)
                        {
                            yFloor--;
                        }

                        yin -= yFloor;
                        int Y = yFloor & 255;
                        double yFade = yin * yin * yin * (yin * (yin * 6.0 - 15.0) + 10.0);

                        ySamples[yi].frac = yin;
                        ySamples[yi].fade = yFade;
                        ySamples[yi].index = Y;

                        if (yi == 0 || Y != lastY)
                        {
                            int A = p[X] + Y;
                            int AA = p[A] + Z;
                            int AB = p[A + 1] + Z;
                            int B = p[X + 1] + Y;
                            int BA = p[B] + Z;
                            int BB = p[B + 1] + Z;

                            lerpY0Z0 = lerp(xFade, grad(p[AA], xin, yin, zin), grad(p[BA], xin - 1.0, yin, zin));
                            lerpY1Z0 = lerp(xFade, grad(p[AB], xin, yin - 1.0, zin),
                                            grad(p[BB], xin - 1.0, yin - 1.0, zin));
                            lerpY0Z1 = lerp(xFade, grad(p[AA + 1], xin, yin, zin - 1.0),
                                            grad(p[BA + 1], xin - 1.0, yin, zin - 1.0));
                            lerpY1Z1 = lerp(xFade, grad(p[AB + 1], xin, yin - 1.0, zin - 1.0),
                                            grad(p[BB + 1], xin - 1.0, yin - 1.0, zin - 1.0));

                            lastY = Y;
                        }

                        double lerpZ0 = lerp(yFade, lerpY0Z0, lerpY1Z0);
                        double lerpZ1 = lerp(yFade, lerpY0Z1, lerpY1Z1);

                        values.data[index++] += lerp(zFade, lerpZ0, lerpZ1) * invNoiseScale;

                        yin = yin + yFloor + yScale;
                    }
                }
                else
                {
                    for (int yi = 0; yi < ySize; ++yi)
                    {
                        double yFrac = ySamples[yi].frac;
                        double yFade = ySamples[yi].fade;
                        int Y = ySamples[yi].index;

                        if (yi == 0 || Y != lastY)
                        {
                            int A = p[X] + Y;
                            int AA = p[A] + Z;
                            int AB = p[A + 1] + Z;
                            int B = p[X + 1] + Y;
                            int BA = p[B] + Z;
                            int BB = p[B + 1] + Z;

                            lerpY0Z0 = lerp(xFade, grad(p[AA], xin, yFrac, zin), grad(p[BA], xin - 1.0, yFrac, zin));
                            lerpY1Z0 = lerp(xFade, grad(p[AB], xin, yFrac - 1.0, zin),
                                            grad(p[BB], xin - 1.0, yFrac - 1.0, zin));
                            lerpY0Z1 = lerp(xFade, grad(p[AA + 1], xin, yFrac, zin - 1.0),
                                            grad(p[BA + 1], xin - 1.0, yFrac, zin - 1.0));
                            lerpY1Z1 = lerp(xFade, grad(p[AB + 1], xin, yFrac - 1.0, zin - 1.0),
                                            grad(p[BB + 1], xin - 1.0, yFrac - 1.0, zin - 1.0));

                            lastY = Y;
                        }

                        double lerpZ0 = lerp(yFade, lerpY0Z0, lerpY1Z0);
                        double lerpZ1 = lerp(yFade, lerpY0Z1, lerpY1Z1);

                        values.data[index++] += lerp(zFade, lerpZ0, lerpZ1) * invNoiseScale;
                    }
                }

                zin = zin + zFloor + zScale;
            }

            xin = xin + xFloor + xScale;
        }
    }
}
