#ifndef FFL_ICON_H_
#define FFL_ICON_H_

#include <nn/ffl/FFLColor.h>
#include <nn/ffl/FFLExpression.h>
#include <nn/ffl/FFLResult.h>

#include <cafe/gx2.h>
#include <cafe/mat.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum FFLIconBackground
{
    FFL_ICON_BACKGROUND_FAVORITE_COLOR = 0,
    FFL_ICON_BACKGROUND_DIRECT = 1
} FFLIconBackground;

typedef struct FFLIconSetting
{
    u16 width;
    u16 height;
    FFLIconBackground backgroundType;
    FFLColor backgroundColor;
    GX2SurfaceFormat format;
    BOOL drawXluOnly;
} FFLIconSetting;
NN_STATIC_ASSERT(sizeof(FFLIconSetting) == 0x20);

typedef struct FFLCharModel FFLCharModel;
typedef struct FFLShaderCallback FFLShaderCallback;

u32 FFLGetIconBufferSize(const FFLIconSetting* pSetting);
u32 FFLGetIconTempBufferSize(const FFLIconSetting* pSetting);

FFLResult FFLMakeIconEx(GX2Texture* pTexture, const FFLCharModel* pModel, FFLExpression expression,
                        const FFLIconSetting* pSetting, void* pBuffer, void* pTempBuffer);
FFLResult FFLMakeIconExWithCallback(GX2Texture* pTexture, const FFLCharModel* pModel, FFLExpression expression,
                                    const FFLIconSetting* pSetting, void* pBuffer, void* pTempBuffer,
                                    const FFLShaderCallback* pCallback);

void FFLGetIconMatrix(Mat44* pProjection, Mat34* pView, u16 width, u16 height);

#ifdef __cplusplus
}
#endif

#endif // FFL_ICON_H_
