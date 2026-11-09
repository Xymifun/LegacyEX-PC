#include "platform/wiiu/MiiIcon/PlainShader.hpp"

#include "game/client/render/Renderer.hpp"
#include "platform/wiiu/MiiIcon/MiiIconAssert.hpp"

#include <cafe/gfd.h>
#include <nn/ffl/FFLColor.h>
#include <nn/ffl/FFLDrawParam.h>
#include <nn/ffl/FFLShaderCallback.h>

s32 GetVertexUniformLocation(const GX2VertexShader* shader, const char* name)
{
    MII_ICON_ASSERT_POINTER(shader);
    MII_ICON_ASSERT_POINTER(name);

    s32 loc = GX2GetVertexUniformVarOffset(shader, name);
    MII_ICON_ASSERT(loc >= 0);
    return loc;
}

s32 GetPixelUniformLocation(const GX2PixelShader* shader, const char* name)
{
    MII_ICON_ASSERT_POINTER(shader);
    MII_ICON_ASSERT_POINTER(name);

    s32 loc = GX2GetPixelUniformVarOffset(shader, name);
    MII_ICON_ASSERT(loc >= 0);
    return loc;
}

s32 GetPixelSamplerLocation(const GX2PixelShader* shader, const char* name)
{
    GX2SamplerVar* var = GX2GetPixelSamplerVar(shader, name);
    s32 location = var == NULL ? -1 : static_cast<s32>(var->location);
    return location;
}

void InitVertexShader(GX2VertexShader** shader, const void* shaderData)
{
    MII_ICON_ASSERT_POINTER(shader);
    MII_ICON_ASSERT_POINTER(shaderData);

    u32 shaderCount = GFDGetVertexShaderCount(shaderData);

    MII_ICON_ASSERT_MESSAGE(shaderCount == 1, "The number of shaders must be 1, but %d", shaderCount);

    u32 headerSize = GFDGetVertexShaderHeaderSize(0, shaderData);
    u32 programSize = GFDGetVertexShaderProgramSize(0, shaderData);

    MII_ICON_ASSERT_MESSAGE(headerSize > 0, "headerSize must be more than zero.");
    MII_ICON_ASSERT_MESSAGE(programSize > 0, "programSize must be more than zero.");

    GX2VertexShader* header = static_cast<GX2VertexShader*>(gRendererInstance->MemoryAllocMEM2(headerSize, 1));

    MII_ICON_ASSERT_POINTER(header);

    void* program = gRendererInstance->MemoryAllocMEM2(programSize, GX2_SHADER_ALIGNMENT);

    MII_ICON_ASSERT_POINTER(program);

    GX2Boolean result = GFDGetVertexShader(header, program, 0, shaderData);

    MII_ICON_ASSERT_MESSAGE(result, "GFDGetVertexShader failed.");

    GX2Invalidate(GX2_INVALIDATE_CPU_SHADER, header->shaderPtr, header->shaderSize);
    *shader = header;
}

void InitPixelShader(GX2PixelShader** shader, const void* shaderData)
{
    MII_ICON_ASSERT_POINTER(shader);
    MII_ICON_ASSERT_POINTER(shaderData);

    u32 shaderCount = GFDGetPixelShaderCount(shaderData);

    MII_ICON_ASSERT_MESSAGE(shaderCount == 1, "The number of shaders must be 1, but %d", shaderCount);

    u32 headerSize = GFDGetPixelShaderHeaderSize(0, shaderData);
    u32 programSize = GFDGetPixelShaderProgramSize(0, shaderData);

    MII_ICON_ASSERT_MESSAGE(headerSize > 0, "headerSize must be more than zero.");
    MII_ICON_ASSERT_MESSAGE(programSize > 0, "programSize must be more than zero.");

    GX2PixelShader* header = static_cast<GX2PixelShader*>(gRendererInstance->MemoryAllocMEM2(headerSize, 1));

    MII_ICON_ASSERT_POINTER(header);

    void* program = gRendererInstance->MemoryAllocMEM2(programSize, GX2_SHADER_ALIGNMENT);

    MII_ICON_ASSERT_POINTER(program);

    GX2Boolean result = GFDGetPixelShader(header, program, 0, shaderData);

    MII_ICON_ASSERT_MESSAGE(result, "GFDGetVertexShader failed.");

    GX2Invalidate(GX2_INVALIDATE_CPU_SHADER, header->shaderPtr, header->shaderSize);
    *shader = header;
}

