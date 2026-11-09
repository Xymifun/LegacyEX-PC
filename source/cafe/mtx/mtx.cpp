#include <cafe/mtx.h>

#include <math.h>

#ifdef __ghs__
#include <ppc_ps.h>
#elif defined(__powerpc__) || defined(__PPC__)
static inline f32 MultiplyAdd(f32 a, f32 c, f32 b)
{
    f32 result;
    __asm__("fmadds %0, %1, %2, %3" : "=f"(result) : "f"(a), "f"(c), "f"(b));
    return result;
}
#endif

extern "C"
{
void ASM_MTXConcat(const Mtx a, const Mtx b, Mtx ab)
{
#ifdef __ghs__
    static const f32 Unit01[2] = {0.0F, 1.0F};
    f32x2 b00 = __PSQ_L(&b[0][0], 0, 0);
    f32x2 b02 = __PSQ_L(&b[0][2], 0, 0);
    f32x2 b10 = __PSQ_L(&b[1][0], 0, 0);
    f32x2 b12 = __PSQ_L(&b[1][2], 0, 0);
    f32x2 b20 = __PSQ_L(&b[2][0], 0, 0);
    f32x2 b22 = __PSQ_L(&b[2][2], 0, 0);
    f32x2 unit01 = __PSQ_L(Unit01, 0, 0);
    u32 i;

    for (i = 0; i < 3; i++)
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
        m02 = __PS_MADDS1(unit01, a02, m02);
        __PSQ_ST(&ab[i][0], m00, 0, 0);
        __PSQ_ST(&ab[i][2], m02, 0, 0);
    }
#elif defined(__powerpc__) || defined(__PPC__)
    const f32 b00 = b[0][0];
    const f32 b01 = b[0][1];
    const f32 b02 = b[0][2];
    const f32 b03 = b[0][3];
    const f32 b10 = b[1][0];
    const f32 b11 = b[1][1];
    const f32 b12 = b[1][2];
    const f32 b13 = b[1][3];
    const f32 b20 = b[2][0];
    const f32 b21 = b[2][1];
    const f32 b22 = b[2][2];
    const f32 b23 = b[2][3];
    u32 i;

    for (i = 0; i < 3; i++)
    {
        const f32 a0 = a[i][0];
        const f32 a1 = a[i][1];
        const f32 a2 = a[i][2];
        const f32 a3 = a[i][3];
        f32 m0 = b00 * a0;
        f32 m1 = b01 * a0;
        f32 m2 = b02 * a0;
        f32 m3 = b03 * a0;

        m0 = MultiplyAdd(b10, a1, m0);
        m1 = MultiplyAdd(b11, a1, m1);
        m2 = MultiplyAdd(b12, a1, m2);
        m3 = MultiplyAdd(b13, a1, m3);
        m0 = MultiplyAdd(b20, a2, m0);
        m1 = MultiplyAdd(b21, a2, m1);
        m2 = MultiplyAdd(b22, a2, m2);
        m3 = MultiplyAdd(b23, a2, m3);
        m2 = MultiplyAdd(0.0F, a3, m2);
        m3 = a3 + m3;
        ab[i][0] = m0;
        ab[i][1] = m1;
        ab[i][2] = m2;
        ab[i][3] = m3;
    }
#else
    Mtx mTmp;
    MtxPtr m;

    if (ab == a || ab == b)
    {
        m = mTmp;
    }
    else
    {
        m = ab;
    }

    m[0][0] = 0 + a[0][2] * b[2][0] + ((a[0][0] * b[0][0]) + (a[0][1] * b[1][0]));
    m[0][1] = 0 + a[0][2] * b[2][1] + ((a[0][0] * b[0][1]) + (a[0][1] * b[1][1]));
    m[0][2] = 0 + a[0][2] * b[2][2] + ((a[0][0] * b[0][2]) + (a[0][1] * b[1][2]));
    m[0][3] = a[0][3] + (a[0][2] * b[2][3] + (a[0][0] * b[0][3] + (a[0][1] * b[1][3])));

    m[1][0] = 0 + a[1][2] * b[2][0] + ((a[1][0] * b[0][0]) + (a[1][1] * b[1][0]));
    m[1][1] = 0 + a[1][2] * b[2][1] + ((a[1][0] * b[0][1]) + (a[1][1] * b[1][1]));
    m[1][2] = 0 + a[1][2] * b[2][2] + ((a[1][0] * b[0][2]) + (a[1][1] * b[1][2]));
    m[1][3] = a[1][3] + (a[1][2] * b[2][3] + (a[1][0] * b[0][3] + (a[1][1] * b[1][3])));

    m[2][0] = 0 + a[2][2] * b[2][0] + ((a[2][0] * b[0][0]) + (a[2][1] * b[1][0]));
    m[2][1] = 0 + a[2][2] * b[2][1] + ((a[2][0] * b[0][1]) + (a[2][1] * b[1][1]));
    m[2][2] = 0 + a[2][2] * b[2][2] + ((a[2][0] * b[0][2]) + (a[2][1] * b[1][2]));
    m[2][3] = a[2][3] + (a[2][2] * b[2][3] + (a[2][0] * b[0][3] + (a[2][1] * b[1][3])));

    if (m == mTmp)
    {
        ASM_MTXCopy(mTmp, ab);
    }
#endif
}

