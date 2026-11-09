#include "game/client/render/Renderer.hpp"

#include "platform/wiiu/MiiIcon/MakeIcon.hpp"
#include "platform/wiiu/CafeThreadLocal.hpp"
#include "platform/wiiu/InputManager.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"

#include <cafe/gx2.h>
#include <cafe/mat.h>
#include <nn/ffl.h>

#include <coreinit/thread.h>
#include <map>
#include <coreinit/memdefaultheap.h>
#include <coreinit/memexpheap.h>
#include <coreinit/memfrmheap.h>
#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <dmae/mem.h>
#include <dmae/sync.h>
#include <png.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>

struct D3DXIMAGE_INFO
{
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int MipLevels;
    unsigned int Format;
    unsigned int ResourceType;
    unsigned int ImageFileFormat;
};

void user_write_data_init(void* buffer, int size);
void user_write_data(png_struct_def* png, unsigned char* data, unsigned int size);
void user_flush_data(png_struct_def* png);

int user_write_data_bytes_written();

namespace nn
{
namespace swkbd
{
void DrawTV();
void DrawDRC();
}

namespace erreula
{
void DrawTV();
void DrawDRC();
}
}

#include "platform/wiiu/shaders/RendererShaders.inc"

const GX2AttribStream gAttribStreamStandard[5] = {
    {0, 0, 0, GX2_ATTRIB_FORMAT_32_32_32_32_FLOAT, GX2_ATTRIB_INDEX_VERTEX_ID, 0, GX2_COMP_SEL_XYZ1,
     GX2_ENDIANSWAP_DEFAULT},
    {1, 0, 20, GX2_ATTRIB_FORMAT_8_8_8_8_UNORM, GX2_ATTRIB_INDEX_VERTEX_ID, 0, GX2_COMP_SEL_XYZW, GX2_ENDIANSWAP_8IN32},
    {2, 0, 24, GX2_ATTRIB_FORMAT_8_8_8_8_SNORM, GX2_ATTRIB_INDEX_VERTEX_ID, 0, GX2_COMP_SEL_XYZW, GX2_ENDIANSWAP_8IN32},
    {3, 0, 12, GX2_ATTRIB_FORMAT_32_32_FLOAT, GX2_ATTRIB_INDEX_VERTEX_ID, 0, GX2_COMP_SEL_XY00, GX2_ENDIANSWAP_DEFAULT},
    {4, 0, 28, GX2_ATTRIB_FORMAT_16_16_SINT, GX2_ATTRIB_INDEX_VERTEX_ID, 0, GX2_COMP_SEL_XY00, GX2_ENDIANSWAP_DEFAULT},
};
const GX2AttribStream gAttribStreamQuad[2] = {
    {0, 0, 0, GX2_ATTRIB_FORMAT_16_16_16_16_SINT, GX2_ATTRIB_INDEX_VERTEX_ID, 0, GX2_COMP_SEL_XYZW,
     GX2_ENDIANSWAP_DEFAULT},
    {1, 0, 8, GX2_ATTRIB_FORMAT_16_16_16_16_SINT, GX2_ATTRIB_INDEX_VERTEX_ID, 0, GX2_COMP_SEL_XYZW,
     GX2_ENDIANSWAP_DEFAULT},
};
const GX2AttribStream gAttribStreamPosition[1] = {
    {0, 0, 0, GX2_ATTRIB_FORMAT_32_32_FLOAT, GX2_ATTRIB_INDEX_VERTEX_ID, 0, GX2_COMP_SEL_XY01, GX2_ENDIANSWAP_DEFAULT},
};
const GX2AttribStream gAttribStreamParticle[3] = {
    {0, 0, 0, GX2_ATTRIB_FORMAT_32_32_FLOAT, GX2_ATTRIB_INDEX_VERTEX_ID, 0, GX2_COMP_SEL_XY01, GX2_ENDIANSWAP_DEFAULT},
    {1, 0, 8, GX2_ATTRIB_FORMAT_32_32_FLOAT, GX2_ATTRIB_INDEX_VERTEX_ID, 0, GX2_COMP_SEL_XY01, GX2_ENDIANSWAP_DEFAULT},
    {2, 0, 16, GX2_ATTRIB_FORMAT_8_8_8_8_UNORM, GX2_ATTRIB_INDEX_VERTEX_ID, 0, GX2_COMP_SEL_XYZW, GX2_ENDIANSWAP_8IN32},
};

static const GX2PrimitiveType sPrimitiveModes[7] = {
    GX2_PRIMITIVE_TRIANGLES, GX2_PRIMITIVE_TRIANGLE_STRIP, GX2_PRIMITIVE_TRIANGLE_FAN, GX2_PRIMITIVE_QUADS,
    GX2_PRIMITIVE_LINES,     GX2_PRIMITIVE_LINE_STRIP,     GX2PrimitiveType(0)};

static const unsigned int SURFACE_FORMAT_FLAG_SRGB = 0x400;
static const unsigned int COMMAND_BUFFER_POOL_SIZE = 0x400000;

Renderer* gRendererInstance = NULL;
static OSFastMutex sTotalAllocMutex;
static int sTotalCommandBufferBytes = 0;
static unsigned long long sPreviousSubmitTimeStamp = 0;
static unsigned long long sLastSubmitTimeStamp = 0;
static void* sFFLWorkMemory = NULL;
static FFLInitDesc sFFLInitDesc = {NULL, NULL, FFL_FONT_REGION_JP_US_EU, FALSE, TRUE};
static GX2Sampler sMiiIconSampler;
static unsigned char* sUserWriteDataStart;
static unsigned char* sUserWriteDataCurrent;
static unsigned char* sUserWriteDataEnd;

void user_write_data_init(void* buffer, int size)
{
    sUserWriteDataStart = (unsigned char*)buffer;
    sUserWriteDataCurrent = (unsigned char*)buffer;
    sUserWriteDataEnd = (unsigned char*)buffer + size;
}

void user_write_data(png_struct_def*, unsigned char* data, unsigned int size)
{
    unsigned int count = (unsigned int)(sUserWriteDataEnd - sUserWriteDataCurrent);
    if (size < count)
    {
        count = size;
    }
    memcpy(sUserWriteDataCurrent, data, count);
    sUserWriteDataCurrent = sUserWriteDataCurrent + count;
}

void user_flush_data(png_struct_def*)
{
}

int user_write_data_bytes_written()
{
    return (int)(sUserWriteDataCurrent - sUserWriteDataStart);
}

Renderer* gRenderer = new Renderer;

namespace
{
std::map<OSThread*, Renderer::Context*> s_rendererTlsContexts;

Renderer::Context*& currentRendererContext()
{
    return CafeThreadLocal::slot(s_rendererTlsContexts);
}
}

Renderer::Renderer()
{
    gRendererInstance = this;
}

void Renderer::Tick()
{
    CBuffTick();
}

void Renderer::UpdateGamma(unsigned short gamma)
{
    mGamma = gamma;
}

void Renderer::InitialiseContext(bool bReset)
{
    currentRendererContext() = new Context(this);
}

void Renderer::InitialiseContext()
{
    InitialiseContext(false);
}

Renderer::Context& Renderer::getContext()
{
    return *currentRendererContext();
}

#ifndef MCWIIU_FIX_BONE_INDEX_ROUNDING
#define MCWIIU_FIX_BONE_INDEX_ROUNDING 1
#endif

static void PatchBoneIndexScale(u32* program, unsigned int words)
{
    int patched = 0;
    for (unsigned int i = 0; i < words; ++i)
    {
        if (program[i] == 0x0000FE42)
        {
            program[i] = 0x0000FF42;
            ++patched;
        }
    }
    if (patched != 1)
        OSReport("PatchBoneIndexScale: expected 1 literal, patched %d\n", patched);
    GX2Invalidate(GX2_INVALIDATE_CPU_SHADER, program, words * sizeof(u32));
}

void Renderer::SetupShaders()
{
#if MCWIIU_FIX_BONE_INDEX_ROUNDING
    PatchBoneIndexScale(sgVertexShaderLayeredProgram, sizeof(sgVertexShaderLayeredProgram) / sizeof(u32));
    PatchBoneIndexScale(sgVertexShaderLayeredLitProgram, sizeof(sgVertexShaderLayeredLitProgram) / sizeof(u32));
#endif

    mVertexShaders = new GX2VertexShader*[C4JRender::eVertexType_Count];
    mFetchShaders = new GX2FetchShader*[C4JRender::eVertexType_Count];
    mVertexStrides = new unsigned int[C4JRender::eVertexType_Count];

    mVertexStrides[0] = 0x20;
    mFetchShaders[0] = new GX2FetchShader;
    mVertexStrides[1] = 0x10;
    mFetchShaders[1] = new GX2FetchShader;
    mVertexStrides[2] = 0x20;
    mFetchShaders[2] = new GX2FetchShader;
    mVertexStrides[3] = 0x20;
    mFetchShaders[3] = new GX2FetchShader;
    mVertexStrides[4] = 8;
    mFetchShaders[4] = new GX2FetchShader;
    mVertexStrides[5] = 8;
    mFetchShaders[5] = new GX2FetchShader;
    mVertexStrides[6] = 8;
    mFetchShaders[6] = new GX2FetchShader;
    mVertexStrides[7] = 0x14;
    mFetchShaders[7] = new GX2FetchShader;
    mVertexStrides[8] = 0x20;
    mFetchShaders[8] = new GX2FetchShader;
    mVertexStrides[9] = 0x20;
    mFetchShaders[9] = new GX2FetchShader;

    mVertexShaders[0] = &gVertexShaderStandard;
    mVertexShaders[1] = &gVertexShaderTerrain;
    mVertexShaders[2] = &gVertexShaderQuad;
    mVertexShaders[3] = &gVertexShaderStandardLit;
    mVertexShaders[4] = &gVertexShaderPosition;
    mVertexShaders[5] = &gVertexShaderPositionLerp;
    mVertexShaders[6] = &gVertexShaderParticle;
    mVertexShaders[7] = &gVertexShaderPositionAlpha;
    mVertexShaders[8] = &gVertexShaderLayered;
    mVertexShaders[9] = &gVertexShaderLayeredLit;

    mClearVertexShader = &gVertexShaderClear;
    mResolveVertexShader = &gVertexShaderScreenQuad;

    mPixelShaders = new GX2PixelShader*[C4JRender::ePixelShaderType_Count];
    mPixelShaders[0] = &gPixelShaderTextured;
    mPixelShaders[1] = &gPixelShaderTexturedAlpha;
    mPixelShaders[2] = &gPixelShaderTexturedMask;
    mPixelShaders[3] = &gPixelShaderTexturedLerp;
    mPixelShaders[4] = &gPixelShaderTexturedFog;
    mPixelShaders[5] = &gPixelShaderTexturedForceLOD;
    mPixelShaders[6] = &gPixelShaderUntextured;
    mPixelShaders[7] = &gPixelShaderOrderIndependentAlpha;
    mPixelShaders[8] = &gPixelShaderLayered;
    mPixelShaders[9] = &gPixelShaderTexturedShadow;
    mPixelShaders[10] = &gPixelShaderTexturedGlow;
    mPixelShaders[11] = &gPixelShaderTexturedBlend;
    mPixelShaders[12] = &gPixelShaderTexturedModulate;
    mPixelShaders[13] = &gPixelShaderTexturedAdd;

    mClearPixelShader = &gPixelShaderClear;
    mCopyPixelShader = &gPixelShaderCopy;
    mGammaPixelShader = &gPixelShaderGamma;
    mAlphaAccumulatePixelShader = &gPixelShaderAlphaAccumulate;
    mAlphaResolvePixelShader = &gPixelShaderAlphaResolve;

    GX2InitFetchShader(mFetchShaders[0], MemoryAllocMEM2(GX2CalcFetchShaderSize(5), GX2_SHADER_ALIGNMENT), 5,
                       gAttribStreamStandard);
    GX2InitFetchShader(mFetchShaders[1], MemoryAllocMEM2(GX2CalcFetchShaderSize(2), GX2_SHADER_ALIGNMENT), 2,
                       gAttribStreamQuad);
    GX2InitFetchShader(mFetchShaders[2], MemoryAllocMEM2(GX2CalcFetchShaderSize(5), GX2_SHADER_ALIGNMENT), 5,
                       gAttribStreamStandard);
    GX2InitFetchShader(mFetchShaders[3], MemoryAllocMEM2(GX2CalcFetchShaderSize(5), GX2_SHADER_ALIGNMENT), 5,
                       gAttribStreamStandard);
    GX2InitFetchShader(mFetchShaders[4], MemoryAllocMEM2(GX2CalcFetchShaderSize(1), GX2_SHADER_ALIGNMENT), 1,
                       gAttribStreamPosition);
    GX2InitFetchShader(mFetchShaders[5], MemoryAllocMEM2(GX2CalcFetchShaderSize(1), GX2_SHADER_ALIGNMENT), 1,
                       gAttribStreamPosition);
    GX2InitFetchShader(mFetchShaders[6], MemoryAllocMEM2(GX2CalcFetchShaderSize(1), GX2_SHADER_ALIGNMENT), 1,
                       gAttribStreamPosition);
    GX2InitFetchShader(mFetchShaders[7], MemoryAllocMEM2(GX2CalcFetchShaderSize(3), GX2_SHADER_ALIGNMENT), 3,
                       gAttribStreamParticle);
    GX2InitFetchShader(mFetchShaders[8], MemoryAllocMEM2(GX2CalcFetchShaderSize(5), GX2_SHADER_ALIGNMENT), 5,
                       gAttribStreamStandard);
    GX2InitFetchShader(mFetchShaders[9], MemoryAllocMEM2(GX2CalcFetchShaderSize(5), GX2_SHADER_ALIGNMENT), 5,
                       gAttribStreamStandard);
}

