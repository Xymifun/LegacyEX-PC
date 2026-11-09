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
void ASM_MTX44MultVec(const Mtx44 m, const Vec* src, Vec* dst)
{
#ifdef __ghs__
    f32x2 xy = __PSQ_L(&src->x, 0, 0);
    f32x2 z1 = __PSQ_L(&src->z, 1, 0);
    f32x2 w;
    f32x2 vx;
    f32x2 vy;
    f32x2 vz;

    w = __PS_MADD(z1, __PSQ_L(&m[3][2], 0, 0), __PS_MUL(xy, __PSQ_L(&m[3][0], 0, 0)));
    w = __PS_SUM0(w, w, w);
    w = __PS_DIV(__PS_MERGE11(z1, z1), __PS_MERGE00(w, w));
    vx = __PS_MADD(z1, __PSQ_L(&m[0][2], 0, 0), __PS_MUL(xy, __PSQ_L(&m[0][0], 0, 0)));
    vy = __PS_MADD(z1, __PSQ_L(&m[1][2], 0, 0), __PS_MUL(xy, __PSQ_L(&m[1][0], 0, 0)));
    vz = __PS_MADD(z1, __PSQ_L(&m[2][2], 0, 0), __PS_MUL(xy, __PSQ_L(&m[2][0], 0, 0)));
    vx = __PS_SUM1(vy, __PS_SUM0(vx, vx, vx), vy);
    vz = __PS_SUM0(vz, vz, vz);
    __PSQ_ST(&dst->x, __PS_MUL(vx, w), 0, 0);
    __PSQ_ST(&dst->z, __PS_MUL(vz, w), 1, 0);
#elif defined(__powerpc__) || defined(__PPC__)
    const f32 x = src->x;
    const f32 y = src->y;
    const f32 z = src->z;
    const f32 wx = m[3][0] * x;
    const f32 wy = m[3][1] * y;
    const f32 wxz = MultiplyAdd(z, m[3][2], wx);
    const f32 wyw = m[3][3] + wy;
    const f32 w = wxz + wyw;
    const f32 wInverse = 1.0F / w;
    const f32 xx = m[0][0] * x;
    const f32 xy = m[0][1] * y;
    const f32 yx = m[1][0] * x;
    const f32 yy = m[1][1] * y;
    const f32 zx = m[2][0] * x;
    const f32 zy = m[2][1] * y;
    const f32 xxz = MultiplyAdd(z, m[0][2], xx);
    const f32 xyw = m[0][3] + xy;
    const f32 yxz = MultiplyAdd(z, m[1][2], yx);
    const f32 yyw = m[1][3] + yy;
    const f32 zxz = MultiplyAdd(z, m[2][2], zx);
    const f32 zyw = m[2][3] + zy;
    const f32 vx = xxz + xyw;
    const f32 vy = yxz + yyw;
    const f32 vz = zxz + zyw;

    dst->x = vx * wInverse;
    dst->y = vy * wInverse;
    dst->z = vz * wInverse;
#else
    Vec vTmp;
    f32 w;

    vTmp.x = m[0][0] * src->x + m[0][1] * src->y + m[0][2] * src->z + m[0][3];
    vTmp.y = m[1][0] * src->x + m[1][1] * src->y + m[1][2] * src->z + m[1][3];
    vTmp.z = m[2][0] * src->x + m[2][1] * src->y + m[2][2] * src->z + m[2][3];
    w = m[3][0] * src->x + m[3][1] * src->y + m[3][2] * src->z + m[3][3];
    w = 1.0f / w;

    dst->x = vTmp.x * w;
    dst->y = vTmp.y * w;
    dst->z = vTmp.z * w;
#endif
}

