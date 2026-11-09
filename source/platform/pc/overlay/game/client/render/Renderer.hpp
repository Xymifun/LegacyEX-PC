#ifndef GAME_CLIENT_RENDER_RENDERER_HPP
#define GAME_CLIENT_RENDER_RENDERER_HPP

#include <vector>

#include "game/client/render/C4JRender.hpp"
#include "game/core/CriticalSection.hpp"

#include <cafe/gx2/gx2Types.h>
#include <nn/ffl/FFLDataSource.h>
#include <nn/ffl/FFLStandard.h>

struct ImageFileBuffer
{
    int mFormat;
    void* mData;
    int mSize;
};

struct D3DXIMAGE_INFO;
struct _XSOCIAL_PREVIEWIMAGE;

class Renderer : public C4JRender
{
public:
    static const int kMaxTextures = 4096;
    static const int kMaxCommandBufferIds = 0x1400000;
    static const int kMatrixModes = 4;
    static const int kMatrixStackDepth = 16;
    static const int kTextureStages = 4;
    static const int kMaxBones = 48;

    struct CommandBuffer;

    struct Context
    {
        Context();
        float mMatrixStack[kMatrixModes][kMatrixStackDepth][4][4];
        int mMatrixStackDepth[kMatrixModes];
        bool mMatrixDirty[kMatrixModes];
        int mMatrixMode;
        int mBoundTexture[kTextureStages];
        int mPixelTexture[kTextureStages];
        bool mTextureIsMask[kTextureStages];
        unsigned int mScissorRect[4];
        CommandBuffer* mRecordingBuffer;
        int mRecordingId;
        int mRecordingVertexType;
        int mRecordingPixelShaderType;
        int mRecordingPrimitiveType;
        bool mDeferredMode;
        std::vector<CommandBuffer*> mDeferredBuffers;
        float mBoneMatrix[kMaxBones][4][4];
        int mNumBoneMatrices;
    };