void Renderer::Initialise(bool bFirstTime)
{
    u32 size;
    GX2Boolean scaleNeeded;
    u32 hiZSize;
    u32 hiZAlignment;

    if (bFirstTime)
    {
        InitialiseContext(true);

        mCommandBufferSlots = new short[0x800000];
        mCommandBuffers = new CommandBuffer*[16000];
        mCommandBufferMatrices = new float[16000 * 16];
        mCommandBufferIds = new int[16000];
        mCommandBufferVertexTypes = new unsigned char[16000];
        mCommandBufferPixelShaderTypes = new int[16000];
        mCommandBufferPrimitiveTypes = new unsigned char[16000];

        memset(mCommandBufferSlots, 0xFF, 0x1000000);
        memset(mCommandBuffers, 0, 64000);
        memset(mCommandBufferIds, 0, 64000);
        memset(mCommandBufferVertexTypes, 0, 16000);
        memset(mCommandBufferPixelShaderTypes, 0, 64000);
        memset(mCommandBufferPrimitiveTypes, 0, 16000);

        mNumCommandBuffers = 0;
        mScreenGrabRequested = false;
        mDRCFrameStarted = false;
        mNumRetiredCommandBuffers = 0;

        SetupShaders();

        void* commandBufferPool = MemoryAllocMEM2(COMMAND_BUFFER_POOL_SIZE, 0x40);
        u32 initAttributes[] = {GX2_INIT_ATTRIB_CB_BASE, (u32)commandBufferPool, GX2_INIT_ATTRIB_CB_SIZE,
                                COMMAND_BUFFER_POOL_SIZE, GX2_INIT_ATTRIB_NULL};
        GX2Init(initAttributes);

        float black[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        SetClearColour(black);

        mForegroundAcquired = true;
    }

    mDisplayEnabled = false;
    MemoryInit();

    mScreenWidth[0] = 1280;
    mScreenHeight[0] = 720;
    mHiDef = true;
    GX2TVScanMode scanMode = GX2GetSystemTVScanMode();
    if (scanMode == GX2_TV_SCAN_MODE_576I || scanMode == GX2_TV_SCAN_MODE_480I || scanMode == GX2_TV_SCAN_MODE_480P)
    {
        mHiDef = false;
    }

    GX2CalcTVSize(GX2_TV_RENDER_720, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE, &size,
                  &scaleNeeded);
    mTVScanBuffer = MemoryAllocBucket(size, 0x1000);
    GX2Invalidate(GX2_INVALIDATE_CPU, mTVScanBuffer, size);
    GX2SetTVBuffer(mTVScanBuffer, size, GX2_TV_RENDER_720, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
                   GX2_BUFFERING_DOUBLE);

    GX2CalcDRCSize(GX2_DRC_SINGLE, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE, &size, &scaleNeeded);
    mDRCScanBuffer = MemoryAllocBucket(size, 0x1000);
    GX2Invalidate(GX2_INVALIDATE_CPU, mTVScanBuffer, size);
    GX2SetDRCBuffer(mDRCScanBuffer, size, GX2_DRC_SINGLE, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
                    GX2_BUFFERING_DOUBLE);

    GX2InitColorBuffer(&mColourBuffer, mScreenWidth[0], mScreenHeight[0], GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
                       GX2_AA_MODE_1X);
    void* colourImage = MemoryAllocMEM1(mColourBuffer.surface.imageSize, mColourBuffer.surface.alignment, false);
    GX2Invalidate(GX2_INVALIDATE_CPU, colourImage, mColourBuffer.surface.imageSize);
    mColourBuffer.surface.imagePtr = colourImage;

    GX2InitTexture(&mColourTexture, mScreenWidth[0], mScreenHeight[0], 1, 1, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
                   GX2_SURFACE_DIM_2D);
    GX2CalcSurfaceSizeAndAlignment(&mColourTexture.surface);
    GX2InitTextureRegs(&mColourTexture);
    GX2InitTexturePtrs(&mColourTexture, colourImage, NULL);

    GX2InitDepthBuffer(&mDepthBuffer, mScreenWidth[0], mScreenHeight[0], GX2_SURFACE_FORMAT_D_D24_S8_UNORM,
                       GX2_AA_MODE_1X);
    void* depthImage = MemoryAllocMEM1(mDepthBuffer.surface.imageSize, mDepthBuffer.surface.alignment, false);
    GX2Invalidate(GX2_INVALIDATE_CPU, depthImage, mDepthBuffer.surface.imageSize);
    mDepthBuffer.surface.imagePtr = depthImage;
    GX2CalcDepthBufferHiZInfo(&mDepthBuffer, &hiZSize, &hiZAlignment);
    mDepthBuffer.hiZPtr = MemoryAllocMEM1(hiZSize, hiZAlignment, false);
    GX2InitDepthBufferHiZEnable(&mDepthBuffer, mDepthBuffer.hiZPtr != NULL ? GX2_TRUE : GX2_FALSE);
    GX2InitDepthBufferHiZEnable(&mDepthBuffer, GX2_TRUE);

    mScreenWidth[1] = 854;
    mScreenHeight[1] = 480;
    GX2InitColorBuffer(&mDRCColourBuffer, 854, 480, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_AA_MODE_1X);
    void* drcImage = MemoryAllocMEM1(mDRCColourBuffer.surface.imageSize, mDRCColourBuffer.surface.alignment, false);
    GX2Invalidate(GX2_INVALIDATE_CPU, drcImage, mDRCColourBuffer.surface.imageSize);
    mDRCColourBuffer.surface.imagePtr = drcImage;

    GX2InitTexture(&mDRCTexture, mScreenWidth[1], mScreenHeight[1], 1, 1, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
                   GX2_SURFACE_DIM_2D);
    GX2CalcSurfaceSizeAndAlignment(&mDRCTexture.surface);
    GX2InitTextureRegs(&mDRCTexture);
    GX2InitTexturePtrs(&mDRCTexture, drcImage, NULL);

    GX2InitSamplerClamping(&mDRCSampler, GX2_TEX_CLAMP_CLAMP, GX2_TEX_CLAMP_CLAMP, GX2_TEX_CLAMP_CLAMP);
    GX2InitSamplerXYFilter(&mDRCSampler, GX2_TEX_XY_FILTER_BILINEAR, GX2_TEX_XY_FILTER_BILINEAR, GX2_TEX_ANISO_1_TO_1);
    GX2InitSamplerZMFilter(&mDRCSampler, GX2_TEX_Z_FILTER_POINT, GX2_TEX_MIP_FILTER_LINEAR);

    GX2InitDepthBuffer(&mDRCDepthBuffer, mScreenWidth[1], mScreenHeight[1], GX2_SURFACE_FORMAT_D_D24_S8_UNORM,
                       GX2_AA_MODE_1X);
    mDRCDepthBuffer.surface.imagePtr = depthImage;

    mReservedMEM1 = NULL;
    GX2InitColorBuffer(&mAlphaColourBuffer, mScreenWidth[0], mScreenHeight[0],
                       GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_FLOAT, GX2_AA_MODE_1X);
    void* alphaImage = MemoryAllocMEM1(0x1600000, mAlphaColourBuffer.surface.alignment, true);
    GX2Invalidate(GX2_INVALIDATE_CPU, alphaImage, mAlphaColourBuffer.surface.imageSize);
    mAlphaColourBuffer.surface.imagePtr = alphaImage;

    GX2InitTexture(&mAlphaTexture, mScreenWidth[0], mScreenHeight[0], 1, 1, GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_FLOAT,
                   GX2_SURFACE_DIM_2D);
    GX2InitTexturePtrs(&mAlphaTexture, alphaImage, NULL);

    GX2InitSamplerClamping(&mAlphaSampler, GX2_TEX_CLAMP_CLAMP, GX2_TEX_CLAMP_CLAMP, GX2_TEX_CLAMP_CLAMP);
    GX2InitSamplerXYFilter(&mAlphaSampler, GX2_TEX_XY_FILTER_POINT, GX2_TEX_XY_FILTER_POINT, GX2_TEX_ANISO_1_TO_1);
    GX2InitSamplerZMFilter(&mAlphaSampler, GX2_TEX_Z_FILTER_POINT, GX2_TEX_MIP_FILTER_LINEAR);

    GX2InitColorBufferFTV(&mTVColourBuffer, mScreenWidth[0], mScreenHeight[0], GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
                          GX2_AA_MODE_1X);
    mTVColourBuffer.surface.imagePtr = alphaImage;

    GX2InitTexture(&mTVTexture, mScreenWidth[0], mScreenHeight[0], 1, 1, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
                   GX2_SURFACE_DIM_2D);
    GX2InitTexturePtrs(&mTVTexture, alphaImage, NULL);

    for (int thumbnail = 0, thumbnailSize = 0x200; thumbnail < 4; ++thumbnail, thumbnailSize >>= 1)
    {
        GX2InitColorBuffer(&mThumbnailColourBuffer[thumbnail], thumbnailSize, thumbnailSize,
                           GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_AA_MODE_1X);
        mThumbnailColourBuffer[thumbnail].surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
        GX2CalcSurfaceSizeAndAlignment(&mThumbnailColourBuffer[thumbnail].surface);
        GX2InitColorBufferRegs(&mThumbnailColourBuffer[thumbnail]);
        void* thumbnailImage = MemoryAllocBucket(mThumbnailColourBuffer[thumbnail].surface.imageSize,
                                                 mThumbnailColourBuffer[thumbnail].surface.alignment);
        GX2Invalidate(GX2_INVALIDATE_CPU, thumbnailImage, mThumbnailColourBuffer[thumbnail].surface.imageSize);
        mThumbnailColourBuffer[thumbnail].surface.imagePtr = thumbnailImage;

        GX2InitTexture(&mThumbnailTexture[thumbnail], thumbnailSize, thumbnailSize, 1, 1,
                       GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_SURFACE_DIM_2D);
        mThumbnailTexture[thumbnail].surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
        GX2CalcSurfaceSizeAndAlignment(&mThumbnailTexture[thumbnail].surface);
        GX2InitTextureRegs(&mThumbnailTexture[thumbnail]);
        GX2InitTexturePtrs(&mThumbnailTexture[thumbnail], thumbnailImage, NULL);
    }

    if (bFirstTime)
    {
        mContextState = static_cast<GX2ContextState*>(
            MemoryAllocMEM2(sizeof(GX2ContextState), GX2_CONTEXT_STATE_ALIGNMENT));
        GX2SetupContextState(mContextState);
        memset(mTextures, 0, sizeof(mTextures));
        mDefaultTexture = TextureCreate();
        TextureBind(mDefaultTexture);

        unsigned char* white = new unsigned char[0x400];
        memset(white, 0xFF, 0x400);
        TextureData(0x10, 0x10, white, 0, C4JRender::eTextureFormat_RGBA8);
        delete[] white;

        mFrameCount = 0;
        mCapturingThumbnail = false;
        mGamma = 0x2AAA;
        OSFastMutex_Init(&mCommandBufferMutex, "commandBufferCS");
        OSFastMutex_Init(&sTotalAllocMutex, "totalAllocCS");
        mNextCommandBufferId = 0;
        mAllowStaticCommandBufferIds = true;
        GX2SetViewport(0.0f, 0.0f, (float)(unsigned int)mScreenWidth[0], (float)(unsigned int)mScreenHeight[0], 0.0f,
                       1.0f);
        GX2SetScissor(0, 0, mScreenWidth[0], mScreenHeight[0]);
        GX2SetSwapInterval(1);
    }
    else
    {
        GX2SetContextState(mContextState);
        GX2SetViewport(0.0f, 0.0f, (float)(unsigned int)mScreenWidth[0], (float)(unsigned int)mScreenHeight[0], 0.0f,
                       1.0f);
        GX2SetScissor(0, 0, mScreenWidth[0], mScreenHeight[0]);
        GX2SetSwapInterval(1);
    }

    GX2SetShaderModeEx(GX2_SHADER_MODE_UNIFORM_REGISTER, 48, 64, 0, 0, 200, 192);

    float one[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    memcpy(mColour, one, sizeof(mColour));
    memset(mColourAdd, 0, sizeof(mColourAdd));
    memcpy(mTexColour[0], one, sizeof(mTexColour[0]));
    memcpy(mTexColour[1], one, sizeof(mTexColour[1]));
    memcpy(mTexColour[2], one, sizeof(mTexColour[2]));
    memcpy(mTexColour[3], one, sizeof(mTexColour[3]));
    mPixelUniformsDirty = true;
    memset(mLayeredParams, 0, sizeof(mLayeredParams));
    memset(mLayeredTextureSize, 0, sizeof(mLayeredTextureSize));
    mLayeredParamsDirty = true;

    GX2SetViewport(0.0f, 0.0f, (float)(unsigned int)mScreenWidth[0], (float)(unsigned int)mScreenHeight[0], 0.0f, 1.0f);
    GX2SetScissor(0, 0, mScreenWidth[0], mScreenHeight[0]);
    StateSetVertexTextureUV(0.0f, 0.0f);
    GX2SetVertexTexture(&mTextures[mDefaultTexture].mTexture, 0);
    GX2SetVertexSampler(&mTextures[mDefaultTexture].mSampler, 0);
    mTextureStateDirty = false;
    mCurrentVertexShader = -1;
    mCurrentPixelShader = -1;
    mCommandBufferVertsDrawn = 0;
    mCommandBufferVertsDrawnLastFrame = 0;
}

void Renderer::Initialise()
{
    Initialise(true);
}

GX2ColorBuffer* Renderer::GetColorBuffer()
{
    return &mColourBuffer;
}

GX2DepthBuffer* Renderer::GetDepthStencilBuffer()
{
    return &mDepthBuffer;
}

GX2ColorBuffer* Renderer::GetColorBufferDRC()
{
    return &mDRCColourBuffer;
}

GX2DepthBuffer* Renderer::GetDepthStencilBufferDRC()
{
    return &mDRCDepthBuffer;
}

void Renderer::StartFrameInternal(int screen, bool bAlphaTest)
{
    mCurrentVertexShader = -1;
    mCurrentPixelShader = -1;
    TextureBindVertex(-1);
    TextureBind(-1);
    StateSetColour(1.0f, 1.0f, 1.0f, 1.0f);
    StateSetDepthFunc(3);
    StateSetDepthMask(true);
    StateSetStencil(7, 0, 0, 0);
    StateSetBlendEnable(true);
    StateSetBlendFunc(4, 5);
    StateSetBlendFactor(0xFFFFFFFF);
    StateSetAlphaTestEnable(false);
    StateSetAlphaFunc(4, 0.1f);
    StateSetFaceCull(true);
    StateSetLineWidth(1.0f);
    StateSetWriteEnable(true, true, true, true);
    StateSetDepthTestEnable(false);
    StateSetAlphaTestEnable(bAlphaTest);

    if (screen == 0)
    {
        GX2SetColorBuffer(&mColourBuffer, GX2_RENDER_TARGET_0);
        GX2SetDepthBuffer(&mDepthBuffer);
        mScreen = 0;
    }
    else
    {
        GX2SetColorBuffer(&mDRCColourBuffer, GX2_RENDER_TARGET_0);
        GX2SetDepthBuffer(&mDRCDepthBuffer);
        mScreen = screen;
    }

    GX2SetViewport(0.0f, 0.0f, (float)mScreenWidth[mScreen], (float)mScreenHeight[mScreen], 0.0f, 1.0f);
    GX2SetScissor(0, 0, mScreenWidth[mScreen], mScreenHeight[mScreen]);
}

void Renderer::StartFrame()
{
    StartFrameInternal(0, true);
}

void Renderer::ResetRenderState()
{
    StartFrameInternal(0, false);
}

void Renderer::GammaCorrect()
{
    Context& context = getContext();

    GX2SetVertexShader(mResolveVertexShader);
    GX2SetPixelShader(mGammaPixelShader);
    GX2SetDepthStencilControl(GX2_FALSE, GX2_FALSE, GX2_COMPARE_ALWAYS, GX2_TRUE, GX2_TRUE, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_FALSE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, 0, GX2_FALSE, GX2_TRUE);
    GX2SetPixelSampler(&mAlphaSampler, 0);

    float quad[4];
    quad[0] = 0.0f;
    quad[1] = 0.0f;
    quad[2] = 1.0f;
    quad[3] = 1.0f;
    GX2SetVertexUniformReg(0x80, 4, quad);

    float gamma[4];
    gamma[0] = 1.0f / ((float)mGamma * 3.0517578125e-05f * 1.5f + 0.5f);
    gamma[1] = 0.0f;
    gamma[2] = 0.0f;
    gamma[3] = 0.0f;
    GX2SetPixelUniformReg(0x18, 4, gamma);

    GX2SetViewport(0.0f, 0.0f, (float)(unsigned int)mScreenWidth[0], (float)(unsigned int)mScreenHeight[0], 0.0f, 1.0f);
    GX2SetPixelTexture(&mColourTexture, 0);
    GX2SetColorBuffer(&mTVColourBuffer, GX2_RENDER_TARGET_0);
    GX2DrawEx(GX2_PRIMITIVE_TRIANGLE_STRIP, 4, 0, 1);

    GX2SetDepthStencilControl(context.mDepthTestEnable ? GX2_TRUE : GX2_FALSE,
                              context.mDepthMask ? GX2_TRUE : GX2_FALSE, context.mDepthFunc, GX2_TRUE, GX2_TRUE,
                              context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP,
                              context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetPolygonControl(GX2_FRONT_FACE_CCW, GX2_FALSE, context.mFaceCull ? GX2_TRUE : GX2_FALSE, GX2_FALSE,
                         GX2_POLYGON_MODE_TRIANGLE, GX2_POLYGON_MODE_TRIANGLE, GX2_TRUE, GX2_TRUE, GX2_FALSE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, context.mBlendEnable, GX2_FALSE, GX2_TRUE);
    GX2SetBlendControl(GX2_RENDER_TARGET_0, context.mBlendSource, context.mBlendDestination, GX2_BLEND_COMBINE_ADD,
                       GX2_FALSE, GX2_BLEND_ZERO, GX2_BLEND_ZERO, GX2_BLEND_COMBINE_ADD);
    GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GEQUAL, context.mAlphaTestEnable ? context.mAlphaTestRef : 0.0f);
    StateSetViewport(mCurrentViewport);

    if (mScreen == 0)
    {
        GX2SetColorBuffer(&mColourBuffer, GX2_RENDER_TARGET_0);
    }

    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
}

void Renderer::CopyFrameToDRC()
{
    Context& context = getContext();

    GX2SetVertexShader(mResolveVertexShader);
    GX2SetPixelShader(mCopyPixelShader);
    GX2SetDepthStencilControl(GX2_FALSE, GX2_FALSE, GX2_COMPARE_ALWAYS, GX2_TRUE, GX2_TRUE, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_FALSE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, 0, GX2_FALSE, GX2_TRUE);
    GX2SetPixelSampler(&mDRCSampler, 0);

    float quad[4];
    quad[0] = 0.0f;
    quad[1] = 0.0f;
    quad[2] = 1.0f;
    quad[3] = 1.0f;
    GX2SetVertexUniformReg(0x80, 4, quad);

    GX2SetViewport(0.0f, 0.0f, (float)(unsigned int)mScreenWidth[1], (float)(unsigned int)mScreenHeight[1], 0.0f, 1.0f);
    GX2SetPixelTexture(&mTVTexture, 0);
    GX2SetColorBuffer(&mDRCColourBuffer, GX2_RENDER_TARGET_0);
    GX2DrawEx(GX2_PRIMITIVE_TRIANGLE_STRIP, 4, 0, 1);

    GX2SetDepthStencilControl(context.mDepthTestEnable ? GX2_TRUE : GX2_FALSE,
                              context.mDepthMask ? GX2_TRUE : GX2_FALSE, context.mDepthFunc, GX2_TRUE, GX2_TRUE,
                              context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP,
                              context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetPolygonControl(GX2_FRONT_FACE_CCW, GX2_FALSE, context.mFaceCull ? GX2_TRUE : GX2_FALSE, GX2_FALSE,
                         GX2_POLYGON_MODE_TRIANGLE, GX2_POLYGON_MODE_TRIANGLE, GX2_TRUE, GX2_TRUE, GX2_FALSE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, context.mBlendEnable, GX2_FALSE, GX2_TRUE);
    GX2SetBlendControl(GX2_RENDER_TARGET_0, context.mBlendSource, context.mBlendDestination, GX2_BLEND_COMBINE_ADD,
                       GX2_FALSE, GX2_BLEND_ZERO, GX2_BLEND_ZERO, GX2_BLEND_COMBINE_ADD);
    GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GEQUAL, context.mAlphaTestEnable ? context.mAlphaTestRef : 0.0f);
    StateSetViewport(mCurrentViewport);

    if (mScreen == 0)
    {
        GX2SetColorBuffer(&mColourBuffer, GX2_RENDER_TARGET_0);
    }

    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
}

void Renderer::RenderOnScreenKeyboard(bool bDRC)
{
    if (!InputManager_IsShowingOnscreenKeyboard() && !InputManager_IsShowingErrorViewer())
    {
        return;
    }

    GX2SetContextState(NULL);

    GX2ColorBuffer* colourBuffer;
    if (bDRC)
    {
        colourBuffer = &mDRCColourBuffer;
    }
    else
    {
        colourBuffer = &mTVColourBuffer;
    }

    GX2SurfaceFormat format = colourBuffer->surface.format;
    colourBuffer->surface.format = (GX2SurfaceFormat)(format | SURFACE_FORMAT_FLAG_SRGB);
    GX2InitColorBufferRegs(colourBuffer);
    GX2SetColorBuffer(colourBuffer, GX2_RENDER_TARGET_0);

    if (InputManager_IsShowingOnscreenKeyboard())
    {
        if (bDRC)
        {
            nn::swkbd::DrawDRC();
        }
        else
        {
            nn::swkbd::DrawTV();
        }
    }

    if (InputManager_IsShowingErrorViewer())
    {
        if (bDRC)
        {
            nn::erreula::DrawDRC();
        }
        else
        {
            nn::erreula::DrawTV();
        }
    }

    colourBuffer->surface.format = format;
    GX2InitColorBufferRegs(colourBuffer);
    GX2SetColorBuffer(colourBuffer, GX2_RENDER_TARGET_0);
}

void Renderer::StartDRCFrame(bool bCopyFrame)
{
    GammaCorrect();

    if (bCopyFrame)
    {
        CopyFrameToDRC();
    }

    RenderOnScreenKeyboard(false);
    StartFrameInternal(1, true);
    mDRCFrameStarted = true;
}

void Renderer::DoScreenGrabOnNextPresent()
{
    mScreenGrabRequested = true;
}

void Renderer::TEMP_PatchStateFromIggy()
{
    StateSetViewport(mCurrentViewport);
    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
}

void Renderer::Present()
{
    Context& context = getContext();

    if (mDRCFrameStarted)
    {
        RenderOnScreenKeyboard(true);
    }
    else
    {
        RenderOnScreenKeyboard(false);
        GammaCorrect();
    }

    mDRCFrameStarted = false;
    GX2Flush();
    GX2CopyColorBufferToScanBuffer(&mDRCColourBuffer, GX2_SCAN_TARGET_DRC_FIRST);
    GX2SetContextState(mContextState);
    GX2CopyColorBufferToScanBuffer(&mTVColourBuffer, GX2_SCAN_TARGET_TV);
    GX2SwapScanBuffers();
    GX2SetContextState(mContextState);
    GX2Flush();

    sPreviousSubmitTimeStamp = sLastSubmitTimeStamp;
    sLastSubmitTimeStamp = GX2GetLastSubmittedTimeStamp();

    if (mFrameCount > 0)
    {
        GX2WaitTimeStamp(sPreviousSubmitTimeStamp);
    }

    if (!mDisplayEnabled)
    {
        GX2SetTVEnable(GX2_TRUE);
        GX2SetDRCEnable(GX2_TRUE);
        mDisplayEnabled = true;
    }

    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
    ++mFrameCount;

    context.mDynamicVertexOffset = 0;
    context.mDynamicVertexBufferIndex = (context.mDynamicVertexBufferIndex + 1) % 3;

    mCommandBufferVertsDrawnLastFrame = mCommandBufferVertsDrawn;
    mCommandBufferVertsDrawn = 0;
}

void Renderer::Clear(int flags)
{
    Context& context = getContext();

    GX2SetVertexShader(mClearVertexShader);
    GX2SetPixelShader(mClearPixelShader);
    GX2SetStencilMask(0, 0, 0, 0, 0, 0);
    GX2SetDepthStencilControl(GX2_TRUE, (flags & 1) ? GX2_TRUE : GX2_FALSE, GX2_COMPARE_ALWAYS, GX2_TRUE, GX2_TRUE,
                              GX2_COMPARE_ALWAYS, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP,
                              GX2_COMPARE_ALWAYS, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_FALSE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, 0, GX2_FALSE, GX2_TRUE);
    GX2SetAlphaTest(GX2_FALSE, GX2_COMPARE_GEQUAL, 0.0f);
    GX2SetTargetChannelMasks((flags & 2) != 0 ? GX2_CHANNEL_MASK_RGBA : GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE,
                             GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE,
                             GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE);
    GX2SetPixelUniformReg(0x10, 4, mClearColour);
    GX2DrawEx(GX2_PRIMITIVE_TRIANGLE_STRIP, 4, 0, 1);

    GX2SetDepthStencilControl(context.mDepthTestEnable ? GX2_TRUE : GX2_FALSE,
                              context.mDepthMask ? GX2_TRUE : GX2_FALSE, context.mDepthFunc, GX2_TRUE, GX2_TRUE,
                              context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP,
                              context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetPolygonControl(GX2_FRONT_FACE_CCW, GX2_FALSE, context.mFaceCull ? GX2_TRUE : GX2_FALSE, GX2_FALSE,
                         GX2_POLYGON_MODE_TRIANGLE, GX2_POLYGON_MODE_TRIANGLE, GX2_TRUE, GX2_TRUE, GX2_FALSE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, context.mBlendEnable, GX2_FALSE, GX2_TRUE);
    GX2SetBlendControl(GX2_RENDER_TARGET_0, context.mBlendSource, context.mBlendDestination, GX2_BLEND_COMBINE_ADD,
                       GX2_FALSE, GX2_BLEND_ZERO, GX2_BLEND_ZERO, GX2_BLEND_COMBINE_ADD);
    GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GEQUAL, context.mAlphaTestEnable ? context.mAlphaTestRef : 0.0f);
    GX2SetTargetChannelMasks(GX2_CHANNEL_MASK_RGBA, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE,
                             GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE,
                             GX2_CHANNEL_MASK_NONE);
    GX2SetStencilMask(0, 255, 0, 0, 255, 0);
    mCurrentVertexShader = -1;
    mCurrentPixelShader = -1;
}

void Renderer::SetClearColour(const float* colour)
{
    mClearColour[0] = colour[0];
    mClearColour[1] = colour[1];
    mClearColour[2] = colour[2];
    mClearColour[3] = colour[3];
}

bool Renderer::IsWidescreen()
{
    return true;
}

bool Renderer::IsHiDef()
{
    return mHiDef;
}

void Renderer::ConvertLinearToPng(ImageFileBuffer* buffer, unsigned char* pixels, unsigned int width,
                                  unsigned int height)
{
    int capacity = (int)((height * width * 24) / 5) + 0x100;
    void* png = malloc(capacity);
    int written = 0;
    SaveTextureDataToMemory(png, capacity, &written, width, height, (int*)pixels);
    buffer->mFormat = 0;
    buffer->mData = png;
    buffer->mSize = written;
}

void Renderer::CaptureThumbnail(ImageFileBuffer* buffer)
{
    Context& context = getContext();

    float left = 0.0f;
    float right = 1.0f;
    float top = 0.0f;
    float bottom = 1.0f;

    switch (mCurrentViewport)
    {
    case C4JRender::eViewportType_FullScreen:
        break;
    case C4JRender::eViewportType_TopHalf:
        bottom = 0.5f;
        break;
    case C4JRender::eViewportType_BottomHalf:
        top = 0.5f;
        break;
    case C4JRender::eViewportType_LeftHalf:
    case C4JRender::eViewportType_RightHalf:
        right = 0.5f;
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
        right = 0.5f;
        bottom = 0.5f;
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        left = 0.5f;
        bottom = 0.5f;
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        right = 0.5f;
        top = 0.5f;
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        left = 0.5f;
        top = 0.5f;
        break;
    default:
        break;
    }

    float aspect = IsWidescreen() ? 1.7777777910232544f : 1.3333333730697632f;
    float height = bottom - top;
    float scaledLeft = left * aspect;
    float scaledRight = right * aspect;
    float width = scaledRight - scaledLeft;

    if (width < height)
    {
        float inset = (height - width) * 0.5f;
        top = top + inset;
        height = (bottom - inset) - top;
    }
    else
    {
        float inset = (width - height) * 0.5f;
        scaledLeft = scaledLeft + inset;
        scaledRight = scaledRight - inset;
    }

    GX2SetVertexShader(mResolveVertexShader);
    GX2SetPixelShader(mCopyPixelShader);
    GX2SetDepthStencilControl(GX2_FALSE, GX2_FALSE, GX2_COMPARE_ALWAYS, GX2_TRUE, GX2_TRUE, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_FALSE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, 0, GX2_FALSE, GX2_TRUE);
    GX2SetPixelSampler(&mAlphaSampler, 0);

    unsigned int size = 0x200;
    float quad[4];
    quad[0] = scaledLeft / aspect;
    quad[1] = top;
    quad[2] = scaledRight / aspect - scaledLeft / aspect;
    quad[3] = height;
    GX2SetVertexUniformReg(0x80, 4, quad);
    GX2SetViewport(0.0f, 0.0f, (float)(int)size, (float)(int)size, 0.0f, 1.0f);
    GX2SetScissor(0, 0, size, size);
    GX2SetPixelTexture(&mColourTexture, 0);
    GX2SetColorBuffer(&mThumbnailColourBuffer[0], GX2_RENDER_TARGET_0);
    GX2Flush();
    GX2Invalidate(GX2_INVALIDATE_COLOR_BUFFER | GX2_INVALIDATE_TEXTURE, mColourTexture.surface.imagePtr,
                  mColourTexture.surface.imageSize);
    GX2DrawEx(GX2_PRIMITIVE_TRIANGLE_STRIP, 4, 0, 1);

    for (int level = 1; level < 4; ++level)
    {
        size >>= 1;
        quad[0] = 0.0f;
        quad[1] = 0.0f;
        quad[2] = 1.0f;
        quad[3] = 1.0f;
        GX2SetVertexUniformReg(0x80, 4, quad);
        GX2SetViewport(0.0f, 0.0f, (float)(int)size, (float)(int)size, 0.0f, 1.0f);
        GX2SetScissor(0, 0, size, size);
        GX2SetPixelTexture(&mThumbnailTexture[level - 1], 0);
        GX2SetColorBuffer(&mThumbnailColourBuffer[level], GX2_RENDER_TARGET_0);
        GX2Flush();
        GX2Invalidate(GX2_INVALIDATE_COLOR_BUFFER | GX2_INVALIDATE_TEXTURE,
                      mThumbnailTexture[level - 1].surface.imagePtr, mThumbnailTexture[level - 1].surface.imageSize);
        GX2DrawEx(GX2_PRIMITIVE_TRIANGLE_STRIP, 4, 0, 1);
    }

    GX2DrawDone();
    GX2Invalidate(GX2_INVALIDATE_COLOR_BUFFER, mThumbnailTexture[3].surface.imagePtr,
                  mThumbnailTexture[3].surface.imageSize);
    ConvertLinearToPng(buffer, (unsigned char*)mThumbnailTexture[3].surface.imagePtr, 0x40, 0x40);

    GX2SetDepthStencilControl(context.mDepthTestEnable ? GX2_TRUE : GX2_FALSE,
                              context.mDepthMask ? GX2_TRUE : GX2_FALSE, context.mDepthFunc, GX2_TRUE, GX2_TRUE,
                              context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP,
                              context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetPolygonControl(GX2_FRONT_FACE_CCW, GX2_FALSE, context.mFaceCull ? GX2_TRUE : GX2_FALSE, GX2_FALSE,
                         GX2_POLYGON_MODE_TRIANGLE, GX2_POLYGON_MODE_TRIANGLE, GX2_TRUE, GX2_TRUE, GX2_FALSE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, context.mBlendEnable, GX2_FALSE, GX2_TRUE);
    GX2SetBlendControl(GX2_RENDER_TARGET_0, context.mBlendSource, context.mBlendDestination, GX2_BLEND_COMBINE_ADD,
                       GX2_FALSE, GX2_BLEND_ZERO, GX2_BLEND_ZERO, GX2_BLEND_COMBINE_ADD);
    GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GEQUAL, context.mAlphaTestEnable ? context.mAlphaTestRef : 0.0f);
    StateSetViewport(mCurrentViewport);
    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
}

void Renderer::CaptureScreen(ImageFileBuffer* buffer, _XSOCIAL_PREVIEWIMAGE* previewImage)
{
}

void Renderer::BeginConditionalSurvey(int survey)
{
}

void Renderer::EndConditionalSurvey()
{
}

void Renderer::BeginConditionalRendering(int survey)
{
}

void Renderer::EndConditionalRendering()
{
}

void Renderer::BeginOrderIndependentAlpha()
{
    Context& context = getContext();

    GX2SetColorBuffer(&mAlphaColourBuffer, GX2_RENDER_TARGET_0);
    GX2SetColorBuffer(&mColourBuffer, GX2_RENDER_TARGET_1);
    GX2SetVertexShader(mClearVertexShader);
    GX2SetPixelShader(mAlphaAccumulatePixelShader);
    GX2SetTargetChannelMasks(GX2_CHANNEL_MASK_RGBA, GX2_CHANNEL_MASK_A, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE,
                             GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE,
                             GX2_CHANNEL_MASK_NONE);
    GX2SetDepthStencilControl(GX2_FALSE, GX2_FALSE, GX2_COMPARE_ALWAYS, GX2_TRUE, GX2_TRUE, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_FALSE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, 0, GX2_FALSE, GX2_TRUE);
    GX2SetAlphaTest(GX2_FALSE, GX2_COMPARE_GEQUAL, 0.0f);
    GX2DrawEx(GX2_PRIMITIVE_TRIANGLE_STRIP, 4, 0, 1);
    mCurrentVertexShader = -1;
    mCurrentPixelShader = -1;
    GX2SetBlendControl(GX2_RENDER_TARGET_0, GX2_BLEND_ONE, GX2_BLEND_ONE, GX2_BLEND_COMBINE_ADD, GX2_FALSE,
                       GX2_BLEND_ZERO, GX2_BLEND_ZERO, GX2_BLEND_COMBINE_ADD);
    GX2SetBlendControl(GX2_RENDER_TARGET_1, GX2_BLEND_ZERO, GX2_BLEND_ONE_MINUS_SRC_ALPHA, GX2_BLEND_COMBINE_ADD,
                       GX2_FALSE, GX2_BLEND_ZERO, GX2_BLEND_ZERO, GX2_BLEND_COMBINE_ADD);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, 3, GX2_FALSE, GX2_TRUE);
    GX2SetDepthStencilControl(GX2_TRUE, GX2_FALSE, GX2_COMPARE_LEQUAL, GX2_TRUE, GX2_TRUE, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_TRUE);
    context.mOrderIndependentAlpha = true;
}

void Renderer::EndOrderIndependentAlpha()
{
    Context& context = getContext();

    void* imagePtr = mAlphaColourBuffer.surface.imagePtr;
    unsigned int imageSize = mAlphaColourBuffer.surface.imageSize;
    context.mOrderIndependentAlpha = false;
    GX2Invalidate(GX2_INVALIDATE_COLOR_BUFFER, imagePtr, imageSize);
    GX2SetColorBuffer(&mColourBuffer, GX2_RENDER_TARGET_0);
    GX2SetVertexShader(mResolveVertexShader);
    GX2SetPixelShader(mAlphaResolvePixelShader);
    GX2SetBlendControl(GX2_RENDER_TARGET_0, GX2_BLEND_ONE_MINUS_DST_ALPHA, GX2_BLEND_DST_ALPHA, GX2_BLEND_COMBINE_ADD,
                       GX2_FALSE, GX2_BLEND_ZERO, GX2_BLEND_ZERO, GX2_BLEND_COMBINE_ADD);
    GX2SetDepthStencilControl(GX2_FALSE, GX2_FALSE, GX2_COMPARE_ALWAYS, GX2_TRUE, GX2_TRUE, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP, GX2_COMPARE_ALWAYS,
                              GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_FALSE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, 3, GX2_FALSE, GX2_TRUE);

    float quad[4];
    quad[0] = context.mViewportScale[2];
    quad[1] = context.mViewportScale[3];
    quad[2] = context.mViewportScale[0];
    quad[3] = context.mViewportScale[1];
    GX2SetVertexUniformReg(0x80, 4, quad);

    GX2SetPixelTexture(&mAlphaTexture, 0);
    GX2SetPixelSampler(&mAlphaSampler, 0);
    GX2DrawEx(GX2_PRIMITIVE_TRIANGLE_STRIP, 4, 0, 1);

    GX2SetDepthStencilControl(context.mDepthTestEnable ? GX2_TRUE : GX2_FALSE,
                              context.mDepthMask ? GX2_TRUE : GX2_FALSE, context.mDepthFunc, GX2_TRUE, GX2_TRUE,
                              context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP,
                              context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    GX2SetPolygonControl(GX2_FRONT_FACE_CCW, GX2_FALSE, context.mFaceCull ? GX2_TRUE : GX2_FALSE, GX2_FALSE,
                         GX2_POLYGON_MODE_TRIANGLE, GX2_POLYGON_MODE_TRIANGLE, GX2_TRUE, GX2_TRUE, GX2_FALSE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, context.mBlendEnable, GX2_FALSE, GX2_TRUE);
    GX2SetBlendControl(GX2_RENDER_TARGET_0, context.mBlendSource, context.mBlendDestination, GX2_BLEND_COMBINE_ADD,
                       GX2_FALSE, GX2_BLEND_ZERO, GX2_BLEND_ZERO, GX2_BLEND_COMBINE_ADD);
    GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GEQUAL, context.mAlphaTestEnable ? context.mAlphaTestRef : 0.0f);
    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
}

void Renderer::ReleaseForeground()
{
    GX2Flush();
    GX2DrawDone();
    MemoryRelease();
    mDisplayEnabled = false;
    mForegroundAcquired = false;
    OSMemoryBarrier();
}

void Renderer::AcquiredForeground()
{
    mForegroundAcquired = true;
    OSMemoryBarrier();
    Initialise(false);
}

void Renderer::WiiUWaitDrawDone()
{
    if (!mForegroundAcquired)
    {
        return;
    }

    GX2DrawDone();
}

int Renderer::GetFramesSinceBoot()
{
    return mFrameCount;
}

void Renderer::BeginEvent(const wchar_t* name)
{
}

void Renderer::EndEvent()
{
}

int Renderer::GetNumCommandBufferVertsDrawnLastFrame()
{
    return mCommandBufferVertsDrawnLastFrame;
}

int Renderer::GetNumVertsInCommandBuffer(int id)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    int slot = mCommandBufferSlots[id];
    int numVertices = 0;
    if (slot >= 0)
    {
        numVertices = mCommandBuffers[slot]->getNumVertices();
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
    return numVertices;
}

void Renderer::MatrixMode(int mode)
{
    Context& context = getContext();
    context.mMatrixMode = mode;
}

void Renderer::MatrixSetIdentity()
{
    Context& context = getContext();
    ASM_MTX44Identity(context.mMatrixStack[context.mMatrixMode][context.mMatrixStackDepth[context.mMatrixMode]]);
    context.mMatrixDirty[context.mMatrixMode] = true;
}

void Renderer::MultWithStack(float (*matrix)[4])
{
    Context& context = getContext();
    ASM_MTX44Concat(matrix, context.mMatrixStack[context.mMatrixMode][context.mMatrixStackDepth[context.mMatrixMode]],
                    context.mMatrixStack[context.mMatrixMode][context.mMatrixStackDepth[context.mMatrixMode]]);
    context.mMatrixDirty[context.mMatrixMode] = true;
}

void Renderer::MatrixTranslate(float x, float y, float z)
{
    float matrix[4][4];
    ASM_MTX44Trans(matrix, x, y, z);
    ASM_MTX44Transpose(matrix, matrix);
    MultWithStack(matrix);
}

void Renderer::MatrixRotate(float angle, float x, float y, float z)
{
    Vec axis;
    axis.x = x;
    axis.y = y;
    axis.z = z;

    float matrix[4][4];
    ASM_MTX44RotAxisRad(matrix, &axis, angle);
    ASM_MTX44Transpose(matrix, matrix);
    MultWithStack(matrix);
}

void Renderer::MatrixScale(float x, float y, float z)
{
    float matrix[4][4];
    ASM_MTX44Scale(matrix, x, y, z);
    MultWithStack(matrix);
}

void Renderer::MatrixPerspective(float fieldOfView, float aspect, float nearDistance, float farDistance)
{
    float matrix[4][4];
    C_MTXPerspective(matrix, fieldOfView, aspect, nearDistance, farDistance);
    ASM_MTX44Transpose(matrix, matrix);
    MultWithStack(matrix);
}

void Renderer::MatrixOrthogonal(float left, float right, float bottom, float top, float nearDistance, float farDistance)
{
    float matrix[4][4];
    C_MTXOrtho(matrix, top, bottom, left, right, nearDistance, farDistance);
    ASM_MTX44Transpose(matrix, matrix);
    MultWithStack(matrix);
}

void Renderer::MatrixPop()
{
    Context& context = getContext();
    --context.mMatrixStackDepth[context.mMatrixMode];
    context.mMatrixDirty[context.mMatrixMode] = true;
}

void Renderer::MatrixPush()
{
    Context& context = getContext();
    memcpy(context.mMatrixStack[context.mMatrixMode][context.mMatrixStackDepth[context.mMatrixMode] + 1],
           context.mMatrixStack[context.mMatrixMode][context.mMatrixStackDepth[context.mMatrixMode]], 0x40);
    ++context.mMatrixStackDepth[context.mMatrixMode];
}

void Renderer::MatrixMult(float* matrix)
{
    float local[4][4];
    memcpy(local, matrix, 0x40);
    MultWithStack(local);
}

float* Renderer::MatrixGet(int mode)
{
    Context& context = getContext();
    return &context.mMatrixStack[mode][context.mMatrixStackDepth[mode]][0][0];
}

void Renderer::Set_matrixDirty()
{
    Context& context = getContext();

    ASM_MTXIdentity(context.mMatrixStack[0][0]);
    ASM_MTXIdentity(context.mMatrixStack[1][0]);
    ASM_MTXIdentity(context.mMatrixStack[3][0]);
    ASM_MTXIdentity(context.mMatrixStack[2][0]);

    context.mMatrixDirty[2] = true;
    mCurrentVertexShader = -1;
    context.mMatrixDirty[1] = true;
    mCurrentPixelShader = -1;
    context.mMatrixDirty[3] = true;
    context.mMatrixDirty[0] = true;
}

void* Renderer::MemoryAllocMEM1(unsigned int size, unsigned int alignment, bool bReserved)
{
    if (alignment < 4)
    {
        alignment = 4;
    }

    if (!bReserved)
    {
        return MEMAllocFromExpHeapEx(mMEM1Heap, size, alignment);
    }

    if (mReservedMEM1 == NULL)
    {
        mReservedMEM1 = MEMAllocFromExpHeapEx(mMEM1Heap, size, alignment);
    }

    return mReservedMEM1;
}

void Renderer::MemoryFreeMEM1(void* memory)
{
    MEMFreeToExpHeap(mMEM1Heap, memory);
}

void* Renderer::MemoryAllocMEM2(unsigned int size, unsigned int alignment)
{
    if (alignment < 4)
    {
        alignment = 4;
    }

    return MEMAllocFromDefaultHeapEx(size, alignment);
}

void Renderer::MemoryFreeMEM2(void* memory)
{
    MEMFreeToDefaultHeap(memory);
}

void* Renderer::MemoryAllocBucket(unsigned int size, unsigned int alignment)
{
    return MEMAllocFromExpHeapEx(mBucketHeap, size, alignment);
}

void Renderer::MemoryFreeBucket(void* memory)
{
    MEMFreeToExpHeap(mBucketHeap, memory);
}

void Renderer::MemoryInit()
{
    MEMHeapHandle heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
    unsigned int size = MEMGetAllocatableSizeForFrmHeapEx(heap, 4);
    void* memory = MEMAllocFromFrmHeapEx(heap, size, 4);
    mMEM1Heap = MEMCreateExpHeapEx(memory, size, 0);

    heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_FG);
    size = MEMGetAllocatableSizeForFrmHeapEx(heap, 4);
    memory = MEMAllocFromFrmHeapEx(heap, size, 4);
    mBucketHeap = MEMCreateExpHeapEx(memory, size, 0);
}

