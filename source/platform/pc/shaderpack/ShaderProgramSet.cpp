// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderProgramSet.cpp
#include "platform/pc/shaderpack/ShaderProgramSet.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>

#include "platform/pc/shaderpack/ShaderPackOptions.hpp"
#include "platform/pc/shaderpack/ShaderPackProperties.hpp"
#include "platform/pc/shaderpack/ShaderUniforms.hpp"

namespace
{
struct GbufferInfo
{
    const char* name;
    int fallback;
};

const GbufferInfo kGbuffers[ShaderGbuffer_Count] = {
    {"gbuffers_basic", -1},
    {"gbuffers_textured", ShaderGbuffer_Basic},
    {"gbuffers_textured_lit", ShaderGbuffer_Textured},
    {"gbuffers_skybasic", ShaderGbuffer_Basic},
    {"gbuffers_skytextured", ShaderGbuffer_Textured},
    {"gbuffers_clouds", ShaderGbuffer_Textured},
    {"gbuffers_terrain", ShaderGbuffer_TexturedLit},
    {"gbuffers_terrain_solid", ShaderGbuffer_Terrain},
    {"gbuffers_terrain_cutout_mip", ShaderGbuffer_Terrain},
    {"gbuffers_terrain_cutout", ShaderGbuffer_Terrain},
    {"gbuffers_damagedblock", ShaderGbuffer_Terrain},
    {"gbuffers_block", ShaderGbuffer_Terrain},
    {"gbuffers_beaconbeam", ShaderGbuffer_Textured},
    {"gbuffers_item", ShaderGbuffer_TexturedLit},
    {"gbuffers_entities", ShaderGbuffer_TexturedLit},
    {"gbuffers_entities_glowing", ShaderGbuffer_Entities},
    {"gbuffers_armor_glint", ShaderGbuffer_Textured},
    {"gbuffers_spidereyes", ShaderGbuffer_Textured},
    {"gbuffers_hand", ShaderGbuffer_TexturedLit},
    {"gbuffers_weather", ShaderGbuffer_TexturedLit},
    {"gbuffers_water", ShaderGbuffer_Terrain},
    {"gbuffers_hand_water", ShaderGbuffer_Hand},
    {"shadow", -1},
    {"shadow_solid", ShaderGbuffer_Shadow},
    {"shadow_cutout", ShaderGbuffer_Shadow},
};

const char* const kPassFamilies[ShaderPass_FamilyCount] = {"setup",    "begin",     "shadowcomp", "prepare",
                                                            "deferred", "composite", "final"};
const char* const kStageExtensions[ShaderStage_Count] = {".vsh", ".gsh", ".fsh"};
const char* const kVersion400Extensions = "#extension GL_ARB_gpu_shader5 : enable\n"
                                          "#extension GL_ARB_texture_query_lod : enable\n"
                                          "#extension GL_ARB_texture_cube_map_array : enable\n"
                                          "#extension GL_ARB_sample_shading : enable\n"
                                          "#extension GL_ARB_shader_subroutine : enable\n";
const GLenum kStageTypes[ShaderStage_Count] = {Gl::VERTEX_SHADER, Gl::GEOMETRY_SHADER, Gl::FRAGMENT_SHADER};

std::string Trim(const std::string& text)
{
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return std::string();
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

bool IsIdentifierChar(char character)
{
    return std::isalnum(static_cast<unsigned char>(character)) || character == '_';
}

bool EvaluateEnabled(const std::string& value, const ShaderMacros& macros)
{
    std::string trimmed = Trim(value);
    if (trimmed == "true")
        return true;
    if (trimmed == "false")
        return false;
    return ShaderEvaluateExpression(trimmed, macros) != 0.0;
}

int ColorTexIndex(const std::string& name)
{
    static const char* const kLegacy[8] = {"gcolor", "gdepth", "gnormal", "composite", "gaux1", "gaux2", "gaux3",
                                           "gaux4"};
    for (int index = 0; index < 8; ++index)
    {
        if (name == kLegacy[index])
            return index;
    }
    if (name.compare(0, 8, "colortex") == 0 && name.size() > 8)
    {
        for (size_t position = 8; position < name.size(); ++position)
        {
            if (!std::isdigit(static_cast<unsigned char>(name[position])))
                return -1;
        }
        int index = std::atoi(name.c_str() + 8);
        return index >= 0 && index < 16 ? index : -1;
    }
    return -1;
}

int ResolveSampler(const std::string& name, bool composite, bool watershadow)
{
    if (name == "lce_TerrainVertices")
        return ShaderSource_TerrainVertices;
    if (name == "lce_QuadData")
        return ShaderSource_QuadData;
    if (name == "lce_BlockMap")
        return ShaderSource_BlockMap;
    if (name == "lce_ChunkOffsets")
        return ShaderSource_ChunkOffsets;
    if (name == "texture" || name == "tex" || name == "gtexture")
        return composite ? ShaderSource_ColorTex0 : ShaderSource_Albedo;
    // LegacyEX: composite and compute programs get the lightmap too, as with Iris (ShaderPipeline::SetLightmapTexture)
    if (name == "lightmap")
        return ShaderSource_Lightmap;
    if (!composite)
    {
        if (name == "normals")
            return ShaderSource_Normals;
        if (name == "specular")
            return ShaderSource_Specular;
    }
    int colortex = ColorTexIndex(name);
    if (colortex >= 0)
        return ShaderSource_ColorTex0 + colortex;
    if (name == "shadow")
        return watershadow ? ShaderSource_ShadowTex1 : ShaderSource_ShadowTex0;
    if (name == "watershadow" || name == "shadowtex0")
        return ShaderSource_ShadowTex0;
    if (name == "shadowtex1" || name == "shadowtex1HW")
        return ShaderSource_ShadowTex1;
    if (name == "shadowtex0HW")
        return ShaderSource_ShadowTex0;
    if (name == "shadowcolor" || name == "shadowcolor0")
        return ShaderSource_ShadowColor0;
    if (name == "shadowcolor1")
        return ShaderSource_ShadowColor1;
    if (name == "depthtex0" || name == "gdepthtex")
        return ShaderSource_DepthTex0;
    if (name == "depthtex1")
        return ShaderSource_DepthTex1;
    if (name == "depthtex2")
        return ShaderSource_DepthTex2;
    if (name == "noisetex")
        return ShaderSource_Noise;
    return ShaderSource_None;
}

int FixedUnit(int source)
{
    switch (source)
    {
    case ShaderSource_Albedo:
        return ShaderTextureUnit_Albedo;
    case ShaderSource_Lightmap:
        return ShaderTextureUnit_Lightmap;
    case ShaderSource_Normals:
        return ShaderTextureUnit_Normals;
    case ShaderSource_Specular:
        return ShaderTextureUnit_Specular;
    case ShaderSource_TerrainVertices:
        return ShaderTextureUnit_TerrainVertices;
    case ShaderSource_QuadData:
        return ShaderTextureUnit_QuadData;
    case ShaderSource_BlockMap:
        return ShaderTextureUnit_BlockMap;
    case ShaderSource_ChunkOffsets:
        return ShaderTextureUnit_ChunkOffsets;
    default:
        return -1;
    }
}

bool IsSamplerType(GLenum type)
{
    return type == Gl::SAMPLER_1D || type == Gl::SAMPLER_2D || type == Gl::SAMPLER_3D || type == Gl::SAMPLER_1D_SHADOW
           || type == Gl::SAMPLER_2D_SHADOW || type == Gl::SAMPLER_BUFFER || type == Gl::INT_SAMPLER_2D
           || type == Gl::INT_SAMPLER_BUFFER || type == Gl::UNSIGNED_INT_SAMPLER_2D
           || type == Gl::UNSIGNED_INT_SAMPLER_BUFFER || type == Gl::INT_SAMPLER_1D || type == Gl::INT_SAMPLER_3D
           || type == Gl::UNSIGNED_INT_SAMPLER_1D || type == Gl::UNSIGNED_INT_SAMPLER_3D;
}

bool IsImageType(GLenum type)
{
    return type == Gl::IMAGE_1D || type == Gl::IMAGE_2D || type == Gl::IMAGE_3D || type == Gl::INT_IMAGE_1D
           || type == Gl::INT_IMAGE_2D || type == Gl::INT_IMAGE_3D || type == Gl::UNSIGNED_INT_IMAGE_1D
           || type == Gl::UNSIGNED_INT_IMAGE_2D || type == Gl::UNSIGNED_INT_IMAGE_3D;
}

int ResolveImage(const std::string& name)
{
    if (name.compare(0, 8, "colorimg") == 0 && name.size() > 8)
    {
        int index = std::atoi(name.c_str() + 8);
        if (index >= 0 && index < 16 && std::to_string(index) == name.substr(8))
            return ShaderSource_ColorTex0 + index;
    }
    if (name == "shadowcolorimg0")
        return ShaderSource_ShadowColor0;
    if (name == "shadowcolorimg1")
        return ShaderSource_ShadowColor1;
    return ShaderSource_None;
}

bool ParseConstVector(const std::string& text, const char* type, const char* name, float* values, int count)
{
    size_t position = 0;
    std::string declaration = std::string(type) + " " + name;
    while ((position = text.find(declaration, position)) != std::string::npos)
    {
        size_t start = position;
        position += declaration.size();
        if ((start > 0 && IsIdentifierChar(text[start - 1])) || (position < text.size() && IsIdentifierChar(text[position])))
            continue;
        size_t open = text.find('(', position);
        size_t close = open == std::string::npos ? open : text.find(')', open);
        size_t end = text.find(';', position);
        if (open == std::string::npos || close == std::string::npos || end == std::string::npos || close > end)
            continue;
        std::string inner = text.substr(open + 1, close - open - 1);
        int parsed = 0;
        size_t cursor = 0;
        while (parsed < count && cursor <= inner.size())
        {
            size_t comma = inner.find(',', cursor);
            std::string part = Trim(inner.substr(cursor, comma == std::string::npos ? std::string::npos : comma - cursor));
            if (part.empty())
                break;
            values[parsed++] = static_cast<float>(std::atof(part.c_str()));
            if (comma == std::string::npos)
                break;
            cursor = comma + 1;
        }
        if (parsed == 1)
        {
            for (int index = 1; index < count; ++index)
                values[index] = values[0];
            parsed = count;
        }
        if (parsed == count)
            return true;
    }
    return false;
}

std::string ReadLog(GLuint object, bool program)
{
    GLint length = 0;
    if (program)
        Gl::GetProgramiv(object, Gl::INFO_LOG_LENGTH, &length);
    else
        Gl::GetShaderiv(object, Gl::INFO_LOG_LENGTH, &length);
    if (length <= 1)
        return std::string();
    std::vector<GLchar> log(static_cast<size_t>(length));
    if (program)
        Gl::GetProgramInfoLog(object, length, nullptr, &log[0]);
    else
        Gl::GetShaderInfoLog(object, length, nullptr, &log[0]);
    return Trim(std::string(&log[0]));
}

// LegacyEX: NVIDIA calls clamp(float, float, int) and the like ambiguous from GLSL 4.00 on (an int converts to
// float or to double), where other drivers take the float version; Rethinking Voxels writes clamp(x, 0.6, 1).
// FloatLiteralArgs makes the whole-number literals in such calls floats, but only in calls that already hold a
// float literal, so calls on ints stay as they are. CompileShader tries it only after an "ambiguous" error.
const char* MIXED_CALLS[] = {"clamp", "min", "max", "mix", "step", "smoothstep"};
const int MIXED_CALL_COUNT = (int)(sizeof(MIXED_CALLS) / sizeof(MIXED_CALLS[0]));

bool IsNameChar(char c)
{
    return std::isalnum((unsigned char)c) || c == '_';
}

// a float literal somewhere in the text (1.5, .5, 2.), not a member access such as dir.y
bool HasFloatLiteral(const std::string& text)
{
    for (size_t i = 0; i < text.size(); i++)
    {
        if (text[i] != '.')
            continue;
        bool digitAfter = i + 1 < text.size() && std::isdigit((unsigned char)text[i + 1]);
        size_t start = i;
        while (start > 0 && std::isdigit((unsigned char)text[start - 1]))
            start--;
        bool digitBefore = start < i;
        bool nameBefore = start > 0 && IsNameChar(text[start - 1]);
        if ((digitAfter || digitBefore) && !nameBefore)
            return true;
    }
    return false;
}

// the argument without what is inside its brackets: a float literal in max(int(x * 2.5), 0) says nothing of
// max's own type, only one at the argument's top level does
std::string TopLevel(const std::string& arg)
{
    std::string out;
    int depth = 0;
    for (size_t i = 0; i < arg.size(); i++)
    {
        char ch = arg[i];
        if (ch == '(' || ch == '[')
            depth++;
        else if ((ch == ')' || ch == ']') && depth > 0)
            depth--;
        else if (depth == 0)
            out += ch;
    }
    return out;
}

// a bare whole number such as 1 or -2, spaces round it allowed
bool IsIntLiteral(const std::string& text)
{
    size_t a = text.find_first_not_of(" \t\r\n");
    size_t b = text.find_last_not_of(" \t\r\n");
    if (a == std::string::npos)
        return false;
    if (text[a] == '-' || text[a] == '+')
        a++;
    if (a > b)
        return false;
    for (size_t i = a; i <= b; i++)
    {
        if (!std::isdigit((unsigned char)text[i]))
            return false;
    }
    return true;
}

std::string FloatLiteralArgs(const std::string& text)
{
    std::string out = text;
    size_t i = 0;
    while (i < out.size())
    {
        int call = -1;
        if (i == 0 || !IsNameChar(out[i - 1]))
        {
            for (int c = 0; c < MIXED_CALL_COUNT && call < 0; c++)
            {
                size_t len = std::strlen(MIXED_CALLS[c]);
                if (out.compare(i, len, MIXED_CALLS[c]) == 0 && (i + len >= out.size() || !IsNameChar(out[i + len])))
                    call = c;
            }
        }
        if (call < 0)
        {
            i++;
            continue;
        }
        size_t open = out.find_first_not_of(" \t", i + std::strlen(MIXED_CALLS[call]));
        if (open == std::string::npos || out[open] != '(')
        {
            i++;
            continue;
        }
        // the call's arguments, split at the commas outside brackets
        std::vector<std::string> args;
        size_t argStart = open + 1;
        int depth = 0;
        size_t close = std::string::npos;
        for (size_t k = open + 1; k < out.size(); k++)
        {
            char ch = out[k];
            if (ch == '(' || ch == '[')
                depth++;
            else if ((ch == ')' || ch == ']') && depth > 0)
                depth--;
            else if (ch == ')')
            {
                args.push_back(out.substr(argStart, k - argStart));
                close = k;
                break;
            }
            else if (ch == ',' && depth == 0)
            {
                args.push_back(out.substr(argStart, k - argStart));
                argStart = k + 1;
            }
        }
        if (close == std::string::npos)
            break;
        bool hasFloat = false;
        for (size_t a = 0; a < args.size(); a++)
            hasFloat = hasFloat || HasFloatLiteral(TopLevel(args[a]));
        if (hasFloat)
        {
            std::string joined;
            for (size_t a = 0; a < args.size(); a++)
            {
                std::string arg = args[a];
                if (IsIntLiteral(arg))
                {
                    size_t last = arg.find_last_not_of(" \t\r\n");
                    arg.insert(last + 1, ".0");
                }
                joined += (a > 0 ? "," : "") + arg;
            }
            out.replace(open + 1, close - open - 1, joined);
        }
        // go on inside the call, so calls within its arguments get the same
        i = open + 1;
    }
    return out;
}

GLuint CompileShader(GLenum type, const std::string& text, std::string& log)
{
    std::string source = text;
    std::string stripped = GlslTranslator::StripUnusedFunctions(text);
    for (int attempt = 0; attempt < 3; ++attempt)
    {
        if (attempt == 1)
        {
            if (stripped == text)
                continue;
            source = stripped;
        }
        if (attempt == 2)
        {
            if (log.find("ambiguous") == std::string::npos)
                break;
            source = FloatLiteralArgs(stripped);
            if (source == stripped)
                break;
        }
        GLuint shader = Gl::CreateShader(type);
        const GLchar* pointer = source.c_str();
        Gl::ShaderSource(shader, 1, &pointer, nullptr);
        Gl::CompileShader(shader);
        GLint status = 0;
        Gl::GetShaderiv(shader, Gl::COMPILE_STATUS, &status);
        if (status != 0)
            return shader;
        log = ReadLog(shader, false);
        Gl::DeleteShader(shader);
    }
    return 0;
}
}

const char* ShaderProgramSet::GbufferName(int program)
{
    return kGbuffers[program].name;
}

int ShaderProgramSet::GbufferFallback(int program)
{
    return kGbuffers[program].fallback;
}

const char* ShaderProgramSet::PassFamilyName(int family)
{
    return kPassFamilies[family];
}

void ShaderProgramSet::SetLookups(CustomSamplerLookup samplers, CustomUniformLookup uniforms, void* user)
{
    mSamplerLookup = samplers;
    mUniformLookup = uniforms;
    mLookupUser = user;
}

bool ShaderProgramSet::Load(const ShaderPack& pack, const ShaderPackOptions& options,
                            const ShaderPackProperties& properties, const ShaderMacros& macros,
                            const std::string& world, const std::vector<std::string>& disabled,
                            std::vector<std::string>& errors)
{
    Destroy();
    mMacros = macros;

    mMacroText.clear();
    for (const auto& macro : macros)
        mMacroText += "#define " + macro.first + (macro.second.empty() ? "" : " " + macro.second) + "\n";

    ShaderMacros enableMacros = macros;
    for (const auto& option : options.Macros())
        enableMacros[option.first] = option.second.empty() ? "1" : option.second;

    auto create = [&](const std::string& name, bool composite) -> ShaderProgramSource* {
        std::unique_ptr<ShaderProgramSource> program(new ShaderProgramSource);
        program->name = name;
        std::string worldKey = "program." + world + "/" + name + ".enabled";
        std::string plainKey = "program." + name + ".enabled";
        if (!world.empty() && properties.Has(worldKey))
            program->enabled = EvaluateEnabled(properties.Get(worldKey), enableMacros);
        else if (properties.Has(plainKey))
            program->enabled = EvaluateEnabled(properties.Get(plainKey), enableMacros);
        for (const std::string& name : disabled)
        {
            if (name == program->name || (!world.empty() && name == world + "/" + program->name))
                program->enabled = false;
        }
        if (!LoadProgram(pack, options, macros, world, *program, composite) && !program->error.empty())
            errors.push_back(name + ": " + program->error);
        mStorage.push_back(std::move(program));
        return mStorage.back().get();
    };

    for (int index = 0; index < ShaderGbuffer_Count; ++index)
    {
        if (index >= ShaderGbuffer_Shadow)
            continue;
        mGbuffers[index] = create(kGbuffers[index].name, false);
    }
    for (int index = ShaderGbuffer_Shadow; index < ShaderGbuffer_Count; ++index)
        mGbuffers[index] = create(kGbuffers[index].name, false);

    for (int family = 0; family < ShaderPass_FamilyCount; ++family)
    {
        int count = family == ShaderPass_Final ? 1 : 100;
        for (int index = 0; index < count; ++index)
        {
            std::string name = kPassFamilies[family];
            if (index > 0)
                name += std::to_string(index);
            ShaderProgramSource* program = create(name, true);
            if (family == ShaderPass_Setup)
                program->present = false;
            if (program->enabled && family != ShaderPass_Final)
                LoadCompute(pack, options, macros, world, *program);
            if ((program->present || !program->compute.empty()) && program->enabled)
                mPasses[family].push_back(program);
        }
    }
    return true;
}

void ShaderProgramSet::Destroy()
{
    for (const std::unique_ptr<ShaderProgramSource>& program : mStorage)
    {
        for (ShaderLinkedProgram& linked : program->linked)
        {
            if (linked.name != 0 && Gl::DeleteProgram != nullptr)
                Gl::DeleteProgram(linked.name);
            linked = ShaderLinkedProgram();
        }
        for (ShaderComputeSource& compute : program->compute)
        {
            if (compute.linked.name != 0 && Gl::DeleteProgram != nullptr)
                Gl::DeleteProgram(compute.linked.name);
            compute.linked = ShaderLinkedProgram();
        }
    }
    mStorage.clear();
    for (ShaderProgramSource*& program : mGbuffers)
        program = nullptr;
    for (std::vector<ShaderProgramSource*>& family : mPasses)
        family.clear();
    mDirectives.clear();
}

bool ShaderProgramSet::LoadProgram(const ShaderPack& pack, const ShaderPackOptions& options,
                                   const ShaderMacros& macros, const std::string& world,
                                   ShaderProgramSource& program, bool composite)
{
    std::string folder;
    if (!world.empty()
        && (pack.HasFile(world + "/" + program.name + ".vsh") || pack.HasFile(world + "/" + program.name + ".fsh")))
        folder = world + "/";

    for (int stage = 0; stage < ShaderStage_Count; ++stage)
    {
        std::string path = folder + program.name + kStageExtensions[stage];
        if (pack.HasFile(path))
            program.path[stage] = path;
    }
    program.present = !program.path[ShaderStage_Vertex].empty() && !program.path[ShaderStage_Fragment].empty();
    if (!program.present || !program.enabled)
        return false;

    for (int stage = 0; stage < ShaderStage_Count; ++stage)
    {
        if (program.path[stage].empty())
            continue;
        ShaderPack::Source source;
        std::string error;
        if (!pack.LoadSource(program.path[stage], source, true, error))
        {
            program.error = program.path[stage] + ": " + error;
            program.present = false;
            return false;
        }
        std::string text = ShaderPreprocessConditionals(options.ApplyToSource(source.text), macros, true);
        program.files[stage] = source.files;
        if (!program.translator[stage].Parse(static_cast<ShaderStage>(stage), text, error))
        {
            program.error = program.path[stage] + ": " + error;
            program.present = false;
            return false;
        }
        ScanDirectives(text, program, static_cast<ShaderStage>(stage));
    }

    const GlslUsage& vertex = program.translator[ShaderStage_Vertex].Usage();
    const GlslUsage& fragment = program.translator[ShaderStage_Fragment].Usage();
    program.linkedUsage = vertex;
    program.linkedUsage.builtins.insert(fragment.builtins.begin(), fragment.builtins.end());
    if (fragment.texCoordCount > program.linkedUsage.texCoordCount)
        program.linkedUsage.texCoordCount = fragment.texCoordCount;

    if (program.drawBuffers.empty())
    {
        int outputs = fragment.outputLocations;
        if (outputs == 0)
            outputs = 1;
        for (int buffer = 0; buffer < outputs && buffer < 16; ++buffer)
            program.drawBuffers.push_back(buffer);
    }
    return true;
}

void ShaderProgramSet::LoadCompute(const ShaderPack& pack, const ShaderPackOptions& options,
                                   const ShaderMacros& macros, const std::string& world, ShaderProgramSource& program)
{
    if (!mComputeSupported)
        return;
    for (int suffix = -1; suffix < 26; ++suffix)
    {
        std::string name = program.name;
        if (suffix >= 0)
            name += std::string("_") + static_cast<char>('a' + suffix);
        std::string path = name + ".csh";
        if (!world.empty() && pack.HasFile(world + "/" + path))
            path = world + "/" + path;
        else if (!pack.HasFile(path))
            continue;

        ShaderComputeSource compute;
        compute.path = path;
        ShaderPack::Source source;
        std::string error;
        if (!pack.LoadSource(path, source, true, error))
        {
            program.error = path + ": " + error;
            continue;
        }
        std::string text = ShaderPreprocessConditionals(options.ApplyToSource(source.text), macros, true);
        compute.files = source.files;

        size_t line = 0;
        while (line < text.size())
        {
            size_t end = text.find('\n', line);
            if (end == std::string::npos)
                end = text.size();
            size_t first = text.find_first_not_of(" \t", line);
            if (first < end && text.compare(first, 8, "#version") == 0)
            {
                // the line stays: the translator reads the pack's version from it (LinkCompute)
                compute.version = std::max(430, std::atoi(text.c_str() + first + 8));
                break;
            }
            line = end + 1;
        }

        int groups[3] = {0, 0, 0};
        float groupValues[3] = {};
        if (ParseConstVector(text, "ivec3", "workGroups", groupValues, 3))
        {
            compute.absolute = true;
            for (int axis = 0; axis < 3; ++axis)
                groups[axis] = std::max(1, static_cast<int>(groupValues[axis]));
            std::copy(groups, groups + 3, compute.workGroups);
        }
        float render[2] = {1.0f, 1.0f};
        if (!compute.absolute && ParseConstVector(text, "vec2", "workGroupsRender", render, 2))
        {
            compute.workGroupsRender[0] = render[0];
            compute.workGroupsRender[1] = render[1];
        }
        compute.text = text;
        program.compute.push_back(std::move(compute));
    }
}

const ShaderLinkedProgram* ShaderProgramSet::LinkCompute(ShaderProgramSource& program, ShaderComputeSource& compute,
                                                         std::string& error)
{
    ShaderLinkedProgram& linked = compute.linked;
    if (compute.attempted)
        return linked.name != 0 ? &linked : nullptr;
    compute.attempted = true;
    if (compute.version > mGlslVersion)
    {
        error = compute.path + " needs GLSL " + std::to_string(compute.version) + " but the driver supports "
                + std::to_string(mGlslVersion);
        return nullptr;
    }

    // compute shaders get the same translation as the other programs (shadow2D, texture2DLod, gl_Fog and the
    // like are gone in the core profile they compile in); the geometry stage is the one without vertex inputs or
    // fragment outputs to rewrite. The pack's main runs from a main of our own, as in the other stages.
    GlslTranslator translator;
    std::string parseError;
    if (!translator.Parse(ShaderStage_Geometry, compute.text, parseError))
    {
        error = compute.path + ": " + parseError;
        return nullptr;
    }
    std::string header = mMacroText + ShaderAdapterHeader(ShaderAdapter_Quad, ShaderStage_Geometry, translator.Usage());
    std::string text = translator.Emit(compute.version, header, "void main()\n{\n    lce_pack_main();\n}\n",
                                       translator.Usage());
    std::string log;
    GLuint shader = CompileShader(Gl::COMPUTE_SHADER, text, log);
    if (shader == 0)
    {
        error = compute.path + " failed to compile:\n" + MapLog(log, compute.files);
        return nullptr;
    }
    GLint status = 0;
    GLuint name = Gl::CreateProgram();
    Gl::AttachShader(name, shader);
    Gl::LinkProgram(name);
    Gl::DeleteShader(shader);
    Gl::GetProgramiv(name, Gl::LINK_STATUS, &status);
    if (status == 0)
    {
        error = compute.path + " failed to link:\n" + ReadLog(name, true);
        Gl::DeleteProgram(name);
        return nullptr;
    }
    Gl::GetProgramiv(name, Gl::COMPUTE_WORK_GROUP_SIZE, compute.localSize);
    for (int& size : compute.localSize)
        size = std::max(1, size);
    linked.name = name;
    BindProgram(linked, program, true);
    return &linked;
}

void ShaderProgramSet::ScanDirectives(const std::string& text, ShaderProgramSource& program, ShaderStage stage)
{
    if (stage == ShaderStage_Fragment)
    {
        size_t position = 0;
        std::vector<int> buffers;
        bool found = false;
        for (;;)
        {
            size_t draw = text.find("DRAWBUFFERS:", position);
            size_t targets = text.find("RENDERTARGETS:", position);
            if (draw == std::string::npos && targets == std::string::npos)
                break;
            buffers.clear();
            found = true;
            if (draw != std::string::npos && (targets == std::string::npos || draw < targets))
            {
                size_t cursor = draw + 12;
                while (cursor < text.size() && (text[cursor] == ' ' || text[cursor] == '\t'))
                    ++cursor;
                while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])))
                    buffers.push_back(text[cursor++] - '0');
                position = cursor;
            }
            else
            {
                size_t cursor = targets + 14;
                for (;;)
                {
                    while (cursor < text.size() && (text[cursor] == ' ' || text[cursor] == '\t'))
                        ++cursor;
                    if (cursor >= text.size() || !std::isdigit(static_cast<unsigned char>(text[cursor])))
                        break;
                    buffers.push_back(std::atoi(text.c_str() + cursor));
                    while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])))
                        ++cursor;
                    while (cursor < text.size() && (text[cursor] == ' ' || text[cursor] == '\t'))
                        ++cursor;
                    if (cursor < text.size() && text[cursor] == ',')
                        ++cursor;
                    else
                        break;
                }
                position = cursor;
            }
        }
        if (found)
        {
            program.drawBuffers.clear();
            for (int buffer : buffers)
            {
                if (buffer >= 0 && buffer < 16)
                    program.drawBuffers.push_back(buffer);
            }
        }
    }

    size_t position = 0;
    while ((position = text.find("const", position)) != std::string::npos)
    {
        size_t start = position;
        position += 5;
        if ((start > 0 && IsIdentifierChar(text[start - 1])) || (position < text.size() && IsIdentifierChar(text[position])))
            continue;
        size_t cursor = position;
        auto skip = [&]() {
            while (cursor < text.size() && (text[cursor] == ' ' || text[cursor] == '\t'))
                ++cursor;
        };
        auto word = [&]() {
            skip();
            size_t begin = cursor;
            while (cursor < text.size() && IsIdentifierChar(text[cursor]))
                ++cursor;
            return text.substr(begin, cursor - begin);
        };
        std::string type = word();
        if (type != "int" && type != "float" && type != "bool" && type != "vec2" && type != "vec3" && type != "vec4")
            continue;
        std::string name = word();
        skip();
        if (name.empty() || cursor >= text.size() || text[cursor] != '=')
            continue;
        size_t end = text.find_first_of(";\n", cursor + 1);
        if (end == std::string::npos || text[end] != ';')
            continue;
        mDirectives[name] = Trim(text.substr(cursor + 1, end - cursor - 1));
        position = end;
    }

    static const char* const kLegacy[][2] = {
        {"SHADOWRES:", "shadowMapResolution"}, {"SHADOWHPL:", "shadowDistance"}, {"SHADOWFOV:", "shadowMapFov"},
        {"GAUX1FORMAT:", "gaux1Format"},       {"GAUX2FORMAT:", "gaux2Format"},  {"GAUX3FORMAT:", "gaux3Format"},
        {"GAUX4FORMAT:", "gaux4Format"},       {"GCOLORFORMAT:", "gcolorFormat"}, {"GDEPTHFORMAT:", "gdepthFormat"},
        {"GNORMALFORMAT:", "gnormalFormat"},   {"COMPOSITEFORMAT:", "compositeFormat"},
    };
    for (const auto& legacy : kLegacy)
    {
        size_t found = text.rfind(legacy[0]);
        if (found == std::string::npos)
            continue;
        size_t cursor = found + std::strlen(legacy[0]);
        size_t end = cursor;
        while (end < text.size() && (IsIdentifierChar(text[end]) || text[end] == '.'))
            ++end;
        if (end > cursor)
            mDirectives[legacy[1]] = text.substr(cursor, end - cursor);
    }
}

