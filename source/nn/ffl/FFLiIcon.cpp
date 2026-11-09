#include <nn/ffl/FFLCharModel.h>
#include <nn/ffl/FFLColor.h>

#include <nn/ffl/FFLiCharModel.h>
#include <nn/ffl/FFLiColor.h>
#include <nn/ffl/FFLiIcon.h>
#include <nn/ffl/FFLiManager.h>
#include <nn/ffl/FFLiRenderTexture.h>
#include <nn/ffl/FFLiShaderCallback.h>

#include <nn/ffl/detail/FFLiBufferAllocator.h>

#include <math.h>

namespace
{

bool IsExpressionContained(const FFLiCharModel* pModel, FFLExpression expression)
{
    return (pModel->charModelDesc.expressionFlag & (1 << expression)) != 0;
}

FFLResult MakeIconImpl(GX2Texture* pTexture, const FFLiCharModel* pModel, FFLExpression expression,
                       const FFLIconSetting* pSetting, void* pBuffer, void* pTempBuffer,
                       const FFLShaderCallback* pCallback, bool waitDrawDone)
{
    if (!FFLiManager::IsConstruct())
        return FFL_RESULT_MANAGER_NOT_CONSTRUCT;

    FFLiManager* pManager = FFLiManager::GetInstance();

    FFLiCharModel charModel = *pModel;
    FFLCharModel* pCharModel = reinterpret_cast<FFLCharModel*>(&charModel);

    if (!IsExpressionContained(pModel, expression))
        return FFL_RESULT_ERROR;

    FFLSetExpression(pCharModel, expression);

    FFLiBufferAllocator allocator;
    allocator.Init(pBuffer, FFLiGetIconBufferSize(pSetting));

    FFLiBufferAllocator tempAllocator;
    tempAllocator.Init(pTempBuffer, FFLiGetIconTempBufferSize(pSetting));

    FFLiRenderTexture* pRenderTexture = static_cast<FFLiRenderTexture*>(
        tempAllocator.Allocate(sizeof(FFLiRenderTexture), 4));
    FFLiInitRenderTexture(pRenderTexture, pSetting->width, pSetting->height, pSetting->format, 1, &allocator);

    u32 depthBufferSize = FFLiGetDepthBufferSizeRenderTexture(pSetting->width, pSetting->height);
    void* pDepthBuffer = tempAllocator.Allocate(depthBufferSize);

    FFLColor clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
    switch (pSetting->backgroundType)
    {
    case FFL_ICON_BACKGROUND_FAVORITE_COLOR:
        clearColor = FFLiGetFavoriteColor(pModel->charInfo.favoriteColor);
        break;
    case FFL_ICON_BACKGROUND_DIRECT:
        clearColor = pSetting->backgroundColor;
        break;
    }

    GX2SetAlphaTest(GX2_FALSE, GX2_COMPARE_ALWAYS, 0.0f);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, 0xFF, GX2_FALSE, GX2_TRUE);
    GX2SetDepthOnlyControl(GX2_TRUE, GX2_TRUE, GX2_COMPARE_LESS);
    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_FALSE);
    GX2SetBlendControl(GX2_RENDER_TARGET_0, GX2_BLEND_SRC_ALPHA, GX2_BLEND_ONE_MINUS_SRC_ALPHA, GX2_BLEND_COMBINE_ADD,
                       GX2_TRUE, GX2_BLEND_SRC_ALPHA, GX2_BLEND_DST_ALPHA, GX2_BLEND_COMBINE_ADD);

    FFLiInvalidateRenderTexture(pRenderTexture);
    FFLiSetupRenderTexture(pRenderTexture, &clearColor, pDepthBuffer, 0, pRenderTexture->gx2Texture.surface.format,
                           &pManager->GetShaderCallback());

    if (!pSetting->drawXluOnly)
        FFLDrawOpaWithCallback(pCharModel, pCallback);

    FFLDrawXluWithCallback(pCharModel, pCallback);

    FFLiFlushRenderTexture(pRenderTexture);

    if (waitDrawDone)
        GX2DrawDone();

    *pTexture = pRenderTexture->gx2Texture;
    return FFL_RESULT_OK;
}

f32 RadianToDegree(f32 radian)
{
    return radian * 180.0f / 3.14159f;
}

}

u32 FFLiGetIconBufferSize(const FFLIconSetting* pSetting)
{
    return FFLiGetBufferRenderTexture(pSetting->width, pSetting->height, pSetting->format, 1);
}

u32 FFLiGetIconTempBufferSize(const FFLIconSetting* pSetting)
{
    return FFLiGetDepthBufferSizeRenderTexture(pSetting->width, pSetting->height) + sizeof(FFLiRenderTexture);
}

FFLResult FFLiMakeIconEx(GX2Texture* pTexture, const FFLiCharModel* pModel, FFLExpression expression,
                         const FFLIconSetting* pSetting, void* pBuffer, void* pTempBuffer,
                         const FFLShaderCallback* pCallback)
{
    return MakeIconImpl(pTexture, pModel, expression, pSetting, pBuffer, pTempBuffer, pCallback, false);
}

void FFLiGetIconMatrix(Mat44* pProjection, Mat34* pView, u16 width, u16 height)
{
    f32 aspect = f32(width) / f32(height);
    f32 heightScale = 43.2f / (width < height ? aspect : 1.0f);
    f32 fovy = RadianToDegree(atan2f(heightScale, 500.0f)) * 2.0f;
    MTXPerspective(pProjection->mtx, fovy, aspect, 500.0f, 700.0f);

    static const Vec cameraUp = {0.0f, 1.0f, 0.0f};
    static const Vec target = {0.0f, 34.5f, 0.0f};
    Vec cameraPosition = {0.0f, 34.5f, 600.0f};
    MTXLookAt(pView->mtx, &cameraPosition, &cameraUp, &target);
}