void Renderer::MemoryRelease()
{
    MEMDestroyExpHeap(mMEM1Heap);
    MEMFreeToFrmHeap(MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1), MEM_FRM_HEAP_FREE_ALL);
    MEMDestroyExpHeap(mBucketHeap);
    MEMFreeToFrmHeap(MEMGetBaseHeapHandle(MEM_BASE_HEAP_FG), MEM_FRM_HEAP_FREE_ALL);
}

void Renderer::StateSetColour(float red, float green, float blue, float alpha)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetColor(red, green, blue, alpha);
        }
    }
    else
    {
        mPixelUniformsDirty = true;
        mColour[2] = blue;
        mColour[3] = alpha;
        mColour[1] = green;
        mColour[0] = red;
    }
}

void Renderer::StateSetColourAdd(float red, float green, float blue, float alpha)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        mColourAdd[0] = red;
        mColourAdd[1] = green;
        mColourAdd[2] = blue;
        mColourAdd[3] = alpha;
        mPixelUniformsDirty = true;
    }
}

void Renderer::StateSetTexColour(int stage, float red, float green, float blue, float alpha)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL && (unsigned int)stage < 4)
    {
        mTexColour[stage][0] = red;
        mTexColour[stage][1] = green;
        mTexColour[stage][2] = blue;
        mTexColour[stage][3] = alpha;
        mPixelUniformsDirty = true;
    }
}