void InitFetchShader(FetchShader* fetchShader, const GX2VertexShader* vertexShader)
{
    MII_ICON_ASSERT_POINTER(fetchShader);
    MII_ICON_ASSERT_POINTER(vertexShader);

    s32 posLoc = GX2GetVertexAttribVarLocation(vertexShader, "a_position");
    s32 texLoc = GX2GetVertexAttribVarLocation(vertexShader, "a_texCoord");

    MII_ICON_ASSERT(posLoc >= 0 && texLoc >= 0);

    GX2InitAttribStream(&fetchShader->attribs[0], posLoc, 0, 0, GX2_ATTRIB_FORMAT_32_32_32_FLOAT);
    GX2InitAttribStream(&fetchShader->attribs[1], texLoc, 1, 0, GX2_ATTRIB_FORMAT_32_32_FLOAT);

    void* shaderPtr = gRendererInstance->MemoryAllocMEM2(GX2CalcFetchShaderSize(2), GX2_SHADER_ALIGNMENT);

    MII_ICON_ASSERT_POINTER(shaderPtr);

    GX2InitFetchShader(&fetchShader->fetchShader, shaderPtr, 2, fetchShader->attribs);
}

void InitShader(PlainShader* shader, const void* shaderData)
{
    MII_ICON_ASSERT_POINTER(shader);
    MII_ICON_ASSERT_POINTER(shaderData);

    InitVertexShader(&shader->vertexShader, shaderData);
    InitPixelShader(&shader->pixelShader, shaderData);
    InitFetchShader(&shader->fetchShader, shader->vertexShader);
}

void DestroyShader(PlainShader* shader)
{
    MII_ICON_ASSERT_POINTER(shader);

    if (shader->vertexShader != NULL)
    {
        if (shader->vertexShader->shaderPtr != NULL)
            gRendererInstance->MemoryFreeMEM2(shader->vertexShader->shaderPtr);
        gRendererInstance->MemoryFreeMEM2(shader->vertexShader);
    }

    if (shader->pixelShader != NULL)
    {
        if (shader->pixelShader->shaderPtr != NULL)
            gRendererInstance->MemoryFreeMEM2(shader->pixelShader->shaderPtr);
        gRendererInstance->MemoryFreeMEM2(shader->pixelShader);
    }

    if (shader->fetchShader.fetchShader.shaderPtr != NULL)
        gRendererInstance->MemoryFreeMEM2(shader->fetchShader.fetchShader.shaderPtr);

    if (shader->callback.pContextState != NULL)
        gRendererInstance->MemoryFreeMEM2(const_cast<GX2ContextState*>(shader->callback.pContextState));
}

u8 ModulateModeToValue(FFLModulateMode mode)
{
    switch (mode)
    {
    default:
        MII_ICON_INVALID_CONDITION(mode);
        return 0;
    case FFL_MODULATE_MODE_0:
        return 0;
    case FFL_MODULATE_MODE_1:
        return 1;
    case FFL_MODULATE_MODE_2:
        return 2;
    case FFL_MODULATE_MODE_3:
        return 3;
    case FFL_MODULATE_MODE_4:
        return 4;
    case FFL_MODULATE_MODE_5:
        return 5;
    }
}

void BindTexture(PlainShader* shader, const FFLModulateParam* modulateParam)
{
    MII_ICON_ASSERT_POINTER(shader);
    MII_ICON_ASSERT_POINTER(modulateParam);

    if (modulateParam->pGX2Texture != NULL)
    {
        GX2SetPixelTexture(modulateParam->pGX2Texture, shader->textureLocation);
        GX2SetPixelSampler(&shader->sampler, shader->textureLocation);
    }
}

void SetConstColor(u32 location, const FFLColor* color)
{
    MII_ICON_ASSERT_POINTER(color);

    GX2SetPixelUniformReg(location, 4, color);
}

void SetModulateMode(PlainShader* shader, FFLModulateMode mode)
{
    MII_ICON_ASSERT_POINTER(shader);

    u32 values[4] = {0, 0, 0, 0};
    values[0] = ModulateModeToValue(mode);
    GX2SetPixelUniformReg(shader->modeLocation, 4, values);
}

void SetModulate(PlainShader* shader, const FFLModulateParam* modulateParam)
{
    MII_ICON_ASSERT_POINTER(shader);
    MII_ICON_ASSERT_POINTER(modulateParam);

    BindTexture(shader, modulateParam);

    switch (modulateParam->mode)
    {
    case FFL_MODULATE_MODE_0:
    case FFL_MODULATE_MODE_3:
    case FFL_MODULATE_MODE_4:
    case FFL_MODULATE_MODE_5:
        SetConstColor(shader->const1Location, modulateParam->pColorR);
        break;
    case FFL_MODULATE_MODE_2:
        SetConstColor(shader->const1Location, modulateParam->pColorR);
        SetConstColor(shader->const2Location, modulateParam->pColorG);
        SetConstColor(shader->const3Location, modulateParam->pColorB);
        break;
    case FFL_MODULATE_MODE_1:
        break;
    }

    SetModulateMode(shader, modulateParam->mode);
}

