// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPipeline.hpp
#ifndef PLATFORM_PC_SHADERPACK_SHADERPIPELINE_HPP
#define PLATFORM_PC_SHADERPACK_SHADERPIPELINE_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "platform/pc/shaderpack/GlApi.hpp"
#include "platform/pc/shaderpack/ShaderAdapters.hpp"
#include "platform/pc/shaderpack/ShaderAlbedoBaker.hpp"
#include "platform/pc/shaderpack/ShaderExpression.hpp"
#include "platform/pc/shaderpack/ShaderFxaa.hpp"
#include "platform/pc/shaderpack/ShaderProgramSet.hpp"
#include "platform/pc/shaderpack/ShaderUniforms.hpp"

class ShaderPack;

struct ShaderWorldState
{
    int dimension = 0;
    double cameraX = 0.0;
    double cameraY = 0.0;
    double cameraZ = 0.0;
    float eyeAltitude = 0.0f;
    // first person: the eye's height over the camera entity's feet, where Iris puts cameraPosition; -1 in third
    // person (the view matrix then gives the camera's place)
    float eyeHeight = -1.0f;
    float view[16] = {};
    float projection[16] = {};
    float celestialAngle = 0.0f;
    long long worldTime = 0;
    int moonPhase = 0;
    float rainStrength = 0.0f;
    float thunderStrength = 0.0f;
    int isEyeInWater = 0;
    float nightVision = 0.0f;
    float blindness = 0.0f;
    int eyeBlockLight = 0;
    int eyeSkyLight = 0;
    int heldItemId = -1;
    int heldBlockLight = 0;
    int heldItemId2 = -1;
    int heldBlockLight2 = 0;
    float skyColor[3] = {};
    float fogColor[3] = {};
    float nearPlane = 0.05f;
    float farPlane = 256.0f;
    bool hideGui = false;
    float screenBrightness = 0.5f;
    float playerMood = 0.0f;
    int bossBattle = 0;
    int biome = 0;
    int biomeCategory = 0;
    int biomePrecipitation = 0;
    float temperature = 0.5f;
    float rainfall = 0.5f;
    int atlasWidth = 0;
    int atlasHeight = 0;
    float sunPathRotation = 0.0f;
    bool entityFlags[16] = {};
    double eyeX = 0.0;
    double eyeY = 0.0;
    double eyeZ = 0.0;
    float lookVector[3] = {0.0f, 0.0f, 1.0f};
    float bodyVector[3] = {0.0f, 0.0f, 1.0f};
    float health = -1.0f;
    float maxHealth = -1.0f;
    float hunger = -1.0f;
    float air = -1.0f;
    float maxAir = -1.0f;
    float armor = -1.0f;
    bool spectator = false;
    bool firstPerson = true;
    bool rightHanded = true;
    bool elytraFlying = false;
    float lightning[4] = {};
    float cloudHeight = 128.0f;
    int heightLimit = 256;
    int logicalHeightLimit = 256;
};

enum ShaderEntityFlag
{
    ShaderEntity_Alive,
    ShaderEntity_Burning,
    ShaderEntity_Child,
    ShaderEntity_Glowing,
    ShaderEntity_Hurt,
    ShaderEntity_InLava,
    ShaderEntity_InWater,
    ShaderEntity_Invisible,
    ShaderEntity_OnGround,
    ShaderEntity_Ridden,
    ShaderEntity_Riding,
    ShaderEntity_Sneaking,
    ShaderEntity_Sprinting,
    ShaderEntity_Wet,
    ShaderEntity_Count
};

struct ShaderDrawValues
{
    int fogMode = 0;
    float fogStart = 0.0f;
    float fogEnd = 0.0f;
    float fogDensity = 0.0f;
    float fogColor[3] = {};
    float alphaRef = 0.0f;
    float entityColor[4] = {};
    int blendFunc[4] = {};
    float lights[5][4] = {};
    float colourScale[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    bool quadData = false;
    bool quadVertices = false;
};

struct ShaderDrawTarget
{
    GLuint framebuffer = 0;
    int width = 0;
    int height = 0;
    bool blendOverride = false;
    bool blendEnable = false;
    GLenum blend[4] = {};
    unsigned int bufferBlendOff = 0;
};

class ShaderPipeline
{
public:
    static ShaderPipeline& Get();