std::string ShaderProgramSet::Directive(const std::string& name, const std::string& fallback) const
{
    auto found = mDirectives.find(name);
    return found == mDirectives.end() ? fallback : found->second;
}

ShaderProgramSource* ShaderProgramSet::Gbuffer(int program) const
{
    return mGbuffers[program];
}

ShaderProgramSource* ShaderProgramSet::ResolveGbuffer(int program) const
{
    while (program >= 0)
    {
        ShaderProgramSource* source = mGbuffers[program];
        if (source != nullptr && source->present && source->enabled)
            return source;
        program = kGbuffers[program].fallback;
    }
    return nullptr;
}

bool ShaderProgramSet::IsComposite(const ShaderProgramSource& program)
{
    const std::string& name = program.name;
    for (const char* family : kPassFamilies)
    {
        size_t length = std::strlen(family);
        if (name.compare(0, length, family) != 0)
            continue;
        bool digits = true;
        for (size_t position = length; position < name.size(); ++position)
            digits = digits && std::isdigit(static_cast<unsigned char>(name[position]));
        if (digits)
            return true;
    }
    return name == "lce_final";
}

bool ShaderProgramSet::LoadInline(ShaderProgramSource& program, const std::string& vertex,
                                  const std::string& fragment, std::string& error)
{
    const std::string* sources[ShaderStage_Count] = {&vertex, nullptr, &fragment};
    program.present = true;
    for (int stage = 0; stage < ShaderStage_Count; ++stage)
    {
        if (sources[stage] == nullptr)
            continue;
        program.path[stage] = "<" + program.name + kStageExtensions[stage] + ">";
        program.files[stage] = std::vector<std::string>(1, program.path[stage]);
        std::string text = ShaderPreprocessConditionals(*sources[stage], mMacros, true);
        if (!program.translator[stage].Parse(static_cast<ShaderStage>(stage), text, error))
        {
            program.present = false;
            return false;
        }
    }
    program.linkedUsage = program.translator[ShaderStage_Vertex].Usage();
    const GlslUsage& fragmentUsage = program.translator[ShaderStage_Fragment].Usage();
    program.linkedUsage.builtins.insert(fragmentUsage.builtins.begin(), fragmentUsage.builtins.end());
    program.drawBuffers.assign(1, 0);
    return true;
}

