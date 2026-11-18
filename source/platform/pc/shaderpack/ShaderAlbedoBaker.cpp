// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderAlbedoBaker.cpp
#include "platform/pc/shaderpack/ShaderAlbedoBaker.hpp"

#include <cstdio>
#include <cstring>
#include <string>

#include "platform/pc/shaderpack/Log.hpp"

static const Logger sLog("Shaders");

namespace
{
const unsigned long long kUnusedFrames = 600;
const size_t kMaxEntries = 512;
const int kMaxSize = 4096;

const char* const kVertexSource = R"(#version 330 core
out vec2 vUv;
void main()
{
    vec2 corner = vec2(float(gl_VertexID & 1), float(gl_VertexID >> 1));
    vUv = corner;
    gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);
}
)";

const char* const kFragmentSource = R"(#version 330 core
in vec2 vUv;
layout(location = 0) out vec4 oColour;
uniform sampler2D uLayer0;
uniform sampler2D uLayer1;
uniform sampler2D uLayer2;
uniform sampler2D uLayer3;
uniform int uMode;
uniform int uLayers;
uniform vec4 uColour0;
uniform vec4 uColour1;
uniform vec4 uColour2;
uniform vec4 uColour3;
uniform vec4 uDiffuse;
uniform vec4 uMask0;
uniform vec4 uMask1;
uniform vec4 uMask2;
uniform vec4 uMask3;
uniform vec4 uMask4;
uniform vec4 uMask5;
uniform vec4 uMask6;
uniform vec4 uMask7;
vec3 Layer(vec3 colour, sampler2D layer, vec4 tint)
{
    vec4 sampled = textureLod(layer, vUv, 0.0);
    return (sampled.rgb * tint.rgb - colour) * sampled.a + colour;
}
vec3 MaskStep(vec3 mask, vec2 scaled, vec2 inverseSize, vec4 data)
{
    float signValue = (data.w > 0.0 ? 1.0 : 0.0) - (0.0 > data.w ? 1.0 : 0.0);
    float whole = signValue * floor(abs(data.w));
    float part = data.w - whole;
    vec2 coordinate = vec2(scaled.x + whole * inverseSize.x, scaled.y + part);
    float amount = textureLod(uLayer1, coordinate, 0.0).r;
    return (data.rgb - mask) * amount + mask;
}
void main()
{
    vec4 base = textureLod(uLayer0, vUv, 0.0);
    if (uMode == 0)
    {
        vec3 colour = base.rgb * uColour0.rgb;
        if (uLayers > 1)
            colour = Layer(colour, uLayer1, uColour1);
        if (uLayers > 2)
            colour = Layer(colour, uLayer2, uColour2);
        if (uLayers > 3)
            colour = Layer(colour, uLayer3, uColour3);
        oColour = vec4(colour, base.a);
    }
    else if (uMode == 1)
    {
        vec2 inverseSize = vec2(uMask0.x == 0.0 ? 0.0 : 1.0 / uMask0.x, uMask0.y == 0.0 ? 0.0 : 1.0 / uMask0.y);
        vec2 scaled = vUv * 64.0 * inverseSize;
        vec3 mask = vec3(1.0);
        mask = MaskStep(mask, scaled, inverseSize, uMask1);
        mask = MaskStep(mask, scaled, inverseSize, uMask2);
        mask = MaskStep(mask, scaled, inverseSize, uMask3);
        mask = MaskStep(mask, scaled, inverseSize, uMask4);
        mask = MaskStep(mask, scaled, inverseSize, uMask5);
        mask = MaskStep(mask, scaled, inverseSize, uMask6);
        mask = MaskStep(mask, scaled, inverseSize, uMask7);
        oColour = vec4(base.rgb * mask, 1.0);
    }
    else if (uMode == 2)
    {
        if (base.a == 0.0)
        {
            oColour = vec4(0.0);
            return;
        }
        vec3 colour = base.rgb * uColour0.rgb;
        colour = (colour * uDiffuse.rgb - colour) * base.a + colour;
        oColour = vec4(colour, 1.0);
    }
    else if (uMode == 3)
    {
        vec3 colour = base.rgb * uColour0.rgb;
        if (((base.a + colour.r) + colour.g) + colour.b == 0.0)
            oColour = vec4(0.0);
        else
            oColour = vec4(colour, base.a);
    }
    else
    {
        if (base.a == 0.0 || base.a == 1.0)
            oColour = vec4(0.0);
        else
            oColour = vec4(base.rgb * uColour0.rgb, base.a);
    }
}
)";