    void RequestReload() { mReloadRequested = true; }
    bool ReloadRequested() const { return mReloadRequested; }
    void Shutdown();
    bool Enabled() const { return mLoaded; }
    const std::vector<std::string>& Errors() const { return mErrors; }
    const std::string& LoadedWorld() const { return mWorld; }

    // by block state element id: the pack's block id and the light the block gives off (65536 of each)
    void SetBlockMap(const std::vector<int>& map, const std::vector<int>& lights);
    void SetEntityId(int id) { mEntityId = id; }
    void SetBlockEntityId(int id) { mBlockEntityId = id; }
    typedef GLuint (*ResourceTextureResolver)(const std::string& name);
    void SetBiomeConstants(const std::map<std::string, float>& constants);
    void SetResourceResolver(ResourceTextureResolver resolver) { mResourceResolver = resolver; }
    // LegacyEX: the game's lightmap, for the lightmap sampler of composite and compute programs (the world's draws
    // use the one the game binds itself)
    void SetLightmapTexture(GLuint texture) { mLightmapTexture = texture; }

    bool BeginWorld(const ShaderWorldState& state, int screenWidth, int screenHeight);
    void EndWorld(GLuint mainFramebuffer, int screenWidth, int screenHeight);
    void AbortWorld();
    bool InWorld() const { return mInWorld; }
    bool InShadow() const { return mInShadow; }

    void SetStage(int stage);
    int Stage() const { return mStage; }
    void BeginTranslucent();
    void BeginHand();
    void EndHand();
    bool InHand() const { return mInHand; }
    void ClearTargets(bool colour, bool depth);

    bool ShadowEnabled() const;
    bool ShadowEntities() const { return mShadowEntities; }
    bool ShadowBlockEntities() const { return mShadowBlockEntities; }
    bool ShadowTerrain() const { return mShadowTerrain; }
    bool ShadowTranslucent() const { return mShadowTranslucent; }
    bool ShadowPlayer() const { return mShadowPlayer; }
    int CloudMode() const { return mCloudMode; }
    bool RenderSkyDisc() const { return mSky; }
    bool RenderSun() const { return mSun; }
    bool RenderMoon() const { return mMoon; }
    bool UnderwaterOverlay() const { return mUnderwaterOverlay; }
    bool Vignette() const { return mVignette; }
    bool RainDepth() const { return mRainDepth; }
    bool BeaconBeamDepth() const { return mBeaconBeamDepth; }
    bool OldLighting() const { return mOldLighting; }
    bool SeparateAo() const { return mSeparateAo; }
    float ShadowRenderDistance() const;
    void ShadowMatrices(float* view, float* projection) const;
    bool BeginShadow();
    void EndShadowOpaque();
    void EndShadow();

    const ShaderLinkedProgram* SelectProgram(ShaderAdapter adapter, int vanillaPixelShader);
    bool PrepareTargets(const ShaderLinkedProgram* program, ShaderDrawTarget& target);
    void ApplyProgram(const ShaderLinkedProgram* program, const ShaderDrawValues& values);
    void BindTerrainData(GLuint vertexTexture, GLuint quadTexture);
    void SetFirstVertex(int firstVertex);
    // LegacyEX: 1 while chunk sections draw from the renderer's shared terrain buffers, 0 otherwise
    void SetChunkOffsetMode(int mode);
    GLuint ResolveAlbedo(const ShaderAlbedoRequest& request, bool& stateChanged)
    {
        return mAlbedo.Resolve(request, mFrame, stateChanged);
    }
    bool CanUseTerrainData() const { return mBlockMapTexture != 0; }

private:
    struct ColorTarget
    {
        GLuint textures[2] = {};
        int current = 0;
        GLenum format = 0;
        int width = 0;
        int height = 0;
        bool used = false;
        bool clear = true;
        float clearColor[4] = {};
        bool mipmap = false;
        bool mipDirty[2] = {};
        float sizeX = 1.0f;
        float sizeY = 1.0f;
        bool absolute = false;
    };

    struct CustomUniform
    {
        std::string name;
        int type;
        int slot;
        bool uniform;
        ShaderExpression expression;
    };