    Renderer();
    virtual void Tick();
    virtual void UpdateGamma(unsigned short gamma);
    virtual void MatrixMode(int mode);
    virtual void MatrixSetIdentity();
    virtual void MatrixTranslate(float x, float y, float z);
    virtual void MatrixRotate(float angle, float x, float y, float z);
    virtual void MatrixScale(float x, float y, float z);
    virtual void MatrixPerspective(float fieldOfView, float aspect, float nearDistance, float farDistance);
    virtual void MatrixOrthogonal(float left, float right, float bottom, float top, float nearDistance,
                                  float farDistance);
    virtual void MatrixPop();
    virtual void MatrixPush();
    virtual void MatrixMult(float* matrix);
    virtual float* MatrixGet(int mode);
    virtual void Set_matrixDirty();
    virtual void Initialise();
    virtual void InitialiseContext();
    virtual GX2ColorBuffer* GetColorBuffer();
    virtual GX2DepthBuffer* GetDepthStencilBuffer();
    virtual GX2ColorBuffer* GetColorBufferDRC();
    virtual GX2DepthBuffer* GetDepthStencilBufferDRC();
    virtual void StartFrame();
    virtual void TEMP_PatchStateFromIggy();
    virtual void ResetRenderState();
    virtual void StartDRCFrame(bool bCopyFrame);
    virtual void DoScreenGrabOnNextPresent();
    virtual void Present();
    virtual void Clear(int flags);
    virtual void SetClearColour(const float* colour);
    virtual bool IsWidescreen();
    virtual bool IsHiDef();
    virtual void CaptureThumbnail(ImageFileBuffer* buffer);
    virtual void CaptureScreen(ImageFileBuffer* buffer, _XSOCIAL_PREVIEWIMAGE* previewImage);
    virtual void BeginConditionalSurvey(int survey);
    virtual void EndConditionalSurvey();
    virtual void BeginConditionalRendering(int survey);
    virtual void EndConditionalRendering();
    virtual void BeginOrderIndependentAlpha();
    virtual void EndOrderIndependentAlpha();
    virtual void ReleaseForeground();
    virtual void AcquiredForeground();
    virtual void WiiUWaitDrawDone();
    virtual int GetFramesSinceBoot();
    virtual void DrawVertices(C4JRender::ePrimitiveType primitiveType, int numVertices, void* vertices,
                              C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType,
                              unsigned char* layers);
    virtual void CBuffLockStaticCreations();
    virtual int CBuffCreate(int count);
    virtual void CBuffDelete(int firstId, int count);
    virtual void CBuffStart(int id, bool bApplyLighting);
    virtual void CBuffClear(int id);
    virtual int CBuffSize(int id);
    virtual void CBuffEnd();
    virtual bool CBuffCall(int id, bool bApplyMatrix, unsigned char layerMask);
    void CBuffCallRun(const int* ids, int count);
    virtual void CBuffTick();
    virtual void CBuffDeferredModeStart();
    virtual void CBuffDeferredModeEnd();
    virtual int GetMaxTextures();
    virtual int TextureCreate();
    virtual void TextureFree(int texture);
    virtual void TextureBind(int texture);
    virtual void TextureBind(int stage, int texture);
    virtual void TextureBindMask(int stage);
    virtual void TextureBindVertex(int texture);
    virtual void TextureSetTextureLevels(int levels);
    virtual int TextureGetTextureLevels();
    virtual void TextureData(int width, int height, void* data, int level, C4JRender::eTextureFormat format);
    virtual void TextureDataUpdate(int x, int y, int width, int height, void* data, int level);
    virtual void TextureSetParam(int parameter, int value);
    virtual void TextureDynamicUpdateStart();
    virtual void TextureDynamicUpdateEnd();
    virtual int LoadTextureData(const char* fileName, D3DXIMAGE_INFO* info, int** data);
    virtual int LoadTextureData(unsigned char* fileData, unsigned int fileSize, D3DXIMAGE_INFO* info, int** data);
    virtual int SaveTextureData(const char* fileName, D3DXIMAGE_INFO* info, int* data);
    virtual int SaveTextureDataToMemory(void* destination, int destinationSize, int* bytesWritten, int width,
                                        int height, int* data);
    virtual void TextureGetStats();
    virtual GX2Texture* TextureGetTexture(int texture);
    virtual void StateSetColour(float red, float green, float blue, float alpha);
    virtual void StateSetColourAdd(float red, float green, float blue, float alpha);
    virtual void StateSetTexColour(int stage, float red, float green, float blue, float alpha);
    virtual void StateSetDepthMask(bool bDepthMask);
    virtual void StateSetBlendEnable(bool bBlend);
    virtual void StateSetBlendFunc(int source, int destination);
    virtual void StateSetBlendFactor(unsigned int factor);
    virtual void StateSetAlphaFunc(int alphaFunc, float reference);
    virtual void StateSetDepthFunc(int depthFunc);
    virtual void StateSetFaceCull(bool bFaceCull);
    virtual void StateSetFaceCullCW(bool bClockwise);
    virtual void StateSetLineWidth(float width);
    virtual void StateSetWriteEnable(bool bRed, bool bGreen, bool bBlue, bool bAlpha);
    virtual void StateSetDepthTestEnable(bool bDepthTest);
    virtual void StateSetAlphaTestEnable(bool bAlphaTest);
    virtual void StateSetDepthSlopeAndBias(float slope, float bias);
    virtual void StateSetFogEnable(bool bFog);
    virtual void StateSetFogMode(int mode);
    virtual void StateSetFogNearDistance(float nearDistance);
    virtual void StateSetFogFarDistance(float farDistance);
    virtual void StateSetFogDensity(float density);
    virtual void StateSetFogColour(float red, float green, float blue);
    virtual void StateSetLightingEnable(bool bLighting);
    virtual void StateSetVertexTextureUV(float u, float v);
    virtual void StateSetTextureScaleUI(float scaleU, float scaleV, float offsetU, float offsetV);
    virtual void StateSetScalingGridUI(float left, float right, float top, float bottom, float innerLeft,
                                       float innerRight, float innerTop, float innerBottom, float scaleU,
                                       float scaleV);
    virtual void StateSetLightColour(int light, float red, float green, float blue);
    virtual void StateSetLightAmbientColour(float red, float green, float blue);
    virtual void StateSetLightDirection(int light, float x, float y, float z);
    virtual void StateSetLightEnable(int light, bool bEnable);
    virtual void StateSetViewport(C4JRender::eViewportType viewport);
    virtual void StateGetScissorRect(unsigned int& left, unsigned int& top, unsigned int& right,
                                     unsigned int& bottom);
    virtual void StateSetScissorRect(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom);
    virtual void StateSetEnableViewportClipPlanes(bool bEnable);
    virtual void StateSetTexGenCol(int coordinate, float x, float y, float z, float w, bool bEyeLinear);
    virtual void StateSetStencil(int stencilFunc, unsigned char reference, unsigned char compareMask,
                                 unsigned char writeMask);
    virtual void StateSetForceLOD(int lod);
    virtual void StateSetLerpColour(float red, float green, float blue, float alpha);
    virtual void SetLayeredColour(int layer, float red, float green, float blue);
    virtual void SetLayeredParam(int layer, int slot, float offset, float scale);
    virtual void SetLayeredTextureSize(int width, int height);
    virtual void StateSetBoneMatrixData(int bone);
    virtual void StateSetBoneMatrixNull(int bone);
    virtual void StateSetVertexAlphaDistance(float distance);
    virtual void StateSetVertexAlpha(float alpha);
    virtual int GetNumCommandBufferVertsDrawnLastFrame();
    virtual int GetNumVertsInCommandBuffer(int id);
    virtual void BeginEvent(const wchar_t* name);
    virtual void EndEvent();
    virtual void* MemoryAllocMEM1(unsigned int size, unsigned int alignment, bool bReserved);
    virtual void MemoryFreeMEM1(void* memory);
    virtual void* MemoryAllocMEM2(unsigned int size, unsigned int alignment);
    virtual void MemoryFreeMEM2(void* memory);
    virtual void* MemoryAllocBucket(unsigned int size, unsigned int alignment);
    virtual void MemoryFreeBucket(void* memory);