void ASM_MTXConcatArray(const Mtx a, const Mtx* srcBase, Mtx* dstBase, u32 count)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        ASM_MTXConcat(a, *srcBase, *dstBase);
        srcBase++;
        dstBase++;
    }
}

void ASM_MTXCopy(const Mtx src, Mtx dst)
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
    }
}

void ASM_MTXIdentity(Mtx m)
{
    m[0][0] = 1;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = 0;
    m[1][0] = 0;
    m[1][1] = 1;
    m[1][2] = 0;
    m[1][3] = 0;
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = 1;
    m[2][3] = 0;
}

u32 ASM_MTXInvXpose(const Mtx src, Mtx invX)
{
    Mtx mTmp;
    MtxPtr m;
    f32 det;

    if (src == invX)
    {
        m = mTmp;
    }
    else
    {
        m = invX;
    }
    det = ((((src[2][1] * (src[0][2] * src[1][0]))
             + ((src[2][2] * (src[0][0] * src[1][1])) + (src[2][0] * (src[0][1] * src[1][2]))))
            - (src[0][2] * (src[2][0] * src[1][1])))
           - (src[2][2] * (src[1][0] * src[0][1])))
          - (src[1][2] * (src[0][0] * src[2][1]));
    if (0 == det)
    {
        return 0;
    }
    det = 1 / det;
    m[0][0] = (det * +((src[1][1] * src[2][2]) - (src[2][1] * src[1][2])));
    m[0][1] = (det * -((src[1][0] * src[2][2]) - (src[2][0] * src[1][2])));
    m[0][2] = (det * +((src[1][0] * src[2][1]) - (src[2][0] * src[1][1])));

    m[1][0] = (det * -((src[0][1] * src[2][2]) - (src[2][1] * src[0][2])));
    m[1][1] = (det * +((src[0][0] * src[2][2]) - (src[2][0] * src[0][2])));
    m[1][2] = (det * -((src[0][0] * src[2][1]) - (src[2][0] * src[0][1])));

    m[2][0] = (det * +((src[0][1] * src[1][2]) - (src[1][1] * src[0][2])));
    m[2][1] = (det * -((src[0][0] * src[1][2]) - (src[1][0] * src[0][2])));
    m[2][2] = (det * +((src[0][0] * src[1][1]) - (src[1][0] * src[0][1])));

    m[0][3] = 0;
    m[1][3] = 0;
    m[2][3] = 0;

    if (m == mTmp)
    {
        ASM_MTXCopy(mTmp, invX);
    }
    return 1;
}

u32 ASM_MTXInverse(const Mtx src, Mtx inv)
{
    Mtx mTmp;
    MtxPtr m;
    f32 det;

    if (src == inv)
    {
        m = mTmp;
    }
    else
    {
        m = inv;
    }
    det = ((((src[2][1] * (src[0][2] * src[1][0]))
             + ((src[2][2] * (src[0][0] * src[1][1])) + (src[2][0] * (src[0][1] * src[1][2]))))
            - (src[0][2] * (src[2][0] * src[1][1])))
           - (src[2][2] * (src[1][0] * src[0][1])))
          - (src[1][2] * (src[0][0] * src[2][1]));
    if (0 == det)
    {
        return 0;
    }
    det = 1 / det;
    m[0][0] = (det * +((src[1][1] * src[2][2]) - (src[2][1] * src[1][2])));
    m[0][1] = (det * -((src[0][1] * src[2][2]) - (src[2][1] * src[0][2])));
    m[0][2] = (det * +((src[0][1] * src[1][2]) - (src[1][1] * src[0][2])));

    m[1][0] = (det * -((src[1][0] * src[2][2]) - (src[2][0] * src[1][2])));
    m[1][1] = (det * +((src[0][0] * src[2][2]) - (src[2][0] * src[0][2])));
    m[1][2] = (det * -((src[0][0] * src[1][2]) - (src[1][0] * src[0][2])));

    m[2][0] = (det * +((src[1][0] * src[2][1]) - (src[2][0] * src[1][1])));
    m[2][1] = (det * -((src[0][0] * src[2][1]) - (src[2][0] * src[0][1])));
    m[2][2] = (det * +((src[0][0] * src[1][1]) - (src[1][0] * src[0][1])));

    m[0][3] = ((-m[0][0] * src[0][3]) - (m[0][1] * src[1][3])) - (m[0][2] * src[2][3]);
    m[1][3] = ((-m[1][0] * src[0][3]) - (m[1][1] * src[1][3])) - (m[1][2] * src[2][3]);
    m[2][3] = ((-m[2][0] * src[0][3]) - (m[2][1] * src[1][3])) - (m[2][2] * src[2][3]);

    if (m == mTmp)
    {
        ASM_MTXCopy(mTmp, inv);
    }
    return 1;
}

void ASM_MTXQuat(Mtx m, const Quaternion* q)
{
    f32 s;
    f32 xs;
    f32 ys;
    f32 zs;
    f32 wx;
    f32 wy;
    f32 wz;
    f32 xx;
    f32 xy;
    f32 xz;
    f32 yy;
    f32 yz;
    f32 zz;

    s = 2 / ((q->w * q->w) + ((q->z * q->z) + ((q->x * q->x) + (q->y * q->y))));
    xs = q->x * s;
    ys = q->y * s;
    zs = q->z * s;
    wx = q->w * xs;
    wy = q->w * ys;
    wz = q->w * zs;
    xx = q->x * xs;
    xy = q->x * ys;
    xz = q->x * zs;
    yy = q->y * ys;
    yz = q->y * zs;
    zz = q->z * zs;
    m[0][0] = (1 - (yy + zz));
    m[0][1] = (xy - wz);
    m[0][2] = (xz + wy);
    m[0][3] = 0;
    m[1][0] = (xy + wz);
    m[1][1] = (1 - (xx + zz));
    m[1][2] = (yz - wx);
    m[1][3] = 0;
    m[2][0] = (xz - wy);
    m[2][1] = (yz + wx);
    m[2][2] = (1 - (xx + yy));
    m[2][3] = 0;
}