void Draw(void* object, const FFLDrawParam* drawParam)
{
    MII_ICON_ASSERT_POINTER(object);
    MII_ICON_ASSERT_POINTER(drawParam);

    PlainShader* shader = static_cast<PlainShader*>(object);

    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_FALSE);
    SetModulate(shader, &drawParam->modulateParam);

    if (drawParam->primitiveParam.pIndexBuffer != NULL)
    {
        const FFLAttributeBuffer& position =
            drawParam->attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION];
        const FFLAttributeBuffer& texCoord =
            drawParam->attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD];

        GX2SetAttribBuffer(0, position.size, position.stride, position.ptr);
        GX2SetAttribBuffer(1, texCoord.size, texCoord.stride, texCoord.ptr);
        GX2DrawIndexedEx(drawParam->primitiveParam.primitiveType, drawParam->primitiveParam.indexCount,
                         drawParam->primitiveParam.indexFormat, drawParam->primitiveParam.pIndexBuffer, 0, 1);
    }
}

void SetMatrix(void* object, const Mat44* matrix)
{
    MII_ICON_ASSERT_POINTER(object);
    MII_ICON_ASSERT_POINTER(matrix);

    const PlainShader* shader = static_cast<const PlainShader*>(object);

    Mat44 transposed;
    MAT44Transpose(matrix, &transposed);
    GX2SetVertexUniformReg(shader->mvpLocation, 16, &transposed);
}

void InitPlainShader(PlainShader* shader, const void* shaderData)
{
    MII_ICON_ASSERT_POINTER(shader);
    MII_ICON_ASSERT_POINTER(shaderData);

    FFLShaderCallback& callback = shader->callback;
    callback.pObj = shader;

    GX2ContextState* contextState = static_cast<GX2ContextState*>(
        gRendererInstance->MemoryAllocMEM2(sizeof(GX2ContextState), GX2_CONTEXT_STATE_ALIGNMENT));

    callback.pContextState = contextState;
    callback.pDrawFunc = Draw;
    callback.pSetMatrixFunc = SetMatrix;

    GX2SetupContextState(contextState);
    GX2SetContextState(callback.pContextState);

    InitShader(shader, shaderData);

    GX2InitSampler(&shader->sampler, GX2_TEX_CLAMP_MIRROR, GX2_TEX_XY_FILTER_BILINEAR);

    shader->mvpLocation = GetVertexUniformLocation(shader->vertexShader, "u_mvp");
    shader->modeLocation = GetPixelUniformLocation(shader->pixelShader, "u_mode");
    shader->const1Location = GetPixelUniformLocation(shader->pixelShader, "u_const1");
    shader->const2Location = GetPixelUniformLocation(shader->pixelShader, "u_const2");
    shader->const3Location = GetPixelUniformLocation(shader->pixelShader, "u_const3");
    shader->textureLocation = GetPixelSamplerLocation(shader->pixelShader, "s_texture");
}

void DestroyPlainShader(PlainShader* shader)
{
    MII_ICON_ASSERT_POINTER(shader);

    DestroyShader(shader);
}

void BindPlainShader(const PlainShader* shader)
{
    MII_ICON_ASSERT_POINTER(shader);

    FFLSetShaderCallback(&shader->callback);

    GX2VertexShader* vertexShader = shader->vertexShader;
    GX2PixelShader* pixelShader = shader->pixelShader;

    GX2SetFetchShader(&shader->fetchShader.fetchShader);
    GX2SetVertexShader(vertexShader);
    GX2SetPixelShader(pixelShader);
}

void SetMatrixPlainShader(const PlainShader* shader, const Mat44* matrix)
{
    MII_ICON_ASSERT_POINTER(shader);
    MII_ICON_ASSERT_POINTER(matrix);

    Mat44 transposed;
    MAT44Transpose(matrix, &transposed);
    GX2SetVertexUniformReg(shader->mvpLocation, 16, &transposed);
}

void BindPlainShaderForIcon(const PlainShader* shader)
{
    GX2SetContextState(shader->callback.pContextState);
    BindPlainShader(shader);
}
