#include <cafe/mtx.h>

#include <math.h>

#ifdef __ghs__
#include <ppc_ghs.h>
#include <ppc_ps.h>
#elif defined(__powerpc__) || defined(__PPC__)
static inline f32 MultiplyAdd(f32 a, f32 c, f32 b)
{
    f32 result;
    __asm__("fmadds %0, %1, %2, %3" : "=f"(result) : "f"(a), "f"(c), "f"(b));
    return result;
}

static inline f32 NegativeMultiplySubtract(f32 a, f32 c, f32 b)
{
    f32 result;
    __asm__("fnmsubs %0, %1, %2, %3" : "=f"(result) : "f"(a), "f"(c), "f"(b));
    return result;
}

static inline f32 ReciprocalSqrtEstimate(f32 value)
{
    f32 estimate;
    __asm__("frsqrte %0, %1" : "=f"(estimate) : "f"(value));
    return estimate;
}
#endif

extern "C"
{
void ASM_MTX34To44(const Mtx src, Mtx44 dst)
{
    dst[0][0] = src[0][0];
    dst[0][1] = src[0][1];
    dst[0][2] = src[0][2];
    dst[0][3] = src[0][3];
    dst[1][0] = src[1][0];
    dst[1][1] = src[1][1];
    dst[1][2] = src[1][2];
    dst[1][3] = src[1][3];
    dst[2][0] = src[2][0];
    dst[2][1] = src[2][1];
    dst[2][2] = src[2][2];
    dst[2][3] = src[2][3];
    dst[3][0] = 0.0f;
    dst[3][1] = 0.0f;
    dst[3][2] = 0.0f;
    dst[3][3] = 1.0f;
}

void ASM_MTX44Concat(const Mtx44 a, const Mtx44 b, Mtx44 ab)
{
#ifdef __ghs__
    f32x2 b00 = __PSQ_L(&b[0][0], 0, 0);
    f32x2 b02 = __PSQ_L(&b[0][2], 0, 0);
    f32x2 b10 = __PSQ_L(&b[1][0], 0, 0);
    f32x2 b12 = __PSQ_L(&b[1][2], 0, 0);
    f32x2 b20 = __PSQ_L(&b[2][0], 0, 0);
    f32x2 b22 = __PSQ_L(&b[2][2], 0, 0);
    f32x2 b30 = __PSQ_L(&b[3][0], 0, 0);
    f32x2 b32 = __PSQ_L(&b[3][2], 0, 0);
    u32 i;

    for (i = 0; i < 4; i++)
    {
        f32x2 a00 = __PSQ_L(&a[i][0], 0, 0);
        f32x2 a02 = __PSQ_L(&a[i][2], 0, 0);
        f32x2 m00;
        f32x2 m02;

        m00 = __PS_MULS0(b00, a00);
        m02 = __PS_MULS0(b02, a00);
        m00 = __PS_MADDS1(b10, a00, m00);
        m02 = __PS_MADDS1(b12, a00, m02);
        m00 = __PS_MADDS0(b20, a02, m00);
        m02 = __PS_MADDS0(b22, a02, m02);
        m00 = __PS_MADDS1(b30, a02, m00);
        m02 = __PS_MADDS1(b32, a02, m02);
        __PSQ_ST(&ab[i][0], m00, 0, 0);
        __PSQ_ST(&ab[i][2], m02, 0, 0);
    }
#elif defined(__powerpc__) || defined(__PPC__)
    Mtx44 bRows;
    u32 i;
    u32 j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            bRows[i][j] = b[i][j];
        }
    }

    for (i = 0; i < 4; i++)
    {
        const f32 a0 = a[i][0];
        const f32 a1 = a[i][1];
        const f32 a2 = a[i][2];
        const f32 a3 = a[i][3];

        for (j = 0; j < 4; j++)
        {
            f32 m = bRows[0][j] * a0;

            m = MultiplyAdd(bRows[1][j], a1, m);
            m = MultiplyAdd(bRows[2][j], a2, m);
            m = MultiplyAdd(bRows[3][j], a3, m);
            ab[i][j] = m;
        }
    }