void ASM_MTXReflect(Mtx m, const Vec* p, const Vec* n)
{
    f32 vxy;
    f32 vxz;
    f32 vyz;
    f32 pdotn;

    vxy = -2 * n->x * n->y;
    vxz = -2 * n->x * n->z;
    vyz = -2 * n->y * n->z;
    pdotn = 2 * ASM_VECDotProduct(p, n);
    m[0][0] = (1 - (2 * n->x * n->x));
    m[0][1] = vxy;
    m[0][2] = vxz;
    m[0][3] = (pdotn * n->x);
    m[1][0] = vxy;
    m[1][1] = (1 - (2 * n->y * n->y));
    m[1][2] = vyz;
    m[1][3] = (pdotn * n->y);
    m[2][0] = vxz;
    m[2][1] = vyz;
    m[2][2] = (1 - (2 * n->z * n->z));
    m[2][3] = (pdotn * n->z);
}

void ASM_MTXReorder(const Mtx src, ROMtx dest)
{
    for (u32 row = 0; row < 3; ++row)
    {
        for (u32 column = 0; column < 4; ++column)
            dest[column][row] = src[row][column];
    }
}

void ASM_MTXRotAxisRad(Mtx m, const Vec* axis, f32 rad)
{
    _ASM_MTXRotAxisRadInternal(m, axis, sinf(rad), cosf(rad));
}

void ASM_MTXRotRad(Mtx m, char axis, f32 rad)
{
    f32 sinA;
    f32 cosA;

    sinA = sinf(rad);
    cosA = cosf(rad);
    ASM_MTXRotTrig(m, axis, sinA, cosA);
}

void ASM_MTXRotTrig(Mtx m, char axis, f32 sinA, f32 cosA)
{
    switch (axis)
    {
    case 'x':
    case 'X':
        m[0][0] = 1;
        m[0][1] = 0;
        m[0][2] = 0;
        m[0][3] = 0;
        m[1][0] = 0;
        m[1][1] = cosA;
        m[1][2] = -sinA;
        m[1][3] = 0;
        m[2][0] = 0;
        m[2][1] = sinA;
        m[2][2] = cosA;
        m[2][3] = 0;
        break;
    case 'y':
    case 'Y':
        m[0][0] = cosA;
        m[0][1] = 0;
        m[0][2] = sinA;
        m[0][3] = 0;
        m[1][0] = 0;
        m[1][1] = 1;
        m[1][2] = 0;
        m[1][3] = 0;
        m[2][0] = -sinA;
        m[2][1] = 0;
        m[2][2] = cosA;
        m[2][3] = 0;
        break;
    case 'z':
    case 'Z':
        m[0][0] = cosA;
        m[0][1] = -sinA;
        m[0][2] = 0;
        m[0][3] = 0;
        m[1][0] = sinA;
        m[1][1] = cosA;
        m[1][2] = 0;
        m[1][3] = 0;
        m[2][0] = 0;
        m[2][1] = 0;
        m[2][2] = 1;
        m[2][3] = 0;
        break;
    default:
        break;
    }
}

void ASM_MTXScale(Mtx m, f32 xS, f32 yS, f32 zS)
{
    m[0][0] = xS;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = 0;
    m[1][0] = 0;
    m[1][1] = yS;
    m[1][2] = 0;
    m[1][3] = 0;
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = zS;
    m[2][3] = 0;
}

void ASM_MTXScaleApply(const Mtx src, Mtx dst, f32 xS, f32 yS, f32 zS)
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
}

void ASM_MTXTrans(Mtx m, f32 xT, f32 yT, f32 zT)
{
    m[0][0] = 1;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = xT;
    m[1][0] = 0;
    m[1][1] = 1;
    m[1][2] = 0;
    m[1][3] = yT;
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = 1;
    m[2][3] = zT;
}

void ASM_MTXTransApply(const Mtx src, Mtx dst, f32 xT, f32 yT, f32 zT)
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
    }

    dst[0][3] = (src[0][3] + xT);
    dst[1][3] = (src[1][3] + yT);
    dst[2][3] = (src[2][3] + zT);
}

void ASM_MTXTranspose(const Mtx src, Mtx xPose)
{
    Mtx mTmp;
    MtxPtr m;

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
    m[0][3] = 0;
    m[1][0] = src[0][1];
    m[1][1] = src[1][1];
    m[1][2] = src[2][1];
    m[1][3] = 0;
    m[2][0] = src[0][2];
    m[2][1] = src[1][2];
    m[2][2] = src[2][2];
    m[2][3] = 0;
    if (m == mTmp)
    {
        ASM_MTXCopy(mTmp, xPose);
    }
}