GLuint CompileStage(GLenum type, const char* source)
{
    GLuint shader = Gl::CreateShader(type);
    Gl::ShaderSource(shader, 1, &source, nullptr);
    Gl::CompileShader(shader);
    GLint status = 0;
    Gl::GetShaderiv(shader, Gl::COMPILE_STATUS, &status);
    if (status == 0)
    {
        char log[1024] = {};
        Gl::GetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOG_ERROR(sLog, "albedo bake shader: %s", log);
        Gl::DeleteShader(shader);
        return 0;
    }
    return shader;
}

int LevelCount(int width, int height)
{
    int levels = 1;
    int size = width > height ? width : height;
    while (size > 1)
    {
        size >>= 1;
        ++levels;
    }
    return levels;
}
}

bool ShaderAlbedoBaker::EnsureResources()
{
    if (mProgram != 0)
        return true;
    if (mFailed)
        return false;

    GLuint vertex = CompileStage(Gl::VERTEX_SHADER, kVertexSource);
    GLuint fragment = CompileStage(Gl::FRAGMENT_SHADER, kFragmentSource);
    if (vertex == 0 || fragment == 0)
    {
        if (vertex != 0)
            Gl::DeleteShader(vertex);
        if (fragment != 0)
            Gl::DeleteShader(fragment);
        mFailed = true;
        return false;
    }

    GLuint program = Gl::CreateProgram();
    Gl::AttachShader(program, vertex);
    Gl::AttachShader(program, fragment);
    Gl::LinkProgram(program);
    Gl::DeleteShader(vertex);
    Gl::DeleteShader(fragment);
    GLint status = 0;
    Gl::GetProgramiv(program, Gl::LINK_STATUS, &status);
    if (status == 0)
    {
        char log[1024] = {};
        Gl::GetProgramInfoLog(program, sizeof(log), nullptr, log);
        LOG_ERROR(sLog, "albedo bake program: %s", log);
        Gl::DeleteProgram(program);
        mFailed = true;
        return false;
    }

    mProgram = program;
    Gl::UseProgram(mProgram);
    for (int layer = 0; layer < 4; ++layer)
    {
        char name[32];
        std::snprintf(name, sizeof(name), "uLayer%d", layer);
        Gl::Uniform1i(Gl::GetUniformLocation(mProgram, name), layer);
        std::snprintf(name, sizeof(name), "uColour%d", layer);
        mColourLocations[layer] = Gl::GetUniformLocation(mProgram, name);
    }
    for (int index = 0; index < 8; ++index)
    {
        char name[32];
        std::snprintf(name, sizeof(name), "uMask%d", index);
        mMaskLocations[index] = Gl::GetUniformLocation(mProgram, name);
    }
    mModeLocation = Gl::GetUniformLocation(mProgram, "uMode");
    mLayersLocation = Gl::GetUniformLocation(mProgram, "uLayers");
    mDiffuseLocation = Gl::GetUniformLocation(mProgram, "uDiffuse");

    Gl::GenFramebuffers(1, &mFramebuffer);
    Gl::GenVertexArrays(1, &mVertexArray);
    Gl::GenSamplers(1, &mSampler);
    Gl::SamplerParameteri(mSampler, Gl::TEXTURE_MIN_FILTER, static_cast<GLint>(Gl::NEAREST));
    Gl::SamplerParameteri(mSampler, Gl::TEXTURE_MAG_FILTER, static_cast<GLint>(Gl::NEAREST));
    Gl::SamplerParameteri(mSampler, Gl::TEXTURE_WRAP_S, static_cast<GLint>(Gl::REPEAT));
    Gl::SamplerParameteri(mSampler, Gl::TEXTURE_WRAP_T, static_cast<GLint>(Gl::REPEAT));
    return true;
}

