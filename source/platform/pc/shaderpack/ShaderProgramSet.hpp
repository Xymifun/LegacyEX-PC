// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderProgramSet.hpp
#ifndef PLATFORM_PC_SHADERPACK_SHADERPROGRAMSET_HPP
#define PLATFORM_PC_SHADERPACK_SHADERPROGRAMSET_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "platform/pc/shaderpack/GlApi.hpp"
#include "platform/pc/shaderpack/GlslTranslator.hpp"
#include "platform/pc/shaderpack/ShaderAdapters.hpp"
#include "platform/pc/shaderpack/ShaderPack.hpp"
#include "platform/pc/shaderpack/ShaderPreprocessor.hpp"

class ShaderPackOptions;
class ShaderPackProperties;

enum ShaderGbuffer
{
    ShaderGbuffer_Basic,
    ShaderGbuffer_Textured,
    ShaderGbuffer_TexturedLit,
    ShaderGbuffer_SkyBasic,
    ShaderGbuffer_SkyTextured,
    ShaderGbuffer_Clouds,
    ShaderGbuffer_Terrain,
    ShaderGbuffer_TerrainSolid,
    ShaderGbuffer_TerrainCutoutMip,
    ShaderGbuffer_TerrainCutout,
    ShaderGbuffer_DamagedBlock,
    ShaderGbuffer_Block,
    ShaderGbuffer_BeaconBeam,
    ShaderGbuffer_Item,
    ShaderGbuffer_Entities,
    ShaderGbuffer_EntitiesGlowing,
    ShaderGbuffer_ArmorGlint,
    ShaderGbuffer_SpiderEyes,
    ShaderGbuffer_Hand,
    ShaderGbuffer_Weather,
    ShaderGbuffer_Water,
    ShaderGbuffer_HandWater,
    ShaderGbuffer_Shadow,
    ShaderGbuffer_ShadowSolid,
    ShaderGbuffer_ShadowCutout,
    ShaderGbuffer_Count
};

enum ShaderPassFamily
{
    ShaderPass_Setup,
    ShaderPass_Begin,
    ShaderPass_ShadowComp,
    ShaderPass_Prepare,
    ShaderPass_Deferred,
    ShaderPass_Composite,
    ShaderPass_Final,
    ShaderPass_FamilyCount
};

enum ShaderTextureSource
{
    ShaderSource_None = -1,
    ShaderSource_Albedo,
    ShaderSource_Lightmap,
    ShaderSource_Normals,
    ShaderSource_Specular,
    ShaderSource_TerrainVertices,
    ShaderSource_QuadData,
    ShaderSource_BlockMap,
    ShaderSource_ChunkOffsets,
    ShaderSource_ColorTex0,
    ShaderSource_DepthTex0 = ShaderSource_ColorTex0 + 16,
    ShaderSource_DepthTex1,
    ShaderSource_DepthTex2,
    ShaderSource_ShadowTex0,
    ShaderSource_ShadowTex1,
    ShaderSource_ShadowColor0,
    ShaderSource_ShadowColor1,
    ShaderSource_Noise,
    ShaderSource_Custom
};

struct ShaderLinkedProgram
{
    struct Uniform
    {
        int id;
        GLint location;
    };

    struct Sampler
    {
        int source;
        int custom;
        GLint unit;
        bool shadow;
        bool integer;
    };

    struct Image
    {
        int source;
        int custom;
        GLuint unit;
    };

    GLuint name = 0;
    std::vector<Uniform> frameUniforms;
    std::vector<Uniform> drawUniforms;
    std::vector<std::pair<int, GLint>> customUniforms;
    std::vector<Sampler> samplers;
    std::vector<Image> images;
    unsigned long long frameStamp = ~0ull;
    std::vector<float> drawCache;
};

struct ShaderComputeSource
{
    std::string path;
    std::vector<std::string> files;
    std::string text;
    int version = 430;
    bool absolute = false;
    int workGroups[3] = {1, 1, 1};
    float workGroupsRender[2] = {1.0f, 1.0f};
    int localSize[3] = {1, 1, 1};
    ShaderLinkedProgram linked;
    bool attempted = false;
};

