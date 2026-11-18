// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderFxaa.cpp
#include "platform/pc/shaderpack/ShaderFxaa.hpp"

#include "platform/pc/shaderpack/Log.hpp"

static const Logger sLog("Shaders");

namespace
{
const char* const kVertexSource = R"(#version 330 core
void main()
{
    vec2 corner = vec2(float(gl_VertexID & 1), float(gl_VertexID >> 1));
    gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);
}
)";

const char* const kFragmentSource = R"(#version 330 core
layout(location = 0) out vec4 oColour;
uniform sampler2D uSource;
uniform vec2 uInverseSize;
uniform bool uLongSearch;

const float kShortSteps[6] = float[6](1.0, 1.5, 2.0, 2.0, 4.0, 8.0);
const float kLongSteps[12] = float[12](1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);

float Luma(vec2 uv)
{
    return dot(texture(uSource, uv).rgb, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec2 uv = gl_FragCoord.xy * uInverseSize;
    vec3 centre = texture(uSource, uv).rgb;
    float lumaCentre = dot(centre, vec3(0.299, 0.587, 0.114));
    float lumaDown = Luma(uv + vec2(0.0, -uInverseSize.y));
    float lumaUp = Luma(uv + vec2(0.0, uInverseSize.y));
    float lumaLeft = Luma(uv + vec2(-uInverseSize.x, 0.0));
    float lumaRight = Luma(uv + vec2(uInverseSize.x, 0.0));
    float lumaMin = min(lumaCentre, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
    float lumaMax = max(lumaCentre, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));
    float range = lumaMax - lumaMin;
    if (range < max(0.0312, lumaMax * 0.125))
    {
        oColour = vec4(centre, 1.0);
        return;
    }

    float lumaDownLeft = Luma(uv - uInverseSize);
    float lumaUpRight = Luma(uv + uInverseSize);
    float lumaUpLeft = Luma(uv + vec2(-uInverseSize.x, uInverseSize.y));
    float lumaDownRight = Luma(uv + vec2(uInverseSize.x, -uInverseSize.y));
    float lumaDownUp = lumaDown + lumaUp;
    float lumaLeftRight = lumaLeft + lumaRight;
    float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
    float lumaDownCorners = lumaDownLeft + lumaDownRight;
    float lumaRightCorners = lumaDownRight + lumaUpRight;
    float lumaUpCorners = lumaUpRight + lumaUpLeft;
    float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs(-2.0 * lumaCentre + lumaDownUp) * 2.0
                           + abs(-2.0 * lumaRight + lumaRightCorners);
    float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) + abs(-2.0 * lumaCentre + lumaLeftRight) * 2.0
                         + abs(-2.0 * lumaDown + lumaDownCorners);
    bool horizontal = edgeHorizontal >= edgeVertical;

    float luma1 = horizontal ? lumaDown : lumaLeft;
    float luma2 = horizontal ? lumaUp : lumaRight;
    float gradient1 = luma1 - lumaCentre;
    float gradient2 = luma2 - lumaCentre;
    bool steepest1 = abs(gradient1) >= abs(gradient2);
    float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));
    float stepLength = horizontal ? uInverseSize.y : uInverseSize.x;
    float lumaLocalAverage = 0.5 * ((steepest1 ? luma1 : luma2) + lumaCentre);
    if (steepest1)
        stepLength = -stepLength;

    vec2 edgeUv = uv;
    if (horizontal)
        edgeUv.y += stepLength * 0.5;
    else
        edgeUv.x += stepLength * 0.5;
    vec2 offset = horizontal ? vec2(uInverseSize.x, 0.0) : vec2(0.0, uInverseSize.y);
    vec2 uv1 = edgeUv - offset;
    vec2 uv2 = edgeUv + offset;
    float lumaEnd1 = Luma(uv1) - lumaLocalAverage;
    float lumaEnd2 = Luma(uv2) - lumaLocalAverage;
    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;
    if (!reached1)
        uv1 -= offset;
    if (!reached2)
        uv2 += offset;

    int steps = uLongSearch ? 12 : 6;
    for (int i = 0; i < steps && !(reached1 && reached2); ++i)
    {
        float scale = uLongSearch ? kLongSteps[i] : kShortSteps[i];
        if (!reached1)
            lumaEnd1 = Luma(uv1) - lumaLocalAverage;
        if (!reached2)
            lumaEnd2 = Luma(uv2) - lumaLocalAverage;
        reached1 = abs(lumaEnd1) >= gradientScaled;
        reached2 = abs(lumaEnd2) >= gradientScaled;
        if (!reached1)
            uv1 -= offset * scale;
        if (!reached2)
            uv2 += offset * scale;
    }

    float distance1 = horizontal ? uv.x - uv1.x : uv.y - uv1.y;
    float distance2 = horizontal ? uv2.x - uv.x : uv2.y - uv.y;
    bool direction1 = distance1 < distance2;
    float edgeLength = max(distance1 + distance2, 1e-6);
    float pixelOffset = 0.5 - min(distance1, distance2) / edgeLength;
    bool centreSmaller = lumaCentre < lumaLocalAverage;
    bool correctVariation = ((direction1 ? lumaEnd1 : lumaEnd2) < 0.0) != centreSmaller;
    float finalOffset = correctVariation ? pixelOffset : 0.0;

    float lumaAverage = (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners) / 12.0;
    float subPixel = clamp(abs(lumaAverage - lumaCentre) / range, 0.0, 1.0);
    subPixel = (-2.0 * subPixel + 3.0) * subPixel * subPixel;
    finalOffset = max(finalOffset, subPixel * subPixel * 0.75);

    vec2 finalUv = uv;
    if (horizontal)
        finalUv.y += finalOffset * stepLength;
    else
        finalUv.x += finalOffset * stepLength;
    oColour = vec4(texture(uSource, finalUv).rgb, 1.0);
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
        LOG_ERROR(sLog, "FXAA shader: %s", log);
        Gl::DeleteShader(shader);
        return 0;
    }
    return shader;
}
}