    struct CustomTexture
    {
        std::string stage;
        std::string name;
        std::string resource;
        int image = -1;
        GLuint texture = 0;
        GLenum target = 0;
        bool linear = false;
        bool clamp = false;
        bool named = false;
    };

    struct CustomImage
    {
        std::string name;
        GLenum target = 0;
        GLenum internal = 0;
        GLenum format = 0;
        GLenum type = 0;
        bool clear = false;
        bool relative = false;
        float size[3] = {};
        int extent[3] = {};
        GLuint texture = 0;
    };

    struct BufferObject
    {
        GLuint buffer = 0;
        GLsizeiptr bytes = 0;
        bool relative = false;
        float scale[2] = {1.0f, 1.0f};
        GLsizeiptr allocated = 0;
        std::string file;
    };


    ShaderPipeline();

    bool Reload(int dimension);
    void Unload();
    void LoadSettings();
    void LoadCustomUniforms();
    static void ReadTextureMeta(ShaderPack& pack, const std::string& path, CustomTexture& texture);
    void LoadCustomTextures();
    void LoadCustomImages();
    void LoadBufferObjects();
    void CreateImages(bool relativeOnly);
    void AllocateBufferObjects(bool relativeOnly);
    void ClearImages(bool all);
    void BindImages(const ShaderLinkedProgram& program);
    void RunCompute(ShaderProgramSource& pass, ShaderComputeSource& compute);
    void Barrier();
    void ApplyPassBlend(const ShaderProgramSource& pass, const std::vector<int>& written);
    void CreateNoiseTexture();
    void CreateDefaultTextures();
    void ResizeTargets(int width, int height);
    void DestroyTargets();
    void CreateShadowTargets();
    void DestroyShadowTargets();
    void UpdateFrameUniforms(const ShaderWorldState& state, int screenWidth, int screenHeight);
    void EvaluateCustomUniforms();
    void RunPasses(int family);
    void RunPass(ShaderProgramSource& pass, bool final, GLuint mainFramebuffer, int screenWidth, int screenHeight);
    void CopyDepth(GLuint destination);
    void BindSamplers(const ShaderLinkedProgram& program, bool composite);
    GLuint SourceTexture(int source, int custom, bool composite, GLenum& target);
    GLuint SamplerFor(int source, bool shadow);
    void UploadFrameUniforms(const ShaderLinkedProgram& program);
    void UploadDrawUniforms(const ShaderLinkedProgram& program, const ShaderDrawValues& values);
    void ReadCenterDepth();
    int GbufferForStage(int vanillaPixelShader, ShaderAdapter adapter) const;
    GLenum BufferFormat(int buffer) const;
    std::string ProgramProperty(const std::string& prefix, const std::string& program) const;

    bool WrittenBefore(int buffer, const std::string& program) const;
    static int SamplerLookup(const std::string& name, const std::string& program, GLenum type, void* user);
    static int UniformLookup(const std::string& name, void* user);
    static int ImageLookup(const std::string& name, void* user);

    bool mReloadRequested = true;
    bool mLoaded = false;
    std::string mWorld;
    int mDimension = 0x7fffffff;
    std::vector<std::string> mErrors;
    ShaderProgramSet mPrograms;
    std::unique_ptr<ShaderProgramSource> mFallback;
    std::unique_ptr<ShaderProgramSource> mFinalCopy;
    ShaderProgramSource* mCurrentSource = nullptr;
    int mCurrentGbuffer = -1;
    std::string mLastError;

    ColorTarget mColor[16];
    GLuint mDepthTextures[3] = {};
    GLuint mGbufferFramebuffer = 0;
    GLuint mCompositeFramebuffer = 0;
    GLuint mCopyFramebuffer = 0;
    GLuint mAttached[8] = {};
    int mWidth = 0;
    int mHeight = 0;
    int mScreenWidth = 0;
    int mScreenHeight = 0;
    float mRenderScale = 1.0f;
    int mAntialiasing = 0;

