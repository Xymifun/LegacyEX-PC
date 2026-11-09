#include <cafe/mtx.h>

#include <math.h>

extern "C"
{
void ASM_QUATAdd(const Quaternion* p, const Quaternion* q, Quaternion* r)
{
    r->x = p->x + q->x;
    r->y = p->y + q->y;
    r->z = p->z + q->z;
    r->w = p->w + q->w;
}

void ASM_QUATDivide(const Quaternion* p, const Quaternion* q, Quaternion* r)
{
    Quaternion qtmp;

    ASM_QUATInverse(q, &qtmp);
    ASM_QUATMultiply(&qtmp, p, r);
}

f32 ASM_QUATDotProduct(const Quaternion* p, const Quaternion* q)
{
    return (q->x * p->x) + (q->y * p->y) + (q->z * p->z) + (q->w * p->w);
}

void ASM_QUATInverse(const Quaternion* src, Quaternion* inv)
{
    f32 mag, norminv;

    mag = (src->x * src->x) + (src->y * src->y) + (src->z * src->z) + (src->w * src->w);
    if (mag == 0.0f)
    {
        mag = 1.0f;
    }

    norminv = 1.0f / mag;
    inv->x = -src->x * norminv;
    inv->y = -src->y * norminv;
    inv->z = -src->z * norminv;
    inv->w = src->w * norminv;
}

void ASM_QUATMultiply(const Quaternion* p, const Quaternion* q, Quaternion* pq)
{
    Quaternion* r;
    Quaternion pqTmp;

    if (p == pq || q == pq)
    {
        r = &pqTmp;
    }
    else
    {
        r = pq;
    }

    r->w = (p->w * q->w) - (p->x * q->x) - (p->y * q->y) - (p->z * q->z);
    r->x = (p->w * q->x) + (p->x * q->w) + (p->y * q->z) - (p->z * q->y);
    r->y = (p->w * q->y) + (p->y * q->w) + (p->z * q->x) - (p->x * q->z);
    r->z = (p->w * q->z) + (p->z * q->w) + (p->x * q->y) - (p->y * q->x);

    if (r == &pqTmp)
    {
        *pq = pqTmp;
    }
}

void ASM_QUATNormalize(const Quaternion* src, Quaternion* unit)
{
    f32 mag;

    mag = (src->x * src->x) + (src->y * src->y) + (src->z * src->z) + (src->w * src->w);
    if (mag >= 0.00001f)
    {
        mag = 1.0f / sqrtf(mag);

        unit->x = src->x * mag;
        unit->y = src->y * mag;
        unit->z = src->z * mag;
        unit->w = src->w * mag;
    }
    else
    {
        unit->x = unit->y = unit->z = unit->w = 0.0f;
    }
}

void ASM_QUATScale(const Quaternion* q, Quaternion* r, f32 scale)
{
    r->x = q->x * scale;
    r->y = q->y * scale;
    r->z = q->z * scale;
    r->w = q->w * scale;
}

void ASM_QUATSubtract(const Quaternion* p, const Quaternion* q, Quaternion* r)
{
    r->x = p->x - q->x;
    r->y = p->y - q->y;
    r->z = p->z - q->z;
    r->w = p->w - q->w;
}
}