std::string ShaderProgramSet::MapLog(const std::string& log, const std::vector<std::string>& files) const
{
    std::string out;
    size_t start = 0;
    while (start < log.size())
    {
        size_t end = log.find('\n', start);
        if (end == std::string::npos)
            end = log.size();
        std::string line = log.substr(start, end - start);
        start = end + 1;

        size_t cursor = 0;
        if (line.compare(0, 7, "ERROR: ") == 0)
            cursor = 7;
        else if (line.compare(0, 9, "WARNING: ") == 0)
            cursor = 9;
        size_t digits = cursor;
        while (digits < line.size() && std::isdigit(static_cast<unsigned char>(line[digits])))
            ++digits;
        if (digits > cursor && digits < line.size() && (line[digits] == ':' || line[digits] == '('))
        {
            int source = std::atoi(line.c_str() + cursor);
            size_t lineStart = digits + 1;
            size_t lineEnd = lineStart;
            while (lineEnd < line.size() && std::isdigit(static_cast<unsigned char>(line[lineEnd])))
                ++lineEnd;
            if (lineEnd > lineStart)
            {
                std::string file = source == GlslTranslator::kHeaderSourceIndex   ? "<generated header>"
                                   : source == GlslTranslator::kFooterSourceIndex ? "<generated footer>"
                                   : source >= 0 && source < static_cast<int>(files.size())
                                       ? files[static_cast<size_t>(source)]
                                       : "<source " + std::to_string(source) + ">";
                size_t rest = lineEnd;
                if (rest < line.size() && line[rest] == ')')
                    ++rest;
                if (rest < line.size() && line[rest] == '(')
                {
                    size_t close = line.find(')', rest);
                    rest = close == std::string::npos ? rest : close + 1;
                }
                line = line.substr(0, cursor) + file + ":" + line.substr(lineStart, lineEnd - lineStart)
                       + line.substr(rest);
            }
        }
        out += line + "\n";
    }
    return Trim(out);
}