struct ShaderProgramSource
{
    std::string name;
    bool present = false;
    bool enabled = true;
    std::string path[ShaderStage_Count];
    std::vector<std::string> files[ShaderStage_Count];
    GlslTranslator translator[ShaderStage_Count];
    GlslUsage linkedUsage;
    std::vector<int> drawBuffers;
    std::string error;
    ShaderLinkedProgram linked[ShaderAdapter_Count];
    bool attempted[ShaderAdapter_Count] = {};
    std::vector<ShaderComputeSource> compute;
};

class ShaderProgramSet
{
public:
    // LegacyEX: with the sampler's GL type, so a 2D and a 3D custom texture can share a sampler name
    typedef int (*CustomSamplerLookup)(const std::string& name, const std::string& program, GLenum type, void* user);
    typedef int (*CustomUniformLookup)(const std::string& name, void* user);
    typedef int (*CustomImageLookup)(const std::string& name, void* user);

    bool Load(const ShaderPack& pack, const ShaderPackOptions& options, const ShaderPackProperties& properties,
              const ShaderMacros& macros, const std::string& world, const std::vector<std::string>& disabled,
              std::vector<std::string>& errors);
    bool LoadInline(ShaderProgramSource& program, const std::string& vertex, const std::string& fragment,
                    std::string& error);
    void Destroy();

    void SetLookups(CustomSamplerLookup samplers, CustomUniformLookup uniforms, void* user);
    void SetImageLookup(CustomImageLookup images) { mImageLookup = images; }
    void SetComputeSupported(bool supported) { mComputeSupported = supported; }
    void SetGlslVersion(int version) { mGlslVersion = version; }

    static const char* GbufferName(int program);
    static int GbufferFallback(int program);
    static const char* PassFamilyName(int family);

    ShaderProgramSource* Gbuffer(int program) const;
    ShaderProgramSource* ResolveGbuffer(int program) const;
    const std::vector<ShaderProgramSource*>& Passes(int family) const { return mPasses[family]; }

    const ShaderLinkedProgram* Link(ShaderProgramSource& program, ShaderAdapter adapter, std::string& error);
    const ShaderLinkedProgram* LinkCompute(ShaderProgramSource& program, ShaderComputeSource& compute,
                                           std::string& error);
    int CompileAll(std::vector<std::string>& errors);

    const std::map<std::string, std::string>& Directives() const { return mDirectives; }
    std::string Directive(const std::string& name, const std::string& fallback = std::string()) const;
    const std::string& StandardMacroText() const { return mMacroText; }

private:
    bool LoadProgram(const ShaderPack& pack, const ShaderPackOptions& options, const ShaderMacros& macros,
                     const std::string& world, ShaderProgramSource& program, bool composite);
    void ScanDirectives(const std::string& preprocessed, ShaderProgramSource& program, ShaderStage stage);
    void LoadCompute(const ShaderPack& pack, const ShaderPackOptions& options, const ShaderMacros& macros,
                     const std::string& world, ShaderProgramSource& program);
    static bool IsComposite(const ShaderProgramSource& program);
    std::string MapLog(const std::string& log, const std::vector<std::string>& files) const;
    void BindProgram(ShaderLinkedProgram& linked, const ShaderProgramSource& program, bool composite);

    std::vector<std::unique_ptr<ShaderProgramSource>> mStorage;
    ShaderProgramSource* mGbuffers[ShaderGbuffer_Count] = {};
    std::vector<ShaderProgramSource*> mPasses[ShaderPass_FamilyCount];
    std::map<std::string, std::string> mDirectives;
    std::string mMacroText;
    ShaderMacros mMacros;
    int mGlslVersion = 330;
    CustomSamplerLookup mSamplerLookup = nullptr;
    CustomUniformLookup mUniformLookup = nullptr;
    CustomImageLookup mImageLookup = nullptr;
    void* mLookupUser = nullptr;
    bool mComputeSupported = false;
};

#endif
