#ifndef FFLI_DEFAULT_SHADER_H_
#define FFLI_DEFAULT_SHADER_H_

#include <nn/ffl/FFLColor.h>
#include <nn/ffl/FFLDefaultShader.h>
#include <nn/ffl/FFLShaderCallback.h>
#include <nn/ffl/FFLVec.h>

#include <nn/ffl/FFLiUtilShader.h>

#include <cafe/gx2.h>
#include <cafe/mat.h>

struct FFLDrawParam;

struct FFLiDefaultShaderLight
{
    BOOL enable;
    FFLVec3 direction;
    FFLColor ambient;
    FFLColor diffuse;
    FFLColor specular;
};
NN_STATIC_ASSERT(sizeof(FFLiDefaultShaderLight) == 0x40);

struct FFLiDefaultShaderMaterial
{
    FFLColor ambient;
    FFLColor diffuse;
    FFLColor specular;
    f32 specularPower;
    FFLSpecularMode specularMode;
};
NN_STATIC_ASSERT(sizeof(FFLiDefaultShaderMaterial) == 0x38);

struct FFLiDefaultShaderParam
{
    Mat44 projection;
    Mat44 modelView;
    f32 inverseTranspose[3][4];
    FFLiDefaultShaderLight light;
    FFLiDefaultShaderMaterial material[FFL_DRAW_TYPE_MAX];
    FFLColor rimColor;
    f32 rimPower;
    BOOL cullBack;
};
NN_STATIC_ASSERT(sizeof(FFLiDefaultShaderParam) == 0x300);

struct FFLiDefaultShader
{
    FFLShaderCallback callback;
    GX2VertexShader* pVertexShader;
    GX2PixelShader* pPixelShader;
    FFLiUtilShader::FetchShader fetchShader;
    GX2Sampler sampler;
    GX2Sampler eyebrowSampler;
    bool useEyebrowSampler;
    u32 projectionLocation;
    u32 modelViewLocation;
    u32 inverseTransposeLocation;
    u32 textureLocation;
    u32 modeLocation;
    u32 const1Location;
    u32 const2Location;
    u32 const3Location;
    u32 lightEnableLocation;
    u32 lightDirectionLocation;
    u32 lightAmbientLocation;
    u32 lightDiffuseLocation;
    u32 lightSpecularLocation;
    u32 materialAmbientLocation;
    u32 materialDiffuseLocation;
    u32 materialSpecularLocation;
    u32 materialSpecularPowerLocation;
    u32 materialSpecularModeLocation;
    u32 rimColorLocation;
    u32 rimPowerLocation;
    FFLiDefaultShaderParam param;
};
NN_STATIC_ASSERT(sizeof(FFLiDefaultShader) == 0x3A8);

u32 FFLiGetBufferSizeDefaultShader(const void* pShaderData);

FFLResult FFLiInitDefaultShader(FFLiDefaultShader* pShader, void* pBuffer, const void* pShaderData);
void FFLiDestroyDefaultShader(FFLiDefaultShader* pShader);

const FFLShaderCallback* FFLiGetCallbackDefaultShader(const FFLiDefaultShader* pShader);

void FFLiSetDefaultShader(const FFLiDefaultShader* pShader);

void FFLiSetupInitCharModelDefaultShader(const FFLiDefaultShader* pShader);
void FFLiSetupInitCharModelDefaultShaderNoSetCallback(const FFLiDefaultShader* pShader);

void FFLiSetupDrawDefaultShader(const FFLiDefaultShader* pShader);
void FFLiSetupDrawDefaultShaderNoSetCallback(const FFLiDefaultShader* pShader);

void FFLiDrawDefaultShader(FFLiDefaultShader* pShader, const FFLDrawParam* pDrawParam);

GX2ContextState* FFLiGetContextStateDefaultShader(FFLiDefaultShader* pShader);
GX2Sampler* FFLiGetSamplerDefaultShader(FFLiDefaultShader* pShader);

void FFLiSetMatrixDefaultShader(FFLiDefaultShader* pShader, const Mat44* pProjection, const Mat34* pModelView);
void FFLiSetMatrixExDefaultShader(FFLiDefaultShader* pShader, const Mat44* pProjection, const Mat34* pModelView,
                                  const Mat34* pInverseTranspose);

void FFLiSetCullBackDefaultShader(FFLiDefaultShader* pShader, BOOL cullBack);

void FFLiSetLightEnableDefaultShader(FFLiDefaultShader* pShader, bool enable);
void FFLiSetLightDirectionDefaultShader(FFLiDefaultShader* pShader, const FFLVec3& direction);
void FFLiSetLightAmbientDefaultShader(FFLiDefaultShader* pShader, const FFLColor& color);
void FFLiSetLightDiffuseDefaultShader(FFLiDefaultShader* pShader, const FFLColor& color);
void FFLiSetLightSpecularDefaultShader(FFLiDefaultShader* pShader, const FFLColor& color);

void FFLiSetMaterialAmbientDefaultShader(FFLiDefaultShader* pShader, FFLDrawType type, const FFLColor& color);
void FFLiSetMaterialDiffuseDefaultShader(FFLiDefaultShader* pShader, FFLDrawType type, const FFLColor& color);
void FFLiSetMaterialSpecularDefaultShader(FFLiDefaultShader* pShader, FFLDrawType type, const FFLColor& color);
void FFLiSetMaterialSpecularPowerDefaultShader(FFLiDefaultShader* pShader, FFLDrawType type, f32 power);
void FFLiSetMaterialSpecularModeDefaultShader(FFLiDefaultShader* pShader, FFLDrawType type, FFLSpecularMode mode);

void FFLiSetRimColorDefaultShader(FFLiDefaultShader* pShader, const FFLColor& color);
void FFLiSetRimPowerDefaultShader(FFLiDefaultShader* pShader, f32 power);

#endif // FFLI_DEFAULT_SHADER_H_