const ShaderLinkedProgram* ShaderProgramSet::Link(ShaderProgramSource& program, ShaderAdapter adapter,
                                                  std::string& error)
{
    ShaderLinkedProgram& linked = program.linked[adapter];
    if (program.attempted[adapter])
        return linked.name != 0 ? &linked : nullptr;
    program.attempted[adapter] = true;

    if (!program.present || !program.enabled)
        return nullptr;

    int version = 330;
    for (int stage = 0; stage < ShaderStage_Count; ++stage)
    {
        if (!program.path[stage].empty() && program.translator[stage].Version() > version)
            version = program.translator[stage].Version();
    }
    if (version > mGlslVersion)
    {
        error = program.name + " needs GLSL " + std::to_string(version) + " but the driver supports "
                + std::to_string(mGlslVersion);
        return nullptr;
    }

    int emitVersion = version;
    if (version == 400)
    {
        emitVersion = 330;
        for (int stage = 0; stage < ShaderStage_Count; ++stage)
        {
            if (!program.path[stage].empty() && program.translator[stage].UsesDoubles())
                emitVersion = version;
        }
    }

    const std::string& output0 = program.translator[ShaderStage_Fragment].Usage().output0;
    GLuint shaders[ShaderStage_Count] = {};
    std::string vertexText;
    bool compiled = true;
    for (int stage = 0; stage < ShaderStage_Count && compiled; ++stage)
    {
        if (program.path[stage].empty())
            continue;
        const GlslTranslator& translator = program.translator[stage];
        std::string header = mMacroText + ShaderAdapterHeader(adapter, static_cast<ShaderStage>(stage), translator.Usage());
        std::string footer = ShaderAdapterFooter(adapter, static_cast<ShaderStage>(stage), output0);
        std::string text = translator.Emit(emitVersion, header, footer, program.linkedUsage);
        if (emitVersion != version)
            text.insert(text.find('\n') + 1, kVersion400Extensions);

        std::string log;
        GLuint shader = CompileShader(kStageTypes[stage], text, log);
        if (stage == ShaderStage_Vertex)
            vertexText = text;
        if (shader == 0 && stage == ShaderStage_Fragment && !vertexText.empty())
        {
            std::string fixed = GlslTranslator::RequalifyDeclarations(
                text, "out", "in", GlslTranslator::DeclaredNames(vertexText, "out"));
            std::string retryLog;
            if (fixed != text)
                shader = CompileShader(kStageTypes[stage], fixed, retryLog);        }
        if (shader == 0)
        {
            error = program.path[stage] + " (" + ShaderAdapterName(adapter) + ") failed to compile:\n"
                    + MapLog(log, program.files[stage]);
            compiled = false;
            break;
        }
        shaders[stage] = shader;
    }

    GLuint name = 0;
    if (compiled)
    {
        name = Gl::CreateProgram();
        for (GLuint shader : shaders)
        {
            if (shader != 0)
                Gl::AttachShader(name, shader);
        }
        Gl::LinkProgram(name);
        GLint status = 0;
        Gl::GetProgramiv(name, Gl::LINK_STATUS, &status);
        if (status == 0)
        {
            error = program.name + " (" + ShaderAdapterName(adapter) + ") failed to link:\n" + ReadLog(name, true);
            Gl::DeleteProgram(name);
            name = 0;
        }
    }
    for (GLuint shader : shaders)
    {
        if (shader != 0)
            Gl::DeleteShader(shader);
    }
    if (name == 0)
        return nullptr;

    linked.name = name;
    BindProgram(linked, program, IsComposite(program));
    return &linked;
}

