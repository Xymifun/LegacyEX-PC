#ifndef FFLI_ICON_H_
#define FFLI_ICON_H_

#include <nn/ffl/FFLExpression.h>
#include <nn/ffl/FFLIcon.h>
#include <nn/ffl/FFLResult.h>

#include <cafe/gx2.h>
#include <cafe/mat.h>

struct FFLiCharModel;
struct FFLShaderCallback;

u32 FFLiGetIconBufferSize(const FFLIconSetting* pSetting);
u32 FFLiGetIconTempBufferSize(const FFLIconSetting* pSetting);

FFLResult FFLiMakeIconEx(GX2Texture* pTexture, const FFLiCharModel* pModel, FFLExpression expression,
                         const FFLIconSetting* pSetting, void* pBuffer, void* pTempBuffer,
                         const FFLShaderCallback* pCallback);

void FFLiGetIconMatrix(Mat44* pProjection, Mat34* pView, u16 width, u16 height);

#endif // FFLI_ICON_H_
