#ifndef CAFE_GX2_H
#define CAFE_GX2_H

#include <string.h>

#include <cafe/gx2/gx2Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

void GX2Invalidate(GX2InvalidateType type, void* ptr, u32 size);
void GX2CalcSurfaceSizeAndAlignment(GX2Surface* surface);
void GX2InitTextureRegs(GX2Texture* texture);
void GX2InitColorBufferRegs(GX2ColorBuffer* colorBuffer);
void GX2InitDepthBufferRegs(GX2DepthBuffer* depthBuffer);
void GX2InitSampler(GX2Sampler* sampler, GX2TexClampMode clampAll, GX2TexXYFilterType minMagFilter);
void GX2InitSamplerLOD(GX2Sampler* sampler, f32 minLod, f32 maxLod, f32 lodBias);
u32 GX2CalcFetchShaderSizeEx(u32 numAttribs, GX2FetchShaderType type, GX2TessellationMode tessellationMode);
void GX2InitFetchShaderEx(GX2FetchShader* fetchShader, void* shaderPtr, u32 numAttribs, const GX2AttribStream* attribs,
                          GX2FetchShaderType type, GX2TessellationMode tessellationMode);
void GX2SetupContextStateEx(GX2ContextState* contextState, GX2Boolean enableProfiling);
void GX2SetContextState(const GX2ContextState* contextState);
void GX2SetDefaultState(void);
void GX2SetShaderModeEx(GX2ShaderMode mode, u32 numVsGprs, u32 numVsStackEntries, u32 numGsGprs, u32 numGsStackEntries,
                        u32 numPsGprs, u32 numPsStackEntries);
void GX2SetFetchShader(const GX2FetchShader* fetchShader);
void GX2SetVertexShader(const GX2VertexShader* vertexShader);
void GX2SetPixelShader(const GX2PixelShader* pixelShader);
void GX2SetVertexUniformReg(u32 offset, u32 count, const void* values);
void GX2SetPixelUniformReg(u32 offset, u32 count, const void* values);
void GX2SetVertexUniformBlock(u32 location, u32 size, const void* data);
void GX2SetPixelUniformBlock(u32 location, u32 size, const void* data);
void GX2SetPixelTexture(const GX2Texture* texture, u32 unit);
void GX2SetPixelSampler(const GX2Sampler* sampler, u32 unit);
void GX2SetAttribBuffer(u32 index, u32 size, u32 stride, const void* ptr);
void GX2SetColorBuffer(const GX2ColorBuffer* colorBuffer, GX2RenderTarget target);
void GX2SetDepthBuffer(const GX2DepthBuffer* depthBuffer);
void GX2SetViewport(f32 originX, f32 originY, f32 width, f32 height, f32 nearZ, f32 farZ);
void GX2SetScissor(u32 left, u32 top, u32 width, u32 height);
void GX2SetAlphaTest(GX2Boolean alphaTestEnable, GX2CompareFunction alphaFunc, f32 ref);
void GX2SetDepthOnlyControl(GX2Boolean depthEnable, GX2Boolean depthWriteEnable, GX2CompareFunction depthFunc);
void GX2SetCullOnlyControl(GX2FrontFaceMode frontFaceMode, GX2Boolean cullFront, GX2Boolean cullBack);
void GX2SetColorControl(GX2LogicOp logicOp, u8 blendEnableMask, GX2Boolean multiWriteEnable,
                        GX2Boolean colorBufferEnable);
void GX2SetBlendControl(GX2RenderTarget target, GX2BlendFunction colorSrcBlend, GX2BlendFunction colorDstBlend,
                        GX2BlendCombine colorCombine, GX2Boolean separateAlphaBlend, GX2BlendFunction alphaSrcBlend,
                        GX2BlendFunction alphaDstBlend, GX2BlendCombine alphaCombine);
void GX2SetTargetChannelMasks(GX2ChannelMask target0, GX2ChannelMask target1, GX2ChannelMask target2,
                              GX2ChannelMask target3, GX2ChannelMask target4, GX2ChannelMask target5,
                              GX2ChannelMask target6, GX2ChannelMask target7);
void GX2DrawEx(GX2PrimitiveType mode, u32 count, u32 firstVertex, u32 numInstances);
void GX2DrawIndexedEx(GX2PrimitiveType mode, u32 count, GX2IndexFormat indexFormat, const void* indexPtr,
                      u32 baseVertex, u32 numInstances);