void ShaderProgramSet::BindProgram(ShaderLinkedProgram& linked, const ShaderProgramSource& program, bool composite)
{
    GLuint name = linked.name;
    // LegacyEX: this renderer's register blocks and their bindings (PcRenderer.cpp GetProgram)
    GLuint vertexBlock = Gl::GetUniformBlockIndex(name, "VertexRegisters");
    if (vertexBlock != Gl::INVALID_INDEX)
        Gl::UniformBlockBinding(name, vertexBlock, 0);
    GLuint pixelBlock = Gl::GetUniformBlockIndex(name, "PixelRegisters");
    if (pixelBlock != Gl::INVALID_INDEX)
        Gl::UniformBlockBinding(name, pixelBlock, 1);
    GLuint boneBlock = Gl::GetUniformBlockIndex(name, "BoneRegisters");
    if (boneBlock != Gl::INVALID_INDEX)
        Gl::UniformBlockBinding(name, boneBlock, 2);

    bool watershadow = false;
    GLint count = 0;
    GLint maxLength = 0;
    Gl::GetProgramiv(name, Gl::ACTIVE_UNIFORMS, &count);
    Gl::GetProgramiv(name, Gl::ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);
    std::vector<GLchar> buffer(static_cast<size_t>(maxLength > 0 ? maxLength + 1 : 256));

    struct Active
    {
        std::string name;
        GLenum type;
        GLint location;
    };
    std::vector<Active> uniforms;
    for (GLint index = 0; index < count; ++index)
    {
        GLsizei length = 0;
        GLint size = 0;
        GLenum type = 0;
        Gl::GetActiveUniform(name, static_cast<GLuint>(index), static_cast<GLsizei>(buffer.size()), &length, &size,
                             &type, &buffer[0]);
        std::string uniform(&buffer[0], static_cast<size_t>(length));
        GLint location = Gl::GetUniformLocation(name, uniform.c_str());
        if (location < 0)
            continue;
        if (uniform.size() > 3 && uniform.compare(uniform.size() - 3, 3, "[0]") == 0
            && uniform.find('.') == std::string::npos)
            uniform.resize(uniform.size() - 3);
        if (uniform == "watershadow")
            watershadow = true;
        uniforms.push_back(Active{uniform, type, location});
    }

    Gl::UseProgram(name);
    int nextUnit = ShaderTextureUnit_FirstPack;
    GLuint nextImage = 0;
    for (const Active& uniform : uniforms)
    {
        if (IsImageType(uniform.type))
        {
            ShaderLinkedProgram::Image image;
            image.custom = mImageLookup != nullptr ? mImageLookup(uniform.name, mLookupUser) : -1;
            image.source = image.custom >= 0 ? static_cast<int>(ShaderSource_Custom) : ResolveImage(uniform.name);
            if (image.source == ShaderSource_None)
                continue;
            image.unit = nextImage++;
            Gl::Uniform1i(uniform.location, static_cast<GLint>(image.unit));
            linked.images.push_back(image);
            continue;
        }
        if (IsSamplerType(uniform.type))
        {
            ShaderLinkedProgram::Sampler sampler;
            sampler.custom = -1;
            sampler.source = ShaderSource_None;
            if (mSamplerLookup != nullptr)
                sampler.custom = mSamplerLookup(uniform.name, program.name, uniform.type, mLookupUser);
            if (sampler.custom >= 0)
                sampler.source = ShaderSource_Custom;
            else
                sampler.source = ResolveSampler(uniform.name, composite, watershadow);
            if (sampler.source == ShaderSource_None)
                continue;
            int unit = sampler.source == ShaderSource_Custom ? -1 : FixedUnit(sampler.source);
            if (unit < 0)
                unit = nextUnit++;
            sampler.unit = unit;
            sampler.shadow = uniform.type == Gl::SAMPLER_2D_SHADOW || uniform.type == Gl::SAMPLER_1D_SHADOW;
            sampler.integer = uniform.type == Gl::INT_SAMPLER_2D || uniform.type == Gl::UNSIGNED_INT_SAMPLER_2D;
            Gl::Uniform1i(uniform.location, unit);
            if (FixedUnit(sampler.source) < 0 || sampler.source == ShaderSource_Custom)
                linked.samplers.push_back(sampler);
            else if (sampler.source != ShaderSource_Albedo
                     && (composite || sampler.source != ShaderSource_Lightmap))
                linked.samplers.push_back(sampler);
            continue;
        }

        int id = ShaderFindUniform(uniform.name.c_str());
        if (id >= 0)
        {
            ShaderLinkedProgram::Uniform binding{id, uniform.location};
            if (ShaderUniformCatalogue(id).perDraw)
                linked.drawUniforms.push_back(binding);
            else
                linked.frameUniforms.push_back(binding);
            continue;
        }
        if (mUniformLookup != nullptr)
        {
            int custom = mUniformLookup(uniform.name, mLookupUser);
            if (custom >= 0)
                linked.customUniforms.push_back(std::make_pair(custom, uniform.location));
        }
    }
    linked.drawCache.assign(linked.drawUniforms.size() * 16, 3.4e38f);
    Gl::UseProgram(0);
}