bool ShaderAlbedoBaker::Bake(const ShaderAlbedoRequest& request, Entry& entry)
{
    int width = 1;
    int height = 1;
    int used = request.mode == ShaderAlbedoMode_Layered ? request.layers
               : request.mode == ShaderAlbedoMode_ColourMask ? 2
                                                             : 1;
    for (int layer = 0; layer < used && layer < 4; ++layer)
    {
        if (request.sizes[layer][0] > width)
            width = request.sizes[layer][0];
        if (request.sizes[layer][1] > height)
            height = request.sizes[layer][1];
    }
    width = width > kMaxSize ? kMaxSize : width;
    height = height > kMaxSize ? kMaxSize : height;

    if (entry.texture == 0 || entry.width != width || entry.height != height)
    {
        if (entry.texture == 0)
            Gl::GenTextures(1, &entry.texture);
        Gl::ActiveTexture(Gl::TEXTURE0);
        Gl::BindTexture(Gl::TEXTURE_2D, entry.texture);
        int levels = LevelCount(width, height);
        for (int level = 0; level < levels; ++level)
        {
            int levelWidth = width >> level;
            int levelHeight = height >> level;
            Gl::TexImage2D(Gl::TEXTURE_2D, level, static_cast<GLint>(Gl::RGBA8), levelWidth > 0 ? levelWidth : 1,
                           levelHeight > 0 ? levelHeight : 1, 0, Gl::RGBA, Gl::UNSIGNED_BYTE, nullptr);
        }
        Gl::TexParameteri(Gl::TEXTURE_2D, Gl::TEXTURE_BASE_LEVEL, 0);
        Gl::TexParameteri(Gl::TEXTURE_2D, Gl::TEXTURE_MAX_LEVEL, levels - 1);
        entry.width = width;
        entry.height = height;
    }

    Gl::BindFramebuffer(Gl::FRAMEBUFFER, mFramebuffer);
    Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0, Gl::TEXTURE_2D, entry.texture, 0);
    if (Gl::CheckFramebufferStatus(Gl::FRAMEBUFFER) != Gl::FRAMEBUFFER_COMPLETE)
    {
        Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
        return false;
    }
    GLenum drawBuffer = Gl::COLOR_ATTACHMENT0;
    Gl::DrawBuffers(1, &drawBuffer);

    Gl::Viewport(0, 0, width, height);
    Gl::Disable(Gl::BLEND);
    Gl::Disable(Gl::DEPTH_TEST);
    Gl::Disable(Gl::SCISSOR_TEST);
    Gl::Disable(Gl::CULL_FACE);
    Gl::Disable(Gl::STENCIL_TEST);
    Gl::ColorMask(1, 1, 1, 1);

    Gl::UseProgram(mProgram);
    Gl::Uniform1i(mModeLocation, request.mode);
    Gl::Uniform1i(mLayersLocation, request.layers);
    for (int layer = 0; layer < 4; ++layer)
    {
        const float* colour = request.colours[layer];
        Gl::Uniform4f(mColourLocations[layer], colour[0], colour[1], colour[2], colour[3]);
    }
    Gl::Uniform4f(mDiffuseLocation, request.diffuse[0], request.diffuse[1], request.diffuse[2], request.diffuse[3]);
    for (int index = 0; index < 8; ++index)
    {
        const float* mask = request.mask[index];
        Gl::Uniform4f(mMaskLocations[index], mask[0], mask[1], mask[2], mask[3]);
    }

    for (int layer = 0; layer < 4; ++layer)
    {
        Gl::ActiveTexture(Gl::TEXTURE0 + static_cast<GLenum>(layer));
        Gl::BindTexture(Gl::TEXTURE_2D, request.textures[layer]);
        Gl::BindSampler(static_cast<GLuint>(layer), mSampler);
    }

    Gl::BindVertexArray(mVertexArray);
    Gl::DrawArrays(Gl::TRIANGLE_STRIP, 0, 4);
    Gl::BindVertexArray(0);

    for (int layer = 0; layer < 4; ++layer)
    {
        Gl::ActiveTexture(Gl::TEXTURE0 + static_cast<GLenum>(layer));
        Gl::BindTexture(Gl::TEXTURE_2D, 0);
        Gl::BindSampler(static_cast<GLuint>(layer), 0);
    }

    Gl::ActiveTexture(Gl::TEXTURE0);
    Gl::BindTexture(Gl::TEXTURE_2D, entry.texture);
    Gl::GenerateMipmap(Gl::TEXTURE_2D);
    Gl::BindTexture(Gl::TEXTURE_2D, 0);
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
    return true;
}

