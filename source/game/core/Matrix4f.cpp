#include "game/core/Matrix4f.hpp"

#include "game/core/Mth.hpp"

Matrix4f::Matrix4f()
{
    m[0] = m[1] = m[2] = m[3] = 0.0f;
    m[4] = m[5] = m[6] = m[7] = 0.0f;
    m[8] = m[9] = m[10] = m[11] = 0.0f;
    m[12] = m[13] = m[14] = m[15] = 0.0f;
}

void Matrix4f::setZYXRot(float z, float y, float x)
{
    float sinZ = 0.0f;
    float cosZ = 1.0f;
    float cosY = 1.0f;
    float sinY = 0.0f;
    float sinX = 0.0f;
    float cosX = 1.0f;

    if (z != 0.0f)
    {
        sinZ = Mth::sin(z);
        cosZ = Mth::cos(z);
    }

    if (y != 0.0f)
    {
        sinY = Mth::sin(y);
        cosY = Mth::cos(y);
    }

    if (x != 0.0f)
    {
        sinX = Mth::sin(x);
        cosX = Mth::cos(x);
    }

    m[0] = cosZ * cosY;
    m[1] = sinZ * cosY;
    m[2] = -sinY;
    m[3] = 0.0f;
    m[4] = cosZ * sinY * sinX - sinZ * cosX;
    m[5] = sinZ * sinY * sinX + cosZ * cosX;
    m[6] = cosY * sinX;
    m[7] = 0.0f;
    m[8] = sinZ * sinX + cosZ * sinY * cosX;
    m[9] = sinZ * sinY * cosX - cosZ * sinX;
    m[10] = cosY * cosX;
    m[11] = 0.0f;
}