void C_MTXConcat(const Mtx a, const Mtx b, Mtx ab)
{
    Mtx mTmp;
    MtxPtr m;

    if (ab == a || ab == b)
    {
        m = mTmp;
    }
    else
    {
        m = ab;
    }

    m[0][0] = 0 + a[0][2] * b[2][0] + ((a[0][0] * b[0][0]) + (a[0][1] * b[1][0]));
    m[0][1] = 0 + a[0][2] * b[2][1] + ((a[0][0] * b[0][1]) + (a[0][1] * b[1][1]));
    m[0][2] = 0 + a[0][2] * b[2][2] + ((a[0][0] * b[0][2]) + (a[0][1] * b[1][2]));
    m[0][3] = a[0][3] + (a[0][2] * b[2][3] + (a[0][0] * b[0][3] + (a[0][1] * b[1][3])));

    m[1][0] = 0 + a[1][2] * b[2][0] + ((a[1][0] * b[0][0]) + (a[1][1] * b[1][0]));
    m[1][1] = 0 + a[1][2] * b[2][1] + ((a[1][0] * b[0][1]) + (a[1][1] * b[1][1]));
    m[1][2] = 0 + a[1][2] * b[2][2] + ((a[1][0] * b[0][2]) + (a[1][1] * b[1][2]));
    m[1][3] = a[1][3] + (a[1][2] * b[2][3] + (a[1][0] * b[0][3] + (a[1][1] * b[1][3])));

    m[2][0] = 0 + a[2][2] * b[2][0] + ((a[2][0] * b[0][0]) + (a[2][1] * b[1][0]));
    m[2][1] = 0 + a[2][2] * b[2][1] + ((a[2][0] * b[0][1]) + (a[2][1] * b[1][1]));
    m[2][2] = 0 + a[2][2] * b[2][2] + ((a[2][0] * b[0][2]) + (a[2][1] * b[1][2]));
    m[2][3] = a[2][3] + (a[2][2] * b[2][3] + (a[2][0] * b[0][3] + (a[2][1] * b[1][3])));

    if (m == mTmp)
    {
        C_MTXCopy(mTmp, ab);
    }
}

void C_MTXConcatArray(const Mtx a, const Mtx* srcBase, Mtx* dstBase, u32 count)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        C_MTXConcat(a, *srcBase, *dstBase);
        srcBase++;
        dstBase++;
    }
}

void C_MTXCopy(const Mtx src, Mtx dst)
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
    }
}

void C_MTXIdentity(Mtx m)
{
    m[0][0] = 1;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = 0;
    m[1][0] = 0;
    m[1][1] = 1;
    m[1][2] = 0;
    m[1][3] = 0;
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = 1;
    m[2][3] = 0;
}

u32 C_MTXInvXpose(const Mtx src, Mtx invX)
{
    Mtx mTmp;
    MtxPtr m;
    f32 det;

    if (src == invX)
    {
        m = mTmp;
    }
    else
    {
        m = invX;
    }
    det = ((((src[2][1] * (src[0][2] * src[1][0]))
             + ((src[2][2] * (src[0][0] * src[1][1])) + (src[2][0] * (src[0][1] * src[1][2]))))
            - (src[0][2] * (src[2][0] * src[1][1])))
           - (src[2][2] * (src[1][0] * src[0][1])))
          - (src[1][2] * (src[0][0] * src[2][1]));
    if (0 == det)
    {
        return 0;
    }
    det = 1 / det;
    m[0][0] = (det * +((src[1][1] * src[2][2]) - (src[2][1] * src[1][2])));
    m[0][1] = (det * -((src[1][0] * src[2][2]) - (src[2][0] * src[1][2])));
    m[0][2] = (det * +((src[1][0] * src[2][1]) - (src[2][0] * src[1][1])));

    m[1][0] = (det * -((src[0][1] * src[2][2]) - (src[2][1] * src[0][2])));
    m[1][1] = (det * +((src[0][0] * src[2][2]) - (src[2][0] * src[0][2])));
    m[1][2] = (det * -((src[0][0] * src[2][1]) - (src[2][0] * src[0][1])));

    m[2][0] = (det * +((src[0][1] * src[1][2]) - (src[1][1] * src[0][2])));
    m[2][1] = (det * -((src[0][0] * src[1][2]) - (src[1][0] * src[0][2])));
    m[2][2] = (det * +((src[0][0] * src[1][1]) - (src[1][0] * src[0][1])));

    m[0][3] = 0;
    m[1][3] = 0;
    m[2][3] = 0;

    if (m == mTmp)
    {
        C_MTXCopy(mTmp, invX);
    }
    return 1;
}