    GLuint mShadowFramebuffer = 0;
    GLuint mShadowDepth[2] = {};
    GLuint mShadowColor[2] = {};
    int mShadowResolution = 0;
    bool mShadowColorUsed[2] = {};
    bool mShadowHardware[2] = {};
    bool mShadowMipmap[2] = {};
    bool mShadowColorMipmap[2] = {};
    bool mShadowNearest[2] = {};
    bool mShadowColorNearest[2] = {};
    bool mShadowColorClear[2] = {true, true};
    float mShadowColorClearColor[2][4] = {{1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}};
    GLenum mShadowColorFormat[2] = {};
    bool mShadowEntities = true;
    bool mShadowBlockEntities = true;
    bool mShadowTerrain = true;
    bool mShadowTranslucent = true;
    bool mShadowPlayer = false;
    int mCloudMode = -1;
    bool mSky = true;
    bool mSun = true;
    bool mMoon = true;
    bool mUnderwaterOverlay = true;
    bool mVignette = true;
    bool mRainDepth = false;
    bool mBeaconBeamDepth = false;
    bool mDynamicHandLight = true;
    bool mOldHandLight = true;
    bool mOldLighting = true;
    bool mSeparateAo = false;
    float mShadowDistance = 160.0f;
    float mShadowDistanceRenderMul = -1.0f;
    float mShadowIntervalSize = 2.0f;
    float mShadowMatrix[16] = {};
    float mShadowDrawMatrix[16] = {};
    float mShadowProjection[16] = {};

    GLuint mNoiseTexture = 0;
    GLuint mNormalsTexture = 0;
    GLuint mSpecularTexture = 0;
    GLuint mBlockMapBuffer = 0;
    GLuint mBlockMapTexture = 0;
    GLuint mEmptyVertexArray = 0;
    GLuint mSamplers[8] = {};
    ShaderDrawValues mLastDrawValues;
    ShaderAlbedoBaker mAlbedo;
    ShaderFxaa mFxaa;
    std::vector<CustomTexture> mCustomTextures;
    std::vector<CustomImage> mImages;
    BufferObject mBufferObjects[9];
    bool mComputeSupported = false;
    bool mImagesSupported = false;
    bool mBuffersSupported = false;
    bool mSetupPending = false;
    // pairs of block id and light, by block state element id (RG32I)
    std::vector<int> mPendingBlockMap;
    GLuint mLightmapTexture = 0;
    bool mBlockMapDirty = false;

    std::vector<CustomUniform> mCustomUniforms;
    ShaderExpressionContext mExpressions;
    std::map<std::string, float> mBiomeConstants;
    int mParameterSlots[ShaderUniform_Count] = {};
    std::vector<std::pair<int, int>> mMatrixSlots;
    ResourceTextureResolver mResourceResolver = nullptr;

    float mFrameValues[ShaderUniform_Count][16] = {};
    unsigned long long mFrame = 0;
    double mFrameTimeCounter = 0.0;
    double mLastFrameSeconds = -1.0;
    float mWetness = 0.0f;
    float mEyeBrightnessSmooth[2] = {};
    float mCenterDepthSmooth = 1.0f;
    float mCenterDepth = 1.0f;
    GLuint mDepthReadBuffer = 0;
    bool mDepthReadPending = false;
    float mPreviousView[16] = {};
    float mPreviousProjection[16] = {};
    double mPreviousCamera[3] = {};
    bool mHasPrevious = false;
    float mWetnessHalflife = 600.0f;
    float mDrynessHalflife = 200.0f;
    float mEyeBrightnessHalflife = 10.0f;
    float mCenterDepthHalflife = 1.0f;
    float mHandDepth = 0.125f;
    float mSunPathRotation = 0.0f;

    bool mInWorld = false;
    bool mInShadow = false;
    bool mInHand = false;
    bool mTranslucent = false;
    bool mPrepareDone = false;
    bool mDepth2Copied = false;
    bool mFlipFinal = false;
    bool mShadowOpaqueCopied = false;
    int mStage = ShaderRenderStage_None;
    int mEntityId = -1;
    int mBlockEntityId = -1;
    const ShaderLinkedProgram* mBoundProgram = nullptr;
    const ShaderLinkedProgram* mDrawProgram = nullptr;
    unsigned long long mBindingsStamp = 0;
    unsigned long long mBoundStamp = ~0ull;
    int mGbufferAlphaFunction[ShaderGbuffer_Count] = {};
    float mGbufferAlphaReference[ShaderGbuffer_Count] = {};
};

#endif
