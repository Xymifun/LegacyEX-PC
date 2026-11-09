#ifndef GAME_CLIENT_RENDER_C4JRENDER_HPP
#define GAME_CLIENT_RENDER_C4JRENDER_HPP

struct _GX2ColorBuffer;
struct _GX2DepthBuffer;
struct _GX2Texture;
typedef _GX2ColorBuffer GX2ColorBuffer;
typedef _GX2DepthBuffer GX2DepthBuffer;
typedef _GX2Texture GX2Texture;
struct ImageFileBuffer;
struct D3DXIMAGE_INFO;
struct _XSOCIAL_PREVIEWIMAGE;

class C4JRender
{
public:
    enum eVertexType
    {
        eVertexType_Default,
        eVertexType_Terrain,
        eVertexType_DefaultLighting,
        eVertexType_TexGen,
        eVertexType_UI,
        eVertexType_ScalingGridUI,
        eVertexType_TiledUI,
        eVertexType_ColouredUI,
        eVertexType_Entity,
        eVertexType_EntityLighting,
        eVertexType_Count
    };

    enum ePixelShaderType
    {
        ePixelShaderType_Default,
        ePixelShaderType_DefaultTwoLayers,
        ePixelShaderType_DefaultThreeLayers,
        ePixelShaderType_DefaultFourLayers,
        ePixelShaderType_Terrain,
        ePixelShaderType_TerrainForceLod,
        ePixelShaderType_OrderIndependentAlpha,
        ePixelShaderType_LayeredColourMask,
        ePixelShaderType_UIColourAdd,
        ePixelShaderType_UI,
        ePixelShaderType_UIAlphaTest,
        ePixelShaderType_EntityChangeColour,
        ePixelShaderType_EntityEmissiveAlpha,
        ePixelShaderType_EntityEmissiveAlphaOnly,
        ePixelShaderType_Count
    };

    enum ePrimitiveType
    {
        ePrimitiveType_Triangles,
        ePrimitiveType_TriangleStrip,
        ePrimitiveType_TriangleFan,
        ePrimitiveType_Quads,
        ePrimitiveType_Lines,
        ePrimitiveType_LineStrip
    };

    enum eViewportType
    {
        eViewportType_FullScreen,
        eViewportType_TopHalf,
        eViewportType_BottomHalf,
        eViewportType_LeftHalf,
        eViewportType_RightHalf,
        eViewportType_TopLeftQuarter,
        eViewportType_TopRightQuarter,
        eViewportType_BottomLeftQuarter,
        eViewportType_BottomRightQuarter
    };

    enum eTextureFormat
    {
        eTextureFormat_RGBA8
    };