void Renderer::StateSetDepthMask(bool bDepthMask)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetDepthMask(bDepthMask);
        }
    }
    else
    {
        context.mDepthMask = bDepthMask;
        GX2SetDepthStencilControl(context.mDepthTestEnable ? GX2_TRUE : GX2_FALSE, bDepthMask ? GX2_TRUE : GX2_FALSE,
                                  context.mDepthFunc, GX2_TRUE, GX2_TRUE, context.mStencilFunc, GX2_STENCIL_REPLACE,
                                  GX2_STENCIL_KEEP, GX2_STENCIL_KEEP, context.mStencilFunc, GX2_STENCIL_REPLACE,
                                  GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    }
}

void Renderer::StateSetBlendEnable(bool bBlend)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetBlendEnable(bBlend);
        }
    }
    else
    {
        context.mBlendEnable = bBlend;
        GX2SetColorControl(GX2_LOGIC_OP_COPY, bBlend, GX2_FALSE, GX2_TRUE);
    }
}

void Renderer::StateSetBlendFunc(int source, int destination)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetBlendFunc(source, destination);
        }
    }
    else
    {
        context.mBlendSource = static_cast<GX2BlendFunction>(source);
        context.mBlendDestination = static_cast<GX2BlendFunction>(destination);
        GX2SetBlendControl(GX2_RENDER_TARGET_0, context.mBlendSource, context.mBlendDestination, GX2_BLEND_COMBINE_ADD,
                           GX2_FALSE, GX2_BLEND_ZERO, GX2_BLEND_ZERO, GX2_BLEND_COMBINE_ADD);
    }
}

void Renderer::StateSetBlendFactor(unsigned int factor)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetBlendFactor(factor);
        }
    }
    else
    {
        float red = (float)(factor & 0xFF) / 255.0f;
        float green = (float)((factor >> 8) & 0xFF) / 255.0f;
        float blue = (float)((factor >> 16) & 0xFF) / 255.0f;
        float alpha = (float)(factor >> 24) / 255.0f;
        GX2SetBlendConstantColor(red, green, blue, alpha);
    }
}

void Renderer::StateSetAlphaFunc(int alphaFunc, float reference)
{
    Context& context = getContext();
    context.mAlphaTestRef = reference;
    GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GEQUAL, context.mAlphaTestEnable ? reference : 0.0f);
}

void Renderer::StateSetDepthFunc(int depthFunc)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetDepthFunc(depthFunc);
        }
    }
    else
    {
        context.mDepthFunc = static_cast<GX2CompareFunction>(depthFunc);
        GX2SetDepthStencilControl(context.mDepthTestEnable ? GX2_TRUE : GX2_FALSE,
                                  context.mDepthMask ? GX2_TRUE : GX2_FALSE, context.mDepthFunc, GX2_TRUE, GX2_TRUE,
                                  context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP,
                                  context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    }
}

void Renderer::StateSetFaceCull(bool bFaceCull)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetFaceCull(bFaceCull);
        }
    }
    else
    {
        context.mFaceCull = bFaceCull;
        GX2SetPolygonControl(GX2_FRONT_FACE_CCW, GX2_FALSE, bFaceCull ? GX2_TRUE : GX2_FALSE, GX2_FALSE,
                             GX2_POLYGON_MODE_TRIANGLE, GX2_POLYGON_MODE_TRIANGLE, GX2_TRUE, GX2_TRUE, GX2_FALSE);
    }
}

void Renderer::StateSetFaceCullCW(bool bClockwise)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        GX2SetPolygonControl(bClockwise ? GX2_FRONT_FACE_CCW : GX2_FRONT_FACE_CW, GX2_FALSE,
                             context.mFaceCull ? GX2_TRUE : GX2_FALSE, GX2_FALSE, GX2_POLYGON_MODE_TRIANGLE,
                             GX2_POLYGON_MODE_TRIANGLE, GX2_TRUE, GX2_TRUE, GX2_FALSE);
    }
}

void Renderer::StateSetLineWidth(float width)
{
}