void GX2ClearColor(GX2ColorBuffer* colorBuffer, f32 red, f32 green, f32 blue, f32 alpha);
void GX2ClearDepthStencilEx(GX2DepthBuffer* depthBuffer, f32 depthValue, u8 stencilValue, GX2ClearMode clearMode);
void GX2Init(u32* initAttribs);
void GX2Shutdown(void);
void GX2Flush(void);
void GX2DrawDone(void);
void GX2SwapScanBuffers(void);
void GX2SetSwapInterval(u32 swapInterval);
u64 GX2GetLastSubmittedTimeStamp(void);
GX2Boolean GX2WaitTimeStamp(u64 timeStamp);
GX2TVScanMode GX2GetSystemTVScanMode(void);
void GX2CalcTVSize(GX2TVRenderMode tvRenderMode, GX2SurfaceFormat format, GX2BufferingMode bufferingMode, u32* size,
                   GX2Boolean* scaleNeeded);
void GX2CalcDRCSize(GX2DRCMode drcMode, GX2SurfaceFormat format, GX2BufferingMode bufferingMode, u32* size,
                    GX2Boolean* scaleNeeded);
void GX2SetTVBuffer(void* buffer, u32 size, GX2TVRenderMode tvRenderMode, GX2SurfaceFormat format,
                    GX2BufferingMode bufferingMode);
void GX2SetDRCBuffer(void* buffer, u32 size, GX2DRCMode drcMode, GX2SurfaceFormat format,
                     GX2BufferingMode bufferingMode);
void GX2SetTVEnable(GX2Boolean enable);
void GX2SetDRCEnable(GX2Boolean enable);
void GX2CopyColorBufferToScanBuffer(const GX2ColorBuffer* colorBuffer, GX2ScanTarget scanTarget);
void GX2CalcDepthBufferHiZInfo(const GX2DepthBuffer* depthBuffer, u32* size, u32* alignment);
void GX2InitDepthBufferHiZEnable(GX2DepthBuffer* depthBuffer, GX2Boolean hiZEnable);
u32 GX2GetSurfaceMipPitch(const GX2Surface* surface, u32 level);
void GX2InitSamplerClamping(GX2Sampler* sampler, GX2TexClampMode clampX, GX2TexClampMode clampY,
                            GX2TexClampMode clampZ);
void GX2InitSamplerXYFilter(GX2Sampler* sampler, GX2TexXYFilterType magFilter, GX2TexXYFilterType minFilter,
                            GX2TexAnisoRatio maxAniso);
void GX2InitSamplerZMFilter(GX2Sampler* sampler, GX2TexZFilterType zFilter, GX2TexMipFilterType mipFilter);
void GX2SetVertexTexture(const GX2Texture* texture, u32 unit);
void GX2SetVertexSampler(const GX2Sampler* sampler, u32 unit);
void GX2SetDepthStencilControl(GX2Boolean depthEnable, GX2Boolean depthWriteEnable, GX2CompareFunction depthFunc,
                               GX2Boolean stencilTestEnable, GX2Boolean backStencilEnable,
                               GX2CompareFunction frontStencilFunc, GX2StencilFunction frontStencilZPass,
                               GX2StencilFunction frontStencilZFail, GX2StencilFunction frontStencilFail,
                               GX2CompareFunction backStencilFunc, GX2StencilFunction backStencilZPass,
                               GX2StencilFunction backStencilZFail, GX2StencilFunction backStencilFail);
void GX2SetStencilMask(u8 preMaskFront, u8 writeMaskFront, u8 refFront, u8 preMaskBack, u8 writeMaskBack, u8 refBack);
void GX2SetPolygonControl(GX2FrontFaceMode frontFaceMode, GX2Boolean cullFront, GX2Boolean cullBack,
                          GX2Boolean enablePolygonModes, GX2PolygonMode polygonModeFront,
                          GX2PolygonMode polygonModeBack, GX2Boolean polyOffsetFrontEnable,
                          GX2Boolean polyOffsetBackEnable, GX2Boolean pointLineOffsetEnable);
void GX2SetPolygonOffset(f32 frontOffset, f32 frontSlopeScale, f32 backOffset, f32 backSlopeScale, f32 clamp);
void GX2SetBlendConstantColor(f32 red, f32 green, f32 blue, f32 alpha);

