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

static inline f32 MultiplySubtract(f32 a, f32 c, f32 b)
{
    f32 result;
    __asm__("fmsubs %0, %1, %2, %3" : "=f"(result) : "f"(a), "f"(c), "f"(b));
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
void ASM_VECAdd(const Vec* a, const Vec* b, Vec* ab)
{
    ab->x = a->x + b->x;
    ab->y = a->y + b->y;
    ab->z = a->z + b->z;
}

void ASM_VECCrossProduct(const Vec* a, const Vec* b, Vec* axb)
{
#ifdef __ghs__
    f32x2 bxy = __PSQ_L(&b->x, 0, 0);
    f32x2 az = __PS_FDUP(a->z);
    f32x2 axy = __PSQ_L(&a->x, 0, 0);
    f32x2 byx = __PS_MERGE10(bxy, bxy);
    f32x2 bz = __PS_FDUP(b->z);
    f32x2 xz = __PS_MSUB(axy, bz, __PS_MUL(bxy, az));
    f32x2 yx = __PS_MSUB(axy, byx, __PS_MULS0(bxy, axy));

    __PSQ_ST(&axb->x, __PS_MERGE11(xz, xz), 1, 0);
    __PSQ_ST(&axb->y, __PS_NEG(__PS_MERGE01(xz, yx)), 0, 0);
#elif defined(__powerpc__) || defined(__PPC__)
    const f32 bx = b->x;
    const f32 by = b->y;
    const f32 az = a->z;
    const f32 ax = a->x;
    const f32 ay = a->y;
    const f32 bz = b->z;
    const f32 bxAz = bx * az;
    const f32 byAz = by * az;
    const f32 byAx = by * ax;

    axb->x = MultiplySubtract(ay, bz, byAz);
    axb->y = NegativeMultiplySubtract(ax, bz, bxAz);
    axb->z = NegativeMultiplySubtract(ay, bx, byAx);
#else
    Vec vTmp;

    vTmp.x = (a->y * b->z) - (a->z * b->y);
    vTmp.y = (a->z * b->x) - (a->x * b->z);
    vTmp.z = (a->x * b->y) - (a->y * b->x);
    axb->x = vTmp.x;
    axb->y = vTmp.y;
    axb->z = vTmp.z;
#endif
}

f32 ASM_VECDistance(const Vec* a, const Vec* b)
{
    return sqrtf(ASM_VECSquareDistance(a, b));
}

f32 ASM_VECDotProduct(const Vec* a, const Vec* b)
{
    f32 dot;

    dot = (a->z * b->z) + ((a->x * b->x) + (a->y * b->y));
    return dot;
}

f32 ASM_VECMag(const Vec* v)
{
    return sqrtf(ASM_VECSquareMag(v));
}

void ASM_VECNormalize(const Vec* src, Vec* unit)
{
#ifdef __ghs__
    f32x2 xy = __PSQ_L(&src->x, 0, 0);
    f32x2 xySq = __PS_MUL(xy, xy);
    f32x2 z1 = __PSQ_L(&src->z, 1, 0);
    f32x2 mag = __PS_SUM0(__PS_MADD(z1, z1, xySq), z1, xySq);
    f32 rsqrt = __FRSQRTEF(mag[0]);

    rsqrt = __FNMSUBS(rsqrt * rsqrt, mag[0], 3.0F) * (rsqrt * 0.5F);
    __PSQ_ST(&unit->x, __PS_MULS0F(xy, rsqrt), 0, 0);
    __PSQ_ST(&unit->z, __PS_MULS0F(z1, rsqrt), 1, 0);
#elif defined(__powerpc__) || defined(__PPC__)
    const f32 x = src->x;
    const f32 y = src->y;
    const f32 z = src->z;
    const f32 xSq = x * x;
    const f32 ySq = y * y;
    const f32 magXz = MultiplyAdd(z, z, xSq);
    const f32 mag = magXz + ySq;
    f32 rsqrt = ReciprocalSqrtEstimate(mag);
    const f32 rsqrtSq = rsqrt * rsqrt;
    const f32 rsqrtHalf = rsqrt * 0.5F;
    const f32 refine = NegativeMultiplySubtract(rsqrtSq, mag, 3.0F);

    rsqrt = refine * rsqrtHalf;
    unit->x = x * rsqrt;
    unit->y = y * rsqrt;
    unit->z = z * rsqrt;
#else
    f32 mag;

    mag = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));

    mag = 1.0f / sqrtf(mag);
    unit->x = src->x * mag;
    unit->y = src->y * mag;
    unit->z = src->z * mag;
#endif
}

void ASM_VECScale(const Vec* src, Vec* dst, f32 scale)
{
    dst->x = (src->x * scale);
    dst->y = (src->y * scale);
    dst->z = (src->z * scale);
}

f32 ASM_VECSquareDistance(const Vec* a, const Vec* b)
{
    Vec diff;

    diff.x = a->x - b->x;
    diff.y = a->y - b->y;
    diff.z = a->z - b->z;
    return (diff.z * diff.z) + ((diff.x * diff.x) + (diff.y * diff.y));
}

f32 ASM_VECSquareMag(const Vec* v)
{
    f32 sqmag;

    sqmag = v->z * v->z + ((v->x * v->x) + (v->y * v->y));
    return sqmag;
}

void ASM_VECSubtract(const Vec* a, const Vec* b, Vec* a_b)
{
    a_b->x = a->x - b->x;
    a_b->y = a->y - b->y;
    a_b->z = a->z - b->z;
}

void C_VECAdd(const Vec* a, const Vec* b, Vec* ab)
{
    ab->x = a->x + b->x;
    ab->y = a->y + b->y;
    ab->z = a->z + b->z;
}