u32 C_MTXInverse(const Mtx src, Mtx inv)
{
    Mtx mTmp;
    MtxPtr m;
    f32 det;

    if (src == inv)
    {
        m = mTmp;
    }
    else
    {
        m = inv;
    }
    det = ((((src[2][1] * (src[0][2] * src[1][0]))
             + ((src[2][2] * (src[0][0] * src[1][1])) + (src[2][0] * (src[0][1] * src[1][2]))))
            - (src[0][2] * (src[2][0] * src[1][1])))
           - (src[2][2] * (src[1][0] * src[0][1])))
          - (src[1][2] * (src[0][0] * src[2][1]));
    if (0 == det)
    {
        return 0;
    }
    det = 1 / det;
    m[0][0] = (det * +((src[1][1] * src[2][2]) - (src[2][1] * src[1][2])));
    m[0][1] = (det * -((src[0][1] * src[2][2]) - (src[2][1] * src[0][2])));
    m[0][2] = (det * +((src[0][1] * src[1][2]) - (src[1][1] * src[0][2])));

    m[1][0] = (det * -((src[1][0] * src[2][2]) - (src[2][0] * src[1][2])));
    m[1][1] = (det * +((src[0][0] * src[2][2]) - (src[2][0] * src[0][2])));
    m[1][2] = (det * -((src[0][0] * src[1][2]) - (src[1][0] * src[0][2])));

    m[2][0] = (det * +((src[1][0] * src[2][1]) - (src[2][0] * src[1][1])));
    m[2][1] = (det * -((src[0][0] * src[2][1]) - (src[2][0] * src[0][1])));
    m[2][2] = (det * +((src[0][0] * src[1][1]) - (src[1][0] * src[0][1])));

    m[0][3] = ((-m[0][0] * src[0][3]) - (m[0][1] * src[1][3])) - (m[0][2] * src[2][3]);
    m[1][3] = ((-m[1][0] * src[0][3]) - (m[1][1] * src[1][3])) - (m[1][2] * src[2][3]);
    m[2][3] = ((-m[2][0] * src[0][3]) - (m[2][1] * src[1][3])) - (m[2][2] * src[2][3]);

    if (m == mTmp)
    {
        C_MTXCopy(mTmp, inv);
    }
    return 1;
}

void C_MTXLightFrustum(Mtx m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 scaleS, f32 scaleT, f32 transS, f32 transT)
{
    f32 tmp;

    tmp = 1 / (r - l);
    m[0][0] = (scaleS * (2 * n * tmp));
    m[0][1] = 0;
    m[0][2] = (scaleS * (tmp * (r + l))) - transS;
    m[0][3] = 0;
    tmp = 1 / (t - b);
    m[1][0] = 0;
    m[1][1] = (scaleT * (2 * n * tmp));
    m[1][2] = (scaleT * (tmp * (t + b))) - transT;
    m[1][3] = 0;
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = -1;
    m[2][3] = 0;
}

void C_MTXLightOrtho(Mtx m, f32 t, f32 b, f32 l, f32 r, f32 scaleS, f32 scaleT, f32 transS, f32 transT)
{
    f32 tmp;

    tmp = 1 / (r - l);
    m[0][0] = (2 * tmp * scaleS);
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = (transS + (scaleS * (tmp * -(r + l))));
    tmp = 1 / (t - b);
    m[1][0] = 0;
    m[1][1] = (2 * tmp * scaleT);
    m[1][2] = 0;
    m[1][3] = (transT + (scaleT * (tmp * -(t + b))));
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = 0;
    m[2][3] = 1;
}

void C_MTXLightPerspective(Mtx m, f32 fovY, f32 aspect, f32 scaleS, f32 scaleT, f32 transS, f32 transT)
{
    f32 angle;
    f32 cot;

    angle = (0.5f * fovY);
    angle = (angle * 0.01745329252f);
    cot = 1 / tanf(angle);
    m[0][0] = (scaleS * (cot / aspect));
    m[0][1] = 0;
    m[0][2] = -transS;
    m[0][3] = 0;
    m[1][0] = 0;
    m[1][1] = (cot * scaleT);
    m[1][2] = -transT;
    m[1][3] = 0;
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = -1;
    m[2][3] = 0;
}

void C_MTXLookAt(Mtx m, const Vec* camPos, const Vec* camUp, const Vec* target)
{
    Vec vLook;
    Vec vRight;
    Vec vUp;

    vLook.x = camPos->x - target->x;
    vLook.y = camPos->y - target->y;
    vLook.z = camPos->z - target->z;
    ASM_VECNormalize(&vLook, &vLook);
    ASM_VECCrossProduct(camUp, &vLook, &vRight);
    ASM_VECNormalize(&vRight, &vRight);
    ASM_VECCrossProduct(&vLook, &vRight, &vUp);
    m[0][0] = vRight.x;
    m[0][1] = vRight.y;
    m[0][2] = vRight.z;
    m[0][3] = -(camPos->x * vRight.x + camPos->y * vRight.y + camPos->z * vRight.z);
    m[1][0] = vUp.x;
    m[1][1] = vUp.y;
    m[1][2] = vUp.z;
    m[1][3] = -(camPos->x * vUp.x + camPos->y * vUp.y + camPos->z * vUp.z);
    m[2][0] = vLook.x;
    m[2][1] = vLook.y;
    m[2][2] = vLook.z;
    m[2][3] = -(camPos->x * vLook.x + camPos->y * vLook.y + camPos->z * vLook.z);
}

void C_MTXReflect(Mtx m, const Vec* p, const Vec* n)
{
    f32 vxy;
    f32 vxz;
    f32 vyz;
    f32 pdotn;

    vxy = -2 * n->x * n->y;
    vxz = -2 * n->x * n->z;
    vyz = -2 * n->y * n->z;
    pdotn = 2 * C_VECDotProduct(p, n);
    m[0][0] = (1 - (2 * n->x * n->x));
    m[0][1] = vxy;
    m[0][2] = vxz;
    m[0][3] = (pdotn * n->x);
    m[1][0] = vxy;
    m[1][1] = (1 - (2 * n->y * n->y));
    m[1][2] = vyz;
    m[1][3] = (pdotn * n->y);
    m[2][0] = vxz;
    m[2][1] = vyz;
    m[2][2] = (1 - (2 * n->z * n->z));
    m[2][3] = (pdotn * n->z);
}