#else
    Mtx44 mTmp;
    Mtx44Ptr m;

    if (ab == a || ab == b)
    {
        m = mTmp;
    }
    else
    {
        m = ab;
    }

    m[0][0] = (a[0][0] * b[0][0]) + (a[0][1] * b[1][0]) + (a[0][2] * b[2][0]) + (a[0][3] * b[3][0]);
    m[0][1] = (a[0][0] * b[0][1]) + (a[0][1] * b[1][1]) + (a[0][2] * b[2][1]) + (a[0][3] * b[3][1]);
    m[0][2] = (a[0][0] * b[0][2]) + (a[0][1] * b[1][2]) + (a[0][2] * b[2][2]) + (a[0][3] * b[3][2]);
    m[0][3] = (a[0][0] * b[0][3]) + (a[0][1] * b[1][3]) + (a[0][2] * b[2][3]) + (a[0][3] * b[3][3]);

    m[1][0] = (a[1][0] * b[0][0]) + (a[1][1] * b[1][0]) + (a[1][2] * b[2][0]) + (a[1][3] * b[3][0]);
    m[1][1] = (a[1][0] * b[0][1]) + (a[1][1] * b[1][1]) + (a[1][2] * b[2][1]) + (a[1][3] * b[3][1]);
    m[1][2] = (a[1][0] * b[0][2]) + (a[1][1] * b[1][2]) + (a[1][2] * b[2][2]) + (a[1][3] * b[3][2]);
    m[1][3] = (a[1][0] * b[0][3]) + (a[1][1] * b[1][3]) + (a[1][2] * b[2][3]) + (a[1][3] * b[3][3]);

    m[2][0] = (a[2][0] * b[0][0]) + (a[2][1] * b[1][0]) + (a[2][2] * b[2][0]) + (a[2][3] * b[3][0]);
    m[2][1] = (a[2][0] * b[0][1]) + (a[2][1] * b[1][1]) + (a[2][2] * b[2][1]) + (a[2][3] * b[3][1]);
    m[2][2] = (a[2][0] * b[0][2]) + (a[2][1] * b[1][2]) + (a[2][2] * b[2][2]) + (a[2][3] * b[3][2]);
    m[2][3] = (a[2][0] * b[0][3]) + (a[2][1] * b[1][3]) + (a[2][2] * b[2][3]) + (a[2][3] * b[3][3]);

    m[3][0] = (a[3][0] * b[0][0]) + (a[3][1] * b[1][0]) + (a[3][2] * b[2][0]) + (a[3][3] * b[3][0]);
    m[3][1] = (a[3][0] * b[0][1]) + (a[3][1] * b[1][1]) + (a[3][2] * b[2][1]) + (a[3][3] * b[3][1]);
    m[3][2] = (a[3][0] * b[0][2]) + (a[3][1] * b[1][2]) + (a[3][2] * b[2][2]) + (a[3][3] * b[3][2]);
    m[3][3] = (a[3][0] * b[0][3]) + (a[3][1] * b[1][3]) + (a[3][2] * b[2][3]) + (a[3][3] * b[3][3]);

    if (m == mTmp)
    {
        ASM_MTX44Copy(mTmp, ab);
    }
#endif
}

void ASM_MTX44Copy(const Mtx44 src, Mtx44 dst)
{
    if (src != dst)
    {
        dst[0][0] = src[0][0];
        dst[0][1] = src[0][1];
        dst[0][2] = src[0][2];
        dst[0][3] = src[0][3];
        dst[1][0] = src[1][0];
        dst[1][1] = src[1][1];
        dst[1][2] = src[1][2];
        dst[1][3] = src[1][3];
        dst[2][0] = src[2][0];
        dst[2][1] = src[2][1];
        dst[2][2] = src[2][2];
        dst[2][3] = src[2][3];
        dst[3][0] = src[3][0];
        dst[3][1] = src[3][1];
        dst[3][2] = src[3][2];
        dst[3][3] = src[3][3];
    }
}