#ifdef __cplusplus
}

inline GX2ChannelMask operator|(GX2ChannelMask a, GX2ChannelMask b)
{
    return static_cast<GX2ChannelMask>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline GX2InvalidateType operator|(GX2InvalidateType a, GX2InvalidateType b)
{
    return static_cast<GX2InvalidateType>(static_cast<u32>(a) | static_cast<u32>(b));
}
#endif

inline void GX2InitTexture(GX2Texture* texture, u32 width, u32 height, u32 depth, u32 numMips, GX2SurfaceFormat format,
                           GX2SurfaceDim dim)
{
    const u32 compSel[54] = {
        GX2_COMP_SEL_NONE, GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE,
        GX2_COMP_SEL_X001, GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZ1,
        GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_WZYX, GX2_COMP_SEL_X001, GX2_COMP_SEL_X001,
        GX2_COMP_SEL_XY01, GX2_COMP_SEL_XY01, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE,
        GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE,
        GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_WZYX, GX2_COMP_SEL_XY01, GX2_COMP_SEL_XY01,
        GX2_COMP_SEL_XY01, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_NONE, GX2_COMP_SEL_XYZW,
        GX2_COMP_SEL_XYZW, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_XYZ1,
        GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_NONE,
        GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZW,
        GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01};
    texture->surface.dim = dim;
    texture->surface.width = width;
    texture->surface.height = height;
    texture->surface.depth = depth;
    texture->surface.numMips = numMips;
    texture->surface.format = format;
    texture->surface.aa = GX2_AA_MODE_1X;
    texture->surface.use = GX2_SURFACE_USE_TEXTURE;
    texture->surface.tileMode = GX2_TILE_MODE_DEFAULT;
    texture->surface.swizzle = 0;
    texture->viewFirstMip = 0;
    texture->viewNumMips = numMips;
    texture->viewFirstSlice = 0;
    texture->viewNumSlices = depth;
    texture->compSel = compSel[format & 0x3f];
    GX2CalcSurfaceSizeAndAlignment(&texture->surface);
    GX2InitTextureRegs(texture);
}

inline void GX2InitTexturePtrs(GX2Texture* texture, void* imagePtr, void* mipPtr)
{
    texture->surface.imagePtr = imagePtr;
    if (texture->surface.numMips > 1 && mipPtr == NULL)
        texture->surface.mipPtr = static_cast<u8*>(imagePtr) + texture->surface.mipOffset[0];
    else
        texture->surface.mipPtr = mipPtr;
}

inline void _GX2InitColorBuffer(GX2ColorBuffer* colorBuffer, u32 width, u32 height, GX2SurfaceFormat format,
                                GX2AAMode aa)
{
    colorBuffer->surface.dim = GX2_SURFACE_DIM_2D;
    colorBuffer->surface.numMips = 1;
    colorBuffer->surface.width = width;
    colorBuffer->surface.height = height;
    colorBuffer->surface.depth = 1;
    colorBuffer->surface.format = format;
    colorBuffer->surface.mipPtr = NULL;
    colorBuffer->surface.tileMode = GX2_TILE_MODE_DEFAULT;
    colorBuffer->surface.swizzle = 0;
    colorBuffer->surface.aa = aa;
    colorBuffer->viewMip = 0;
    colorBuffer->viewFirstSlice = 0;
    colorBuffer->viewNumSlices = 1;
    GX2CalcSurfaceSizeAndAlignment(&colorBuffer->surface);
    GX2InitColorBufferRegs(colorBuffer);
}

inline void GX2InitColorBuffer(GX2ColorBuffer* colorBuffer, u32 width, u32 height, GX2SurfaceFormat format,
                               GX2AAMode aa)
{
    colorBuffer->surface.use = GX2_SURFACE_USE_COLOR_BUFFER_TEXTURE;
    _GX2InitColorBuffer(colorBuffer, width, height, format, aa);
}

inline void GX2InitColorBufferFTV(GX2ColorBuffer* colorBuffer, u32 width, u32 height, GX2SurfaceFormat format,
                                  GX2AAMode aa)
{
    colorBuffer->surface.use = GX2_SURFACE_USE_COLOR_BUFFER_TEXTURE_FTV;
    _GX2InitColorBuffer(colorBuffer, width, height, format, aa);
    if (colorBuffer->surface.aa != aa)
    {
        GX2Surface surface = colorBuffer->surface;
        surface.aa = aa;
        surface.use = GX2_SURFACE_USE_COLOR_BUFFER_TEXTURE;
        GX2CalcSurfaceSizeAndAlignment(&surface);
        colorBuffer->surface.imageSize = surface.imageSize;
        colorBuffer->surface.alignment = surface.alignment;
    }
}

inline void GX2InitDepthBuffer(GX2DepthBuffer* depthBuffer, u32 width, u32 height, GX2SurfaceFormat format,
                               GX2AAMode aa)
{
    depthBuffer->surface.dim = GX2_SURFACE_DIM_2D;
    depthBuffer->surface.width = width;
    depthBuffer->surface.height = height;
    depthBuffer->surface.depth = 1;
    depthBuffer->surface.numMips = 1;
    depthBuffer->surface.format = format;
    depthBuffer->surface.aa = aa;
    if (format == GX2_SURFACE_FORMAT_D_D24_S8_UNORM || format == GX2_SURFACE_FORMAT_D_D24_S8_FLOAT)
        depthBuffer->surface.use = GX2_SURFACE_USE_DEPTH_BUFFER;
    else
        depthBuffer->surface.use = GX2_SURFACE_USE_DEPTH_BUFFER_TEXTURE;
    depthBuffer->clearDepth = 1.0f;
    depthBuffer->viewMip = 0;
    depthBuffer->viewNumSlices = 1;
    depthBuffer->viewFirstSlice = 0;
    depthBuffer->hiZSize = 0;
    depthBuffer->clearStencil = 0;
    depthBuffer->hiZPtr = NULL;
    depthBuffer->surface.tileMode = GX2_TILE_MODE_DEFAULT;
    depthBuffer->surface.swizzle = 0;
    GX2CalcSurfaceSizeAndAlignment(&depthBuffer->surface);
    GX2InitDepthBufferRegs(depthBuffer);
}

inline void GX2InitColorBufferPtr(GX2ColorBuffer* colorBuffer, void* ptr)
{
    colorBuffer->surface.imagePtr = ptr;
}

inline void GX2InitDepthBufferPtr(GX2DepthBuffer* depthBuffer, void* ptr)
{
    depthBuffer->surface.imagePtr = ptr;
}

inline void GX2InitAttribStream(GX2AttribStream* attribStream, u32 location, u32 buffer, u32 offset,
                                GX2AttribFormat format)
{
    const u32 destSel[20] = {GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01, GX2_COMP_SEL_X001, GX2_COMP_SEL_X001,
                             GX2_COMP_SEL_XY01, GX2_COMP_SEL_X001, GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01,
                             GX2_COMP_SEL_XY01, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW,
                             GX2_COMP_SEL_XY01, GX2_COMP_SEL_XY01, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW,
                             GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW};
    attribStream->location = location;
    attribStream->buffer = buffer;
    attribStream->offset = offset;
    attribStream->indexType = GX2_ATTRIB_INDEX_VERTEX_ID;
    attribStream->format = format;
    attribStream->endianSwap = GX2_ENDIANSWAP_DEFAULT;
    attribStream->destSel = destSel[format & 0xff];
    attribStream->aluDivisor = 0;
}

inline u32 GX2CalcFetchShaderSize(u32 numAttribs)
{
    return GX2CalcFetchShaderSizeEx(numAttribs, GX2_FETCH_SHADER_TESSELATION_NONE, GX2_TESSELLATION_MODE_DISCRETE);
}

inline void GX2InitFetchShader(GX2FetchShader* fetchShader, void* shaderPtr, u32 numAttribs,
                               const GX2AttribStream* attribs)
{
    GX2InitFetchShaderEx(fetchShader, shaderPtr, numAttribs, attribs, GX2_FETCH_SHADER_TESSELATION_NONE,
                         GX2_TESSELLATION_MODE_DISCRETE);
}

inline void GX2SetupContextState(GX2ContextState* contextState)
{
    GX2SetupContextStateEx(contextState, GX2_TRUE);
}

inline void GX2SetShaderMode(GX2ShaderMode mode)
{
    if (mode == GX2_SHADER_MODE_GEOMETRY_SHADER)
        GX2SetShaderModeEx(mode, 0x2c, 0x20, 0x40, 0x30, 0x4c, 0xb0);
    else
        GX2SetShaderModeEx(mode, 0x30, 0x40, 0, 0, 0xc8, 0xc0);
}

inline void GX2Draw(GX2PrimitiveType mode, u32 count)
{
    GX2DrawEx(mode, count, 0, 1);
}

inline void GX2DrawIndexed(GX2PrimitiveType mode, u32 count, GX2IndexFormat indexFormat, const void* indexPtr)
{
    GX2DrawIndexedEx(mode, count, indexFormat, indexPtr, 0, 1);
}

inline void GX2ClearDepthStencil(GX2DepthBuffer* depthBuffer, GX2ClearMode clearMode)
{
    GX2ClearDepthStencilEx(depthBuffer, depthBuffer->clearDepth, static_cast<u8>(depthBuffer->clearStencil), clearMode);
}

inline GX2UniformBlock* GX2GetVertexUniformBlock(const GX2VertexShader* shader, const char* name)
{
    for (u32 i = 0; i < shader->numUniformBlocks; ++i)
    {
        if (strcmp(shader->uniformBlocks[i].name, name) == 0)
            return &shader->uniformBlocks[i];
    }
    return NULL;
}

inline GX2UniformBlock* GX2GetPixelUniformBlock(const GX2PixelShader* shader, const char* name)
{
    for (u32 i = 0; i < shader->numUniformBlocks; ++i)
    {
        if (strcmp(shader->uniformBlocks[i].name, name) == 0)
            return &shader->uniformBlocks[i];
    }
    return NULL;
}

inline GX2UniformVar* GX2GetVertexUniformVar(const GX2VertexShader* shader, const char* name)
{
    u32 i = 0;
    while (i < shader->numUniforms)
    {
        if (strcmp(shader->uniformVars[i].name, name) == 0)
            return &shader->uniformVars[i];
        ++i;
    }
    return NULL;
}

inline GX2UniformVar* GX2GetPixelUniformVar(const GX2PixelShader* shader, const char* name)
{
    u32 i = 0;
    while (i < shader->numUniforms)
    {
        if (strcmp(shader->uniformVars[i].name, name) == 0)
            return &shader->uniformVars[i];
        ++i;
    }
    return NULL;
}

inline s32 GX2GetVertexUniformVarOffset(const GX2VertexShader* shader, const char* name)
{
    GX2UniformVar* uniformVar = GX2GetVertexUniformVar(shader, name);
    return uniformVar == NULL ? GX2_UNIFORM_VAR_INVALID_OFFSET : static_cast<s32>(uniformVar->offset);
}

inline s32 GX2GetPixelUniformVarOffset(const GX2PixelShader* shader, const char* name)
{
    GX2UniformVar* uniformVar = GX2GetPixelUniformVar(shader, name);
    return uniformVar == NULL ? GX2_UNIFORM_VAR_INVALID_OFFSET : static_cast<s32>(uniformVar->offset);
}

inline GX2AttribVar* GX2GetVertexAttribVar(const GX2VertexShader* shader, const char* name)
{
    u32 i = 0;
    while (i < shader->numAttribs)
    {
        if (strcmp(shader->attribVars[i].name, name) == 0)
            return &shader->attribVars[i];
        ++i;
    }
    return NULL;
}

inline s32 GX2GetVertexAttribVarLocation(const GX2VertexShader* shader, const char* name)
{
    GX2AttribVar* attribVar = GX2GetVertexAttribVar(shader, name);
    if (attribVar == NULL)
        return -1;
    return static_cast<s32>(attribVar->location);
}

inline GX2SamplerVar* GX2GetPixelSamplerVar(const GX2PixelShader* shader, const char* name)
{
    u32 i = 0;
    while (i < shader->numSamplers)
    {
        if (strcmp(shader->samplerVars[i].name, name) == 0)
            return &shader->samplerVars[i];
        ++i;
    }
    return NULL;
}

inline s32 GX2GetPixelSamplerVarLocation(const GX2PixelShader* shader, const char* name)
{
    GX2SamplerVar* samplerVar = GX2GetPixelSamplerVar(shader, name);
    return samplerVar != NULL ? static_cast<s32>(samplerVar->location) : -1;
}

#endif
