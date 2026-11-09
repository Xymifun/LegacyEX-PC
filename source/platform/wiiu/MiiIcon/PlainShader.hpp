#ifndef PLATFORM_WIIU_MIIICON_PLAINSHADER_HPP
#define PLATFORM_WIIU_MIIICON_PLAINSHADER_HPP

#include <cafe/gx2.h>
#include <cafe/mat.h>
#include <nn/ffl/FFLModulateParam.h>
#include <nn/ffl/FFLShaderCallback.h>

struct FFLColor;
struct FFLDrawParam;

struct FetchShader
{
    GX2FetchShader fetchShader;
    GX2AttribStream attribs[2];
};

struct PlainShader
{
    FFLShaderCallback callback;
    GX2VertexShader* vertexShader;
    GX2PixelShader* pixelShader;
    FetchShader fetchShader;
    GX2Sampler sampler;
    u32 mvpLocation;
    u32 modeLocation;
    u32 const1Location;
    u32 const2Location;
    u32 const3Location;
    u32 textureLocation;
};

s32 GetVertexUniformLocation(const GX2VertexShader* shader, const char* name);
s32 GetPixelUniformLocation(const GX2PixelShader* shader, const char* name);
s32 GetPixelSamplerLocation(const GX2PixelShader* shader, const char* name);

void InitVertexShader(GX2VertexShader** shader, const void* shaderData);
void InitPixelShader(GX2PixelShader** shader, const void* shaderData);
void InitFetchShader(FetchShader* fetchShader, const GX2VertexShader* vertexShader);
void InitShader(PlainShader* shader, const void* shaderData);
void DestroyShader(PlainShader* shader);

u8 ModulateModeToValue(FFLModulateMode mode);
void BindTexture(PlainShader* shader, const FFLModulateParam* modulateParam);
void SetConstColor(u32 location, const FFLColor* color);
void SetModulateMode(PlainShader* shader, FFLModulateMode mode);
void SetModulate(PlainShader* shader, const FFLModulateParam* modulateParam);
void Draw(void* object, const FFLDrawParam* drawParam);
void SetMatrix(void* object, const Mat44* matrix);

void InitPlainShader(PlainShader* shader, const void* shaderData);
void DestroyPlainShader(PlainShader* shader);
void BindPlainShader(const PlainShader* shader);
void SetMatrixPlainShader(const PlainShader* shader, const Mat44* matrix);
void BindPlainShaderForIcon(const PlainShader* shader);

#endif
