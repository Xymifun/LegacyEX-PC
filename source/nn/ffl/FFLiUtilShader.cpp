#include <nn/ffl/FFLiInvalidate.h>
#include <nn/ffl/FFLiUtil.h>
#include <nn/ffl/FFLiUtilShader.h>
#include <nn/ffl/FFLiUtilShaderAllocator.h>

#include <nn/ffl/detail/FFLiBug.h>

#include <cafe/gfd.h>

static void InvalidateFetchShader(GX2FetchShader* pShader);

static bool GetPixelUniformBlockLocation(u32* pLocation, const GX2PixelShader* pShader, const char* name);
static bool GetVertexUniformBlockLocation(u32* pLocation, const GX2VertexShader* pShader, const char* name);

class FFLiUtilShaderPixel
{
public:
    static u32 GetCount(const void* pShaderData) { return GFDGetPixelShaderCount(pShaderData); }

    static u32 GetHeaderSize(u32 shaderIndex, const void* pShaderData)
    {
        return GFDGetPixelShaderHeaderSize(shaderIndex, pShaderData);
    }

    static u32 GetProgramSize(u32 shaderIndex, const void* pShaderData)
    {
        return GFDGetPixelShaderProgramSize(shaderIndex, pShaderData);
    }

    static bool GetShader(GX2PixelShader* pHeader, void* pProgram, u32 shaderIndex, const void* pShaderData)
    {
        return GFDGetPixelShader(pHeader, pProgram, shaderIndex, pShaderData);
    }
};

class FFLiUtilShaderVertex
{
public:
    static u32 GetCount(const void* pShaderData) { return GFDGetVertexShaderCount(pShaderData); }

    static u32 GetHeaderSize(u32 shaderIndex, const void* pShaderData)
    {
        return GFDGetVertexShaderHeaderSize(shaderIndex, pShaderData);
    }

    static u32 GetProgramSize(u32 shaderIndex, const void* pShaderData)
    {
        return GFDGetVertexShaderProgramSize(shaderIndex, pShaderData);
    }

    static bool GetShader(GX2VertexShader* pHeader, void* pProgram, u32 shaderIndex, const void* pShaderData)
    {
        return GFDGetVertexShader(pHeader, pProgram, shaderIndex, pShaderData);
    }
};

u32 FFLiUtilShader::GetBufferSize(u32 numAttribs, const void* pShaderData, u32 shaderIndex)
{
    u32 ret = GX2_SHADER_ALIGNMENT;
    ret += GetProgramBufferSize(numAttribs, pShaderData, shaderIndex);
    ret += GetHeaderBufferSize(numAttribs, pShaderData, shaderIndex);

    return ret;
}

u32 FFLiUtilShader::GetBufferSizeForFetch(u32 numAttribs)
{
    u32 ret = GX2_SHADER_ALIGNMENT;
    ret += GetProgramBufferSizeForFetch(numAttribs);
    ret += GetHeaderBufferSizeForFetch(numAttribs);

    return ret;
}

u32 FFLiUtilShader::GetHeaderBufferSize(u32 numAttribs, const void* pShaderData, u32 shaderIndex)
{
    return GetHeaderBufferSizeForFetch(numAttribs)
           + FFLiRoundUp(GFDGetPixelShaderHeaderSize(shaderIndex, pShaderData), 4)
           + FFLiRoundUp(GFDGetVertexShaderHeaderSize(shaderIndex, pShaderData), 4);
}

u32 FFLiUtilShader::GetHeaderBufferSizeForFetch(u32 numAttribs)
{
    return FFLiRoundUp(sizeof(GX2AttribStream) * numAttribs, 4);
}

u32 FFLiUtilShader::GetProgramBufferSize(u32 numAttribs, const void* pShaderData, u32 shaderIndex)
{
    return GetProgramBufferSizeForFetch(numAttribs)
           + FFLiRoundUp(GFDGetPixelShaderProgramSize(shaderIndex, pShaderData), GX2_SHADER_ALIGNMENT)
           + FFLiRoundUp(GFDGetVertexShaderProgramSize(shaderIndex, pShaderData), GX2_SHADER_ALIGNMENT);
}

u32 FFLiUtilShader::GetProgramBufferSizeForFetch(u32 numAttribs)
{
    return FFLiRoundUp(GX2CalcFetchShaderSize(numAttribs), GX2_SHADER_ALIGNMENT);
}

bool FFLiUtilShader::BuildShader(FFLiUtilShader::FetchShader* pFetchShader, const GX2VertexShader* pVertexShader,
                                 FFLiUtilShaderAllocator* pAllocator,
                                 const FFLiUtilShader::AttributeStreamDesc* pAttribs, u32 numAttribs, bool invalidate)
{
    u32 size = GX2CalcFetchShaderSize(numAttribs);
    void* pBuffer = pAllocator->ProgramAllocator().Allocate(size, GX2_SHADER_ALIGNMENT);

    pFetchShader->pStreams = static_cast<GX2AttribStream*>(
        pAllocator->HeaderAllocator().Allocate(sizeof(GX2AttribStream) * numAttribs, 4));

    for (u32 i = 0; i < numAttribs; i++)
    {
        const FFLiUtilShader::AttributeStreamDesc& desc = pAttribs[i];
        s32 location = GX2GetVertexAttribVarLocation(pVertexShader, desc.name);
        if (location < 0)
            return false;

        GX2InitAttribStream(&(pFetchShader->pStreams[i]), location, desc.buffer, desc.offset, desc.format);
    }

    GX2InitFetchShader(&pFetchShader->fetchShader, pBuffer, numAttribs, pFetchShader->pStreams);
    FFLiBugEndianSwap(pBuffer, size);

    if (invalidate)
        InvalidateFetchShader(&pFetchShader->fetchShader);

    return true;
}