GLuint ShaderAlbedoBaker::Resolve(const ShaderAlbedoRequest& request, unsigned long long frame, bool& stateChanged)
{
    stateChanged = false;
    for (Entry& entry : mEntries)
    {
        if (std::memcmp(&entry.request, &request, sizeof(request)) == 0)
        {
            entry.lastUsed = frame;
            return entry.texture;
        }
    }

    if (request.textures[0] == 0)
        return 0;
    stateChanged = true;
    if (!EnsureResources())
        return 0;

    ShaderAlbedoRequest unversioned = request;
    std::memset(unversioned.versions, 0, sizeof(unversioned.versions));
    Entry* reuse = nullptr;
    for (Entry& entry : mEntries)
    {
        ShaderAlbedoRequest previous = entry.request;
        std::memset(previous.versions, 0, sizeof(previous.versions));
        if (std::memcmp(&previous, &unversioned, sizeof(unversioned)) == 0)
        {
            reuse = &entry;
            break;
        }
    }
    if (reuse == nullptr)
    {
        if (mEntries.size() >= kMaxEntries)
            Trim(frame + kUnusedFrames);
        mEntries.emplace_back();
        reuse = &mEntries.back();
    }

    if (!Bake(request, *reuse))
    {
        if (reuse->texture != 0)
            Gl::DeleteTextures(1, &reuse->texture);
        reuse->texture = 0;
        reuse->request = ShaderAlbedoRequest();
        return 0;
    }
    reuse->request = request;
    reuse->lastUsed = frame;
    return reuse->texture;
}

void ShaderAlbedoBaker::Trim(unsigned long long frame)
{
    size_t kept = 0;
    for (size_t index = 0; index < mEntries.size(); ++index)
    {
        Entry& entry = mEntries[index];
        bool stale = entry.texture == 0 || entry.lastUsed + kUnusedFrames < frame;
        if (stale)
        {
            if (entry.texture != 0)
                Gl::DeleteTextures(1, &entry.texture);
            continue;
        }
        if (kept != index)
            mEntries[kept] = entry;
        ++kept;
    }
    mEntries.resize(kept);
}

void ShaderAlbedoBaker::Release()
{
    for (Entry& entry : mEntries)
    {
        if (entry.texture != 0)
            Gl::DeleteTextures(1, &entry.texture);
    }
    mEntries.clear();
    if (mProgram != 0)
        Gl::DeleteProgram(mProgram);
    if (mFramebuffer != 0)
        Gl::DeleteFramebuffers(1, &mFramebuffer);
    if (mVertexArray != 0)
        Gl::DeleteVertexArrays(1, &mVertexArray);
    if (mSampler != 0)
        Gl::DeleteSamplers(1, &mSampler);
    mProgram = 0;
    mFramebuffer = 0;
    mVertexArray = 0;
    mSampler = 0;
    mFailed = false;
}