void Renderer::StateSetWriteEnable(bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
    getContext();
    GX2SetTargetChannelMasks((bRed ? GX2_CHANNEL_MASK_R : GX2_CHANNEL_MASK_NONE)
                                 | (bGreen ? GX2_CHANNEL_MASK_G : GX2_CHANNEL_MASK_NONE)
                                 | (bBlue ? GX2_CHANNEL_MASK_B : GX2_CHANNEL_MASK_NONE)
                                 | (bAlpha ? GX2_CHANNEL_MASK_A : GX2_CHANNEL_MASK_NONE),
                             GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE,
                             GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE);
}

void Renderer::StateSetDepthTestEnable(bool bDepthTest)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetDepthTestEnable(bDepthTest);
        }
    }
    else
    {
        context.mDepthTestEnable = bDepthTest;
        GX2SetDepthStencilControl(bDepthTest ? GX2_TRUE : GX2_FALSE, context.mDepthMask ? GX2_TRUE : GX2_FALSE,
                                  context.mDepthFunc, GX2_TRUE, GX2_TRUE, context.mStencilFunc, GX2_STENCIL_REPLACE,
                                  GX2_STENCIL_KEEP, GX2_STENCIL_KEEP, context.mStencilFunc, GX2_STENCIL_REPLACE,
                                  GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    }
}

void Renderer::StateSetAlphaTestEnable(bool bAlphaTest)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        context.mAlphaTestEnable = bAlphaTest;
        GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GEQUAL, bAlphaTest ? context.mAlphaTestRef : 0.0f);
    }
}

void Renderer::StateSetDepthSlopeAndBias(float slope, float bias)
{
    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer == NULL)
    {
        GX2SetPolygonControl(GX2_FRONT_FACE_CCW, GX2_FALSE, context.mFaceCull ? GX2_TRUE : GX2_FALSE, GX2_FALSE,
                             GX2_POLYGON_MODE_TRIANGLE, GX2_POLYGON_MODE_TRIANGLE, GX2_TRUE, GX2_TRUE, GX2_FALSE);
        GX2SetPolygonOffset(bias, slope, bias, slope, 0.0f);
    }
}

void Renderer::StateSetFogEnable(bool bFog)
{
    Context& context = getContext();
    context.mFogEnable = bFog;
}

void Renderer::StateSetFogMode(int mode)
{
    Context& context = getContext();
    context.mFogMode = mode;
}

void Renderer::StateSetFogNearDistance(float nearDistance)
{
    Context& context = getContext();
    context.mFogNearDistance = nearDistance;
}

void Renderer::StateSetFogFarDistance(float farDistance)
{
    Context& context = getContext();
    context.mFogFarDistance = farDistance;
}

void Renderer::StateSetFogDensity(float density)
{
    Context& context = getContext();
    context.mFogDensity = density;
}

void Renderer::StateSetFogColour(float red, float green, float blue)
{
    Context& context = getContext();
    mFogColour[0] = red;
    mFogColour[1] = green;
    mFogColour[2] = blue;
    mFogColour[3] = 1.0f;
    mPixelUniformsDirty = true;
}

void Renderer::StateSetLightingEnable(bool bLighting)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL && context.mRecordingBuffer->mRecordState)
    {
        context.mRecordingBuffer->SetLightingEnable(bLighting);
        return;
    }

    context.mLightingEnable = bLighting;
}

void Renderer::StateSetVertexTextureUV(float u, float v)
{
    float uniform[4];
    uniform[0] = u - 1.0f;
    uniform[1] = v - 1.0f;
    uniform[2] = 0.0f;
    uniform[3] = 0.0f;
    GX2SetVertexUniformReg(0x40, 4, uniform);
}

void Renderer::StateSetTextureScaleUI(float scaleU, float scaleV, float offsetU, float offsetV)
{
    float uniform[4];
    uniform[0] = scaleU;
    uniform[1] = scaleV;
    uniform[2] = offsetU;
    uniform[3] = offsetV;
    GX2SetVertexUniformReg(0x84, 4, uniform);
}

void Renderer::StateSetScalingGridUI(float left, float right, float top, float bottom, float innerLeft,
                                     float innerRight, float innerTop, float innerBottom, float scaleU, float scaleV)
{
    float edges[4];
    float inner[4];
    float scale[4];

    edges[0] = left;
    edges[1] = right;
    edges[2] = top;
    edges[3] = bottom;
    inner[0] = innerLeft;
    inner[1] = innerRight;
    inner[2] = innerTop;
    inner[3] = innerBottom;
    scale[0] = scaleU;
    scale[1] = scaleV;
    scale[2] = 0.0f;
    scale[3] = 0.0f;

    GX2SetVertexUniformReg(0x88, 4, edges);
    GX2SetVertexUniformReg(0x8C, 4, inner);
    GX2SetVertexUniformReg(0x90, 4, scale);
}

void Renderer::StateSetLightColour(int light, float red, float green, float blue)
{
    if (light >= 2)
    {
        return;
    }

    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetLightColour(light, red, green, blue);
        }
    }
    else
    {
        context.mLightColour[light][0] = red;
        context.mLightColour[light][1] = green;
        context.mLightingDirty = true;
        context.mLightColour[light][3] = 1.0f;
        context.mLightColour[light][2] = blue;
    }
}

void Renderer::StateSetLightAmbientColour(float red, float green, float blue)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetLightAmbientColour(red, green, blue);
        }
    }
    else
    {
        context.mLightAmbientColour[0] = red;
        context.mLightAmbientColour[2] = blue;
        context.mLightAmbientColour[3] = 1.0f;
        context.mLightingDirty = true;
        context.mLightAmbientColour[1] = green;
    }
}

void Renderer::StateSetLightDirection(int light, float x, float y, float z)
{
    if (light >= 2)
    {
        return;
    }

    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetLightDirection(light, x, y, z);
        }
    }
    else
    {
        Vec direction;
        direction.x = x;
        direction.y = y;
        direction.z = z;

        float transposed[4][4];
        ASM_MTX44Transpose(context.mMatrixStack[0][context.mMatrixStackDepth[0]], transposed);
        ASM_MTX44MultVecSR(transposed, &direction, &direction);
        ASM_VECNormalize(&direction, &direction);

        context.mLightDirection[light][0] = direction.x;
        context.mLightDirection[light][1] = direction.y;
        context.mLightDirection[light][2] = direction.z;
        context.mLightDirection[light][3] = 0.0f;
        context.mLightingDirty = true;
    }
}

void Renderer::StateSetLightEnable(int light, bool bEnable)
{
    if (light >= 2)
    {
        return;
    }

    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        if (context.mRecordingBuffer->mRecordState)
        {
            context.mRecordingBuffer->SetLightEnable(light, bEnable);
        }
    }
    else
    {
        context.mLightEnable[light] = bEnable;
        context.mLightingDirty = true;
    }
}

void Renderer::StateSetViewport(C4JRender::eViewportType viewport)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        return;
    }

    mCurrentViewport = viewport;

    float screenWidth = (float)mScreenWidth[mScreen];
    float screenHeight = (float)mScreenHeight[mScreen];
    float x = 0.0f;
    float y = 0.0f;
    float width = screenWidth;
    float height = screenHeight;

    switch (viewport)
    {
    case C4JRender::eViewportType_FullScreen:
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = x / screenWidth;
        context.mViewportScale[3] = y / screenHeight;
        break;
    case C4JRender::eViewportType_TopHalf:
        height = screenHeight * 0.5f;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = x / screenWidth;
        context.mViewportScale[3] = y / screenHeight;
        break;
    case C4JRender::eViewportType_BottomHalf:
        height = screenHeight * 0.5f;
        y = height;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = x / screenWidth;
        context.mViewportScale[3] = height / screenHeight;
        break;
    case C4JRender::eViewportType_LeftHalf:
        width = screenWidth * 0.5f;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = x / screenWidth;
        context.mViewportScale[3] = y / screenHeight;
        break;
    case C4JRender::eViewportType_RightHalf:
        width = screenWidth * 0.5f;
        x = width;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = width / screenWidth;
        context.mViewportScale[3] = y / screenHeight;
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
        width = screenWidth * 0.5f;
        height = screenHeight * 0.5f;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = x / screenWidth;
        context.mViewportScale[3] = y / screenHeight;
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        width = screenWidth * 0.5f;
        height = screenHeight * 0.5f;
        x = width;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = width / screenWidth;
        context.mViewportScale[3] = y / screenHeight;
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        width = screenWidth * 0.5f;
        height = screenHeight * 0.5f;
        y = height;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = x / screenWidth;
        context.mViewportScale[3] = height / screenHeight;
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        width = screenWidth * 0.5f;
        height = screenHeight * 0.5f;
        x = width;
        y = height;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = width / screenWidth;
        context.mViewportScale[3] = height / screenHeight;
        break;
    default:
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = x / screenWidth;
        context.mViewportScale[3] = y / screenHeight;
        break;
    }

    GX2SetViewport(x, y, width, height, 0.0f, 1.0f);
    GX2SetScissor((unsigned int)x, (unsigned int)y, (unsigned int)width, (unsigned int)height);
}

void Renderer::StateGetScissorRect(unsigned int& left, unsigned int& top, unsigned int& right, unsigned int& bottom)
{
    Context& context = getContext();
    left = context.mScissorRect[0];
    top = context.mScissorRect[1];
    right = context.mScissorRect[2];
    bottom = context.mScissorRect[3];
}

void Renderer::StateSetScissorRect(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom)
{
    Context& context = getContext();
    unsigned int width = right - left;
    unsigned int height = bottom - top;
    context.mScissorRect[0] = left;
    context.mScissorRect[1] = top;
    context.mScissorRect[2] = right;
    context.mScissorRect[3] = bottom;
    GX2SetScissor(left, top, width, height);
}

void Renderer::StateSetEnableViewportClipPlanes(bool bEnable)
{
}

void Renderer::Mtx44Vec4Transform(float (*matrix)[4], _Vec4* source, _Vec4* destination)
{
    destination->x = matrix[0][0] * source->x + matrix[0][1] * source->y + matrix[0][2] * source->z
                     + matrix[0][3] * source->w;
    destination->y = matrix[1][0] * source->x + matrix[1][1] * source->y + matrix[1][2] * source->z
                     + matrix[1][3] * source->w;
    destination->z = matrix[2][0] * source->x + matrix[2][1] * source->y + matrix[2][2] * source->z
                     + matrix[2][3] * source->w;
    destination->w = matrix[3][0] * source->x + matrix[3][1] * source->y + matrix[3][2] * source->z
                     + matrix[3][3] * source->w;
}

void Renderer::StateSetTexGenCol(int coordinate, float x, float y, float z, float w, bool bEyeLinear)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        return;
    }

    _Vec4 plane;
    plane.x = x;
    plane.y = y;
    plane.z = z;
    plane.w = w;

    if (bEyeLinear)
    {
        float modelView[4][4];
        memcpy(modelView, MatrixGet(0), 0x40);
        ASM_MTX44Transpose(modelView, modelView);

        float inverse[4][4];
        C_MTX44Inverse(modelView, inverse);

        float transposed[4][4];
        ASM_MTX44Transpose(inverse, transposed);

        _Vec4 transformed;
        Mtx44Vec4Transform(transposed, &plane, &transformed);
        plane = transformed;
    }

    int matrix = bEyeLinear ? 0 : 1;
    context.mTexGenMatrix[matrix][0][coordinate] = plane.x;
    context.mTexGenMatrix[matrix][1][coordinate] = plane.y;
    context.mTexGenMatrix[matrix][2][coordinate] = plane.z;
    context.mTexGenMatrix[matrix][3][coordinate] = plane.w;

    int other = bEyeLinear ? 1 : 0;
    context.mTexGenMatrix[other][0][coordinate] = 0.0f;
    context.mTexGenMatrix[other][1][coordinate] = 0.0f;
    context.mTexGenMatrix[other][2][coordinate] = 0.0f;
    context.mTexGenMatrix[other][3][coordinate] = 0.0f;
}

void Renderer::StateSetStencil(int stencilFunc, unsigned char reference, unsigned char compareMask,
                               unsigned char writeMask)
{
    Context& context = getContext();
    context.mStencilFunc = static_cast<GX2CompareFunction>(stencilFunc);

    if (context.mRecordingBuffer == NULL)
    {
        GX2SetDepthStencilControl(context.mDepthTestEnable ? GX2_TRUE : GX2_FALSE,
                                  context.mDepthMask ? GX2_TRUE : GX2_FALSE, context.mDepthFunc, GX2_TRUE, GX2_TRUE,
                                  context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP,
                                  context.mStencilFunc, GX2_STENCIL_REPLACE, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
        GX2SetStencilMask(compareMask, writeMask, reference, compareMask, writeMask, reference);
    }
}

void Renderer::StateSetForceLOD(int lod)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        context.mForceLOD = lod;
    }
}

void Renderer::StateSetLerpColour(float red, float green, float blue, float alpha)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        mLerpColour[0] = red;
        mLerpColour[1] = green;
        mLerpColour[2] = blue;
        mLerpColour[3] = alpha;
        mPixelUniformsDirty = true;
    }
}

void Renderer::SetLayeredColour(int layer, float red, float green, float blue)
{
    Context& context = getContext();
    context.mLayeredColour[layer][0] = red;
    context.mLayeredColour[layer][1] = green;
    context.mLayeredColour[layer][2] = blue;
    context.mLayeredColour[layer][3] = 1.0f;
}

void Renderer::SetLayeredParam(int slot, int layer, float offset, float scale)
{
    Context& context = getContext();
    mLayeredParams[slot][0] = context.mLayeredColour[layer][0];
    mLayeredParams[slot][1] = context.mLayeredColour[layer][1];
    mLayeredParams[slot][2] = context.mLayeredColour[layer][2];
    mLayeredParams[slot][3] = offset + scale;
    mLayeredParamsDirty = true;
}

void Renderer::SetLayeredTextureSize(int width, int height)
{
    getContext();
    mLayeredTextureSize[0] = (float)width;
    mLayeredTextureSize[1] = (float)height;
    mLayeredParamsDirty = true;
}

void Renderer::StateSetBoneMatrixData(int bone)
{
    Context& context = getContext();
    memcpy(&context.mBoneMatrix[bone], MatrixGet(0), 0x40);

    int count = bone + 1;
    if (count > context.mNumBoneMatrices)
    {
        context.mNumBoneMatrices = count;
    }
}

void Renderer::StateSetBoneMatrixNull(int bone)
{
    Context& context = getContext();
    memset(&context.mBoneMatrix[bone], 0, 0x40);

    int count = bone + 1;
    if (count > context.mNumBoneMatrices)
    {
        context.mNumBoneMatrices = count;
    }
}

void Renderer::StateSetVertexAlphaDistance(float distance)
{
    Context& context = getContext();
    context.mVertexAlphaDistance = distance;
}

void Renderer::StateSetVertexAlpha(float alpha)
{
    Context& context = getContext();

    if (context.mVertexAlpha != alpha)
    {
        context.mVertexAlpha = alpha;
        GX2SetVertexUniformReg(0xA0, 4, &context.mVertexAlpha);
    }
}

void Renderer::UpdateFogState()
{
    Context& context = getContext();

    float uniform[4];
    uniform[0] = 0.0f;
    uniform[1] = 0.0f;
    uniform[2] = 0.0f;
    uniform[3] = 0.0f;

    if (!context.mFogEnable)
    {
        uniform[2] = 0.0f;
        GX2SetVertexUniformReg(0x44, 4, uniform);
        return;
    }

    if (context.mFogMode == 1)
    {
        uniform[0] = context.mFogFarDistance;
        uniform[1] = context.mFogNearDistance;
        uniform[2] = 1.0f;
        uniform[3] = context.mVertexAlphaDistance;
    }
    else
    {
        uniform[0] = context.mFogDensity;
        if (context.mFogMode == 2)
        {
            uniform[2] = 2.0f;
        }
        else
        {
            uniform[2] = 3.0f;
        }
        uniform[3] = context.mVertexAlphaDistance;
    }

    GX2SetVertexUniformReg(0x44, 4, uniform);
}

