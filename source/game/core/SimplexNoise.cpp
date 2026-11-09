#include "game/core/SimplexNoise.hpp"

#include "game/util/Random.hpp"

#include <cmath>

int SimplexNoise::grad3[12][3] = {{1, 1, 0},  {-1, 1, 0},  {1, -1, 0}, {-1, -1, 0}, {1, 0, 1},  {-1, 0, 1},
                                  {1, 0, -1}, {-1, 0, -1}, {0, 1, 1},  {0, -1, 1},  {0, 1, -1}, {0, -1, -1}};

double SimplexNoise::F2 = 0.5 * (std::sqrt(3.0) - 1.0);
double SimplexNoise::G2 = (3.0 - std::sqrt(3.0)) / 6.0;
const double SimplexNoise::F3 = 1.0 / 3.0;
const double SimplexNoise::G3 = 1.0 / 6.0;

SimplexNoise::SimplexNoise() : p(NULL), pLength(0), pMask(0), xo(0.0), yo(0.0), zo(0.0)
{
}

SimplexNoise::SimplexNoise(Random* random)
{
    init(random);
}

SimplexNoise::~SimplexNoise()
{
    delete[] p;
}

void SimplexNoise::init(Random* random)
{
    p = new int[512];
    xo = random->nextDouble() * 256.0;
    yo = random->nextDouble() * 256.0;
    zo = random->nextDouble() * 256.0;

    for (int i = 0; i < 256; ++i)
    {
        p[i] = i;
    }

    for (int i = 0; i < 256; ++i)
    {
        int j = random->nextInt(256 - i) + i;
        int swap = p[i];
        p[i] = p[j];
        p[j] = swap;
        p[i + 256] = p[i];
    }
}

int SimplexNoise::fastfloor(double value)
{
    return value > 0.0 ? (int)value : (int)value - 1;
}

double SimplexNoise::dot(int* grad, double x, double y)
{
    return grad[0] * x + grad[1] * y;
}

double SimplexNoise::dot(int* grad, double x, double y, double z)
{
    return grad[0] * x + grad[1] * y + grad[2] * z;
}

double SimplexNoise::getValue(double x, double y)
{
    double n0;
    double n1;
    double n2;
    double s = (x + y) * F2;
    int i = fastfloor(x + s);
    int j = fastfloor(y + s);
    double t = (i + j) * G2;
    double X0 = i - t;
    double Y0 = j - t;
    double x0 = x - X0;
    double y0 = y - Y0;

    int i1;
    int j1;
    if (x0 > y0)
    {
        i1 = 1;
        j1 = 0;
    }
    else
    {
        i1 = 0;
        j1 = 1;
    }

    double x1 = x0 - i1 + G2;
    double y1 = y0 - j1 + G2;
    double x2 = x0 - 1.0 + 2.0 * G2;
    double y2 = y0 - 1.0 + 2.0 * G2;

    int ii = i & 255;
    int jj = j & 255;
    int gi0 = p[ii + p[jj]] % 12;
    int gi1 = p[ii + i1 + p[jj + j1]] % 12;
    int gi2 = p[ii + 1 + p[jj + 1]] % 12;

    double t0 = 0.5 - x0 * x0 - y0 * y0;
    if (t0 < 0.0)
    {
        n0 = 0.0;
    }
    else
    {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0);
    }

    double t1 = 0.5 - x1 * x1 - y1 * y1;
    if (t1 < 0.0)
    {
        n1 = 0.0;
    }
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
    }

    double t2 = 0.5 - x2 * x2 - y2 * y2;
    if (t2 < 0.0)
    {
        n2 = 0.0;
    }
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
    }

    return 70.0 * (n0 + n1 + n2);
}