void C_MTXReorder(const Mtx src, ROMtx dest)
{
    for (u32 row = 0; row < 3; ++row)
    {
        for (u32 column = 0; column < 4; ++column)
            dest[column][row] = src[row][column];
    }
}

void C_MTXRotAxisRad(Mtx m, const Vec* axis, f32 rad)
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
    m[0][3] = 0;
    m[1][0] = ((y * (t * x)) + (s * z));
    m[1][1] = (c + (t * ySq));
    m[1][2] = ((z * (t * y)) - (s * x));
    m[1][3] = 0;
    m[2][0] = ((z * (t * x)) - (s * y));
    m[2][1] = ((z * (t * y)) + (s * x));
    m[2][2] = (c + (t * zSq));
    m[2][3] = 0;
}

void C_MTXRotRad(Mtx m, char axis, f32 rad)
{
    f32 sinA;
    f32 cosA;

    sinA = sinf(rad);
    cosA = cosf(rad);
    C_MTXRotTrig(m, axis, sinA, cosA);
}

void C_MTXRotTrig(Mtx m, char axis, f32 sinA, f32 cosA)
{
    switch (axis)
    {
    case 'x':
    case 'X':
        m[0][0] = 1;
        m[0][1] = 0;
        m[0][2] = 0;
        m[0][3] = 0;
        m[1][0] = 0;
        m[1][1] = cosA;
        m[1][2] = -sinA;
        m[1][3] = 0;
        m[2][0] = 0;
        m[2][1] = sinA;
        m[2][2] = cosA;
        m[2][3] = 0;
        break;
    case 'y':
    case 'Y':
        m[0][0] = cosA;
        m[0][1] = 0;
        m[0][2] = sinA;
        m[0][3] = 0;
        m[1][0] = 0;
        m[1][1] = 1;
        m[1][2] = 0;
        m[1][3] = 0;
        m[2][0] = -sinA;
        m[2][1] = 0;
        m[2][2] = cosA;
        m[2][3] = 0;
        break;
    case 'z':
    case 'Z':
        m[0][0] = cosA;
        m[0][1] = -sinA;
        m[0][2] = 0;
        m[0][3] = 0;
        m[1][0] = sinA;
        m[1][1] = cosA;
        m[1][2] = 0;
        m[1][3] = 0;
        m[2][0] = 0;
        m[2][1] = 0;
        m[2][2] = 1;
        m[2][3] = 0;
        break;
    default:
        break;
    }
}

void C_MTXScale(Mtx m, f32 xS, f32 yS, f32 zS)
{
    m[0][0] = xS;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = 0;
    m[1][0] = 0;
    m[1][1] = yS;
    m[1][2] = 0;
    m[1][3] = 0;
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = zS;
    m[2][3] = 0;
}

void C_MTXScaleApply(const Mtx src, Mtx dst, f32 xS, f32 yS, f32 zS)
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
}

void C_MTXTrans(Mtx m, f32 xT, f32 yT, f32 zT)
{
    m[0][0] = 1;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = xT;
    m[1][0] = 0;
    m[1][1] = 1;
    m[1][2] = 0;
    m[1][3] = yT;
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = 1;
    m[2][3] = zT;
}

void C_MTXTransApply(const Mtx src, Mtx dst, f32 xT, f32 yT, f32 zT)
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
    }

    dst[0][3] = (src[0][3] + xT);
    dst[1][3] = (src[1][3] + yT);
    dst[2][3] = (src[2][3] + zT);
}

void C_MTXTranspose(const Mtx src, Mtx xPose)
{
    Mtx mTmp;
    MtxPtr m;

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
    m[0][3] = 0;
    m[1][0] = src[0][1];
    m[1][1] = src[1][1];
    m[1][2] = src[2][1];
    m[1][3] = 0;
    m[2][0] = src[0][2];
    m[2][1] = src[1][2];
    m[2][2] = src[2][2];
    m[2][3] = 0;
    if (m == mTmp)
    {
        C_MTXCopy(mTmp, xPose);
    }
}

void PSMTXConcat(const Mtx a, const Mtx b, Mtx ab)
{
    Mtx mTmp;
    MtxPtr m;

    if (ab == a || ab == b)
    {
        m = mTmp;
    }
    else
    {
        m = ab;
    }

    m[0][0] = 0 + a[0][2] * b[2][0] + ((a[0][0] * b[0][0]) + (a[0][1] * b[1][0]));
    m[0][1] = 0 + a[0][2] * b[2][1] + ((a[0][0] * b[0][1]) + (a[0][1] * b[1][1]));
    m[0][2] = 0 + a[0][2] * b[2][2] + ((a[0][0] * b[0][2]) + (a[0][1] * b[1][2]));
    m[0][3] = a[0][3] + (a[0][2] * b[2][3] + (a[0][0] * b[0][3] + (a[0][1] * b[1][3])));

    m[1][0] = 0 + a[1][2] * b[2][0] + ((a[1][0] * b[0][0]) + (a[1][1] * b[1][0]));
    m[1][1] = 0 + a[1][2] * b[2][1] + ((a[1][0] * b[0][1]) + (a[1][1] * b[1][1]));
    m[1][2] = 0 + a[1][2] * b[2][2] + ((a[1][0] * b[0][2]) + (a[1][1] * b[1][2]));
    m[1][3] = a[1][3] + (a[1][2] * b[2][3] + (a[1][0] * b[0][3] + (a[1][1] * b[1][3])));

    m[2][0] = 0 + a[2][2] * b[2][0] + ((a[2][0] * b[0][0]) + (a[2][1] * b[1][0]));
    m[2][1] = 0 + a[2][2] * b[2][1] + ((a[2][0] * b[0][1]) + (a[2][1] * b[1][1]));
    m[2][2] = 0 + a[2][2] * b[2][2] + ((a[2][0] * b[0][2]) + (a[2][1] * b[1][2]));
    m[2][3] = a[2][3] + (a[2][2] * b[2][3] + (a[2][0] * b[0][3] + (a[2][1] * b[1][3])));

    if (m == mTmp)
    {
        PSMTXCopy(mTmp, ab);
    }
}

