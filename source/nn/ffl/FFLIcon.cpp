#include <nn/ffl/FFLIcon.h>

#include <nn/ffl/FFLiIcon.h>
#include <nn/ffl/FFLiManager.h>
#include <nn/ffl/FFLiShaderCallback.h>

namespace
{

const FFLiCharModel* GetCharModel(const FFLCharModel* pModel)
{
    return reinterpret_cast<const FFLiCharModel*>(pModel);
}

}

u32 FFLGetIconBufferSize(const FFLIconSetting* pSetting)
{
    return FFLiGetIconBufferSize(pSetting);
}

u32 FFLGetIconTempBufferSize(const FFLIconSetting* pSetting)
{
    return FFLiGetIconTempBufferSize(pSetting);
}

FFLResult FFLMakeIconExWithCallback(GX2Texture* pTexture, const FFLCharModel* pModel, FFLExpression expression,
                                    const FFLIconSetting* pSetting, void* pBuffer, void* pTempBuffer,
                                    const FFLShaderCallback* pCallback)
{
    return FFLiMakeIconEx(pTexture, GetCharModel(pModel), expression, pSetting, pBuffer, pTempBuffer, pCallback);
}

FFLResult FFLMakeIconEx(GX2Texture* pTexture, const FFLCharModel* pModel, FFLExpression expression,
                        const FFLIconSetting* pSetting, void* pBuffer, void* pTempBuffer)
{
    const FFLShaderCallback* pCallback = NULL;
    if (FFLiManager::IsConstruct())
        pCallback = FFLiManager::GetInstance()->GetShaderCallback().Get();

    return FFLMakeIconExWithCallback(pTexture, pModel, expression, pSetting, pBuffer, pTempBuffer, pCallback);
}

void FFLGetIconMatrix(Mat44* pProjection, Mat34* pView, u16 width, u16 height)
{
    FFLiGetIconMatrix(pProjection, pView, width, height);
}
