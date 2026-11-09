#ifndef FFL_DEFAULT_SHADER_H_
#define FFL_DEFAULT_SHADER_H_

#include <nn/ffl/FFLColor.h>
#include <nn/ffl/FFLResult.h>
#include <nn/ffl/FFLVec.h>

#include <cafe/gx2.h>
#include <cafe/mat.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum FFLDrawType
{
    FFL_DRAW_TYPE_FACELINE = 0,
    FFL_DRAW_TYPE_BEARD = 1,
    FFL_DRAW_TYPE_NOSE = 2,
    FFL_DRAW_TYPE_FOREHEAD = 3,
    FFL_DRAW_TYPE_HAIR = 4,
    FFL_DRAW_TYPE_CAP = 5,
    FFL_DRAW_TYPE_MASK = 6,
    FFL_DRAW_TYPE_NOSELINE = 7,
    FFL_DRAW_TYPE_GLASS = 8,
    FFL_DRAW_TYPE_MAX = 9
} FFLDrawType;

typedef enum FFLSpecularMode
{
    FFL_SPECULAR_MODE_BLINN = 0,
    FFL_SPECULAR_MODE_ANISOTROPIC = 1,
    FFL_SPECULAR_MODE_MAX = 2
} FFLSpecularMode;

typedef struct FFLDrawParam FFLDrawParam;
typedef struct FFLShaderCallback FFLShaderCallback;

typedef struct FFLDefaultShader
{
    u32 data[0x3A8 / sizeof(u32)];
} FFLDefaultShader;
NN_STATIC_ASSERT(sizeof(FFLDefaultShader) == 0x3A8);

u32 FFLGetBufferSizeDefaultShader(const void* pShaderData);

FFLResult FFLInitDefaultShader(FFLDefaultShader* pShader, void* pBuffer, const void* pShaderData);
void FFLDestroyDefaultShader(FFLDefaultShader* pShader);

void FFLSetDefaultShader(const FFLDefaultShader* pShader);

void FFLSetupInitCharModelDefaultShader(const FFLDefaultShader* pShader);
void FFLSetupInitCharModelDefaultShaderNoSetCallback(const FFLDefaultShader* pShader);

void FFLSetupDrawDefaultShader(const FFLDefaultShader* pShader);
void FFLSetupDrawDefaultShaderNoSetCallback(const FFLDefaultShader* pShader);

const FFLShaderCallback* FFLGetCallbackDefaultShader(const FFLDefaultShader* pShader);
const FFLShaderCallback* FFLGetShaderCallbackDefaultShader(const FFLDefaultShader* pShader);

GX2ContextState* FFLGetContextStateDefaultShader(FFLDefaultShader* pShader);
GX2Sampler* FFLGetSamplerDefaultShader(FFLDefaultShader* pShader);

void FFLSetMatrixDefaultShader(FFLDefaultShader* pShader, const Mat44* pProjection, const Mat34* pModelView);
void FFLSetMatrixExDefaultShader(FFLDefaultShader* pShader, const Mat44* pProjection, const Mat34* pModelView,
                                 const Mat34* pInverseTranspose);

void FFLSetCullBackDefaultShader(FFLDefaultShader* pShader, BOOL cullBack);

void FFLSetLightEnableDefaultShader(FFLDefaultShader* pShader, BOOL enable);
void FFLSetLightDirectionDefaultShader(FFLDefaultShader* pShader, const FFLVec3* pDirection);
void FFLSetLightAmbientDefaultShader(FFLDefaultShader* pShader, const FFLColor* pColor);
void FFLSetLightDiffuseDefaultShader(FFLDefaultShader* pShader, const FFLColor* pColor);
void FFLSetLightSpecularDefaultShader(FFLDefaultShader* pShader, const FFLColor* pColor);

void FFLSetMaterialAmbientDefaultShader(FFLDefaultShader* pShader, FFLDrawType type, const FFLColor* pColor);
void FFLSetMaterialDiffuseDefaultShader(FFLDefaultShader* pShader, FFLDrawType type, const FFLColor* pColor);
void FFLSetMaterialSpecularDefaultShader(FFLDefaultShader* pShader, FFLDrawType type, const FFLColor* pColor);
void FFLSetMaterialSpecularPowerDefaultShader(FFLDefaultShader* pShader, FFLDrawType type, f32 power);
void FFLSetMaterialSpecularModeDefaultShader(FFLDefaultShader* pShader, FFLDrawType type, FFLSpecularMode mode);

void FFLSetRimColorDefaultShader(FFLDefaultShader* pShader, const FFLColor* pColor);
void FFLSetRimPowerDefaultShader(FFLDefaultShader* pShader, f32 power);

void FFLpSetPolygonModeDefaultShader(GX2Boolean enable, GX2PolygonMode frontMode, GX2PolygonMode backMode);

#ifdef __cplusplus
}
#endif

#endif // FFL_DEFAULT_SHADER_H_