void C_VECCrossProduct(const Vec* a, const Vec* b, Vec* axb)
{
    Vec vTmp;

    vTmp.x = (a->y * b->z) - (a->z * b->y);
    vTmp.y = (a->z * b->x) - (a->x * b->z);
    vTmp.z = (a->x * b->y) - (a->y * b->x);
    axb->x = vTmp.x;
    axb->y = vTmp.y;
    axb->z = vTmp.z;
}

f32 C_VECDistance(const Vec* a, const Vec* b)
{
    return sqrtf(C_VECSquareDistance(a, b));
}

f32 C_VECDotProduct(const Vec* a, const Vec* b)
{
    f32 dot;

    dot = (a->z * b->z) + ((a->x * b->x) + (a->y * b->y));
    return dot;
}

void C_VECHalfAngle(const Vec* a, const Vec* b, Vec* half)
{
    Vec aTmp;
    Vec bTmp;
    Vec hTmp;

    aTmp.x = -a->x;
    aTmp.y = -a->y;
    aTmp.z = -a->z;
    bTmp.x = -b->x;
    bTmp.y = -b->y;
    bTmp.z = -b->z;

    C_VECNormalize(&aTmp, &aTmp);
    C_VECNormalize(&bTmp, &bTmp);
    C_VECAdd(&aTmp, &bTmp, &hTmp);

    if (C_VECDotProduct(&hTmp, &hTmp) > 0.0f)
    {
        C_VECNormalize(&hTmp, half);
        return;
    }
    *half = hTmp;
}

f32 C_VECMag(const Vec* v)
{
    return sqrtf(C_VECSquareMag(v));
}

void C_VECNormalize(const Vec* src, Vec* unit)
{
    f32 mag;

    mag = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));

    mag = 1.0f / sqrtf(mag);
    unit->x = src->x * mag;
    unit->y = src->y * mag;
    unit->z = src->z * mag;
}

void C_VECReflect(const Vec* src, const Vec* normal, Vec* dst)
{
    f32 cosA;
    Vec uI;
    Vec uN;

    uI.x = -src->x;
    uI.y = -src->y;
    uI.z = -src->z;

    C_VECNormalize(&uI, &uI);
    C_VECNormalize(normal, &uN);

    cosA = C_VECDotProduct(&uI, &uN);
    dst->x = (2.0f * uN.x * cosA) - uI.x;
    dst->y = (2.0f * uN.y * cosA) - uI.y;
    dst->z = (2.0f * uN.z * cosA) - uI.z;
    C_VECNormalize(dst, dst);
}

void C_VECScale(const Vec* src, Vec* dst, f32 scale)
{
    dst->x = (src->x * scale);
    dst->y = (src->y * scale);
    dst->z = (src->z * scale);
}

f32 C_VECSquareDistance(const Vec* a, const Vec* b)
{
    Vec diff;

    diff.x = a->x - b->x;
    diff.y = a->y - b->y;
    diff.z = a->z - b->z;
    return (diff.z * diff.z) + ((diff.x * diff.x) + (diff.y * diff.y));
}

f32 C_VECSquareMag(const Vec* v)
{
    f32 sqmag;

    sqmag = v->z * v->z + ((v->x * v->x) + (v->y * v->y));
    return sqmag;
}

void C_VECSubtract(const Vec* a, const Vec* b, Vec* a_b)
{
    a_b->x = a->x - b->x;
    a_b->y = a->y - b->y;
    a_b->z = a->z - b->z;
}

void PSVECAdd(const Vec* a, const Vec* b, Vec* ab)
{
    ab->x = a->x + b->x;
    ab->y = a->y + b->y;
    ab->z = a->z + b->z;
}

void PSVECCrossProduct(const Vec* a, const Vec* b, Vec* axb)
{
    Vec vTmp;

    vTmp.x = (a->y * b->z) - (a->z * b->y);
    vTmp.y = (a->z * b->x) - (a->x * b->z);
    vTmp.z = (a->x * b->y) - (a->y * b->x);
    axb->x = vTmp.x;
    axb->y = vTmp.y;
    axb->z = vTmp.z;
}

f32 PSVECDistance(const Vec* a, const Vec* b)
{
    return sqrtf(PSVECSquareDistance(a, b));
}

f32 PSVECDotProduct(const Vec* a, const Vec* b)
{
    f32 dot;

    dot = (a->z * b->z) + ((a->x * b->x) + (a->y * b->y));
    return dot;
}

f32 PSVECMag(const Vec* v)
{
    return sqrtf(PSVECSquareMag(v));
}

void PSVECNormalize(const Vec* src, Vec* unit)
{
    f32 mag;

    mag = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));

    mag = 1.0f / sqrtf(mag);
    unit->x = src->x * mag;
    unit->y = src->y * mag;
    unit->z = src->z * mag;
}

void PSVECScale(const Vec* src, Vec* dst, f32 scale)
{
    dst->x = (src->x * scale);
    dst->y = (src->y * scale);
    dst->z = (src->z * scale);
}

f32 PSVECSquareDistance(const Vec* a, const Vec* b)
{
    Vec diff;

    diff.x = a->x - b->x;
    diff.y = a->y - b->y;
    diff.z = a->z - b->z;
    return (diff.z * diff.z) + ((diff.x * diff.x) + (diff.y * diff.y));
}

f32 PSVECSquareMag(const Vec* v)
{
    f32 sqmag;

    sqmag = v->z * v->z + ((v->x * v->x) + (v->y * v->y));
    return sqmag;
}

void PSVECSubtract(const Vec* a, const Vec* b, Vec* a_b)
{
    a_b->x = a->x - b->x;
    a_b->y = a->y - b->y;
    a_b->z = a->z - b->z;
}
}
