#ifndef CAFE_MTX_H
#define CAFE_MTX_H

#include <nn/types.h>

typedef struct Vec
{
    f32 x;
    f32 y;
    f32 z;
} Vec;

typedef Vec* VecPtr;

typedef struct Quaternion
{
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Quaternion;

typedef f32 Mtx[3][4];
typedef f32 (*MtxPtr)[4];
typedef f32 Mtx44[4][4];
typedef f32 (*Mtx44Ptr)[4];
typedef f32 ROMtx[4][3];

#ifdef __cplusplus
extern "C"
{
#endif

void ASM_MTX34To44(const Mtx src, Mtx44 dst);
void C_MTX34To44(const Mtx src, Mtx44 dst);
void PSMTX34To44(const Mtx src, Mtx44 dst);
void ASM_MTX44Concat(const Mtx44 a, const Mtx44 b, Mtx44 ab);
void C_MTX44Concat(const Mtx44 a, const Mtx44 b, Mtx44 ab);
void PSMTX44Concat(const Mtx44 a, const Mtx44 b, Mtx44 ab);
void ASM_MTX44Copy(const Mtx44 src, Mtx44 dst);
void C_MTX44Copy(const Mtx44 src, Mtx44 dst);
void PSMTX44Copy(const Mtx44 src, Mtx44 dst);
void ASM_MTX44Identity(Mtx44 m);
void C_MTX44Identity(Mtx44 m);
void PSMTX44Identity(Mtx44 m);
u32 C_MTX44Inverse(const Mtx44 src, Mtx44 inv);
void ASM_MTX44MultVec(const Mtx44 m, const Vec* src, Vec* dst);
void ASM_MTX44MultVecArray(const Mtx44 m, const Vec* srcBase, Vec* dstBase, u32 count);
void ASM_MTX44MultVecArraySR(const Mtx44 m, const Vec* srcBase, Vec* dstBase, u32 count);
void ASM_MTX44MultVecSR(const Mtx44 m, const Vec* src, Vec* dst);
void ASM_MTX44RotAxisRad(Mtx44 m, const Vec* axis, f32 rad);
void C_MTX44RotAxisRad(Mtx44 m, const Vec* axis, f32 rad);
void PSMTX44RotAxisRad(Mtx44 m, const Vec* axis, f32 rad);
void _ASM_MTX44RotAxisRadInternal(Mtx44 m, const Vec* axis, f32 sinA, f32 cosA);
void ASM_MTX44RotRad(Mtx44 m, char axis, f32 rad);
void C_MTX44RotRad(Mtx44 m, char axis, f32 rad);
void PSMTX44RotRad(Mtx44 m, char axis, f32 rad);
void ASM_MTX44RotTrig(Mtx44 m, char axis, f32 sinA, f32 cosA);
void C_MTX44RotTrig(Mtx44 m, char axis, f32 sinA, f32 cosA);
void PSMTX44RotTrig(Mtx44 m, char axis, f32 sinA, f32 cosA);
void ASM_MTX44Scale(Mtx44 m, f32 xS, f32 yS, f32 zS);
void C_MTX44Scale(Mtx44 m, f32 xS, f32 yS, f32 zS);
void PSMTX44Scale(Mtx44 m, f32 xS, f32 yS, f32 zS);
void ASM_MTX44ScaleApply(const Mtx44 src, Mtx44 dst, f32 xS, f32 yS, f32 zS);
void C_MTX44ScaleApply(const Mtx44 src, Mtx44 dst, f32 xS, f32 yS, f32 zS);
void PSMTX44ScaleApply(const Mtx44 src, Mtx44 dst, f32 xS, f32 yS, f32 zS);
void ASM_MTX44Trans(Mtx44 m, f32 xT, f32 yT, f32 zT);
void C_MTX44Trans(Mtx44 m, f32 xT, f32 yT, f32 zT);
void PSMTX44Trans(Mtx44 m, f32 xT, f32 yT, f32 zT);
void ASM_MTX44TransApply(const Mtx44 src, Mtx44 dst, f32 xT, f32 yT, f32 zT);
void C_MTX44TransApply(const Mtx44 src, Mtx44 dst, f32 xT, f32 yT, f32 zT);
void PSMTX44TransApply(const Mtx44 src, Mtx44 dst, f32 xT, f32 yT, f32 zT);
void ASM_MTX44Transpose(const Mtx44 src, Mtx44 xPose);
void C_MTX44Transpose(const Mtx44 src, Mtx44 xPose);
void PSMTX44Transpose(const Mtx44 src, Mtx44 xPose);
void ASM_MTXConcat(const Mtx a, const Mtx b, Mtx ab);
void C_MTXConcat(const Mtx a, const Mtx b, Mtx ab);
void PSMTXConcat(const Mtx a, const Mtx b, Mtx ab);
void ASM_MTXConcatArray(const Mtx a, const Mtx* srcBase, Mtx* dstBase, u32 count);
void C_MTXConcatArray(const Mtx a, const Mtx* srcBase, Mtx* dstBase, u32 count);
void PSMTXConcatArray(const Mtx a, const Mtx* srcBase, Mtx* dstBase, u32 count);
void ASM_MTXCopy(const Mtx src, Mtx dst);
void C_MTXCopy(const Mtx src, Mtx dst);
void PSMTXCopy(const Mtx src, Mtx dst);
void C_MTXFrustum(Mtx44 m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
void ASM_MTXIdentity(Mtx m);
void C_MTXIdentity(Mtx m);
void PSMTXIdentity(Mtx m);
u32 ASM_MTXInvXpose(const Mtx src, Mtx invX);
u32 C_MTXInvXpose(const Mtx src, Mtx invX);
u32 PSMTXInvXpose(const Mtx src, Mtx invX);
u32 ASM_MTXInverse(const Mtx src, Mtx inv);
u32 C_MTXInverse(const Mtx src, Mtx inv);
u32 PSMTXInverse(const Mtx src, Mtx inv);
void C_MTXLightFrustum(Mtx m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 scaleS, f32 scaleT, f32 transS, f32 transT);
void C_MTXLightOrtho(Mtx m, f32 t, f32 b, f32 l, f32 r, f32 scaleS, f32 scaleT, f32 transS, f32 transT);
void C_MTXLightPerspective(Mtx m, f32 fovY, f32 aspect, f32 scaleS, f32 scaleT, f32 transS, f32 transT);
void C_MTXLookAt(Mtx m, const Vec* camPos, const Vec* camUp, const Vec* target);
void ASM_MTXMultVec(const Mtx m, const Vec* src, Vec* dst);
void C_MTXMultVec(const Mtx m, const Vec* src, Vec* dst);
void PSMTXMultVec(const Mtx m, const Vec* src, Vec* dst);
void ASM_MTXMultVecArray(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count);
void C_MTXMultVecArray(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count);
void PSMTXMultVecArray(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count);
void ASM_MTXMultVecArraySR(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count);
void C_MTXMultVecArraySR(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count);
void PSMTXMultVecArraySR(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count);
void ASM_MTXMultVecSR(const Mtx m, const Vec* src, Vec* dst);
void C_MTXMultVecSR(const Mtx m, const Vec* src, Vec* dst);
void PSMTXMultVecSR(const Mtx m, const Vec* src, Vec* dst);
void C_MTXOrtho(Mtx44 m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
void C_MTXPerspective(Mtx44 m, f32 fovY, f32 aspect, f32 n, f32 f);
void ASM_MTXQuat(Mtx m, const Quaternion* q);
void C_MTXROMultVecArray(const ROMtx m, const Vec* srcBase, Vec* dstBase, u32 count);
void ASM_MTXReflect(Mtx m, const Vec* p, const Vec* n);
void C_MTXReflect(Mtx m, const Vec* p, const Vec* n);
void PSMTXReflect(Mtx m, const Vec* p, const Vec* n);
void ASM_MTXReorder(const Mtx src, ROMtx dest);
void C_MTXReorder(const Mtx src, ROMtx dest);
void PSMTXReorder(const Mtx src, ROMtx dest);
void ASM_MTXRotAxisRad(Mtx m, const Vec* axis, f32 rad);
void C_MTXRotAxisRad(Mtx m, const Vec* axis, f32 rad);
void PSMTXRotAxisRad(Mtx m, const Vec* axis, f32 rad);
void _ASM_MTXRotAxisRadInternal(Mtx m, const Vec* axis, f32 sinA, f32 cosA);
void ASM_MTXRotRad(Mtx m, char axis, f32 rad);
void C_MTXRotRad(Mtx m, char axis, f32 rad);
void PSMTXRotRad(Mtx m, char axis, f32 rad);
void ASM_MTXRotTrig(Mtx m, char axis, f32 sinA, f32 cosA);
void C_MTXRotTrig(Mtx m, char axis, f32 sinA, f32 cosA);
void PSMTXRotTrig(Mtx m, char axis, f32 sinA, f32 cosA);
void ASM_MTXScale(Mtx m, f32 xS, f32 yS, f32 zS);
void C_MTXScale(Mtx m, f32 xS, f32 yS, f32 zS);
void PSMTXScale(Mtx m, f32 xS, f32 yS, f32 zS);
void ASM_MTXScaleApply(const Mtx src, Mtx dst, f32 xS, f32 yS, f32 zS);
void C_MTXScaleApply(const Mtx src, Mtx dst, f32 xS, f32 yS, f32 zS);
void PSMTXScaleApply(const Mtx src, Mtx dst, f32 xS, f32 yS, f32 zS);
void ASM_MTXTrans(Mtx m, f32 xT, f32 yT, f32 zT);
void C_MTXTrans(Mtx m, f32 xT, f32 yT, f32 zT);
void PSMTXTrans(Mtx m, f32 xT, f32 yT, f32 zT);
void ASM_MTXTransApply(const Mtx src, Mtx dst, f32 xT, f32 yT, f32 zT);
void C_MTXTransApply(const Mtx src, Mtx dst, f32 xT, f32 yT, f32 zT);
void PSMTXTransApply(const Mtx src, Mtx dst, f32 xT, f32 yT, f32 zT);
void ASM_MTXTranspose(const Mtx src, Mtx xPose);
void C_MTXTranspose(const Mtx src, Mtx xPose);
void PSMTXTranspose(const Mtx src, Mtx xPose);
void ASM_QUATAdd(const Quaternion* p, const Quaternion* q, Quaternion* r);
void ASM_QUATDivide(const Quaternion* p, const Quaternion* q, Quaternion* r);
f32 ASM_QUATDotProduct(const Quaternion* p, const Quaternion* q);
void ASM_QUATInverse(const Quaternion* src, Quaternion* inv);
void ASM_QUATMultiply(const Quaternion* p, const Quaternion* q, Quaternion* pq);
void ASM_QUATNormalize(const Quaternion* src, Quaternion* unit);
void ASM_QUATScale(const Quaternion* q, Quaternion* r, f32 scale);
void ASM_QUATSubtract(const Quaternion* p, const Quaternion* q, Quaternion* r);
void ASM_VECAdd(const Vec* a, const Vec* b, Vec* ab);
void C_VECAdd(const Vec* a, const Vec* b, Vec* ab);
void PSVECAdd(const Vec* a, const Vec* b, Vec* ab);
void ASM_VECCrossProduct(const Vec* a, const Vec* b, Vec* axb);
void C_VECCrossProduct(const Vec* a, const Vec* b, Vec* axb);
void PSVECCrossProduct(const Vec* a, const Vec* b, Vec* axb);
f32 ASM_VECDistance(const Vec* a, const Vec* b);
f32 C_VECDistance(const Vec* a, const Vec* b);
f32 PSVECDistance(const Vec* a, const Vec* b);
f32 ASM_VECDotProduct(const Vec* a, const Vec* b);
f32 C_VECDotProduct(const Vec* a, const Vec* b);
f32 PSVECDotProduct(const Vec* a, const Vec* b);
void C_VECHalfAngle(const Vec* a, const Vec* b, Vec* half);
f32 ASM_VECMag(const Vec* v);
f32 C_VECMag(const Vec* v);
f32 PSVECMag(const Vec* v);
void ASM_VECNormalize(const Vec* src, Vec* unit);
void C_VECNormalize(const Vec* src, Vec* unit);
void PSVECNormalize(const Vec* src, Vec* unit);
void C_VECReflect(const Vec* src, const Vec* normal, Vec* dst);
void ASM_VECScale(const Vec* src, Vec* dst, f32 scale);
void C_VECScale(const Vec* src, Vec* dst, f32 scale);
void PSVECScale(const Vec* src, Vec* dst, f32 scale);
f32 ASM_VECSquareDistance(const Vec* a, const Vec* b);
f32 C_VECSquareDistance(const Vec* a, const Vec* b);
f32 PSVECSquareDistance(const Vec* a, const Vec* b);
f32 ASM_VECSquareMag(const Vec* v);
f32 C_VECSquareMag(const Vec* v);
f32 PSVECSquareMag(const Vec* v);
void ASM_VECSubtract(const Vec* a, const Vec* b, Vec* aMinusB);
void C_VECSubtract(const Vec* a, const Vec* b, Vec* aMinusB);
void PSVECSubtract(const Vec* a, const Vec* b, Vec* aMinusB);

#ifdef __cplusplus
}
#endif

#define MTXOrtho C_MTXOrtho
#define MTXPerspective C_MTXPerspective
#define MTXFrustum C_MTXFrustum
#define MTXLookAt C_MTXLookAt

#endif