    virtual void Tick() = 0;
    virtual void UpdateGamma(unsigned short gamma) = 0;
    virtual void MatrixMode(int mode) = 0;
    virtual void MatrixSetIdentity() = 0;
    virtual void MatrixTranslate(float x, float y, float z) = 0;
    virtual void MatrixRotate(float angle, float x, float y, float z) = 0;
    virtual void MatrixScale(float x, float y, float z) = 0;
    virtual void MatrixPerspective(float fieldOfView, float aspect, float nearDistance, float farDistance) = 0;
    virtual void MatrixOrthogonal(float left, float right, float bottom, float top, float nearDistance,
                                  float farDistance) = 0;
    virtual void MatrixPop() = 0;
    virtual void MatrixPush() = 0;
    virtual void MatrixMult(float* matrix) = 0;
    virtual float* MatrixGet(int mode) = 0;
    virtual void Set_matrixDirty() = 0;
    virtual void Initialise() = 0;
    virtual void InitialiseContext() = 0;
    virtual GX2ColorBuffer* GetColorBuffer() = 0;
    virtual GX2DepthBuffer* GetDepthStencilBuffer() = 0;
    virtual GX2ColorBuffer* GetColorBufferDRC() = 0;
    virtual GX2DepthBuffer* GetDepthStencilBufferDRC() = 0;
    virtual void StartFrame() = 0;
    virtual void TEMP_PatchStateFromIggy() = 0;
    virtual void ResetRenderState() = 0;
    virtual void StartDRCFrame(bool bCopyFrame) = 0;
    virtual void DoScreenGrabOnNextPresent() = 0;
    virtual void Present() = 0;
    virtual void Clear(int flags) = 0;
    virtual void SetClearColour(const float* colour) = 0;
    virtual bool IsWidescreen() = 0;
    virtual bool IsHiDef() = 0;
    virtual void CaptureThumbnail(ImageFileBuffer* buffer) = 0;
    virtual void CaptureScreen(ImageFileBuffer* buffer, _XSOCIAL_PREVIEWIMAGE* previewImage) = 0;
    virtual void BeginConditionalSurvey(int survey) = 0;
    virtual void EndConditionalSurvey() = 0;
    virtual void BeginConditionalRendering(int survey) = 0;
    virtual void EndConditionalRendering() = 0;
    virtual void BeginOrderIndependentAlpha() = 0;
    virtual void EndOrderIndependentAlpha() = 0;
    virtual void ReleaseForeground() = 0;
    virtual void AcquiredForeground() = 0;
    virtual void WiiUWaitDrawDone() = 0;
    virtual int GetFramesSinceBoot() = 0;
    virtual void DrawVertices(ePrimitiveType primitiveType, int numVertices, void* vertices, eVertexType vertexType,
                              ePixelShaderType pixelShaderType, unsigned char* layers) = 0;
    virtual void CBuffLockStaticCreations() = 0;
    virtual int CBuffCreate(int count) = 0;
    virtual void CBuffDelete(int firstId, int count) = 0;
    virtual void CBuffStart(int id, bool bApplyLighting) = 0;
    virtual void CBuffClear(int id) = 0;
    virtual int CBuffSize(int id) = 0;
    virtual void CBuffEnd() = 0;
    virtual bool CBuffCall(int id, bool bApplyMatrix, unsigned char layerMask) = 0;
    virtual void CBuffTick() = 0;
    virtual void CBuffDeferredModeStart() = 0;
    virtual void CBuffDeferredModeEnd() = 0;
    virtual int GetMaxTextures() = 0;
    virtual int TextureCreate() = 0;
    virtual void TextureFree(int texture) = 0;
    virtual void TextureBind(int texture) = 0;
    virtual void TextureBind(int stage, int texture) = 0;
    virtual void TextureBindMask(int stage) = 0;
    virtual void TextureBindVertex(int texture) = 0;
    virtual void TextureSetTextureLevels(int levels) = 0;
    virtual int TextureGetTextureLevels() = 0;
    virtual void TextureData(int width, int height, void* data, int level, eTextureFormat format) = 0;
    virtual void TextureDataUpdate(int x, int y, int width, int height, void* data, int level) = 0;
    virtual void TextureSetParam(int parameter, int value) = 0;
    virtual void TextureDynamicUpdateStart() = 0;
    virtual void TextureDynamicUpdateEnd() = 0;
    virtual int LoadTextureData(const char* fileName, D3DXIMAGE_INFO* info, int** data) = 0;
    virtual int LoadTextureData(unsigned char* fileData, unsigned int fileSize, D3DXIMAGE_INFO* info, int** data) = 0;
    virtual int SaveTextureData(const char* fileName, D3DXIMAGE_INFO* info, int* data) = 0;
    virtual int SaveTextureDataToMemory(void* destination, int destinationSize, int* bytesWritten, int width,
                                        int height, int* data) = 0;
    virtual void TextureGetStats() = 0;
    virtual GX2Texture* TextureGetTexture(int texture) = 0;
    virtual void StateSetColour(float red, float green, float blue, float alpha) = 0;
    virtual void StateSetColourAdd(float red, float green, float blue, float alpha) = 0;
    virtual void StateSetTexColour(int stage, float red, float green, float blue, float alpha) = 0;
    virtual void StateSetDepthMask(bool bDepthMask) = 0;
    virtual void StateSetBlendEnable(bool bBlend) = 0;
    virtual void StateSetBlendFunc(int source, int destination) = 0;
    virtual void StateSetBlendFactor(unsigned int factor) = 0;
    virtual void StateSetAlphaFunc(int alphaFunc, float reference) = 0;
    virtual void StateSetDepthFunc(int depthFunc) = 0;
    virtual void StateSetFaceCull(bool bFaceCull) = 0;
    virtual void StateSetFaceCullCW(bool bClockwise) = 0;
    virtual void StateSetLineWidth(float width) = 0;
    virtual void StateSetWriteEnable(bool bRed, bool bGreen, bool bBlue, bool bAlpha) = 0;
    virtual void StateSetDepthTestEnable(bool bDepthTest) = 0;
    virtual void StateSetAlphaTestEnable(bool bAlphaTest) = 0;
    virtual void StateSetDepthSlopeAndBias(float slope, float bias) = 0;
    virtual void StateSetFogEnable(bool bFog) = 0;
    virtual void StateSetFogMode(int mode) = 0;
    virtual void StateSetFogNearDistance(float nearDistance) = 0;
    virtual void StateSetFogFarDistance(float farDistance) = 0;
    virtual void StateSetFogDensity(float density) = 0;
    virtual void StateSetFogColour(float red, float green, float blue) = 0;
    virtual void StateSetLightingEnable(bool bLighting) = 0;
    virtual void StateSetVertexTextureUV(float u, float v) = 0;
    virtual void StateSetTextureScaleUI(float scaleU, float scaleV, float offsetU, float offsetV) = 0;
    virtual void StateSetScalingGridUI(float left, float right, float top, float bottom, float innerLeft,
                                       float innerRight, float innerTop, float innerBottom, float scaleU,
                                       float scaleV) = 0;
    virtual void StateSetLightColour(int light, float red, float green, float blue) = 0;
    virtual void StateSetLightAmbientColour(float red, float green, float blue) = 0;
    virtual void StateSetLightDirection(int light, float x, float y, float z) = 0;
    virtual void StateSetLightEnable(int light, bool bEnable) = 0;
    virtual void StateSetViewport(eViewportType viewport) = 0;
    virtual void StateGetScissorRect(unsigned int& left, unsigned int& top, unsigned int& right,
                                     unsigned int& bottom) = 0;
    virtual void StateSetScissorRect(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom) = 0;
    virtual void StateSetEnableViewportClipPlanes(bool bEnable) = 0;
    virtual void StateSetTexGenCol(int coordinate, float x, float y, float z, float w, bool bEyeLinear) = 0;
    virtual void StateSetStencil(int stencilFunc, unsigned char reference, unsigned char compareMask,
                                 unsigned char writeMask) = 0;
    virtual void StateSetForceLOD(int lod) = 0;
    virtual void StateSetLerpColour(float red, float green, float blue, float alpha) = 0;
    virtual void SetLayeredColour(int layer, float red, float green, float blue) = 0;
    virtual void SetLayeredParam(int layer, int slot, float offset, float scale) = 0;
    virtual void SetLayeredTextureSize(int width, int height) = 0;
    virtual void StateSetBoneMatrixData(int bone) = 0;
    virtual void StateSetBoneMatrixNull(int bone) = 0;
    virtual void StateSetVertexAlphaDistance(float distance) = 0;
    virtual void StateSetVertexAlpha(float alpha) = 0;
    virtual int GetNumCommandBufferVertsDrawnLastFrame() = 0;
    virtual int GetNumVertsInCommandBuffer(int id) = 0;
    virtual void BeginEvent(const wchar_t* name) = 0;
    virtual void EndEvent() = 0;
    virtual void* MemoryAllocMEM1(unsigned int size, unsigned int alignment, bool bReserved) = 0;
    virtual void MemoryFreeMEM1(void* memory) = 0;
    virtual void* MemoryAllocMEM2(unsigned int size, unsigned int alignment) = 0;
    virtual void MemoryFreeMEM2(void* memory) = 0;
    virtual void* MemoryAllocBucket(unsigned int size, unsigned int alignment) = 0;
    virtual void MemoryFreeBucket(void* memory) = 0;
};

#endif