void ASM_MTX44Identity(Mtx44 m)
{
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = 1.0f;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f;
    m[2][3] = 0.0f;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void ASM_MTX44RotAxisRad(Mtx44 m, const Vec* axis, f32 rad)
{
    _ASM_MTX44RotAxisRadInternal(m, axis, sinf(rad), cosf(rad));
}

void ASM_MTX44RotRad(Mtx44 m, char axis, f32 rad)
{
    f32 sinA;
    f32 cosA;

    sinA = sinf(rad);
    cosA = cosf(rad);
    ASM_MTX44RotTrig(m, axis, sinA, cosA);
}

void ASM_MTX44RotTrig(Mtx44 m, char axis, f32 sinA, f32 cosA)
{
    axis |= 0x20;
    switch (axis)
    {
    case 'x':
        m[0][0] = 1.0f;
        m[0][1] = 0.0f;
        m[0][2] = 0.0f;
        m[0][3] = 0.0f;
        m[1][0] = 0.0f;
        m[1][1] = cosA;
        m[1][2] = -sinA;
        m[1][3] = 0.0f;
        m[2][0] = 0.0f;
        m[2][1] = sinA;
        m[2][2] = cosA;
        m[2][3] = 0.0f;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
        break;
    case 'y':
        m[0][0] = cosA;
        m[0][1] = 0.0f;
        m[0][2] = sinA;
        m[0][3] = 0.0f;
        m[1][0] = 0.0f;
        m[1][1] = 1.0f;
        m[1][2] = 0.0f;
        m[1][3] = 0.0f;
        m[2][0] = -sinA;
        m[2][1] = 0.0f;
        m[2][2] = cosA;
        m[2][3] = 0.0f;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
        break;
    case 'z':
        m[0][0] = cosA;
        m[0][1] = -sinA;
        m[0][2] = 0.0f;
        m[0][3] = 0.0f;
        m[1][0] = sinA;
        m[1][1] = cosA;
        m[1][2] = 0.0f;
        m[1][3] = 0.0f;
        m[2][0] = 0.0f;
        m[2][1] = 0.0f;
        m[2][2] = 1.0f;
        m[2][3] = 0.0f;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
        break;
    default:
        break;
    }
}

void ASM_MTX44Scale(Mtx44 m, f32 xS, f32 yS, f32 zS)
{
    m[0][0] = xS;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;

    m[1][0] = 0.0f;
    m[1][1] = yS;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = zS;
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void ASM_MTX44ScaleApply(const Mtx44 src, Mtx44 dst, f32 xS, f32 yS, f32 zS)
{
    dst[0][0] = (src[0][0] * xS);
    dst[0][1] = (src[0][1] * xS);
    dst[0][2] = (src[0][2] * xS);
    dst[0][3] = (src[0][3] * xS);

    dst[1][0] = (src[1][0] * yS);
    dst[1][1] = (src[1][1] * yS);
    dst[1][2] = (src[1][2] * yS);
    dst[1][3] = (src[1][3] * yS);

    dst[2][0] = (src[2][0] * zS);
    dst[2][1] = (src[2][1] * zS);
    dst[2][2] = (src[2][2] * zS);
    dst[2][3] = (src[2][3] * zS);

    dst[3][0] = src[3][0];
    dst[3][1] = src[3][1];
    dst[3][2] = src[3][2];
    dst[3][3] = src[3][3];
}

void ASM_MTX44Trans(Mtx44 m, f32 xT, f32 yT, f32 zT)
{
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = xT;

    m[1][0] = 0.0f;
    m[1][1] = 1.0f;
    m[1][2] = 0.0f;
    m[1][3] = yT;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f;
    m[2][3] = zT;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void ASM_MTX44TransApply(const Mtx44 src, Mtx44 dst, f32 xT, f32 yT, f32 zT)
{
    if (src != dst)
    {
        dst[0][0] = src[0][0];
        dst[0][1] = src[0][1];
        dst[0][2] = src[0][2];

        dst[1][0] = src[1][0];
        dst[1][1] = src[1][1];
        dst[1][2] = src[1][2];

        dst[2][0] = src[2][0];
        dst[2][1] = src[2][1];
        dst[2][2] = src[2][2];

        dst[3][0] = src[3][0];
        dst[3][1] = src[3][1];
        dst[3][2] = src[3][2];

        dst[3][3] = src[3][3];
    }

    dst[0][3] = (src[0][3] + xT);
    dst[1][3] = (src[1][3] + yT);
    dst[2][3] = (src[2][3] + zT);
}

void ASM_MTX44Transpose(const Mtx44 src, Mtx44 xPose)
{
    Mtx44 mTmp;
    Mtx44Ptr m;

    if (src == xPose)
    {
        m = mTmp;
    }
    else
    {
        m = xPose;
    }

    m[0][0] = src[0][0];
    m[0][1] = src[1][0];
    m[0][2] = src[2][0];
    m[0][3] = src[3][0];
    m[1][0] = src[0][1];
    m[1][1] = src[1][1];
    m[1][2] = src[2][1];
    m[1][3] = src[3][1];
    m[2][0] = src[0][2];
    m[2][1] = src[1][2];
    m[2][2] = src[2][2];
    m[2][3] = src[3][2];
    m[3][0] = src[0][3];
    m[3][1] = src[1][3];
    m[3][2] = src[2][3];
    m[3][3] = src[3][3];

    if (m == mTmp)
    {
        ASM_MTX44Copy(mTmp, xPose);
    }
}

void C_MTX34To44(const Mtx src, Mtx44 dst)
{
    dst[0][0] = src[0][0];
    dst[0][1] = src[0][1];
    dst[0][2] = src[0][2];
    dst[0][3] = src[0][3];
    dst[1][0] = src[1][0];
    dst[1][1] = src[1][1];
    dst[1][2] = src[1][2];
    dst[1][3] = src[1][3];
    dst[2][0] = src[2][0];
    dst[2][1] = src[2][1];
    dst[2][2] = src[2][2];
    dst[2][3] = src[2][3];
    dst[3][0] = 0.0f;
    dst[3][1] = 0.0f;
    dst[3][2] = 0.0f;
    dst[3][3] = 1.0f;
}

void C_MTX44Concat(const Mtx44 a, const Mtx44 b, Mtx44 ab)
{
    Mtx44 mTmp;
    Mtx44Ptr m;

    if (ab == a || ab == b)
    {
        m = mTmp;
    }
    else
    {
        m = ab;
    }

    m[0][0] = (a[0][0] * b[0][0]) + (a[0][1] * b[1][0]) + (a[0][2] * b[2][0]) + (a[0][3] * b[3][0]);
    m[0][1] = (a[0][0] * b[0][1]) + (a[0][1] * b[1][1]) + (a[0][2] * b[2][1]) + (a[0][3] * b[3][1]);
    m[0][2] = (a[0][0] * b[0][2]) + (a[0][1] * b[1][2]) + (a[0][2] * b[2][2]) + (a[0][3] * b[3][2]);
    m[0][3] = (a[0][0] * b[0][3]) + (a[0][1] * b[1][3]) + (a[0][2] * b[2][3]) + (a[0][3] * b[3][3]);

    m[1][0] = (a[1][0] * b[0][0]) + (a[1][1] * b[1][0]) + (a[1][2] * b[2][0]) + (a[1][3] * b[3][0]);
    m[1][1] = (a[1][0] * b[0][1]) + (a[1][1] * b[1][1]) + (a[1][2] * b[2][1]) + (a[1][3] * b[3][1]);
    m[1][2] = (a[1][0] * b[0][2]) + (a[1][1] * b[1][2]) + (a[1][2] * b[2][2]) + (a[1][3] * b[3][2]);
    m[1][3] = (a[1][0] * b[0][3]) + (a[1][1] * b[1][3]) + (a[1][2] * b[2][3]) + (a[1][3] * b[3][3]);

    m[2][0] = (a[2][0] * b[0][0]) + (a[2][1] * b[1][0]) + (a[2][2] * b[2][0]) + (a[2][3] * b[3][0]);
    m[2][1] = (a[2][0] * b[0][1]) + (a[2][1] * b[1][1]) + (a[2][2] * b[2][1]) + (a[2][3] * b[3][1]);
    m[2][2] = (a[2][0] * b[0][2]) + (a[2][1] * b[1][2]) + (a[2][2] * b[2][2]) + (a[2][3] * b[3][2]);
    m[2][3] = (a[2][0] * b[0][3]) + (a[2][1] * b[1][3]) + (a[2][2] * b[2][3]) + (a[2][3] * b[3][3]);

    m[3][0] = (a[3][0] * b[0][0]) + (a[3][1] * b[1][0]) + (a[3][2] * b[2][0]) + (a[3][3] * b[3][0]);
    m[3][1] = (a[3][0] * b[0][1]) + (a[3][1] * b[1][1]) + (a[3][2] * b[2][1]) + (a[3][3] * b[3][1]);
    m[3][2] = (a[3][0] * b[0][2]) + (a[3][1] * b[1][2]) + (a[3][2] * b[2][2]) + (a[3][3] * b[3][2]);
    m[3][3] = (a[3][0] * b[0][3]) + (a[3][1] * b[1][3]) + (a[3][2] * b[2][3]) + (a[3][3] * b[3][3]);

    if (m == mTmp)
    {
        C_MTX44Copy(mTmp, ab);
    }
}

void C_MTX44Copy(const Mtx44 src, Mtx44 dst)
{
    if (src != dst)
    {
        dst[0][0] = src[0][0];
        dst[0][1] = src[0][1];
        dst[0][2] = src[0][2];
        dst[0][3] = src[0][3];
        dst[1][0] = src[1][0];
        dst[1][1] = src[1][1];
        dst[1][2] = src[1][2];
        dst[1][3] = src[1][3];
        dst[2][0] = src[2][0];
        dst[2][1] = src[2][1];
        dst[2][2] = src[2][2];
        dst[2][3] = src[2][3];
        dst[3][0] = src[3][0];
        dst[3][1] = src[3][1];
        dst[3][2] = src[3][2];
        dst[3][3] = src[3][3];
    }
}

void C_MTX44Identity(Mtx44 m)
{
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = 1.0f;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f;
    m[2][3] = 0.0f;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

u32 C_MTX44Inverse(const Mtx44 src, Mtx44 inv)
{
    Mtx44 gjm;
    s32 i;
    s32 j;
    s32 k;
    f32 w;
    f32 max;
    s32 swp;
    f32 ftmp;

    C_MTX44Copy(src, gjm);
    C_MTX44Identity(inv);

    for (i = 0; i < 4; i++)
    {
        max = 0.0f;
        swp = i;

        for (k = i; k < 4; k++)
        {
            ftmp = fabsf(gjm[k][i]);
            if (ftmp > max)
            {
                max = ftmp;
                swp = k;
            }
        }

        if (max == 0.0f)
        {
            return 0;
        }

        if (swp != i)
        {
            for (k = 0; k < 4; k++)
            {
                {
                    f32 swapped = gjm[i][k];
                    gjm[i][k] = gjm[swp][k];
                    gjm[swp][k] = swapped;
                }
                {
                    f32 swapped = inv[i][k];
                    inv[i][k] = inv[swp][k];
                    inv[swp][k] = swapped;
                }
            }
        }

        w = 1.0f / gjm[i][i];
        for (j = 0; j < 4; j++)
        {
            gjm[i][j] *= w;
            inv[i][j] *= w;
        }

        for (k = 0; k < 4; k++)
        {
            if (k != i)
            {
                w = gjm[k][i];
                for (j = 0; j < 4; j++)
                {
                    gjm[k][j] -= gjm[i][j] * w;
                    inv[k][j] -= inv[i][j] * w;
                }
            }
        }
    }

    return 1;
}

void C_MTX44RotAxisRad(Mtx44 m, const Vec* axis, f32 rad)
{
    Vec vN;
    f32 s;
    f32 c;
    f32 t;
    f32 x;
    f32 y;
    f32 z;
    f32 xSq;
    f32 ySq;
    f32 zSq;

    s = sinf(rad);
    c = cosf(rad);
    t = 1 - c;

    C_VECNormalize(axis, &vN);

    x = vN.x;
    y = vN.y;
    z = vN.z;

    xSq = (x * x);
    ySq = (y * y);
    zSq = (z * z);

    m[0][0] = (c + (t * xSq));
    m[0][1] = (y * (t * x)) - (s * z);
    m[0][2] = (z * (t * x)) + (s * y);
    m[0][3] = 0.0f;

    m[1][0] = ((y * (t * x)) + (s * z));
    m[1][1] = (c + (t * ySq));
    m[1][2] = ((z * (t * y)) - (s * x));
    m[1][3] = 0.0f;

    m[2][0] = ((z * (t * x)) - (s * y));
    m[2][1] = ((z * (t * y)) + (s * x));
    m[2][2] = (c + (t * zSq));
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void C_MTX44RotRad(Mtx44 m, char axis, f32 rad)
{
    f32 sinA;
    f32 cosA;

    sinA = sinf(rad);
    cosA = cosf(rad);
    C_MTX44RotTrig(m, axis, sinA, cosA);
}

void C_MTX44RotTrig(Mtx44 m, char axis, f32 sinA, f32 cosA)
{
    axis |= 0x20;
    switch (axis)
    {
    case 'x':
        m[0][0] = 1.0f;
        m[0][1] = 0.0f;
        m[0][2] = 0.0f;
        m[0][3] = 0.0f;
        m[1][0] = 0.0f;
        m[1][1] = cosA;
        m[1][2] = -sinA;
        m[1][3] = 0.0f;
        m[2][0] = 0.0f;
        m[2][1] = sinA;
        m[2][2] = cosA;
        m[2][3] = 0.0f;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
        break;
    case 'y':
        m[0][0] = cosA;
        m[0][1] = 0.0f;
        m[0][2] = sinA;
        m[0][3] = 0.0f;
        m[1][0] = 0.0f;
        m[1][1] = 1.0f;
        m[1][2] = 0.0f;
        m[1][3] = 0.0f;
        m[2][0] = -sinA;
        m[2][1] = 0.0f;
        m[2][2] = cosA;
        m[2][3] = 0.0f;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
        break;
    case 'z':
        m[0][0] = cosA;
        m[0][1] = -sinA;
        m[0][2] = 0.0f;
        m[0][3] = 0.0f;
        m[1][0] = sinA;
        m[1][1] = cosA;
        m[1][2] = 0.0f;
        m[1][3] = 0.0f;
        m[2][0] = 0.0f;
        m[2][1] = 0.0f;
        m[2][2] = 1.0f;
        m[2][3] = 0.0f;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
        break;
    default:
        break;
    }
}

void C_MTX44Scale(Mtx44 m, f32 xS, f32 yS, f32 zS)
{
    m[0][0] = xS;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;

    m[1][0] = 0.0f;
    m[1][1] = yS;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = zS;
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void C_MTX44ScaleApply(const Mtx44 src, Mtx44 dst, f32 xS, f32 yS, f32 zS)
{
    dst[0][0] = (src[0][0] * xS);
    dst[0][1] = (src[0][1] * xS);
    dst[0][2] = (src[0][2] * xS);
    dst[0][3] = (src[0][3] * xS);

    dst[1][0] = (src[1][0] * yS);
    dst[1][1] = (src[1][1] * yS);
    dst[1][2] = (src[1][2] * yS);
    dst[1][3] = (src[1][3] * yS);

    dst[2][0] = (src[2][0] * zS);
    dst[2][1] = (src[2][1] * zS);
    dst[2][2] = (src[2][2] * zS);
    dst[2][3] = (src[2][3] * zS);

    dst[3][0] = src[3][0];
    dst[3][1] = src[3][1];
    dst[3][2] = src[3][2];
    dst[3][3] = src[3][3];
}

void C_MTX44Trans(Mtx44 m, f32 xT, f32 yT, f32 zT)
{
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = xT;

    m[1][0] = 0.0f;
    m[1][1] = 1.0f;
    m[1][2] = 0.0f;
    m[1][3] = yT;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f;
    m[2][3] = zT;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void C_MTX44TransApply(const Mtx44 src, Mtx44 dst, f32 xT, f32 yT, f32 zT)
{
    if (src != dst)
    {
        dst[0][0] = src[0][0];
        dst[0][1] = src[0][1];
        dst[0][2] = src[0][2];

        dst[1][0] = src[1][0];
        dst[1][1] = src[1][1];
        dst[1][2] = src[1][2];

        dst[2][0] = src[2][0];
        dst[2][1] = src[2][1];
        dst[2][2] = src[2][2];

        dst[3][0] = src[3][0];
        dst[3][1] = src[3][1];
        dst[3][2] = src[3][2];

        dst[3][3] = src[3][3];
    }

    dst[0][3] = (src[0][3] + xT);
    dst[1][3] = (src[1][3] + yT);
    dst[2][3] = (src[2][3] + zT);
}

void C_MTX44Transpose(const Mtx44 src, Mtx44 xPose)
{
    Mtx44 mTmp;
    Mtx44Ptr m;

    if (src == xPose)
    {
        m = mTmp;
    }
    else
    {
        m = xPose;
    }

    m[0][0] = src[0][0];
    m[0][1] = src[1][0];
    m[0][2] = src[2][0];
    m[0][3] = src[3][0];
    m[1][0] = src[0][1];
    m[1][1] = src[1][1];
    m[1][2] = src[2][1];
    m[1][3] = src[3][1];
    m[2][0] = src[0][2];
    m[2][1] = src[1][2];
    m[2][2] = src[2][2];
    m[2][3] = src[3][2];
    m[3][0] = src[0][3];
    m[3][1] = src[1][3];
    m[3][2] = src[2][3];
    m[3][3] = src[3][3];

    if (m == mTmp)
    {
        C_MTX44Copy(mTmp, xPose);
    }
}

void C_MTXFrustum(Mtx44 m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f)
{
    f32 tmp;

    tmp = 1 / (r - l);
    m[0][0] = (2 * n * tmp);
    m[0][1] = 0;
    m[0][2] = (tmp * (r + l));
    m[0][3] = 0;
    tmp = 1 / (t - b);
    m[1][0] = 0;
    m[1][1] = (2 * n * tmp);
    m[1][2] = (tmp * (t + b));
    m[1][3] = 0;
    m[2][0] = 0;
    m[2][1] = 0;
    tmp = 1 / (f - n);
    m[2][2] = (-n * tmp);
    m[2][3] = (tmp * -(f * n));
    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = -1;
    m[3][3] = 0;
}

void C_MTXOrtho(Mtx44 m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f)
{
    f32 tmp;

    tmp = 1 / (r - l);
    m[0][0] = 2 * tmp;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = -(r + l) * tmp;
    tmp = 1 / (t - b);
    m[1][0] = 0;
    m[1][1] = 2 * tmp;
    m[1][2] = 0;
    m[1][3] = -(t + b) * tmp;
    m[2][0] = 0;
    m[2][1] = 0;
    tmp = 1 / (f - n);
    m[2][2] = (-2 * tmp);
    m[2][3] = -(f + n) * tmp;
    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;
}

void C_MTXPerspective(Mtx44 m, f32 fovY, f32 aspect, f32 n, f32 f)
{
    f32 angle;
    f32 cot;
    f32 tmp;

    angle = fovY * 0.5f;
    angle = (angle * 0.01745329252f);
    cot = 1 / tanf(angle);
    m[0][0] = (cot / aspect);
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = 0;
    m[1][0] = 0;
    m[1][1] = (cot);
    m[1][2] = 0;
    m[1][3] = 0;
    m[2][0] = 0;
    m[2][1] = 0;
    tmp = 1 / (f - n);
    m[2][2] = -(f + n) * tmp;
    m[2][3] = -(2 * f * n) * tmp;
    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = -1;
    m[3][3] = 0;
}

void PSMTX34To44(const Mtx src, Mtx44 dst)
{
    dst[0][0] = src[0][0];
    dst[0][1] = src[0][1];
    dst[0][2] = src[0][2];
    dst[0][3] = src[0][3];
    dst[1][0] = src[1][0];
    dst[1][1] = src[1][1];
    dst[1][2] = src[1][2];
    dst[1][3] = src[1][3];
    dst[2][0] = src[2][0];
    dst[2][1] = src[2][1];
    dst[2][2] = src[2][2];
    dst[2][3] = src[2][3];
    dst[3][0] = 0.0f;
    dst[3][1] = 0.0f;
    dst[3][2] = 0.0f;
    dst[3][3] = 1.0f;
}

void PSMTX44Concat(const Mtx44 a, const Mtx44 b, Mtx44 ab)
{
    Mtx44 mTmp;
    Mtx44Ptr m;

    if (ab == a || ab == b)
    {
        m = mTmp;
    }
    else
    {
        m = ab;
    }

    m[0][0] = (a[0][0] * b[0][0]) + (a[0][1] * b[1][0]) + (a[0][2] * b[2][0]) + (a[0][3] * b[3][0]);
    m[0][1] = (a[0][0] * b[0][1]) + (a[0][1] * b[1][1]) + (a[0][2] * b[2][1]) + (a[0][3] * b[3][1]);
    m[0][2] = (a[0][0] * b[0][2]) + (a[0][1] * b[1][2]) + (a[0][2] * b[2][2]) + (a[0][3] * b[3][2]);
    m[0][3] = (a[0][0] * b[0][3]) + (a[0][1] * b[1][3]) + (a[0][2] * b[2][3]) + (a[0][3] * b[3][3]);

    m[1][0] = (a[1][0] * b[0][0]) + (a[1][1] * b[1][0]) + (a[1][2] * b[2][0]) + (a[1][3] * b[3][0]);
    m[1][1] = (a[1][0] * b[0][1]) + (a[1][1] * b[1][1]) + (a[1][2] * b[2][1]) + (a[1][3] * b[3][1]);
    m[1][2] = (a[1][0] * b[0][2]) + (a[1][1] * b[1][2]) + (a[1][2] * b[2][2]) + (a[1][3] * b[3][2]);
    m[1][3] = (a[1][0] * b[0][3]) + (a[1][1] * b[1][3]) + (a[1][2] * b[2][3]) + (a[1][3] * b[3][3]);

    m[2][0] = (a[2][0] * b[0][0]) + (a[2][1] * b[1][0]) + (a[2][2] * b[2][0]) + (a[2][3] * b[3][0]);
    m[2][1] = (a[2][0] * b[0][1]) + (a[2][1] * b[1][1]) + (a[2][2] * b[2][1]) + (a[2][3] * b[3][1]);
    m[2][2] = (a[2][0] * b[0][2]) + (a[2][1] * b[1][2]) + (a[2][2] * b[2][2]) + (a[2][3] * b[3][2]);
    m[2][3] = (a[2][0] * b[0][3]) + (a[2][1] * b[1][3]) + (a[2][2] * b[2][3]) + (a[2][3] * b[3][3]);

    m[3][0] = (a[3][0] * b[0][0]) + (a[3][1] * b[1][0]) + (a[3][2] * b[2][0]) + (a[3][3] * b[3][0]);
    m[3][1] = (a[3][0] * b[0][1]) + (a[3][1] * b[1][1]) + (a[3][2] * b[2][1]) + (a[3][3] * b[3][1]);
    m[3][2] = (a[3][0] * b[0][2]) + (a[3][1] * b[1][2]) + (a[3][2] * b[2][2]) + (a[3][3] * b[3][2]);
    m[3][3] = (a[3][0] * b[0][3]) + (a[3][1] * b[1][3]) + (a[3][2] * b[2][3]) + (a[3][3] * b[3][3]);

    if (m == mTmp)
    {
        PSMTX44Copy(mTmp, ab);
    }
}

void PSMTX44Copy(const Mtx44 src, Mtx44 dst)
{
    if (src != dst)
    {
        dst[0][0] = src[0][0];
        dst[0][1] = src[0][1];
        dst[0][2] = src[0][2];
        dst[0][3] = src[0][3];
        dst[1][0] = src[1][0];
        dst[1][1] = src[1][1];
        dst[1][2] = src[1][2];
        dst[1][3] = src[1][3];
        dst[2][0] = src[2][0];
        dst[2][1] = src[2][1];
        dst[2][2] = src[2][2];
        dst[2][3] = src[2][3];
        dst[3][0] = src[3][0];
        dst[3][1] = src[3][1];
        dst[3][2] = src[3][2];
        dst[3][3] = src[3][3];
    }
}

void PSMTX44Identity(Mtx44 m)
{
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = 1.0f;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f;
    m[2][3] = 0.0f;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void PSMTX44RotAxisRad(Mtx44 m, const Vec* axis, f32 rad)
{
    Vec vN;
    f32 s;
    f32 c;
    f32 t;
    f32 x;
    f32 y;
    f32 z;
    f32 xSq;
    f32 ySq;
    f32 zSq;

    s = sinf(rad);
    c = cosf(rad);
    t = 1 - c;

    PSVECNormalize(axis, &vN);

    x = vN.x;
    y = vN.y;
    z = vN.z;

    xSq = (x * x);
    ySq = (y * y);
    zSq = (z * z);

    m[0][0] = (c + (t * xSq));
    m[0][1] = (y * (t * x)) - (s * z);
    m[0][2] = (z * (t * x)) + (s * y);
    m[0][3] = 0.0f;

    m[1][0] = ((y * (t * x)) + (s * z));
    m[1][1] = (c + (t * ySq));
    m[1][2] = ((z * (t * y)) - (s * x));
    m[1][3] = 0.0f;

    m[2][0] = ((z * (t * x)) - (s * y));
    m[2][1] = ((z * (t * y)) + (s * x));
    m[2][2] = (c + (t * zSq));
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void PSMTX44RotRad(Mtx44 m, char axis, f32 rad)
{
    f32 sinA;
    f32 cosA;

    sinA = sinf(rad);
    cosA = cosf(rad);
    PSMTX44RotTrig(m, axis, sinA, cosA);
}

void PSMTX44RotTrig(Mtx44 m, char axis, f32 sinA, f32 cosA)
{
    axis |= 0x20;
    switch (axis)
    {
    case 'x':
        m[0][0] = 1.0f;
        m[0][1] = 0.0f;
        m[0][2] = 0.0f;
        m[0][3] = 0.0f;
        m[1][0] = 0.0f;
        m[1][1] = cosA;
        m[1][2] = -sinA;
        m[1][3] = 0.0f;
        m[2][0] = 0.0f;
        m[2][1] = sinA;
        m[2][2] = cosA;
        m[2][3] = 0.0f;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
        break;
    case 'y':
        m[0][0] = cosA;
        m[0][1] = 0.0f;
        m[0][2] = sinA;
        m[0][3] = 0.0f;
        m[1][0] = 0.0f;
        m[1][1] = 1.0f;
        m[1][2] = 0.0f;
        m[1][3] = 0.0f;
        m[2][0] = -sinA;
        m[2][1] = 0.0f;
        m[2][2] = cosA;
        m[2][3] = 0.0f;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
        break;
    case 'z':
        m[0][0] = cosA;
        m[0][1] = -sinA;
        m[0][2] = 0.0f;
        m[0][3] = 0.0f;
        m[1][0] = sinA;
        m[1][1] = cosA;
        m[1][2] = 0.0f;
        m[1][3] = 0.0f;
        m[2][0] = 0.0f;
        m[2][1] = 0.0f;
        m[2][2] = 1.0f;
        m[2][3] = 0.0f;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
        break;
    default:
        break;
    }
}

void PSMTX44Scale(Mtx44 m, f32 xS, f32 yS, f32 zS)
{
    m[0][0] = xS;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;

    m[1][0] = 0.0f;
    m[1][1] = yS;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = zS;
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void PSMTX44ScaleApply(const Mtx44 src, Mtx44 dst, f32 xS, f32 yS, f32 zS)
{
    dst[0][0] = (src[0][0] * xS);
    dst[0][1] = (src[0][1] * xS);
    dst[0][2] = (src[0][2] * xS);
    dst[0][3] = (src[0][3] * xS);

    dst[1][0] = (src[1][0] * yS);
    dst[1][1] = (src[1][1] * yS);
    dst[1][2] = (src[1][2] * yS);
    dst[1][3] = (src[1][3] * yS);

    dst[2][0] = (src[2][0] * zS);
    dst[2][1] = (src[2][1] * zS);
    dst[2][2] = (src[2][2] * zS);
    dst[2][3] = (src[2][3] * zS);

    dst[3][0] = src[3][0];
    dst[3][1] = src[3][1];
    dst[3][2] = src[3][2];
    dst[3][3] = src[3][3];
}

void PSMTX44Trans(Mtx44 m, f32 xT, f32 yT, f32 zT)
{
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = xT;

    m[1][0] = 0.0f;
    m[1][1] = 1.0f;
    m[1][2] = 0.0f;
    m[1][3] = yT;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f;
    m[2][3] = zT;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void PSMTX44TransApply(const Mtx44 src, Mtx44 dst, f32 xT, f32 yT, f32 zT)
{
    if (src != dst)
    {
        dst[0][0] = src[0][0];
        dst[0][1] = src[0][1];
        dst[0][2] = src[0][2];

        dst[1][0] = src[1][0];
        dst[1][1] = src[1][1];
        dst[1][2] = src[1][2];

        dst[2][0] = src[2][0];
        dst[2][1] = src[2][1];
        dst[2][2] = src[2][2];

        dst[3][0] = src[3][0];
        dst[3][1] = src[3][1];
        dst[3][2] = src[3][2];

        dst[3][3] = src[3][3];
    }

    dst[0][3] = (src[0][3] + xT);
    dst[1][3] = (src[1][3] + yT);
    dst[2][3] = (src[2][3] + zT);
}

void PSMTX44Transpose(const Mtx44 src, Mtx44 xPose)
{
    Mtx44 mTmp;
    Mtx44Ptr m;

    if (src == xPose)
    {
        m = mTmp;
    }
    else
    {
        m = xPose;
    }

    m[0][0] = src[0][0];
    m[0][1] = src[1][0];
    m[0][2] = src[2][0];
    m[0][3] = src[3][0];
    m[1][0] = src[0][1];
    m[1][1] = src[1][1];
    m[1][2] = src[2][1];
    m[1][3] = src[3][1];
    m[2][0] = src[0][2];
    m[2][1] = src[1][2];
    m[2][2] = src[2][2];
    m[2][3] = src[3][2];
    m[3][0] = src[0][3];
    m[3][1] = src[1][3];
    m[3][2] = src[2][3];
    m[3][3] = src[3][3];

    if (m == mTmp)
    {
        PSMTX44Copy(mTmp, xPose);
    }
}

void _ASM_MTX44RotAxisRadInternal(Mtx44 m, const Vec* axis, f32 s, f32 c)
{
#ifdef __ghs__
    f32x2 xy = __PSQ_L(&axis->x, 0, 0);
    f32x2 zz = __PS_FDUP(axis->z);
    f32x2 xySq = __PS_MUL(xy, xy);
    f32x2 mag = __PS_SUM0(__PS_MADD(zz, zz, xySq), zz, xySq);
    f32 t = 1.0F - c;
    f32 rsqrt = __FRSQRTEF(mag[0]);
    f32x2 zero = __PS_FDUP(0.0F);
    f32x2 cc = __PS_FDUP(c);
    f32x2 txy;
    f32x2 sxy;
    f32x2 nsxy;
    f32x2 tz;
    f32x2 txyX;
    f32x2 txyY;
    f32x2 txyZ;
    f32 m01;
    f32 m10;

    rsqrt = __FNMSUBS(rsqrt * rsqrt, mag[0], 3.0F) * (rsqrt * 0.5F);
    xy = __PS_MULS0F(xy, rsqrt);
    zz = __PS_MULS0F(zz, rsqrt);
    txy = __PS_MULS0F(xy, t);
    sxy = __PS_MULS0F(xy, s);
    tz = __PS_MULS0F(zz, t);
    txyY = __PS_MULS1(txy, xy);
    txyX = __PS_MULS0(txy, xy);
    txyZ = __PS_MULS0(txy, zz);
    m01 = __FNMSUBS(zz[0], s, txyY[0]);
    m10 = __FMADDS(zz[0], s, txyY[0]);
    nsxy = __PS_NEG(sxy);

    __PSQ_ST(&m[0][0], __PS_SUM0(txyX, __PS_FDUP(m01), cc), 0, 0);
    __PSQ_ST(&m[0][2], __PS_SUM0(txyZ, zero, sxy), 0, 0);
    __PSQ_ST(&m[1][0], __PS_SUM1(cc, __PS_FDUP(m10), txyY), 0, 0);
    __PSQ_ST(&m[1][2], __PS_SUM0(nsxy, zero, txyZ), 0, 0);
    __PSQ_ST(&m[2][0], __PS_SUM1(sxy, __PS_SUM0(txyZ, txyZ, nsxy), txyZ), 0, 0);
    __PSQ_ST(&m[2][2], __PS_SUM0(__PS_MULS0(tz, zz), zero, cc), 0, 0);
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
#elif defined(__powerpc__) || defined(__PPC__)
    const f32 ax = axis->x;
    const f32 ay = axis->y;
    const f32 az = axis->z;
    const f32 axSq = ax * ax;
    const f32 aySq = ay * ay;
    const f32 magXz = MultiplyAdd(az, az, axSq);
    const f32 mag = magXz + aySq;
    const f32 t = 1.0F - c;
    f32 rsqrt = ReciprocalSqrtEstimate(mag);
    const f32 rsqrtSq = rsqrt * rsqrt;
    const f32 rsqrtHalf = rsqrt * 0.5F;
    const f32 refine = NegativeMultiplySubtract(rsqrtSq, mag, 3.0F);

    rsqrt = refine * rsqrtHalf;

    const f32 x = ax * rsqrt;
    const f32 y = ay * rsqrt;
    const f32 z = az * rsqrt;
    const f32 tx = x * t;
    const f32 ty = y * t;
    const f32 sx = x * s;
    const f32 sy = y * s;
    const f32 tz = z * t;
    const f32 txy = tx * y;
    const f32 tyy = ty * y;
    const f32 txx = tx * x;
    const f32 txz = tx * z;
    const f32 tyz = ty * z;
    const f32 m01 = NegativeMultiplySubtract(z, s, txy);
    const f32 m10 = MultiplyAdd(z, s, txy);
    const f32 nsx = -sx;
    const f32 nsy = -sy;
    const f32 tzz = tz * z;

    m[0][0] = txx + c;
    m[0][1] = m01;
    m[0][2] = txz + sy;
    m[0][3] = 0.0F;
    m[1][0] = m10;
    m[1][1] = c + tyy;
    m[1][2] = nsx + tyz;
    m[1][3] = 0.0F;
    m[2][0] = txz + nsy;
    m[2][1] = sx + tyz;
    m[2][2] = tzz + c;
    m[2][3] = 0.0F;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
#else
    Vec vN;
    f32 t;
    f32 x;
    f32 y;
    f32 z;
    f32 xSq;
    f32 ySq;
    f32 zSq;

    t = 1 - c;

    ASM_VECNormalize(axis, &vN);

    x = vN.x;
    y = vN.y;
    z = vN.z;

    xSq = (x * x);
    ySq = (y * y);
    zSq = (z * z);

    m[0][0] = (c + (t * xSq));
    m[0][1] = (y * (t * x)) - (s * z);
    m[0][2] = (z * (t * x)) + (s * y);
    m[0][3] = 0.0f;

    m[1][0] = ((y * (t * x)) + (s * z));
    m[1][1] = (c + (t * ySq));
    m[1][2] = ((z * (t * y)) - (s * x));
    m[1][3] = 0.0f;

    m[2][0] = ((z * (t * x)) - (s * y));
    m[2][1] = ((z * (t * y)) + (s * x));
    m[2][2] = (c + (t * zSq));
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
#endif
}
}