bool ShaderFxaa::EnsureProgram()
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
        LOG_ERROR(sLog, "FXAA program: %s", log);
        Gl::DeleteProgram(program);
        mFailed = true;
        return false;
    }

    mProgram = program;
    mSourceLocation = Gl::GetUniformLocation(mProgram, "uSource");
    mInverseSizeLocation = Gl::GetUniformLocation(mProgram, "uInverseSize");
    mLongSearchLocation = Gl::GetUniformLocation(mProgram, "uLongSearch");
    Gl::GenFramebuffers(1, &mFramebuffer);
    Gl::GenVertexArrays(1, &mVertexArray);
    Gl::GenSamplers(1, &mSampler);
    Gl::SamplerParameteri(mSampler, Gl::TEXTURE_MIN_FILTER, static_cast<GLint>(Gl::LINEAR));
    Gl::SamplerParameteri(mSampler, Gl::TEXTURE_MAG_FILTER, static_cast<GLint>(Gl::LINEAR));
    Gl::SamplerParameteri(mSampler, Gl::TEXTURE_WRAP_S, static_cast<GLint>(Gl::CLAMP_TO_EDGE));
    Gl::SamplerParameteri(mSampler, Gl::TEXTURE_WRAP_T, static_cast<GLint>(Gl::CLAMP_TO_EDGE));
    return true;
}

GLuint ShaderFxaa::Target(int width, int height)
{
    if (width <= 0 || height <= 0 || !EnsureProgram())
        return 0;
    if (mTexture == 0 || width != mWidth || height != mHeight)
    {
        if (mTexture == 0)
            Gl::GenTextures(1, &mTexture);
        Gl::BindTexture(Gl::TEXTURE_2D, mTexture);
        Gl::TexImage2D(Gl::TEXTURE_2D, 0, static_cast<GLint>(Gl::RGBA8), width, height, 0, Gl::RGBA,
                       Gl::UNSIGNED_BYTE, nullptr);
        Gl::BindTexture(Gl::TEXTURE_2D, 0);
        Gl::BindFramebuffer(Gl::FRAMEBUFFER, mFramebuffer);
        Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0, Gl::TEXTURE_2D, mTexture, 0);
        bool complete = Gl::CheckFramebufferStatus(Gl::FRAMEBUFFER) == Gl::FRAMEBUFFER_COMPLETE;
        Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
        if (!complete)
        {
            LOG_ERROR(sLog, "FXAA target %dx%d is incomplete", width, height);
            Release();
            mFailed = true;
            return 0;
        }
        mWidth = width;
        mHeight = height;
    }
    return mFramebuffer;
}

// LegacyEX: our main framebuffer is the GL way up like the pack's buffers, so neither path turns the image over
void ShaderFxaa::Apply(GLuint destination, int width, int height, int level, GLenum textureUnit)
{
    if (mProgram == 0 || mTexture == 0)
        return;
    if (level <= 0)
    {
        Gl::BindFramebuffer(Gl::READ_FRAMEBUFFER, mFramebuffer);
        Gl::BindFramebuffer(Gl::DRAW_FRAMEBUFFER, destination);
        Gl::Disable(Gl::SCISSOR_TEST);
        Gl::BlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, width, height, Gl::COLOR_BUFFER_BIT, Gl::NEAREST);
        Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
        return;
    }
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, destination);
    GLenum buffer = Gl::COLOR_ATTACHMENT0;
    Gl::DrawBuffers(1, &buffer);
    Gl::Viewport(0, 0, width, height);
    Gl::UseProgram(mProgram);
    Gl::ActiveTexture(textureUnit);
    Gl::BindTexture(Gl::TEXTURE_2D, mTexture);
    Gl::BindSampler(textureUnit - Gl::TEXTURE0, mSampler);
    Gl::Uniform1i(mSourceLocation, static_cast<GLint>(textureUnit - Gl::TEXTURE0));
    Gl::Uniform2f(mInverseSizeLocation, 1.0f / static_cast<float>(mWidth), 1.0f / static_cast<float>(mHeight));
    Gl::Uniform1i(mLongSearchLocation, level >= 4 ? 1 : 0);
    Gl::BindVertexArray(mVertexArray);
    Gl::DrawArrays(Gl::TRIANGLE_STRIP, 0, 4);
    Gl::BindSampler(textureUnit - Gl::TEXTURE0, 0);
    Gl::BindTexture(Gl::TEXTURE_2D, 0);
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
}

void ShaderFxaa::Release()
{
    if (mProgram != 0)
        Gl::DeleteProgram(mProgram);
    if (mFramebuffer != 0)
        Gl::DeleteFramebuffers(1, &mFramebuffer);
    if (mTexture != 0)
        Gl::DeleteTextures(1, &mTexture);
    if (mVertexArray != 0)
        Gl::DeleteVertexArrays(1, &mVertexArray);
    if (mSampler != 0)
        Gl::DeleteSamplers(1, &mSampler);
    mProgram = 0;
    mFramebuffer = 0;
    mTexture = 0;
    mVertexArray = 0;
    mSampler = 0;
    mWidth = 0;
    mHeight = 0;
    mFailed = false;
}