void ASM_MTX44MultVecArray(const Mtx44 m, const Vec* srcBase, Vec* dstBase, u32 count)
{
    u32 i;
    Vec vTmp;
    f32 w;

    for (i = 0; i < count; i++)
    {
        vTmp.x = m[0][0] * srcBase->x + m[0][1] * srcBase->y + m[0][2] * srcBase->z + m[0][3];
        vTmp.y = m[1][0] * srcBase->x + m[1][1] * srcBase->y + m[1][2] * srcBase->z + m[1][3];
        vTmp.z = m[2][0] * srcBase->x + m[2][1] * srcBase->y + m[2][2] * srcBase->z + m[2][3];
        w = m[3][0] * srcBase->x + m[3][1] * srcBase->y + m[3][2] * srcBase->z + m[3][3];
        w = 1.0f / w;
        dstBase->x = vTmp.x * w;
        dstBase->y = vTmp.y * w;
        dstBase->z = vTmp.z * w;
        srcBase++;
        dstBase++;
    }
}

void ASM_MTX44MultVecArraySR(const Mtx44 m, const Vec* srcBase, Vec* dstBase, u32 count)
{
    u32 i;
    Vec vTmp;

    for (i = 0; i < count; i++)
    {
        vTmp.x = (m[0][2] * srcBase->z) + ((m[0][0] * srcBase->x) + (m[0][1] * srcBase->y));
        vTmp.y = (m[1][2] * srcBase->z) + ((m[1][0] * srcBase->x) + (m[1][1] * srcBase->y));
        vTmp.z = (m[2][2] * srcBase->z) + ((m[2][0] * srcBase->x) + (m[2][1] * srcBase->y));
        dstBase->x = vTmp.x;
        dstBase->y = vTmp.y;
        dstBase->z = vTmp.z;
        srcBase++;
        dstBase++;
    }
}

void ASM_MTX44MultVecSR(const Mtx44 m, const Vec* src, Vec* dst)
{
#ifdef __ghs__
    f32x2 xy = __PSQ_L(&src->x, 0, 0);
    f32x2 z1 = __PSQ_L(&src->z, 1, 0);
    f32x2 vx = __PS_MUL(__PSQ_L(&m[0][0], 0, 0), xy);
    f32x2 vy = __PS_MUL(__PSQ_L(&m[1][0], 0, 0), xy);
    f32x2 vz = __PS_MUL(__PSQ_L(&m[2][0], 0, 0), xy);

    vx = __PS_SUM0(vx, vx, vx);
    vy = __PS_SUM0(vy, vy, vy);
    vz = __PS_SUM0(vz, vz, vz);
    __PSQ_ST(&dst->x, __PS_MADD(__PSQ_L(&m[0][2], 0, 0), z1, vx), 1, 0);
    __PSQ_ST(&dst->y, __PS_MADD(__PSQ_L(&m[1][2], 0, 0), z1, vy), 1, 0);
    __PSQ_ST(&dst->z, __PS_MADD(__PSQ_L(&m[2][2], 0, 0), z1, vz), 1, 0);
#elif defined(__powerpc__) || defined(__PPC__)
    const f32 x = src->x;
    const f32 y = src->y;
    const f32 z = src->z;
    const f32 xx = m[0][0] * x;
    const f32 xy = m[0][1] * y;
    const f32 yx = m[1][0] * x;
    const f32 yy = m[1][1] * y;
    const f32 zx = m[2][0] * x;
    const f32 zy = m[2][1] * y;
    const f32 vx = xx + xy;
    const f32 vy = yx + yy;
    const f32 vz = zx + zy;

    dst->x = MultiplyAdd(m[0][2], z, vx);
    dst->y = MultiplyAdd(m[1][2], z, vy);
    dst->z = MultiplyAdd(m[2][2], z, vz);
#else
    Vec vTmp;

    vTmp.x = (m[0][2] * src->z) + ((m[0][0] * src->x) + (m[0][1] * src->y));
    vTmp.y = (m[1][2] * src->z) + ((m[1][0] * src->x) + (m[1][1] * src->y));
    vTmp.z = (m[2][2] * src->z) + ((m[2][0] * src->x) + (m[2][1] * src->y));
    dst->x = vTmp.x;
    dst->y = vTmp.y;
    dst->z = vTmp.z;
#endif
}
}