void Renderer::UpdateLightingState()
{
    Context& context = getContext();

    if (!context.mLightingDirty || !context.mLightingEnable)
    {
        return;
    }

    if (!context.mLightEnable[0])
    {
        memset(context.mLightDirection[0], 0, 16);
        memset(context.mLightColour[0], 0, 16);
    }

    if (!context.mLightEnable[1])
    {
        memset(context.mLightDirection[1], 0, 16);
        memset(context.mLightColour[1], 0, 16);
    }

    GX2SetVertexUniformReg(0x48, 0x14, context.mLightDirection);
    context.mLightingDirty = false;
}

void Renderer::UpdateViewportState()
{
}

void Renderer::UpdateTexGenState()
{
    Context& context = getContext();
    GX2SetVertexUniformReg(0x5C, 0x10, context.mTexGenMatrix[0]);
    GX2SetVertexUniformReg(0x6C, 0x10, context.mTexGenMatrix[1]);
}

void Renderer::StateUpdate()
{
    Context& context = getContext();
    StateSetFaceCull(context.mFaceCull);
    StateSetDepthMask(context.mDepthMask);
    StateSetDepthTestEnable(context.mDepthTestEnable);
    StateSetAlphaTestEnable(context.mAlphaTestEnable);
}

int Renderer::GetMaxTextures()
{
    return 512;
}

int Renderer::TextureCreate()
{
    for (int texture = 0; texture < 512; ++texture)
    {
        if (!mTextures[texture].mUsed)
        {
            mTextures[texture].mTexture.surface.imagePtr = NULL;
            mTextures[texture].mTexture.surface.mipPtr = NULL;
            mTextures[texture].mFlags = 0;
            mTextures[texture].mUsed = true;
            mTextures[texture].mLevels = 1;
            GX2InitSampler(&mTextures[texture].mSampler, GX2_TEX_CLAMP_WRAP, GX2_TEX_XY_FILTER_POINT);
            return texture;
        }
    }

    return -1;
}

void Renderer::TextureFree(int texture)
{
    if (mTextures[texture].mTexture.surface.imagePtr != NULL)
    {
        MemoryFreeMEM2(mTextures[texture].mTexture.surface.imagePtr);
    }

    if (mTextures[texture].mTexture.surface.mipPtr != NULL)
    {
        MemoryFreeMEM2(mTextures[texture].mTexture.surface.mipPtr);
    }

    mTextures[texture].mTexture.surface.imagePtr = NULL;
    mTextures[texture].mTexture.surface.mipPtr = NULL;
    mTextures[texture].mUsed = false;
}

void Renderer::TextureBind(int texture)
{
    TextureBind(0, texture);
}

void Renderer::UpdateTextureState(int stage)
{
    Context& context = getContext();

    if (mTextures[context.mBoundTexture[stage]].mTexture.surface.imagePtr == NULL)
    {
        return;
    }

    if (context.mTextureIsVertex[stage])
    {
        GX2SetVertexTexture(&mTextures[context.mBoundTexture[stage]].mTexture, stage);
        GX2SetVertexSampler(&mTextures[context.mBoundTexture[stage]].mSampler, stage);
    }
    else
    {
        GX2SetPixelTexture(&mTextures[context.mBoundTexture[stage]].mTexture, stage);
        GX2SetPixelSampler(&mTextures[context.mBoundTexture[stage]].mSampler, stage);
    }
}

void Renderer::TextureBind(int stage, int texture)
{
    if (texture == -1)
    {
        texture = mDefaultTexture;
    }

    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer != NULL && buffer->mRecordState)
    {
        buffer->BindTexture(texture);
    }

    context.mBoundTexture[stage] = texture;
    context.mTextureIsMask[stage] = false;
    context.mTextureIsVertex[stage] = false;
    UpdateTextureState(stage);
}

void Renderer::TextureBindMask(int stage)
{
    Context& context = getContext();
    context.mTextureIsMask[stage] = true;
}

void Renderer::TextureBindVertex(int texture)
{
    if (texture == -1)
    {
        texture = mDefaultTexture;
    }

    Context& context = getContext();
    context.mBoundTexture[0] = texture;
    context.mTextureIsMask[0] = false;
    context.mTextureIsVertex[0] = true;
    UpdateTextureState(0);
}

void Renderer::TextureSetTextureLevels(int levels)
{
    Context& context = getContext();
    mTextures[context.mBoundTexture[0]].mLevels = levels;
}

int Renderer::TextureGetTextureLevels()
{
    Context& context = getContext();
    return mTextures[context.mBoundTexture[0]].mLevels;
}

void Renderer::TextureData(int width, int height, void* data, int level, C4JRender::eTextureFormat format)
{
    Context& context = getContext();
    Texture& texture = mTextures[context.mBoundTexture[0]];
    texture.mFormat = format;

    if (level == 0)
    {
        GX2InitTexture(&texture.mTexture, width, height, 1, texture.mLevels, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
                       GX2_SURFACE_DIM_2D);
        texture.mTexture.surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
        GX2CalcSurfaceSizeAndAlignment(&texture.mTexture.surface);
        GX2InitTextureRegs(&texture.mTexture);

        void* image = NULL;
        void* mipImage = NULL;

        if (texture.mTexture.surface.imageSize != 0)
        {
            image = MemoryAllocMEM2(texture.mTexture.surface.imageSize, texture.mTexture.surface.alignment);
        }

        if (texture.mTexture.surface.mipSize != 0)
        {
            mipImage = MemoryAllocMEM2(texture.mTexture.surface.mipSize, texture.mTexture.surface.alignment);
        }

        texture.mTexture.surface.imagePtr = image;

        if (texture.mTexture.surface.numMips > 1 && mipImage == NULL)
        {
            mipImage = (unsigned char*)image + texture.mTexture.surface.mipOffset[0];
        }

        texture.mTexture.surface.mipPtr = mipImage;

        if (image == NULL || data == NULL)
        {
            return;
        }

        unsigned int sourcePitch = texture.mTexture.surface.width * 4;
        unsigned int destinationPitch = texture.mTexture.surface.pitch * 4;
        unsigned char* destination = (unsigned char*)image;
        unsigned char* source = (unsigned char*)data;

        for (unsigned int row = 0; row < texture.mTexture.surface.height; ++row)
        {
            memcpy(destination, source, sourcePitch);
            source += sourcePitch;
            destination += destinationPitch;
        }

        GX2Invalidate(GX2_INVALIDATE_CPU, texture.mTexture.surface.imagePtr, texture.mTexture.surface.imageSize);
    }
    else
    {
        if (texture.mTexture.surface.mipPtr == NULL || data == NULL)
        {
            return;
        }

        unsigned int mipOffset = 0;
        if (level != 1)
        {
            mipOffset = texture.mTexture.surface.mipOffset[level - 1];
        }

        unsigned int rows = texture.mTexture.surface.height >> level;
        unsigned int sourcePitch = (texture.mTexture.surface.width >> level) * 4;
        unsigned int destinationPitch = GX2GetSurfaceMipPitch(&texture.mTexture.surface, level) * 4;
        unsigned char* destination = (unsigned char*)texture.mTexture.surface.mipPtr + mipOffset;
        unsigned char* source = (unsigned char*)data;

        for (unsigned int row = 0; row < rows; ++row)
        {
            memcpy(destination, source, sourcePitch);
            source += sourcePitch;
            destination += destinationPitch;
        }

        GX2Invalidate(GX2_INVALIDATE_CPU, (unsigned char*)texture.mTexture.surface.mipPtr + mipOffset,
                      destinationPitch * rows);
    }
}

void Renderer::TextureDataUpdate(int x, int y, int width, int height, void* data, int level)
{
    Context& context = getContext();
    Texture& texture = mTextures[context.mBoundTexture[0]];

    if (x != 0 || y != 0 || data == NULL)
    {
        return;
    }

    unsigned int levelWidth = texture.mTexture.surface.width >> level;
    if ((unsigned int)width != levelWidth)
    {
        return;
    }

    unsigned int levelHeight = texture.mTexture.surface.height >> level;
    if ((unsigned int)height != levelHeight)
    {
        return;
    }

    if (level == 0)
    {
        unsigned int sourcePitch = texture.mTexture.surface.width * 4;
        unsigned int destinationPitch = texture.mTexture.surface.pitch * 4;
        unsigned char* destination = (unsigned char*)texture.mTexture.surface.imagePtr;
        unsigned char* source = (unsigned char*)data;

        if (destination == NULL)
        {
            return;
        }

        if (sourcePitch == destinationPitch)
        {
            GX2Invalidate(GX2_INVALIDATE_CPU, destination, texture.mTexture.surface.imageSize);
            GX2Invalidate(GX2_INVALIDATE_CPU, source, texture.mTexture.surface.imageSize);
            DMAETimeStamp timestamp = DMAECopyMem(destination, source, texture.mTexture.surface.imageSize >> 2,
                                                  DMAE_SWAP_NONE);
            DMAEWaitDone(timestamp);
            return;
        }

        for (unsigned int row = 0; row < texture.mTexture.surface.height; ++row)
        {
            memcpy(destination, source, sourcePitch);
            destination += destinationPitch;
            source += sourcePitch;
        }

        GX2Invalidate(GX2_INVALIDATE_CPU, texture.mTexture.surface.imagePtr, texture.mTexture.surface.imageSize);
        return;
    }

    if (texture.mTexture.surface.mipPtr == NULL)
    {
        return;
    }

    unsigned int mipOffset = 0;
    if (level != 1)
    {
        mipOffset = texture.mTexture.surface.mipOffset[level - 1];
    }

    unsigned char* destination = (unsigned char*)texture.mTexture.surface.mipPtr + mipOffset;
    unsigned int sourcePitch = levelWidth * 4;
    unsigned int destinationPitch = GX2GetSurfaceMipPitch(&texture.mTexture.surface, level) * 4;
    unsigned char* source = (unsigned char*)data;
    unsigned char* row = destination;

    for (unsigned int line = 0; line < levelHeight; ++line)
    {
        memcpy(row, source, sourcePitch);
        source += sourcePitch;
        row += destinationPitch;
    }

    GX2Invalidate(GX2_INVALIDATE_CPU, destination, destinationPitch * levelHeight);
}

void Renderer::TextureSetParam(int parameter, int value)
{
    Context& context = getContext();

    if (parameter == 0)
    {
        return;
    }

    Texture& texture = mTextures[context.mBoundTexture[0]];
    unsigned int flags = texture.mFlags;

    if (parameter == 1)
    {
        flags &= ~4u;
        texture.mFlags = flags;
        if (value == 1)
        {
            flags |= 4u;
            texture.mFlags = flags;
        }
    }
    else if (parameter == 2)
    {
        flags &= ~8u;
        texture.mFlags = flags;
        if (value == 1)
        {
            flags |= 8u;
            texture.mFlags = flags;
        }
    }
    else if (parameter == 3)
    {
        flags &= ~1u;
        texture.mFlags = flags;
        if (value == 0)
        {
            flags |= 1u;
            texture.mFlags = flags;
        }
    }
    else if (parameter == 4)
    {
        flags &= ~2u;
        texture.mFlags = flags;
        if (value == 0)
        {
            flags |= 2u;
            texture.mFlags = flags;
        }
    }

    GX2InitSamplerClamping(&texture.mSampler, (flags & 1) != 0 ? GX2_TEX_CLAMP_CLAMP : GX2_TEX_CLAMP_WRAP,
                           (flags & 2) != 0 ? GX2_TEX_CLAMP_CLAMP : GX2_TEX_CLAMP_WRAP, GX2_TEX_CLAMP_CLAMP);
    flags = mTextures[context.mBoundTexture[0]].mFlags;
    GX2InitSamplerXYFilter(&mTextures[context.mBoundTexture[0]].mSampler,
                           (flags & 8) != 0 ? GX2_TEX_XY_FILTER_BILINEAR : GX2_TEX_XY_FILTER_POINT,
                           (flags & 4) != 0 ? GX2_TEX_XY_FILTER_BILINEAR : GX2_TEX_XY_FILTER_POINT,
                           GX2_TEX_ANISO_1_TO_1);
    GX2InitSamplerZMFilter(&mTextures[context.mBoundTexture[0]].mSampler, GX2_TEX_Z_FILTER_POINT,
                           GX2_TEX_MIP_FILTER_LINEAR);
    UpdateTextureState(0);
}

void Renderer::TextureDynamicUpdateStart()
{
}

void Renderer::TextureDynamicUpdateEnd()
{
}

int Renderer::LoadTextureData(const char* fileName, D3DXIMAGE_INFO* info, int** data)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = 1;

    int file = CreateFileA(fileName, 0x80000000, 0, NULL, 3, 0x80, 0);
    if (file == -1)
    {
        return -1;
    }

    unsigned int fileSize = GetFileSize(file, NULL);
    void* fileData = malloc(fileSize);
    ReadFile(file, fileData, fileSize, NULL, NULL);
    png_image_begin_read_from_memory(&image, fileData, fileSize);
    CloseHandle(file);

    if ((image.warning_or_error & 3) > 1)
    {
        return -1;
    }

    image.format = 0x23;
    *data = (int*)new unsigned char[image.height * image.width * 4];

    if (*data != NULL && png_image_finish_read(&image, NULL, *data, 0, NULL))
    {
        free(fileData);
        info->Width = image.width;
        info->Height = image.height;
        return 0;
    }

    free(fileData);
    return -1;
}

int Renderer::LoadTextureData(unsigned char* fileData, unsigned int fileSize, D3DXIMAGE_INFO* info, int** data)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = 1;

    png_image_begin_read_from_memory(&image, fileData, fileSize);

    if ((image.warning_or_error & 3) > 1)
    {
        return -1;
    }

    image.format = 0x23;
    *data = (int*)new unsigned char[image.height * image.width * 4];

    if (*data != NULL && png_image_finish_read(&image, NULL, *data, 0, NULL))
    {
        info->Width = image.width;
        info->Height = image.height;
        return 0;
    }

    return -1;
}

int Renderer::SaveTextureData(const char* fileName, D3DXIMAGE_INFO* info, int* data)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = 1;
    image.width = info->Width;
    image.height = info->Height;
    image.format = 3;
    png_image_write_to_file(&image, fileName, 0, data, 0, NULL);
    return 0;
}

int Renderer::SaveTextureDataToMemory(void* destination, int destinationSize, int* bytesWritten, int width, int height,
                                      int* data)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = 1;
    image.width = width;
    image.height = height;
    image.format = 3;
    user_write_data_init(destination, destinationSize);
    png_image_write_to_stdio(&image, NULL, 0, data, 0, NULL, reinterpret_cast<void*>(user_write_data),
                             reinterpret_cast<void*>(user_flush_data));
    *bytesWritten = user_write_data_bytes_written();
    return 0;
}

void Renderer::TextureGetStats()
{
}

GX2Texture* Renderer::TextureGetTexture(int texture)
{
    if ((unsigned int)texture < 512 && mTextures[texture].mUsed)
    {
        return &mTextures[texture].mTexture;
    }

    return NULL;
}

C4JRender::ePixelShaderType Renderer::FinalisePixelShaderType(C4JRender::ePixelShaderType pixelShaderType)
{
    Context& context = getContext();

    if (pixelShaderType != C4JRender::ePixelShaderType_Default)
    {
        return pixelShaderType;
    }

    if (context.mOrderIndependentAlpha)
    {
        return C4JRender::ePixelShaderType_OrderIndependentAlpha;
    }

    if (context.mBoundTexture[1] == mDefaultTexture)
    {
        return C4JRender::ePixelShaderType_Default;
    }

    if (context.mBoundTexture[2] == mDefaultTexture)
    {
        if (context.mTextureIsMask[1])
        {
            return C4JRender::ePixelShaderType_LayeredColourMask;
        }

        return C4JRender::ePixelShaderType_DefaultTwoLayers;
    }

    if (context.mBoundTexture[3] == mDefaultTexture)
    {
        return C4JRender::ePixelShaderType_DefaultThreeLayers;
    }

    return C4JRender::ePixelShaderType_DefaultFourLayers;
}

void Renderer::TextureSetTexture(int texture, GX2Texture& source)
{
    mTextures[texture].mTexture = source;
}