    Context& getContext();

    bool HasWindow();
    void PumpWindow();
    void SetPresentSize(int width, int height);
    void RequestScreenshot(const char* path);
    void PcCheckLeftovers(const char* where);

private:
    struct Texture
    {
        bool mUsed;
        unsigned int mName;
        int mWidth;
        int mHeight;
        int mLevels;
        unsigned int mFlags;
        bool mStorageDirty;
        bool mParametersDirty;
        std::vector<std::vector<unsigned char> > mPendingLevels;
    };

    void MultWithStack(float (*matrix)[4]);
    bool OnRenderThread();
    void CreateRenderTargets();
    void BindRenderTarget(int screen);
    void ResizeTvTarget(int width, int height);
    void DrawMenuCursor(int windowWidth, int windowHeight, int frameWidth);
    unsigned int GetProgram(int vertexShader, int pixelShader);
    int FinalisePixelShaderType(int pixelShaderType);
    void PrepareTexture(int unit, int texture);
    void BindTextures();
    void ApplyRenderState();
    void ApplyUniforms(int vertexShader, int pixelShader, unsigned int program);
    void SetVertexRegisters(int first, int count, const float* values);
    void SetPixelRegisters(int first, int count, const float* values);
    void UploadRegisters();
    void SetupVertexFormat(int vertexShader, unsigned int offset);
    void IssueDraw(int primitiveType, int firstVertex, int numVertices);
    void UpdateFogRegisters();
    void UpdateLightingRegisters();
    void RenderCommandBuffer(CommandBuffer* buffer, int vertexShader, bool bApplyMatrix);
    void DeleteCommandBuffer(CommandBuffer* buffer);
    void ReleasePendingObjects();
    void WriteScreenshot();

    OSFastMutex mCommandBufferMutex;
    OSFastMutex mTextureMutex;
    std::vector<signed char> mCommandBufferState;
    std::vector<CommandBuffer*> mCommandBuffers;
    int mNextCommandBufferId;
    bool mAllowStaticCommandBufferIds;
    int mFrameCount;
    int mVerticesDrawnThisFrame;
    int mVerticesDrawnLastFrame;
    void* mReservedMEM1;
    Texture mTextures[kMaxTextures];
    int mDefaultTexture;
    bool mDefaultTextureLocked;
    int mTexturesInUse;
    int mTexturesLogged;
    int mVertexTexture;
    std::vector<unsigned int> mPendingBufferDeletes;
    std::vector<unsigned int> mPendingVertexArrayDeletes;
    std::vector<unsigned int> mPendingTextureDeletes;
};
extern Renderer* gRenderer;
extern Renderer* gRendererInstance;

#endif