void PSMTXConcatArray(const Mtx a, const Mtx* srcBase, Mtx* dstBase, u32 count)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        PSMTXConcat(a, *srcBase, *dstBase);
        srcBase++;
        dstBase++;
    }
}

void PSMTXCopy(const Mtx src, Mtx dst)
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
    }
}

void PSMTXIdentity(Mtx m)
{
    m[0][0] = 1;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = 0;
    m[1][0] = 0;
    m[1][1] = 1;
    m[1][2] = 0;
    m[1][3] = 0;
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = 1;
    m[2][3] = 0;
}

u32 PSMTXInvXpose(const Mtx src, Mtx invX)
{
    Mtx mTmp;
    MtxPtr m;
    f32 det;

    if (src == invX)
    {
        m = mTmp;
    }
    else
    {
        m = invX;
    }
    det = ((((src[2][1] * (src[0][2] * src[1][0]))
             + ((src[2][2] * (src[0][0] * src[1][1])) + (src[2][0] * (src[0][1] * src[1][2]))))
            - (src[0][2] * (src[2][0] * src[1][1])))
           - (src[2][2] * (src[1][0] * src[0][1])))
          - (src[1][2] * (src[0][0] * src[2][1]));
    if (0 == det)
    {
        return 0;
    }
    det = 1 / det;
    m[0][0] = (det * +((src[1][1] * src[2][2]) - (src[2][1] * src[1][2])));
    m[0][1] = (det * -((src[1][0] * src[2][2]) - (src[2][0] * src[1][2])));
    m[0][2] = (det * +((src[1][0] * src[2][1]) - (src[2][0] * src[1][1])));

    m[1][0] = (det * -((src[0][1] * src[2][2]) - (src[2][1] * src[0][2])));
    m[1][1] = (det * +((src[0][0] * src[2][2]) - (src[2][0] * src[0][2])));
    m[1][2] = (det * -((src[0][0] * src[2][1]) - (src[2][0] * src[0][1])));

    m[2][0] = (det * +((src[0][1] * src[1][2]) - (src[1][1] * src[0][2])));
    m[2][1] = (det * -((src[0][0] * src[1][2]) - (src[1][0] * src[0][2])));
    m[2][2] = (det * +((src[0][0] * src[1][1]) - (src[1][0] * src[0][1])));

    m[0][3] = 0;
    m[1][3] = 0;
    m[2][3] = 0;

    if (m == mTmp)
    {
        PSMTXCopy(mTmp, invX);
    }
    return 1;
}

u32 PSMTXInverse(const Mtx src, Mtx inv)
{
    Mtx mTmp;
    MtxPtr m;
    f32 det;

    if (src == inv)
    {
        m = mTmp;
    }
    else
    {
        m = inv;
    }
    det = ((((src[2][1] * (src[0][2] * src[1][0]))
             + ((src[2][2] * (src[0][0] * src[1][1])) + (src[2][0] * (src[0][1] * src[1][2]))))
            - (src[0][2] * (src[2][0] * src[1][1])))
           - (src[2][2] * (src[1][0] * src[0][1])))
          - (src[1][2] * (src[0][0] * src[2][1]));
    if (0 == det)
    {
        return 0;
    }
    det = 1 / det;
    m[0][0] = (det * +((src[1][1] * src[2][2]) - (src[2][1] * src[1][2])));
    m[0][1] = (det * -((src[0][1] * src[2][2]) - (src[2][1] * src[0][2])));
    m[0][2] = (det * +((src[0][1] * src[1][2]) - (src[1][1] * src[0][2])));

    m[1][0] = (det * -((src[1][0] * src[2][2]) - (src[2][0] * src[1][2])));
    m[1][1] = (det * +((src[0][0] * src[2][2]) - (src[2][0] * src[0][2])));
    m[1][2] = (det * -((src[0][0] * src[1][2]) - (src[1][0] * src[0][2])));

    m[2][0] = (det * +((src[1][0] * src[2][1]) - (src[2][0] * src[1][1])));
    m[2][1] = (det * -((src[0][0] * src[2][1]) - (src[2][0] * src[0][1])));
    m[2][2] = (det * +((src[0][0] * src[1][1]) - (src[1][0] * src[0][1])));

    m[0][3] = ((-m[0][0] * src[0][3]) - (m[0][1] * src[1][3])) - (m[0][2] * src[2][3]);
    m[1][3] = ((-m[1][0] * src[0][3]) - (m[1][1] * src[1][3])) - (m[1][2] * src[2][3]);
    m[2][3] = ((-m[2][0] * src[0][3]) - (m[2][1] * src[1][3])) - (m[2][2] * src[2][3]);

    if (m == mTmp)
    {
        PSMTXCopy(mTmp, inv);
    }
    return 1;
}

