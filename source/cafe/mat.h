#ifndef CAFE_MAT_H
#define CAFE_MAT_H

#include <cafe/mtx.h>

typedef struct _Mat34
{
    Mtx mtx;
} Mat34;

typedef struct _Mat44
{
    Mtx44 mtx;
} Mat44;

#define MAT34Identity(m) ASM_MTXIdentity((m)->mtx)
#define MAT34Copy(src, dst) ASM_MTXCopy((src)->mtx, (dst)->mtx)
#define MAT34Concat(a, b, ab) ASM_MTXConcat((a)->mtx, (b)->mtx, (ab)->mtx)
#define MAT34Trans(m, x, y, z) ASM_MTXTrans((m)->mtx, x, y, z)
#define MAT34Scale(m, x, y, z) ASM_MTXScale((m)->mtx, x, y, z)
#define MAT34RotRad(m, axis, rad) ASM_MTXRotRad((m)->mtx, axis, rad)
#define MAT34To44(src, dst) ASM_MTX34To44((src)->mtx, (dst)->mtx)
#define MAT34MultVec(m, src, dst) ASM_MTXMultVec((m)->mtx, src, dst)
#define MAT44Identity(m) ASM_MTX44Identity((m)->mtx)
#define MAT44Copy(src, dst) ASM_MTX44Copy((src)->mtx, (dst)->mtx)
#define MAT44Concat(a, b, ab) ASM_MTX44Concat((a)->mtx, (b)->mtx, (ab)->mtx)
#define MAT44Transpose(src, dst) ASM_MTX44Transpose((src)->mtx, (dst)->mtx)
#define MAT44Trans(m, x, y, z) ASM_MTX44Trans((m)->mtx, x, y, z)
#define MAT44Scale(m, x, y, z) ASM_MTX44Scale((m)->mtx, x, y, z)
#define MAT44MultVec(m, src, dst) ASM_MTX44MultVec((m)->mtx, src, dst)
#define MAT44MultVecSR(m, src, dst) ASM_MTX44MultVecSR((m)->mtx, src, dst)

#endif