template <typename T, typename Shader>
bool FFLiUtilShader::BuildShaderImpl(Shader** ppShader, FFLiUtilShaderAllocator* pAllocator, const void* pShaderData,
                                     u32 shaderIndex, bool invalidate)
{
    if (shaderIndex >= T::GetCount(pShaderData))
        return false;

    u32 headerSize = T::GetHeaderSize(shaderIndex, pShaderData);
    u32 programSize = T::GetProgramSize(shaderIndex, pShaderData);

    if (headerSize == 0 || programSize == 0)
        return false;

    void* pHeader = pAllocator->HeaderAllocator().Allocate(headerSize, 4);
    void* pProgram = pAllocator->ProgramAllocator().Allocate(programSize, GX2_SHADER_ALIGNMENT);

    Shader* pShader = static_cast<Shader*>(pHeader);
    if (!T::GetShader(pShader, pProgram, shaderIndex, pShaderData))
        return false;

    FFLiBugEndianSwap(pProgram, programSize);

    *ppShader = pShader;

    if (invalidate)
        FFLiInvalidateShader(pShader->shaderPtr, pShader->shaderSize);

    return true;
}

bool FFLiUtilShader::BuildShader(GX2PixelShader** ppShader, FFLiUtilShaderAllocator* pAllocator,
                                 const void* pShaderData, u32 shaderIndex, bool invalidate)
{
    return BuildShaderImpl<FFLiUtilShaderPixel>(ppShader, pAllocator, pShaderData, shaderIndex, invalidate);
}

bool FFLiUtilShader::BuildShader(GX2VertexShader** ppShader, FFLiUtilShaderAllocator* pAllocator,
                                 const void* pShaderData, u32 shaderIndex, bool invalidate)
{
    return BuildShaderImpl<FFLiUtilShaderVertex>(ppShader, pAllocator, pShaderData, shaderIndex, invalidate);
}

const GX2UniformVar* FFLiUtilShader::GetUniformVar(const GX2PixelShader* pShader, const char* name)
{
    return GX2GetPixelUniformVar(pShader, name);
}

const GX2UniformVar* FFLiUtilShader::GetUniformVar(const GX2VertexShader* pShader, const char* name)
{
    return GX2GetVertexUniformVar(pShader, name);
}

const GX2UniformBlock* FFLiUtilShader::GetUniformBlock(const GX2PixelShader* pShader, const char* name)
{
    return GX2GetPixelUniformBlock(pShader, name);
}

const GX2UniformBlock* FFLiUtilShader::GetUniformBlock(const GX2VertexShader* pShader, const char* name)
{
    return GX2GetVertexUniformBlock(pShader, name);
}

bool FFLiUtilShader::GetUniformLocation(u32* pLocation, const GX2PixelShader* pShader, const char* name)
{
    s32 location = GX2GetPixelUniformVarOffset(pShader, name);
    if (location < 0)
        return false;

    *pLocation = location;
    return true;
}

bool FFLiUtilShader::GetUniformLocation(u32* pLocation, const GX2VertexShader* pShader, const char* name)
{
    s32 location = GX2GetVertexUniformVarOffset(pShader, name);
    if (location < 0)
        return false;

    *pLocation = location;
    return true;
}

bool FFLiUtilShader::GetUniformBlockLocation(u32* pLocation, const GX2PixelShader* pShader, const char* name)
{
    return GetPixelUniformBlockLocation(pLocation, pShader, name);
}

bool FFLiUtilShader::GetUniformBlockLocation(u32* pLocation, const GX2VertexShader* pShader, const char* name)
{
    return GetVertexUniformBlockLocation(pLocation, pShader, name);
}

bool FFLiUtilShader::GetSamplerLocation(u32* pLocation, const GX2PixelShader* pShader, const char* name)
{
    s32 location = GX2GetPixelSamplerVarLocation(pShader, name);
    if (location < 0)
        return false;

    *pLocation = location;
    return true;
}

static void InvalidateFetchShader(GX2FetchShader* pShader)
{
    FFLiInvalidateShader(pShader->shaderPtr, pShader->shaderSize);
}

static bool GetPixelUniformBlockLocation(u32* pLocation, const GX2PixelShader* pShader, const char* name)
{
    const GX2UniformBlock* pUB = GX2GetPixelUniformBlock(pShader, name);
    if (pUB == NULL)
        return false;

    *pLocation = pUB->location;
    return true;
}

static bool GetVertexUniformBlockLocation(u32* pLocation, const GX2VertexShader* pShader, const char* name)
{
    const GX2UniformBlock* pUB = GX2GetVertexUniformBlock(pShader, name);
    if (pUB == NULL)
        return false;

    *pLocation = pUB->location;
    return true;
}
