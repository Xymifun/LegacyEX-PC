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
struct MEMHeapHeader;

class Renderer : public C4JRender
{
public:
    struct _Vec4
    {
        float x;
        float y;
        float z;
        float w;
    };

    class CommandBuffer;
    class Context;

    struct DeferredCBuff
    {
        DeferredCBuff(int id, CommandBuffer* buffer, int vertexType, C4JRender::ePixelShaderType pixelShaderType,
                      int primitiveType, float (*matrix)[4]);

        CommandBuffer* mBuffer;
        int mId;
        int mVertexType;
        C4JRender::ePixelShaderType mPixelShaderType;
        int mPrimitiveType;
        float mMatrix[4][4];
    };

    class CommandBuffer
    {
    public:
        enum eCommandType
        {
            eCommandType_Matrix,
            eCommandType_Vertices,
            eCommandType_BindTexture,
            eCommandType_Colour,
            eCommandType_DepthFunc,
            eCommandType_DepthMask,
            eCommandType_DepthTestEnable,
            eCommandType_LightingEnable,
            eCommandType_LightEnable,
            eCommandType_LightDirection,
            eCommandType_LightColour,
            eCommandType_LightAmbientColour,
            eCommandType_BlendEnable,
            eCommandType_BlendFunc,
            eCommandType_BlendFactor,
            eCommandType_FaceCull
        };

        struct Command
        {
            eCommandType mType;

            union
            {
                float mMatrix[16];
                float mColour[4];
                int mTexture;
                int mDepthFunc;
                bool mEnable;
                unsigned int mBlendFactor;

                struct
                {
                    unsigned int mOffset;
                    unsigned int mCount;
                    unsigned char mLayer;
                } mVertices;

                struct
                {
                    int mIndex;
                    bool mEnable;
                } mLightEnable;

                struct
                {
                    int mIndex;
                    float mValue[4];
                } mLight;

                struct
                {
                    int mSource;
                    int mDestination;
                } mBlendFunc;
            };
        };

        friend class Renderer;

        CommandBuffer(bool bRecordState);
        ~CommandBuffer();

        void StartRecording();
        void EndRecording(unsigned int vertexStride);
        void Render(C4JRender::eVertexType vertexType, Context& context, int primitiveType, bool bApplyMatrix,
                    unsigned char layerMask);
        bool IsBusy();

        void AddMatrix(const float* matrix);
        void AddVertices(unsigned int vertexStride, unsigned int numVertices, void* vertices, Context& context,
                         unsigned char* layers);
        void BindTexture(int texture);
        void SetColor(float red, float green, float blue, float alpha);
        void SetDepthFunc(int depthFunc);
        void SetDepthMask(bool bDepthMask);
        void SetDepthTestEnable(bool bDepthTest);
        void SetLightingEnable(bool bLighting);
        void SetLightEnable(int light, bool bEnable);
        void SetLightDirection(int light, float x, float y, float z);
        void SetLightColour(int light, float red, float green, float blue);
        void SetLightAmbientColour(float red, float green, float blue);
        void SetBlendEnable(bool bBlend);
        void SetBlendFunc(int source, int destination);
        void SetBlendFactor(unsigned int factor);
        void SetFaceCull(bool bFaceCull);

        int getNumVertices();

    private:
        unsigned char* mVertices;
        unsigned int mVerticesUsed;
        std::vector<Command> mCommands;
        unsigned int mVerticesCapacity;
        int mLastRenderedFrame;
        bool mRecordState;
    };

    class Context
    {
    public:
        Context(Renderer* renderer);

        Renderer* mRenderer;
        float mMatrixStack[4][16][4][4];
        bool mMatrixDirty[4];
        int mMatrixStackDepth[4];
        int mMatrixMode;
        int mBoundTexture[4];
        bool mTextureIsMask[4];
        bool mTextureIsVertex[4];
        bool mFaceCull;
        bool mAlphaTestEnable;
        float mAlphaTestRef;
        bool mDepthMask;
        bool mDepthTestEnable;
        GX2CompareFunction mStencilFunc;
        GX2CompareFunction mDepthFunc;
        GX2BlendFunction mBlendSource;
        GX2BlendFunction mBlendDestination;
        bool mBlendEnable;
        bool mFogEnable;
        float mFogNearDistance;
        float mFogFarDistance;
        float mFogDensity;
        int mFogMode;
        bool mLightingEnable;
        bool mLightEnable[2];
        bool mLightingDirty;
        int mForceLOD;
        bool mOrderIndependentAlpha;
        float mVertexAlphaDistance;
        float mVertexAlpha;
        float mLightDirection[2][4];
        float mLightColour[2][4];
        float mLightAmbientColour[4];
        float mBoneMatrix[48][4][4];
        int mNumBoneMatrices;
        void* mDynamicVertexBuffer[3];
        unsigned int mDynamicVertexOffset;
        int mDynamicVertexBufferIndex;
        float mTexGenMatrix[2][4][4];
        CommandBuffer* mRecordingBuffer;
        int mRecordingId;
        C4JRender::eVertexType mRecordingVertexType;
        C4JRender::ePixelShaderType mRecordingPixelShaderType;
        C4JRender::ePrimitiveType mRecordingPrimitiveType;
        bool mDeferredMode;
        std::vector<DeferredCBuff> mDeferredBuffers;
        int mDeferredVertexCount;
        int mDeferredCommandCount;
        float mLastQuadUniform[4];
        float mViewportScale[4];
        float mLayeredColour[16][4];
        unsigned int mScissorRect[4];
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
                                       float innerRight, float innerTop, float innerBottom, float scaleU, float scaleV);
    virtual void StateSetLightColour(int light, float red, float green, float blue);
    virtual void StateSetLightAmbientColour(float red, float green, float blue);
    virtual void StateSetLightDirection(int light, float x, float y, float z);
    virtual void StateSetLightEnable(int light, bool bEnable);
    virtual void StateSetViewport(C4JRender::eViewportType viewport);
    virtual void StateGetScissorRect(unsigned int& left, unsigned int& top, unsigned int& right, unsigned int& bottom);
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