int ShaderProgramSet::CompileAll(std::vector<std::string>& errors)
{
    static const ShaderAdapter kDefaults[ShaderGbuffer_Count][3] = {
        {ShaderAdapter_Standard, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Standard, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Standard, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Standard, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Standard, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Standard, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Terrain, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Terrain, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Terrain, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Terrain, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Standard, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Entity, ShaderAdapter_Standard, ShaderAdapter_Count},
        {ShaderAdapter_Standard, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Standard, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Entity, ShaderAdapter_Standard, ShaderAdapter_Count},
        {ShaderAdapter_Entity, ShaderAdapter_Standard, ShaderAdapter_Count},
        {ShaderAdapter_TexGen, ShaderAdapter_Entity, ShaderAdapter_Count},
        {ShaderAdapter_Entity, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Entity, ShaderAdapter_Standard, ShaderAdapter_Count},
        {ShaderAdapter_Standard, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Terrain, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Entity, ShaderAdapter_Standard, ShaderAdapter_Count},
        {ShaderAdapter_Terrain, ShaderAdapter_Entity, ShaderAdapter_Standard},
        {ShaderAdapter_Terrain, ShaderAdapter_Count, ShaderAdapter_Count},
        {ShaderAdapter_Terrain, ShaderAdapter_Count, ShaderAdapter_Count},
    };

    int linkedCount = 0;
    for (int program = 0; program < ShaderGbuffer_Count; ++program)
    {
        ShaderProgramSource* source = ResolveGbuffer(program);
        if (source == nullptr)
            continue;
        for (ShaderAdapter adapter : kDefaults[program])
        {
            if (adapter == ShaderAdapter_Count || source->attempted[adapter])
                continue;
            std::string error;
            if (Link(*source, adapter, error) != nullptr)
                ++linkedCount;
            else if (!error.empty())
                errors.push_back(error);
        }
    }
    for (const std::vector<ShaderProgramSource*>& family : mPasses)
    {
        for (ShaderProgramSource* pass : family)
        {
            std::string error;
            for (ShaderComputeSource& compute : pass->compute)
            {
                error.clear();
                if (LinkCompute(*pass, compute, error) != nullptr)
                    ++linkedCount;
                else if (!error.empty())
                    errors.push_back(error);
            }
            if (!pass->present)
                continue;
            error.clear();
            if (Link(*pass, ShaderAdapter_Quad, error) != nullptr)
                ++linkedCount;
            else if (!error.empty())
                errors.push_back(error);
        }
    }
    return linkedCount;
}