void PSMTXReflect(Mtx m, const Vec* p, const Vec* n)
{
    f32 vxy;
    f32 vxz;
    f32 vyz;
    f32 pdotn;

    vxy = -2 * n->x * n->y;
    vxz = -2 * n->x * n->z;
    vyz = -2 * n->y * n->z;
    pdotn = 2 * PSVECDotProduct(p, n);
    m[0][0] = (1 - (2 * n->x * n->x));
    m[0][1] = vxy;
    m[0][2] = vxz;
    m[0][3] = (pdotn * n->x);
    m[1][0] = vxy;
    m[1][1] = (1 - (2 * n->y * n->y));
    m[1][2] = vyz;
    m[1][3] = (pdotn * n->y);
    m[2][0] = vxz;
    m[2][1] = vyz;
    m[2][2] = (1 - (2 * n->z * n->z));
    m[2][3] = (pdotn * n->z);
}

void PSMTXReorder(const Mtx src, ROMtx dest)
{
    for (u32 row = 0; row < 3; ++row)
    {
        for (u32 column = 0; column < 4; ++column)
            dest[column][row] = src[row][column];
    }
}

void PSMTXRotAxisRad(Mtx m, const Vec* axis, f32 rad)
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
    m[0][3] = 0;
    m[1][0] = ((y * (t * x)) + (s * z));
    m[1][1] = (c + (t * ySq));
    m[1][2] = ((z * (t * y)) - (s * x));
    m[1][3] = 0;
    m[2][0] = ((z * (t * x)) - (s * y));
    m[2][1] = ((z * (t * y)) + (s * x));
    m[2][2] = (c + (t * zSq));
    m[2][3] = 0;
}

void PSMTXRotRad(Mtx m, char axis, f32 rad)
{
    f32 sinA;
    f32 cosA;

    sinA = sinf(rad);
    cosA = cosf(rad);
    PSMTXRotTrig(m, axis, sinA, cosA);
}

void PSMTXRotTrig(Mtx m, char axis, f32 sinA, f32 cosA)
{
    switch (axis)
    {
    case 'x':
    case 'X':
        m[0][0] = 1;
        m[0][1] = 0;
        m[0][2] = 0;
        m[0][3] = 0;
        m[1][0] = 0;
        m[1][1] = cosA;
        m[1][2] = -sinA;
        m[1][3] = 0;
        m[2][0] = 0;
        m[2][1] = sinA;
        m[2][2] = cosA;
        m[2][3] = 0;
        break;
    case 'y':
    case 'Y':
        m[0][0] = cosA;
        m[0][1] = 0;
        m[0][2] = sinA;
        m[0][3] = 0;
        m[1][0] = 0;
        m[1][1] = 1;
        m[1][2] = 0;
        m[1][3] = 0;
        m[2][0] = -sinA;
        m[2][1] = 0;
        m[2][2] = cosA;
        m[2][3] = 0;
        break;
    case 'z':
    case 'Z':
        m[0][0] = cosA;
        m[0][1] = -sinA;
        m[0][2] = 0;
        m[0][3] = 0;
        m[1][0] = sinA;
        m[1][1] = cosA;
        m[1][2] = 0;
        m[1][3] = 0;
        m[2][0] = 0;
        m[2][1] = 0;
        m[2][2] = 1;
        m[2][3] = 0;
        break;
    default:
        break;
    }
}

void PSMTXScale(Mtx m, f32 xS, f32 yS, f32 zS)
{
    m[0][0] = xS;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = 0;
    m[1][0] = 0;
    m[1][1] = yS;
    m[1][2] = 0;
    m[1][3] = 0;
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = zS;
    m[2][3] = 0;
}

void PSMTXScaleApply(const Mtx src, Mtx dst, f32 xS, f32 yS, f32 zS)
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
}

void PSMTXTrans(Mtx m, f32 xT, f32 yT, f32 zT)
{
    m[0][0] = 1;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = xT;
    m[1][0] = 0;
    m[1][1] = 1;
    m[1][2] = 0;
    m[1][3] = yT;
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = 1;
    m[2][3] = zT;
}

void PSMTXTransApply(const Mtx src, Mtx dst, f32 xT, f32 yT, f32 zT)
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
    }

    dst[0][3] = (src[0][3] + xT);
    dst[1][3] = (src[1][3] + yT);
    dst[2][3] = (src[2][3] + zT);
}

void PSMTXTranspose(const Mtx src, Mtx xPose)
{
    Mtx mTmp;
    MtxPtr m;

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
    m[0][3] = 0;
    m[1][0] = src[0][1];
    m[1][1] = src[1][1];
    m[1][2] = src[2][1];
    m[1][3] = 0;
    m[2][0] = src[0][2];
    m[2][1] = src[1][2];
    m[2][2] = src[2][2];
    m[2][3] = 0;
    if (m == mTmp)
    {
        PSMTXCopy(mTmp, xPose);
    }
}

void _ASM_MTXRotAxisRadInternal(Mtx m, const Vec* axis, f32 s, f32 c)
{
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
    m[0][3] = 0;
    m[1][0] = ((y * (t * x)) + (s * z));
    m[1][1] = (c + (t * ySq));
    m[1][2] = ((z * (t * y)) - (s * x));
    m[1][3] = 0;
    m[2][0] = ((z * (t * x)) - (s * y));
    m[2][1] = ((z * (t * y)) + (s * x));
    m[2][2] = (c + (t * zSq));
    m[2][3] = 0;
}
}