    void InitialiseContext(bool bReset);
    void SetupShaders();
    void Initialise(bool bFirstTime);
    Context& getContext();
    void StartFrameInternal(int screen, bool bAlphaTest);
    void GammaCorrect();
    void CopyFrameToDRC();
    void RenderOnScreenKeyboard(bool bDRC);
    void ConvertLinearToPng(ImageFileBuffer* buffer, unsigned char* pixels, unsigned int width, unsigned int height);
    void MultWithStack(float (*matrix)[4]);
    void MemoryInit();
    void MemoryRelease();
    void Mtx44Vec4Transform(float (*matrix)[4], _Vec4* source, _Vec4* destination);
    void UpdateFogState();
    void UpdateLightingState();
    void UpdateViewportState();
    void UpdateTexGenState();
    void StateUpdate();
    void UpdateTextureState(int stage);
    C4JRender::ePixelShaderType FinalisePixelShaderType(C4JRender::ePixelShaderType pixelShaderType);
    void TextureSetTexture(int texture, GX2Texture& source);
    void TextureSetSampler(int texture, GX2Sampler& source);
    void* GetDynamicVertexMemory(C4JRender::eVertexType vertexType, int numVertices);
    void DrawVertexSetup(C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType,
                         C4JRender::ePrimitiveType primitiveType);
    bool InitFFL();
    void ShutdownFFL();
    void* CreateMiiIconTexture(FFLDataSource dataSource, FFLStoreData* storeData, int index, int* texture);
    void FreeMiiIconTexture(int texture, void* memory);
    void DeleteInternalBuffer(int slot);

private:
    friend class CommandBuffer;

    struct Texture
    {
        bool mUsed;
        GX2Texture mTexture;
        int mReserved;
        int mLevels;
        C4JRender::eTextureFormat mFormat;
        GX2Sampler mSampler;
        unsigned int mFlags;
    };

    float mClearColour[4];
    bool mDRCFrameStarted;
    GX2ContextState* mContextState;
    bool mDisplayEnabled;
    bool mForegroundAcquired;
    void* mTVScanBuffer;
    void* mDRCScanBuffer;
    GX2ColorBuffer mColourBuffer;
    GX2ColorBuffer mThumbnailColourBuffer[4];
    GX2Texture mColourTexture;
    GX2Texture mThumbnailTexture[4];
    GX2DepthBuffer mDepthBuffer;
    GX2ColorBuffer mAlphaColourBuffer;
    GX2Texture mAlphaTexture;
    GX2Sampler mAlphaSampler;
    GX2ColorBuffer mTVColourBuffer;
    GX2Texture mTVTexture;
    GX2ColorBuffer mDRCColourBuffer;
    GX2Texture mDRCTexture;
    GX2Sampler mDRCSampler;
    GX2DepthBuffer mDRCDepthBuffer;
    GX2VertexShader** mVertexShaders;
    GX2FetchShader** mFetchShaders;
    GX2VertexShader* mResolveVertexShader;
    GX2VertexShader* mClearVertexShader;
    GX2PixelShader** mPixelShaders;
    GX2PixelShader* mCopyPixelShader;
    GX2PixelShader* mClearPixelShader;
    GX2PixelShader* mAlphaResolvePixelShader;
    GX2PixelShader* mAlphaAccumulatePixelShader;
    GX2PixelShader* mGammaPixelShader;
    unsigned int* mVertexStrides;
    int mDefaultTexture;
    unsigned short mGamma;
    int mFrameCount;
    bool mCapturingThumbnail;
    OSFastMutex mCommandBufferMutex;
    int mCurrentVertexShader;
    int mCurrentPixelShader;
    C4JRender::eViewportType mCurrentViewport;
    bool mTextureStateDirty;
    Texture mTextures[512];
    int mScreenWidth[2];
    int mScreenHeight[2];
    int mScreen;
    bool mHiDef;
    bool mAllowStaticCommandBufferIds;
    int mNextCommandBufferId;
    short* mCommandBufferSlots;
    CommandBuffer** mCommandBuffers;
    float* mCommandBufferMatrices;
    int* mCommandBufferIds;
    unsigned char* mCommandBufferVertexTypes;
    int* mCommandBufferPixelShaderTypes;
    unsigned char* mCommandBufferPrimitiveTypes;
    int mNumCommandBuffers;
    int mNumRetiredCommandBuffers;
    int mCommandBufferVertsDrawn;
    int mCommandBufferVertsDrawnLastFrame;
    MEMHeapHeader* mMEM1Heap;
    MEMHeapHeader* mBucketHeap;
    void* mReservedMEM1;
    bool mScreenGrabRequested;
    float mColour[4];
    float mFogColour[4];
    float mLerpColour[4];
    float mColourAdd[4];
    float mTexColour[4][4];
    bool mPixelUniformsDirty;
    float mLayeredTextureSize[4];
    float mLayeredParams[7][4];
    bool mLayeredParamsDirty;
};

extern Renderer* gRenderer;
extern Renderer* gRendererInstance;

#endif