double SimplexNoise::getValue(double x, double y, double z)
{
    double s = (x + y + z) * F3;
    int i = fastfloor(x + s);
    int j = fastfloor(y + s);
    int k = fastfloor(z + s);
    double t = (i + j + k) * G3;
    double x0 = x - (i - t);
    double y0 = y - (j - t);
    double z0 = z - (k - t);

    int i1;
    int j1;
    int k1;
    int i2;
    int j2;
    int k2;
    if (x0 >= y0)
    {
        if (y0 >= z0)
        {
            i1 = 1;
            j1 = 0;
            k1 = 0;
            i2 = 1;
            j2 = 1;
            k2 = 0;
        }
        else if (x0 >= z0)
        {
            i1 = 1;
            j1 = 0;
            k1 = 0;
            i2 = 1;
            j2 = 0;
            k2 = 1;
        }
        else
        {
            i1 = 0;
            j1 = 0;
            k1 = 1;
            i2 = 1;
            j2 = 0;
            k2 = 1;
        }
    }
    else if (y0 < z0)
    {
        i1 = 0;
        j1 = 0;
        k1 = 1;
        i2 = 0;
        j2 = 1;
        k2 = 1;
    }
    else if (x0 < z0)
    {
        i1 = 0;
        j1 = 1;
        k1 = 0;
        i2 = 0;
        j2 = 1;
        k2 = 1;
    }
    else
    {
        i1 = 0;
        j1 = 1;
        k1 = 0;
        i2 = 1;
        j2 = 1;
        k2 = 0;
    }

    double x1 = x0 - i1 + G3;
    double y1 = y0 - j1 + G3;
    double z1 = z0 - k1 + G3;
    double x2 = x0 - i2 + 2.0 * G3;
    double y2 = y0 - j2 + 2.0 * G3;
    double z2 = z0 - k2 + 2.0 * G3;
    double x3 = x0 - 1.0 + 3.0 * G3;
    double y3 = y0 - 1.0 + 3.0 * G3;
    double z3 = z0 - 1.0 + 3.0 * G3;

    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = p[ii + p[jj + p[kk]]] % 12;
    int gi1 = p[ii + i1 + p[jj + j1 + p[kk + k1]]] % 12;
    int gi2 = p[ii + i2 + p[jj + j2 + p[kk + k2]]] % 12;
    int gi3 = p[ii + 1 + p[jj + 1 + p[kk + 1]]] % 12;

    double n0;
    double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 < 0.0)
    {
        n0 = 0.0;
    }
    else
    {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
    }

    double n1;
    double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 < 0.0)
    {
        n1 = 0.0;
    }
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
    }

    double n2;
    double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 < 0.0)
    {
        n2 = 0.0;
    }
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
    }

    double n3;
    double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 < 0.0)
    {
        n3 = 0.0;
    }
    else
    {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
    }

    return 32.0 * (n0 + n1 + n2 + n3);
}

void SimplexNoise::add(arrayWithLength<double> values, double x, double y, int xSize, int ySize, double xScale,
                       double yScale, double noiseScale)
{
    int index = 0;
    for (int yy = 0; yy < ySize; ++yy)
    {
        double yin = (y + yy) * yScale + yo;
        for (int xx = 0; xx < xSize; ++xx)
        {
            double xin = (x + xx) * xScale + xo;
            double s = (xin + yin) * F2;
            int i = fastfloor(xin + s);
            int j = fastfloor(yin + s);
            double t = (i + j) * G2;
            double x0 = xin - (i - t);
            double y0 = yin - (j - t);

            int i1;
            int j1;
            if (x0 > y0)
            {
                i1 = 1;
                j1 = 0;
            }
            else
            {
                i1 = 0;
                j1 = 1;
            }

            double x1 = x0 - i1 + G2;
            double y1 = y0 - j1 + G2;
            double x2 = x0 - 1.0 + 2.0 * G2;
            double y2 = y0 - 1.0 + 2.0 * G2;

            int ii = i & 255;
            int jj = j & 255;
            int gi0 = p[ii + p[jj]] % 12;
            int gi1 = p[ii + i1 + p[jj + j1]] % 12;
            int gi2 = p[ii + 1 + p[jj + 1]] % 12;

            double n0;
            double t0 = 0.5 - x0 * x0 - y0 * y0;
            if (t0 < 0.0)
            {
                n0 = 0.0;
            }
            else
            {
                t0 *= t0;
                n0 = t0 * t0 * dot(grad3[gi0], x0, y0);
            }

            double n1;
            double t1 = 0.5 - x1 * x1 - y1 * y1;
            if (t1 < 0.0)
            {
                n1 = 0.0;
            }
            else
            {
                t1 *= t1;
                n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
            }

            double n2;
            double t2 = 0.5 - x2 * x2 - y2 * y2;
            if (t2 < 0.0)
            {
                n2 = 0.0;
            }
            else
            {
                t2 *= t2;
                n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
            }

            values[index++] += 70.0 * (n0 + n1 + n2) * noiseScale;
        }
    }
}

void SimplexNoise::add(arrayWithLength<double> values, double x, double y, double z, int xSize, int ySize, int zSize,
                       double xScale, double yScale, double zScale, double noiseScale)
{
    int index = 0;
    for (int zz = 0; zz < zSize; ++zz)
    {
        double zin = (z + zz) * zScale + zo;
        for (int yy = 0; yy < ySize; ++yy)
        {
            double yin = (y + yy) * yScale + yo;
            for (int xx = 0; xx < xSize; ++xx)
            {
                double xin = (x + xx) * xScale + xo;
                values[index++] += getValue(xin, yin, zin) * noiseScale;
            }
        }
    }
}
