#include <nn/ffl/FFLDefaultShader.h>

#include <nn/ffl/FFLiDefaultShader.h>

namespace
{

FFLiDefaultShader* GetShader(FFLDefaultShader* pShader)
{
    return reinterpret_cast<FFLiDefaultShader*>(pShader);
}

const FFLiDefaultShader* GetShader(const FFLDefaultShader* pShader)
{
    return reinterpret_cast<const FFLiDefaultShader*>(pShader);
}

}

u32 FFLGetBufferSizeDefaultShader(const void* pShaderData)
{
    return FFLiGetBufferSizeDefaultShader(pShaderData);
}

FFLResult FFLInitDefaultShader(FFLDefaultShader* pShader, void* pBuffer, const void* pShaderData)
{
    return FFLiInitDefaultShader(GetShader(pShader), pBuffer, pShaderData);
}

void FFLDestroyDefaultShader(FFLDefaultShader* pShader)
{
    FFLiDestroyDefaultShader(GetShader(pShader));
}

void FFLSetDefaultShader(const FFLDefaultShader* pShader)
{
    FFLiSetDefaultShader(GetShader(pShader));
}

void FFLSetupInitCharModelDefaultShader(const FFLDefaultShader* pShader)
{
    FFLiSetupInitCharModelDefaultShader(GetShader(pShader));
}

void FFLSetupInitCharModelDefaultShaderNoSetCallback(const FFLDefaultShader* pShader)
{
    FFLiSetupInitCharModelDefaultShaderNoSetCallback(GetShader(pShader));
}

void FFLSetupDrawDefaultShader(const FFLDefaultShader* pShader)
{
    FFLiSetupDrawDefaultShader(GetShader(pShader));
}

void FFLSetupDrawDefaultShaderNoSetCallback(const FFLDefaultShader* pShader)
{
    FFLiSetupDrawDefaultShaderNoSetCallback(GetShader(pShader));
}

const FFLShaderCallback* FFLGetCallbackDefaultShader(const FFLDefaultShader* pShader)
{
    return FFLiGetCallbackDefaultShader(GetShader(pShader));
}

const FFLShaderCallback* FFLGetShaderCallbackDefaultShader(const FFLDefaultShader* pShader)
{
    return FFLiGetCallbackDefaultShader(GetShader(pShader));
}

GX2ContextState* FFLGetContextStateDefaultShader(FFLDefaultShader* pShader)
{
    return FFLiGetContextStateDefaultShader(GetShader(pShader));
}

GX2Sampler* FFLGetSamplerDefaultShader(FFLDefaultShader* pShader)
{
    return FFLiGetSamplerDefaultShader(GetShader(pShader));
}

void FFLSetMatrixDefaultShader(FFLDefaultShader* pShader, const Mat44* pProjection, const Mat34* pModelView)
{
    FFLiSetMatrixDefaultShader(GetShader(pShader), pProjection, pModelView);
}

void FFLSetMatrixExDefaultShader(FFLDefaultShader* pShader, const Mat44* pProjection, const Mat34* pModelView,
                                 const Mat34* pInverseTranspose)
{
    FFLiSetMatrixExDefaultShader(GetShader(pShader), pProjection, pModelView, pInverseTranspose);
}

void FFLSetCullBackDefaultShader(FFLDefaultShader* pShader, BOOL cullBack)
{
    FFLiSetCullBackDefaultShader(GetShader(pShader), cullBack);
}

void FFLSetLightEnableDefaultShader(FFLDefaultShader* pShader, BOOL enable)
{
    FFLiSetLightEnableDefaultShader(GetShader(pShader), enable != FALSE);
}

void FFLSetLightDirectionDefaultShader(FFLDefaultShader* pShader, const FFLVec3* pDirection)
{
    FFLiSetLightDirectionDefaultShader(GetShader(pShader), *pDirection);
}

void FFLSetLightAmbientDefaultShader(FFLDefaultShader* pShader, const FFLColor* pColor)
{
    FFLiSetLightAmbientDefaultShader(GetShader(pShader), *pColor);
}

void FFLSetLightDiffuseDefaultShader(FFLDefaultShader* pShader, const FFLColor* pColor)
{
    FFLiSetLightDiffuseDefaultShader(GetShader(pShader), *pColor);
}

void FFLSetLightSpecularDefaultShader(FFLDefaultShader* pShader, const FFLColor* pColor)
{
    FFLiSetLightSpecularDefaultShader(GetShader(pShader), *pColor);
}

void FFLSetMaterialAmbientDefaultShader(FFLDefaultShader* pShader, FFLDrawType type, const FFLColor* pColor)
{
    FFLiSetMaterialAmbientDefaultShader(GetShader(pShader), type, *pColor);
}

void FFLSetMaterialDiffuseDefaultShader(FFLDefaultShader* pShader, FFLDrawType type, const FFLColor* pColor)
{
    FFLiSetMaterialDiffuseDefaultShader(GetShader(pShader), type, *pColor);
}

void FFLSetMaterialSpecularDefaultShader(FFLDefaultShader* pShader, FFLDrawType type, const FFLColor* pColor)
{
    FFLiSetMaterialSpecularDefaultShader(GetShader(pShader), type, *pColor);
}

void FFLSetMaterialSpecularPowerDefaultShader(FFLDefaultShader* pShader, FFLDrawType type, f32 power)
{
    FFLiSetMaterialSpecularPowerDefaultShader(GetShader(pShader), type, power);
}

void FFLSetMaterialSpecularModeDefaultShader(FFLDefaultShader* pShader, FFLDrawType type, FFLSpecularMode mode)
{
    FFLiSetMaterialSpecularModeDefaultShader(GetShader(pShader), type, mode);
}

void FFLSetRimColorDefaultShader(FFLDefaultShader* pShader, const FFLColor* pColor)
{
    FFLiSetRimColorDefaultShader(GetShader(pShader), *pColor);
}

void FFLSetRimPowerDefaultShader(FFLDefaultShader* pShader, f32 power)
{
    FFLiSetRimPowerDefaultShader(GetShader(pShader), power);
}