void Renderer::TextureSetSampler(int texture, GX2Sampler& source)
{
    mTextures[texture].mSampler = source;
}

void* Renderer::GetDynamicVertexMemory(C4JRender::eVertexType vertexType, int numVertices)
{
    Context& context = getContext();

    unsigned int offset = context.mDynamicVertexOffset;
    unsigned char* buffer = (unsigned char*)context.mDynamicVertexBuffer[context.mDynamicVertexBufferIndex];
    unsigned int used = offset + ((mVertexStrides[vertexType] * numVertices + 0x1F) & 0xFFFFFFE0);
    context.mDynamicVertexOffset = used;

    if (used < 0x200000)
    {
        return buffer + offset;
    }

    return NULL;
}

void Renderer::DrawVertexSetup(C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType,
                               C4JRender::ePrimitiveType primitiveType)
{
    Context& context = getContext();

    int vertexShader = vertexType;
    if (vertexType == C4JRender::eVertexType_Default && context.mLightingEnable)
    {
        vertexShader = C4JRender::eVertexType_DefaultLighting;
    }

    if (vertexShader != mCurrentVertexShader)
    {
        GX2SetFetchShader(mFetchShaders[vertexShader]);
        GX2SetVertexShader(mVertexShaders[vertexShader]);
        mCurrentVertexShader = vertexShader;
    }

    C4JRender::ePixelShaderType finalPixelShader = FinalisePixelShaderType(pixelShaderType);

    if (finalPixelShader == C4JRender::ePixelShaderType_LayeredColourMask
        && (mCurrentPixelShader != C4JRender::ePixelShaderType_LayeredColourMask || mLayeredParamsDirty))
    {
        GX2SetPixelUniformReg(0x24, 0x80, mLayeredTextureSize);
        mLayeredParamsDirty = false;
    }

    if (finalPixelShader != mCurrentPixelShader || mPixelUniformsDirty)
    {
        GX2SetPixelUniformReg(0, 0x20, mColour);
        mPixelUniformsDirty = false;
    }

    if (finalPixelShader != mCurrentPixelShader)
    {
        GX2SetPixelShader(mPixelShaders[finalPixelShader]);
        mCurrentPixelShader = finalPixelShader;
    }

    if (context.mMatrixDirty[0])
    {
        GX2SetVertexUniformReg(0, 0x10, MatrixGet(0));
        context.mMatrixDirty[0] = false;
    }

    if (context.mMatrixDirty[1])
    {
        GX2SetVertexUniformReg(0x20, 0x10, MatrixGet(1));
        context.mMatrixDirty[1] = false;
    }

    if (context.mMatrixDirty[2])
    {
        GX2SetVertexUniformReg(0x30, 0x10, MatrixGet(2));
        context.mMatrixDirty[2] = false;
    }

    float identity[4][4];
    ASM_MTX44Identity(identity);
    GX2SetVertexUniformReg(0x10, 0x10, identity);

    UpdateFogState();
    UpdateViewportState();
    UpdateLightingState();
    UpdateTexGenState();
}

void Renderer::DrawVertices(C4JRender::ePrimitiveType primitiveType, int numVertices, void* vertices,
                            C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType,
                            unsigned char* layers)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        int recordedVertexType = vertexType;
        if (vertexType == C4JRender::eVertexType_Default && context.mLightingEnable)
        {
            recordedVertexType = C4JRender::eVertexType_DefaultLighting;
        }

        context.mRecordingVertexType = (C4JRender::eVertexType)recordedVertexType;
        context.mRecordingPrimitiveType = primitiveType;
        context.mRecordingPixelShaderType = pixelShaderType;
        context.mRecordingBuffer->AddVertices(mVertexStrides[recordedVertexType], numVertices, vertices, context,
                                              layers);
        return;
    }

    void* destination = GetDynamicVertexMemory(vertexType, numVertices);
    if (destination == NULL)
    {
        return;
    }

    DrawVertexSetup(vertexType, pixelShaderType, primitiveType);

    unsigned int size = numVertices * mVertexStrides[vertexType];
    memcpy(destination, vertices, size);
    GX2Invalidate(GX2_INVALIDATE_CPU, destination, size);
    StateUpdate();
    GX2SetAttribBuffer(0, size, mVertexStrides[vertexType], destination);
    GX2DrawEx(sPrimitiveModes[primitiveType], numVertices, 0, 1);
}

bool Renderer::InitFFL()
{
    unsigned int workSize = FFLGetWorkSize(&sFFLInitDesc);
    sFFLWorkMemory = malloc(workSize);

    if (FFLInitRes(sFFLWorkMemory, &sFFLInitDesc) != 0)
    {
        return false;
    }

    FFLInitResGPUStep();
    return true;
}

void Renderer::ShutdownFFL()
{
    FFLExit();
    free(sFFLWorkMemory);
}

void* Renderer::CreateMiiIconTexture(FFLDataSource dataSource, FFLStoreData* storeData, int index, int* texture)
{
    OSReport("Setup FFL...\n");

    int created = gRendererInstance->TextureCreate();
    GX2Texture* miiTexture = gRendererInstance->TextureGetTexture(created);
    void* result = MakeIcon(miiTexture, ICON_SHADER_TYPE_DEFAULT, dataSource, storeData, index);
    GX2Invalidate(GX2_INVALIDATE_CPU_TEXTURE, miiTexture->surface.imagePtr, miiTexture->surface.imageSize);
    GX2InitSampler(&sMiiIconSampler, GX2_TEX_CLAMP_CLAMP, GX2_TEX_XY_FILTER_BILINEAR);
    gRendererInstance->TextureSetTexture(created, *miiTexture);
    *texture = created;
    return result;
}

void Renderer::FreeMiiIconTexture(int texture, void* memory)
{
    free(memory);
    mTextures[texture].mTexture.surface.imagePtr = NULL;
    mTextures[texture].mTexture.surface.mipPtr = NULL;
    mTextures[texture].mUsed = false;
}

void Renderer::CBuffLockStaticCreations()
{
    mAllowStaticCommandBufferIds = false;
}

int Renderer::CBuffCreate(int count)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    int first = mNextCommandBufferId;
    for (;;)
    {
        if (first > 0x7FFFFF)
        {
            OSFastMutex_Unlock(&mCommandBufferMutex);
            return -1;
        }

        bool bFree = true;
        for (int index = 0; index < count; ++index)
        {
            if (mCommandBufferSlots[first + index] != -1)
            {
                bFree = false;
                break;
            }
        }

        if (bFree)
        {
            break;
        }

        ++first;
    }

    for (int index = 0; index < count; ++index)
    {
        mCommandBufferSlots[first + index] = -2;
    }

    if (mAllowStaticCommandBufferIds)
    {
        mNextCommandBufferId = first + count;
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
    return first;
}

void Renderer::DeleteInternalBuffer(int slot)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    int retired = mNumRetiredCommandBuffers + 1;
    int retiredSlot = 16000 - retired;
    mCommandBuffers[retiredSlot] = mCommandBuffers[slot];
    mNumRetiredCommandBuffers = retired;
    memcpy(&mCommandBufferMatrices[retiredSlot * 16], &mCommandBufferMatrices[slot * 16], 0x40);

    int last = mNumCommandBuffers - 1;
    mNumCommandBuffers = last;

    if (last != 0)
    {
        mCommandBuffers[slot] = mCommandBuffers[last];
        memcpy(&mCommandBufferMatrices[slot * 16], &mCommandBufferMatrices[last * 16], 0x40);
        mCommandBufferVertexTypes[slot] = mCommandBufferVertexTypes[mNumCommandBuffers];
        mCommandBufferPixelShaderTypes[slot] = mCommandBufferPixelShaderTypes[mNumCommandBuffers];
        mCommandBufferPrimitiveTypes[slot] = mCommandBufferPrimitiveTypes[mNumCommandBuffers];
        int id = mCommandBufferIds[mNumCommandBuffers];
        mCommandBufferIds[slot] = id;
        mCommandBufferSlots[id] = (short)slot;
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
}

void Renderer::CBuffDelete(int firstId, int count)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    for (int index = 0; index < count; ++index)
    {
        if (mCommandBufferSlots[firstId + index] >= 0)
        {
            DeleteInternalBuffer(mCommandBufferSlots[firstId + index]);
        }

        mCommandBufferSlots[firstId + index] = -1;
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
}

void Renderer::CBuffStart(int id, bool bRecordState)
{
    Context& context = getContext();

    CommandBuffer* buffer = new CommandBuffer(bRecordState);
    context.mMatrixMode = 3;
    context.mRecordingBuffer = buffer;
    context.mMatrixStackDepth[3] = 0;
    context.mRecordingId = id;
    ASM_MTX44Identity(context.mMatrixStack[3][0]);
    context.mMatrixDirty[3] = true;
}

void Renderer::CBuffClear(int id)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    if (mCommandBufferSlots[id] >= 0)
    {
        DeleteInternalBuffer(mCommandBufferSlots[id]);
        mCommandBufferSlots[id] = -2;
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
}

int Renderer::CBuffSize(int id)
{
    if (id == -1)
    {
        return sTotalCommandBufferBytes < 0 ? 0 : sTotalCommandBufferBytes;
    }

    OSFastMutex_Lock(&mCommandBufferMutex);

    int size = 0;
    int slot = mCommandBufferSlots[id];
    if (slot >= 0)
    {
        size = mCommandBuffers[slot]->mVerticesCapacity;
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
    return size;
}

void Renderer::CBuffEnd()
{
    Context& context = getContext();

    OSFastMutex_Lock(&mCommandBufferMutex);

    if (context.mDeferredMode)
    {
        context.mDeferredBuffers.push_back(DeferredCBuff(
            context.mRecordingId, context.mRecordingBuffer, context.mRecordingVertexType,
            context.mRecordingPixelShaderType, context.mRecordingPrimitiveType, context.mMatrixStack[3][0]));
        context.mMatrixMode = 0;
        context.mRecordingBuffer->EndRecording(mVertexStrides[context.mRecordingVertexType]);
        context.mRecordingBuffer = NULL;
        OSFastMutex_Unlock(&mCommandBufferMutex);
        return;
    }

    if (mCommandBufferSlots[context.mRecordingId] >= 0)
    {
        DeleteInternalBuffer(mCommandBufferSlots[context.mRecordingId]);
    }

    if (mNumCommandBuffers + mNumRetiredCommandBuffers + 10 > 16000)
    {
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-"
                "LegacyPlatforms\\4JLibraries_Source\\WiiU\\Render\\RendererCBuff.cpp",
                749, "Out of command buffers");
    }

    int slot = mNumCommandBuffers;
    mNumCommandBuffers = slot + 1;
    mCommandBufferIds[slot] = context.mRecordingId;
    mCommandBufferSlots[context.mRecordingId] = (short)slot;
    mCommandBufferVertexTypes[slot] = (unsigned char)context.mRecordingVertexType;
    mCommandBufferPixelShaderTypes[slot] = context.mRecordingPixelShaderType;
    mCommandBufferPrimitiveTypes[slot] = (unsigned char)context.mRecordingPrimitiveType;
    mCommandBuffers[slot] = context.mRecordingBuffer;
    memcpy(&mCommandBufferMatrices[slot * 16], context.mMatrixStack[3][0], 0x40);

    context.mMatrixMode = 0;
    context.mRecordingBuffer->EndRecording(mVertexStrides[context.mRecordingVertexType]);
    context.mRecordingBuffer = NULL;
    OSFastMutex_Unlock(&mCommandBufferMutex);
}

bool Renderer::CBuffCall(int id, bool bApplyMatrix, unsigned char layerMask)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    int slot = mCommandBufferSlots[id];
    if (slot < 0)
    {
        OSFastMutex_Unlock(&mCommandBufferMutex);
        return false;
    }

    Context& context = getContext();
    int vertexType = mCommandBufferVertexTypes[slot];
    unsigned char primitiveType = mCommandBufferPrimitiveTypes[slot];

    if (!bApplyMatrix)
    {
        mCommandBuffers[slot]->Render((C4JRender::eVertexType)vertexType, context, primitiveType, false, layerMask);
        OSFastMutex_Unlock(&mCommandBufferMutex);
        return true;
    }

    if (context.mMatrixDirty[0])
    {
        GX2SetVertexUniformReg(0, 0x10, MatrixGet(0));
        context.mMatrixDirty[0] = false;
    }

    if (context.mMatrixDirty[1])
    {
        GX2SetVertexUniformReg(0x20, 0x10, MatrixGet(1));
        context.mMatrixDirty[1] = false;
    }

    if (context.mMatrixDirty[2])
    {
        GX2SetVertexUniformReg(0x30, 0x10, MatrixGet(2));
        context.mMatrixDirty[2] = false;
    }

    if (context.mNumBoneMatrices > 0)
    {
        GX2SetVertexUniformReg(0xA4, context.mNumBoneMatrices << 4, context.mBoneMatrix);
        context.mNumBoneMatrices = 0;
    }

    UpdateFogState();
    UpdateLightingState();

    if (vertexType == C4JRender::eVertexType_Default && context.mLightingEnable)
    {
        vertexType = C4JRender::eVertexType_DefaultLighting;
    }
    else if (vertexType == C4JRender::eVertexType_DefaultLighting && !context.mLightingEnable)
    {
        vertexType = C4JRender::eVertexType_Default;
    }
    else if (vertexType == C4JRender::eVertexType_Entity && context.mLightingEnable)
    {
        vertexType = C4JRender::eVertexType_EntityLighting;
    }
    else if (vertexType == C4JRender::eVertexType_EntityLighting && !context.mLightingEnable)
    {
        vertexType = C4JRender::eVertexType_Entity;
    }

    UpdateViewportState();
    UpdateTexGenState();

    if (vertexType != mCurrentVertexShader)
    {
        GX2SetFetchShader(mFetchShaders[vertexType]);
        GX2SetVertexShader(mVertexShaders[vertexType]);
        mCurrentVertexShader = vertexType;
    }

    C4JRender::ePixelShaderType pixelShaderType = (C4JRender::ePixelShaderType)mCommandBufferPixelShaderTypes[slot];

    if (context.mForceLOD >= 0)
    {
        float forceLOD[4];
        forceLOD[0] = (float)context.mForceLOD;
        forceLOD[1] = 0.0f;
        forceLOD[2] = 0.0f;
        forceLOD[3] = 0.0f;
        pixelShaderType = C4JRender::ePixelShaderType_TerrainForceLod;
        GX2SetPixelUniformReg(0x20, 4, forceLOD);
    }

    C4JRender::ePixelShaderType finalPixelShader = FinalisePixelShaderType(pixelShaderType);

    if (finalPixelShader != mCurrentPixelShader || mPixelUniformsDirty)
    {
        GX2SetPixelUniformReg(0, 0x20, mColour);
        mPixelUniformsDirty = false;
    }

    if (finalPixelShader == C4JRender::ePixelShaderType_LayeredColourMask
        && (mCurrentPixelShader != C4JRender::ePixelShaderType_LayeredColourMask || mLayeredParamsDirty))
    {
        GX2SetPixelUniformReg(0x24, 0x80, mLayeredTextureSize);
        mLayeredParamsDirty = false;
    }

    if (finalPixelShader != mCurrentPixelShader)
    {
        GX2SetPixelShader(mPixelShaders[finalPixelShader]);
        mCurrentPixelShader = finalPixelShader;
    }

    mCommandBuffers[slot]->Render((C4JRender::eVertexType)vertexType, context, primitiveType, bApplyMatrix, layerMask);

    MultWithStack((float (*)[4]) & mCommandBufferMatrices[slot * 16]);
    ASM_MTX44Identity(context.mMatrixStack[3][0]);
    context.mMatrixDirty[3] = true;

    OSFastMutex_Unlock(&mCommandBufferMutex);
    return true;
}

void Renderer::CBuffTick()
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    int remaining = mNumRetiredCommandBuffers;
    int examined = 0;
    int slot = 15999;

    while (examined < remaining)
    {
        CommandBuffer* buffer = mCommandBuffers[slot];

        if (buffer->IsBusy())
        {
            ++examined;
            --slot;
            continue;
        }

        delete buffer;
        remaining = mNumRetiredCommandBuffers - 1;
        mNumRetiredCommandBuffers = remaining;
        mCommandBuffers[slot] = NULL;

        if (remaining == 0)
        {
            ++examined;
            --slot;
            break;
        }

        mCommandBuffers[slot] = mCommandBuffers[15999 - remaining];
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
}

void Renderer::CBuffDeferredModeStart()
{
    Context& context = getContext();
    context.mDeferredMode = true;
}

void Renderer::CBuffDeferredModeEnd()
{
    Context& context = getContext();

    if (!context.mDeferredMode)
    {
        return;
    }

    OSFastMutex_Lock(&mCommandBufferMutex);
    context.mDeferredMode = false;

    for (unsigned int index = 0; index < context.mDeferredBuffers.size(); ++index)
    {
        const DeferredCBuff& deferred = context.mDeferredBuffers[index];

        if (mCommandBufferSlots[deferred.mId] >= 0)
        {
            DeleteInternalBuffer(mCommandBufferSlots[deferred.mId]);
        }

        if (mNumCommandBuffers + mNumRetiredCommandBuffers + 10 > 16000)
        {
            OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-"
                    "LegacyPlatforms\\4JLibraries_Source\\WiiU\\Render\\RendererCBuff.cpp",
                    709, "Out of command buffers");
        }

        int slot = mNumCommandBuffers;
        mNumCommandBuffers = slot + 1;
        mCommandBufferIds[slot] = deferred.mId;
        mCommandBufferSlots[deferred.mId] = (short)slot;
        mCommandBufferVertexTypes[slot] = (unsigned char)deferred.mVertexType;
        mCommandBufferPixelShaderTypes[slot] = deferred.mPixelShaderType;
        mCommandBufferPrimitiveTypes[slot] = (unsigned char)deferred.mPrimitiveType;
        mCommandBuffers[slot] = deferred.mBuffer;
        memcpy(&mCommandBufferMatrices[slot * 16], deferred.mMatrix, 0x40);
    }

    context.mDeferredBuffers.clear();
    OSFastMutex_Unlock(&mCommandBufferMutex);
}

Renderer::Context::Context(Renderer* renderer)
{
    mRenderer = renderer;
    ASM_MTXIdentity(mMatrixStack[0][0]);
    mMatrixStackDepth[0] = 0;
    ASM_MTXIdentity(mMatrixStack[1][0]);
    mMatrixStackDepth[1] = 0;
    ASM_MTXIdentity(mMatrixStack[2][0]);
    mMatrixStackDepth[2] = 0;
    ASM_MTXIdentity(mMatrixStack[3][0]);
    mMatrixStackDepth[3] = 0;

    mTextureIsVertex[0] = false;
    mBoundTexture[3] = 0;
    mAlphaTestEnable = false;
    mAlphaTestRef = 1.0f;
    mTextureIsMask[2] = false;
    mTextureIsMask[3] = false;
    mTextureIsMask[1] = false;
    mTextureIsMask[0] = false;
    mTextureIsVertex[2] = false;
    mTextureIsVertex[3] = false;
    mTextureIsVertex[1] = false;
    mMatrixMode = 0;
    mBoundTexture[0] = 0;
    mFaceCull = true;
    mDeferredMode = false;
    mDepthTestEnable = true;
    mBoundTexture[1] = 0;
    mBoundTexture[2] = 0;
    mDepthFunc = GX2_COMPARE_ALWAYS;
    mStencilFunc = GX2_COMPARE_ALWAYS;
    mFogEnable = false;
    mRecordingBuffer = NULL;
    mLightingEnable = false;
    mDepthMask = true;
    mForceLOD = -1;

    memset(mLightDirection, 0, 80);

    mLightingDirty = false;
    mDynamicVertexBuffer[0] = renderer->MemoryAllocMEM2(0x200000, 0x20);
    mDynamicVertexBuffer[1] = renderer->MemoryAllocMEM2(0x200000, 0x20);
    mDynamicVertexBuffer[2] = renderer->MemoryAllocMEM2(0x200000, 0x20);

    mViewportScale[1] = 1.0f;
    mVertexAlpha = 0.0f;
    mViewportScale[2] = 0.0f;
    mVertexAlphaDistance = 0.0f;
    mViewportScale[3] = 0.0f;
    mViewportScale[0] = 1.0f;
    mOrderIndependentAlpha = false;
    mDynamicVertexOffset = 0;
    mDynamicVertexBufferIndex = 0;
}

Renderer::DeferredCBuff::DeferredCBuff(int id, CommandBuffer* buffer, int vertexType,
                                       C4JRender::ePixelShaderType pixelShaderType, int primitiveType,
                                       float (*matrix)[4])
{
    mId = id;
    mBuffer = buffer;
    mVertexType = vertexType;
    mPixelShaderType = pixelShaderType;
    mPrimitiveType = primitiveType;
    memcpy(mMatrix, matrix, 0x40);
}

Renderer::CommandBuffer::CommandBuffer(bool bRecordState)
{
    mRecordState = bRecordState;
    mVerticesUsed = 0;
    mVertices = (unsigned char*)malloc(0x1000);
    mVerticesCapacity = 0x1000;

    OSFastMutex_Lock(&sTotalAllocMutex);
    sTotalCommandBufferBytes += mVerticesCapacity;
    OSFastMutex_Unlock(&sTotalAllocMutex);

    mLastRenderedFrame = 0;
}

Renderer::CommandBuffer::~CommandBuffer()
{
    free(mVertices);

    OSFastMutex_Lock(&sTotalAllocMutex);
    sTotalCommandBufferBytes -= mVerticesCapacity;
    OSFastMutex_Unlock(&sTotalAllocMutex);
}

void Renderer::CommandBuffer::StartRecording()
{
}

void Renderer::CommandBuffer::EndRecording(unsigned int vertexStride)
{
    if (mVerticesUsed != 0)
    {
        GX2Invalidate(GX2_INVALIDATE_CPU, mVertices, mVerticesUsed);
        return;
    }

    free(mVertices);
    mVertices = NULL;
}

void Renderer::CommandBuffer::Render(C4JRender::eVertexType vertexType, Context& context, int primitiveType,
                                     bool bApplyMatrix, unsigned char layerMask)
{
    if (mVertices == NULL)
    {
        return;
    }

    unsigned int batchOffset[32];
    int batchCount[32];
    unsigned int numBatches = 0;
    bool bMatrixPushed = false;
    int shaderVertexType = vertexType;

    for (unsigned int index = 0; index < mCommands.size(); ++index)
    {
        const Command& command = mCommands[index];

        if (command.mType == eCommandType_Matrix)
        {
            if (shaderVertexType == C4JRender::eVertexType_Terrain)
            {
                if (!bApplyMatrix && command.mMatrix[13] == context.mLastQuadUniform[2]
                    && command.mMatrix[14] == context.mLastQuadUniform[3])
                {
                    continue;
                }

                GX2SetVertexUniformReg(0x48, 4, &command.mMatrix[12]);
                context.mLastQuadUniform[2] = command.mMatrix[13];
                context.mLastQuadUniform[3] = command.mMatrix[14];
            }
            else
            {
                GX2SetVertexUniformReg(0x10, 0x10, command.mMatrix);
                bMatrixPushed = true;
            }
        }
        else if (command.mType == eCommandType_Vertices)
        {
            if (mRecordState)
            {
                gRendererInstance->UpdateLightingState();

                if (shaderVertexType == C4JRender::eVertexType_Default && context.mLightingEnable)
                {
                    shaderVertexType = C4JRender::eVertexType_DefaultLighting;
                }
                else if (shaderVertexType == C4JRender::eVertexType_DefaultLighting && !context.mLightingEnable)
                {
                    shaderVertexType = C4JRender::eVertexType_Default;
                }
                else if (shaderVertexType == C4JRender::eVertexType_Entity && context.mLightingEnable)
                {
                    shaderVertexType = C4JRender::eVertexType_EntityLighting;
                }
                else if (shaderVertexType == C4JRender::eVertexType_EntityLighting && !context.mLightingEnable)
                {
                    shaderVertexType = C4JRender::eVertexType_Entity;
                }

                if (shaderVertexType != gRendererInstance->mCurrentVertexShader)
                {
                    GX2SetFetchShader(gRendererInstance->mFetchShaders[shaderVertexType]);
                    GX2SetVertexShader(gRendererInstance->mVertexShaders[shaderVertexType]);
                    gRendererInstance->mCurrentVertexShader = shaderVertexType;
                }
            }

            if (shaderVertexType == C4JRender::eVertexType_Terrain)
            {
                if (numBatches != 0
                    && command.mVertices.mOffset
                           == batchOffset[numBatches - 1]
                                  + batchCount[numBatches - 1]
                                        * gRendererInstance->mVertexStrides[C4JRender::eVertexType_Terrain])
                {
                    batchCount[numBatches - 1] += command.mVertices.mCount;
                }
                else
                {
                    batchOffset[numBatches] = command.mVertices.mOffset;
                    batchCount[numBatches] = command.mVertices.mCount;
                    ++numBatches;
                }
            }
            else
            {
                GX2SetAttribBuffer(0, mVerticesUsed, gRendererInstance->mVertexStrides[shaderVertexType], mVertices);
                GX2DrawEx(sPrimitiveModes[primitiveType], command.mVertices.mCount,
                          command.mVertices.mOffset / gRendererInstance->mVertexStrides[shaderVertexType], 1);
                gRendererInstance->mCommandBufferVertsDrawn += command.mVertices.mCount;
            }
        }
    }

    GX2SetAttribBuffer(0, mVerticesUsed, gRendererInstance->mVertexStrides[shaderVertexType], mVertices);

    for (unsigned int batch = 0; batch < numBatches; ++batch)
    {
        GX2DrawEx(sPrimitiveModes[primitiveType], batchCount[batch],
                  batchOffset[batch] / gRendererInstance->mVertexStrides[shaderVertexType], 1);
        gRendererInstance->mCommandBufferVertsDrawn += batchCount[batch];
    }

    if (bMatrixPushed)
    {
        float identity[4][4];
        ASM_MTX44Identity(identity);
        GX2SetVertexUniformReg(0x10, 0x10, identity);
    }

    mLastRenderedFrame = gRendererInstance->mFrameCount;
}

bool Renderer::CommandBuffer::IsBusy()
{
    return mLastRenderedFrame >= gRendererInstance->mFrameCount - 3;
}

void Renderer::CommandBuffer::AddMatrix(const float* matrix)
{
    Command command;
    command.mType = eCommandType_Matrix;
    memcpy(command.mMatrix, matrix, 0x40);
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::AddVertices(unsigned int vertexStride, unsigned int numVertices, void* vertices,
                                          Context& context, unsigned char* layers)
{
    if (context.mMatrixDirty[3])
    {
        AddMatrix(gRendererInstance->MatrixGet(3));
        context.mMatrixDirty[3] = false;
    }

    Command command;
    unsigned int offset = mVerticesUsed;
    command.mType = eCommandType_Vertices;
    mVerticesUsed = offset + numVertices * vertexStride;
    command.mVertices.mOffset = offset;
    command.mVertices.mCount = numVertices;

    if (mVerticesCapacity < mVerticesUsed)
    {
        OSFastMutex_Lock(&sTotalAllocMutex);
        sTotalCommandBufferBytes -= mVerticesCapacity;
        OSFastMutex_Unlock(&sTotalAllocMutex);

        mVerticesCapacity = (mVerticesUsed + 0xFFF) & 0xFFFFF000;
        mVertices = (unsigned char*)realloc(mVertices, mVerticesCapacity);

        OSFastMutex_Lock(&sTotalAllocMutex);
        sTotalCommandBufferBytes += mVerticesCapacity;
        OSFastMutex_Unlock(&sTotalAllocMutex);
    }

    if (layers == NULL)
    {
        memcpy(mVertices + offset, vertices, numVertices * vertexStride);
        command.mVertices.mLayer = 0;
        mCommands.push_back(command);
        return;
    }

    unsigned int quadSize = vertexStride * 4;
    unsigned int destination = offset;

    for (unsigned char layer = 0; layer < 7; ++layer)
    {
        unsigned int count = 0;

        for (unsigned int quad = 0; quad < numVertices / 4; ++quad)
        {
            if (layers[quad] == layer)
            {
                memcpy(mVertices + destination, (unsigned char*)vertices + quad * quadSize, quadSize);
                count += 4;
                destination += quadSize;
            }
        }

        if (count > 0)
        {
            command.mVertices.mLayer = layer;
            command.mVertices.mOffset = offset;
            command.mVertices.mCount = count;
            mCommands.push_back(command);
        }

        offset = destination;
    }
}

void Renderer::CommandBuffer::BindTexture(int texture)
{
    Command command;
    command.mType = eCommandType_BindTexture;
    command.mTexture = texture;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetColor(float red, float green, float blue, float alpha)
{
    Command command;
    command.mColour[1] = green;
    command.mColour[2] = blue;
    command.mType = eCommandType_Colour;
    command.mColour[0] = red;
    command.mColour[3] = alpha;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetDepthFunc(int depthFunc)
{
    Command command;
    command.mType = eCommandType_DepthFunc;
    command.mDepthFunc = depthFunc;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetDepthMask(bool bDepthMask)
{
    Command command;
    command.mType = eCommandType_DepthMask;
    command.mEnable = bDepthMask;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetDepthTestEnable(bool bDepthTest)
{
    Command command;
    command.mType = eCommandType_DepthTestEnable;
    command.mEnable = bDepthTest;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetLightingEnable(bool bLighting)
{
    Command command;
    command.mType = eCommandType_LightingEnable;
    command.mEnable = bLighting;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetLightEnable(int light, bool bEnable)
{
    Command command;
    command.mType = eCommandType_LightEnable;
    command.mLightEnable.mIndex = light;
    command.mLightEnable.mEnable = bEnable;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetLightDirection(int light, float x, float y, float z)
{
    Command command;
    command.mType = eCommandType_LightDirection;
    command.mLight.mIndex = light;

    Context& context = gRendererInstance->getContext();

    Vec direction;
    float transposed[4][4];
    direction.y = y;
    direction.z = z;
    direction.x = x;
    ASM_MTX44Transpose(context.mMatrixStack[3][context.mMatrixStackDepth[3]], transposed);
    ASM_MTX44MultVecSR(transposed, &direction, &direction);
    ASM_VECNormalize(&direction, &direction);

    command.mLight.mValue[1] = direction.y;
    command.mLight.mValue[0] = direction.x;
    command.mLight.mValue[2] = direction.z;
    command.mLight.mValue[3] = 0.0f;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetLightColour(int light, float red, float green, float blue)
{
    Command command;
    command.mLight.mValue[0] = red;
    command.mLight.mValue[1] = green;
    command.mType = eCommandType_LightColour;
    command.mLight.mValue[2] = blue;
    command.mLight.mIndex = light;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetLightAmbientColour(float red, float green, float blue)
{
    Command command;
    command.mLight.mValue[0] = red;
    command.mType = eCommandType_LightAmbientColour;
    command.mLight.mValue[1] = green;
    command.mLight.mValue[2] = blue;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetBlendEnable(bool bBlend)
{
    Command command;
    command.mType = eCommandType_BlendEnable;
    command.mEnable = bBlend;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetBlendFunc(int source, int destination)
{
    Command command;
    command.mType = eCommandType_BlendFunc;
    command.mBlendFunc.mSource = source;
    command.mBlendFunc.mDestination = destination;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetBlendFactor(unsigned int factor)
{
    Command command;
    command.mType = eCommandType_BlendFactor;
    command.mBlendFactor = factor;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetFaceCull(bool bFaceCull)
{
    Command command;
    command.mType = eCommandType_FaceCull;
    command.mEnable = bFaceCull;
    mCommands.push_back(command);
}

int Renderer::CommandBuffer::getNumVertices()
{
    if (mVertices == NULL)
    {
        return 0;
    }

    int numVertices = 0;

    for (unsigned int index = 0; index < mCommands.size(); ++index)
    {
        if (mCommands[index].mType == eCommandType_Vertices)
        {
            numVertices += mCommands[index].mVertices.mCount;
        }
    }

    return numVertices;
}
