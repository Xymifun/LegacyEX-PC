// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPipeline.cpp
#include "platform/pc/shaderpack/ShaderPipeline.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <random>
#include <sstream>

#include <png.h>
#include <stb_image.h>

#include "platform/pc/shaderpack/Log.hpp"
#include "platform/pc/PcRendererShaders.hpp"
#include "platform/pc/shaderpack/ShaderPackManager.hpp"

static const Logger sLog("Shaders");

namespace
{
const float kPi = 3.14159265358979f;

const char* const kFallbackVertex = R"(#version 120
varying vec2 lce_texcoord;
varying vec2 lce_lmcoord;
varying vec4 lce_color;
void main()
{
    gl_Position = ftransform();
    lce_texcoord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
    lce_lmcoord = (gl_TextureMatrix[1] * gl_MultiTexCoord1).xy;
    lce_color = gl_Color;
    gl_FogFragCoord = length((gl_ModelViewMatrix * gl_Vertex).xyz);
}
)";

const char* const kFallbackFragment = R"(#version 120
uniform sampler2D texture;
uniform sampler2D lightmap;
uniform int fogMode;
varying vec2 lce_texcoord;
varying vec2 lce_lmcoord;
varying vec4 lce_color;
void main()
{
    vec4 color = texture2D(texture, lce_texcoord) * texture2D(lightmap, lce_lmcoord) * lce_color;
    float fog = 1.0;
    if (fogMode == 9729)
        fog = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);
    else if (fogMode == 2048)
        fog = clamp(exp(-gl_Fog.density * gl_FogFragCoord), 0.0, 1.0);
    else if (fogMode == 2049)
        fog = clamp(exp(-pow(gl_Fog.density * gl_FogFragCoord, 2.0)), 0.0, 1.0);
    color.rgb = mix(gl_Fog.color.rgb, color.rgb, fog);
    gl_FragData[0] = color;
}
)";

const char* const kFinalVertex = R"(#version 120
varying vec2 lce_texcoord;
void main()
{
    gl_Position = ftransform();
    lce_texcoord = gl_MultiTexCoord0.xy;
}
)";

const char* const kFinalFragment = R"(#version 120
uniform sampler2D gcolor;
varying vec2 lce_texcoord;
void main()
{
    gl_FragColor = vec4(texture2D(gcolor, lce_texcoord).rgb, 1.0);
}
)";

enum SamplerKind
{
    SamplerKind_Linear,
    SamplerKind_Nearest,
    SamplerKind_LinearMip,
    SamplerKind_NearestMip,
    SamplerKind_ShadowLinear,
    SamplerKind_ShadowNearest,
    SamplerKind_Repeat,
    SamplerKind_NearestRepeat,
    SamplerKind_Count
};

void Identity(float* matrix)
{
    std::memset(matrix, 0, sizeof(float) * 16);
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
}

void Multiply(const float* a, const float* b, float* out)
{
    float result[16];
    for (int column = 0; column < 4; ++column)
    {
        for (int row = 0; row < 4; ++row)
        {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k)
                sum += a[k * 4 + row] * b[column * 4 + k];
            result[column * 4 + row] = sum;
        }
    }
    std::memcpy(out, result, sizeof(result));
}

void Transform(const float* matrix, const float* vector, float* out)
{
    float result[4];
    for (int row = 0; row < 4; ++row)
    {
        result[row] = matrix[row] * vector[0] + matrix[4 + row] * vector[1] + matrix[8 + row] * vector[2]
                      + matrix[12 + row] * vector[3];
    }
    std::memcpy(out, result, sizeof(result));
}

bool Invert(const float* m, float* out)
{
    float inverse[16];
    inverse[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14]
                 + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inverse[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14]
                 - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    inverse[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13]
                 + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inverse[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13]
                  - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    inverse[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14]
                 - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inverse[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14]
                 + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inverse[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13]
                 - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inverse[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13]
                  + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inverse[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14]
                 + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inverse[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14]
                 - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inverse[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13]
                  + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inverse[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13]
                  - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inverse[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10]
                 - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inverse[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10]
                 + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inverse[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9]
                  - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inverse[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9]
                  + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
    float determinant = m[0] * inverse[0] + m[1] * inverse[4] + m[2] * inverse[8] + m[3] * inverse[12];
    if (determinant == 0.0f)
    {
        Identity(out);
        return false;
    }
    float scale = 1.0f / determinant;
    for (int index = 0; index < 16; ++index)
        out[index] = inverse[index] * scale;
    return true;
}

void Rotate(float* matrix, float degrees, float x, float y, float z)
{
    float radians = degrees * kPi / 180.0f;
    float length = std::sqrt(x * x + y * y + z * z);
    x /= length;
    y /= length;
    z /= length;
    float c = std::cos(radians);
    float s = std::sin(radians);
    float t = 1.0f - c;
    float rotation[16] = {t * x * x + c,     t * x * y + s * z, t * x * z - s * y, 0.0f,
                          t * x * y - s * z, t * y * y + c,     t * y * z + s * x, 0.0f,
                          t * x * z + s * y, t * y * z - s * x, t * z * z + c,     0.0f,
                          0.0f,              0.0f,              0.0f,              1.0f};
    Multiply(matrix, rotation, matrix);
}

void Translate(float* matrix, float x, float y, float z)
{
    float translation[16];
    Identity(translation);
    translation[12] = x;
    translation[13] = y;
    translation[14] = z;
    Multiply(matrix, translation, matrix);
}

void Ortho(float* matrix, float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    std::memset(matrix, 0, sizeof(float) * 16);
    matrix[0] = 2.0f / (right - left);
    matrix[5] = 2.0f / (top - bottom);
    matrix[10] = -2.0f / (farPlane - nearPlane);
    matrix[12] = -(right + left) / (right - left);
    matrix[13] = -(top + bottom) / (top - bottom);
    matrix[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    matrix[15] = 1.0f;
}

void Perspective(float* matrix, float fov, float aspect, float nearPlane, float farPlane)
{
    std::memset(matrix, 0, sizeof(float) * 16);
    float cotangent = 1.0f / std::tan(fov * 0.5f * kPi / 180.0f);
    matrix[0] = cotangent / aspect;
    matrix[5] = cotangent;
    matrix[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    matrix[11] = -1.0f;
    matrix[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
}

std::string Trim(const std::string& text)
{
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return std::string();
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

std::string Upper(std::string text)
{
    for (char& character : text)
        character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    return text;
}

std::string Lower(std::string text)
{
    for (char& character : text)
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    return text;
}

std::vector<std::string> Split(const std::string& text)
{
    std::vector<std::string> parts;
    std::istringstream stream(text);
    std::string part;
    while (stream >> part)
        parts.push_back(part);
    return parts;
}

bool ParseBool(const std::string& text, bool fallback)
{
    std::string value = Trim(text);
    if (value == "true" || value == "1" || value == "on")
        return true;
    if (value == "false" || value == "0" || value == "off")
        return false;
    return fallback;
}

bool DecodeImage(const std::string& data, int& width, int& height, std::vector<unsigned char>& pixels)
{
    static const unsigned char kPngSignature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    if (data.size() > 24 && std::memcmp(data.data(), kPngSignature, sizeof(kPngSignature)) == 0
        && static_cast<unsigned char>(data[24]) < 8)
    {
        png_image image;
        std::memset(&image, 0, sizeof(image));
        image.version = PNG_IMAGE_VERSION;
        if (!png_image_begin_read_from_memory(&image, data.data(), data.size()))
            return false;
        image.format = PNG_FORMAT_RGBA;
        pixels.resize(PNG_IMAGE_SIZE(image));
        if (!png_image_finish_read(&image, nullptr, pixels.data(), 0, nullptr))
        {
            png_image_free(&image);
            return false;
        }
        width = static_cast<int>(image.width);
        height = static_cast<int>(image.height);
        return true;
    }

    int channels = 0;
    unsigned char* decoded = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(data.data()),
                                                   static_cast<int>(data.size()), &width, &height, &channels, 4);
    if (decoded == nullptr)
        return false;
    pixels.assign(decoded, decoded + static_cast<size_t>(width) * height * 4);
    stbi_image_free(decoded);
    return true;
}

bool ResolveTristate(ShaderSettings::Tristate setting, const std::string& property)
{
    if (setting == ShaderSettings::On)
        return true;
    if (setting == ShaderSettings::Off)
        return false;
    return ParseBool(property, true);
}

bool ParseVec4(const std::string& text, float* out)
{
    size_t open = text.find('(');
    size_t close = text.rfind(')');
    std::string inside = open == std::string::npos ? text : text.substr(open + 1, close - open - 1);
    std::replace(inside.begin(), inside.end(), ',', ' ');
    std::vector<std::string> parts = Split(inside);
    if (parts.empty())
        return false;
    for (int index = 0; index < 4; ++index)
        out[index] = static_cast<float>(std::atof(parts[std::min<size_t>(index, parts.size() - 1)].c_str()));
    return true;
}

GLenum FormatFromName(const std::string& rawName)
{
    static const struct
    {
        const char* name;
        GLenum format;
    } kFormats[] = {
        {"R8", Gl::R8},
        {"RG8", Gl::RG8},
        {"RGB8", Gl::RGB8},
        {"RGBA8", Gl::RGBA8},
        {"RGBA", Gl::RGBA8},
        {"RGB", Gl::RGB8},
        {"RG", Gl::RG8},
        {"R", Gl::R8},
        {"R8_SNORM", Gl::R8_SNORM},
        {"RG8_SNORM", Gl::RG8_SNORM},
        {"RGB8_SNORM", Gl::RGB8_SNORM},
        {"RGBA8_SNORM", Gl::RGBA8_SNORM},
        {"R16", Gl::R16},
        {"RG16", Gl::RG16},
        {"RGB16", Gl::RGB16},
        {"RGBA16", Gl::RGBA16},
        {"R16_SNORM", Gl::R16_SNORM},
        {"RG16_SNORM", Gl::RG16_SNORM},
        {"RGB16_SNORM", Gl::RGB16_SNORM},
        {"RGBA16_SNORM", Gl::RGBA16_SNORM},
        {"R16F", Gl::R16F},
        {"RG16F", Gl::RG16F},
        {"RGB16F", Gl::RGB16F},
        {"RGBA16F", Gl::RGBA16F},
        {"R32F", Gl::R32F},
        {"RG32F", Gl::RG32F},
        {"RGB32F", Gl::RGB32F},
        {"RGBA32F", Gl::RGBA32F},
        {"R8I", Gl::R8I},
        {"RG8I", Gl::RG8I},
        {"RGB8I", Gl::RGB8I},
        {"RGBA8I", Gl::RGBA8I},
        {"R8UI", Gl::R8UI},
        {"RG8UI", Gl::RG8UI},
        {"RGB8UI", Gl::RGB8UI},
        {"RGBA8UI", Gl::RGBA8UI},
        {"R16I", Gl::R16I},
        {"RG16I", Gl::RG16I},
        {"RGB16I", Gl::RGB16I},
        {"RGBA16I", Gl::RGBA16I},
        {"R16UI", Gl::R16UI},
        {"RG16UI", Gl::RG16UI},
        {"RGB16UI", Gl::RGB16UI},
        {"RGBA16UI", Gl::RGBA16UI},
        {"R32I", Gl::R32I},
        {"RG32I", Gl::RG32I},
        {"RGB32I", Gl::RGB32I},
        {"RGBA32I", Gl::RGBA32I},
        {"R32UI", Gl::R32UI},
        {"RG32UI", Gl::RG32UI},
        {"RGB32UI", Gl::RGB32UI},
        {"RGBA32UI", Gl::RGBA32UI},
        {"R3_G3_B2", Gl::R3_G3_B2},
        {"RGB5_A1", Gl::RGB5_A1},
        {"RGB10_A2", Gl::RGB10_A2},
        {"RGB10_A2UI", Gl::RGB10_A2UI},
        {"R11F_G11F_B10F", Gl::R11F_G11F_B10F},
        {"RGB9_E5", Gl::RGB9_E5},
        {"RGBA2", Gl::RGBA2},
        {"RGBA4", Gl::RGBA4},
        {"RGBA12", Gl::RGBA12},
        {"RGB4", Gl::RGB4},
        {"RGB5", Gl::RGB5},
        {"RGB10", Gl::RGB10},
        {"RGB12", Gl::RGB12},
    };
    std::string name = Upper(Trim(rawName));
    for (const auto& entry : kFormats)
    {
        if (name == entry.name)
            return entry.format;
    }
    return 0;
}

bool IsIntegerFormat(GLenum format)
{
    switch (format)
    {
    case Gl::R8I:
    case Gl::RG8I:
    case Gl::RGB8I:
    case Gl::RGBA8I:
    case Gl::R16I:
    case Gl::RG16I:
    case Gl::RGB16I:
    case Gl::RGBA16I:
    case Gl::R32I:
    case Gl::RG32I:
    case Gl::RGB32I:
    case Gl::RGBA32I:
        return true;
    default:
        return false;
    }
}

bool IsUnsignedIntegerFormat(GLenum format)
{
    switch (format)
    {
    case Gl::R8UI:
    case Gl::RG8UI:
    case Gl::RGB8UI:
    case Gl::RGBA8UI:
    case Gl::R16UI:
    case Gl::RG16UI:
    case Gl::RGB16UI:
    case Gl::RGBA16UI:
    case Gl::R32UI:
    case Gl::RG32UI:
    case Gl::RGB32UI:
    case Gl::RGBA32UI:
    case Gl::RGB10_A2UI:
        return true;
    default:
        return false;
    }
}

void TransferFormat(GLenum internal, GLenum& format, GLenum& type)
{
    if (IsIntegerFormat(internal))
    {
        format = Gl::RGBA_INTEGER;
        type = Gl::INT;
    }
    else if (IsUnsignedIntegerFormat(internal))
    {
        format = Gl::RGBA_INTEGER;
        type = Gl::UNSIGNED_INT;
    }
    else
    {
        format = Gl::RGBA;
        type = Gl::FLOAT;
    }
}

GLenum PixelFormatFromName(const std::string& rawName)
{
    std::string name = Upper(Trim(rawName));
    if (name == "RED")
        return Gl::RED;
    if (name == "RG")
        return Gl::RG;
    if (name == "RGB")
        return Gl::RGB;
    if (name == "RGBA")
        return Gl::RGBA;
    if (name == "RED_INTEGER")
        return Gl::RED_INTEGER;
    if (name == "RG_INTEGER")
        return Gl::RG_INTEGER;
    if (name == "RGB_INTEGER")
        return Gl::RGB_INTEGER;
    if (name == "RGBA_INTEGER")
        return Gl::RGBA_INTEGER;
    return 0;
}

GLenum PixelTypeFromName(const std::string& rawName)
{
    std::string name = Upper(Trim(rawName));
    if (name == "BYTE")
        return Gl::BYTE;
    if (name == "UNSIGNED_BYTE")
        return Gl::UNSIGNED_BYTE;
    if (name == "SHORT")
        return Gl::SHORT;
    if (name == "UNSIGNED_SHORT")
        return Gl::UNSIGNED_SHORT;
    if (name == "INT")
        return Gl::INT;
    if (name == "UNSIGNED_INT")
        return Gl::UNSIGNED_INT;
    if (name == "FLOAT")
        return Gl::FLOAT;
    if (name == "HALF_FLOAT")
        return Gl::HALF_FLOAT;
    return 0;
}

size_t PixelSize(GLenum format, GLenum type)
{
    size_t channels = 4;
    if (format == Gl::RED || format == Gl::RED_INTEGER)
        channels = 1;
    else if (format == Gl::RG || format == Gl::RG_INTEGER)
        channels = 2;
    else if (format == Gl::RGB || format == Gl::RGB_INTEGER)
        channels = 3;
    size_t bytes = 1;
    if (type == Gl::SHORT || type == Gl::UNSIGNED_SHORT || type == Gl::HALF_FLOAT)
        bytes = 2;
    else if (type == Gl::INT || type == Gl::UNSIGNED_INT || type == Gl::FLOAT)
        bytes = 4;
    return channels * bytes;
}

GLenum BlendFactor(const std::string& rawName, bool& valid)
{
    static const struct
    {
        const char* name;
        GLenum factor;
    } kFactors[] = {
        {"ZERO", Gl::ZERO},
        {"ONE", Gl::ONE},
        {"SRC_COLOR", Gl::SRC_COLOR},
        {"ONE_MINUS_SRC_COLOR", Gl::ONE_MINUS_SRC_COLOR},
        {"DST_COLOR", Gl::DST_COLOR},
        {"ONE_MINUS_DST_COLOR", Gl::ONE_MINUS_DST_COLOR},
        {"SRC_ALPHA", Gl::SRC_ALPHA},
        {"ONE_MINUS_SRC_ALPHA", Gl::ONE_MINUS_SRC_ALPHA},
        {"DST_ALPHA", Gl::DST_ALPHA},
        {"ONE_MINUS_DST_ALPHA", Gl::ONE_MINUS_DST_ALPHA},
        {"SRC_ALPHA_SATURATE", Gl::SRC_ALPHA_SATURATE},
        {"CONSTANT_COLOR", Gl::CONSTANT_COLOR},
        {"ONE_MINUS_CONSTANT_COLOR", Gl::ONE_MINUS_CONSTANT_COLOR},
        {"CONSTANT_ALPHA", Gl::CONSTANT_ALPHA},
        {"ONE_MINUS_CONSTANT_ALPHA", Gl::ONE_MINUS_CONSTANT_ALPHA},
    };
    std::string name = Upper(Trim(rawName));
    for (const auto& entry : kFactors)
    {
        if (name == entry.name)
        {
            valid = true;
            return entry.factor;
        }
    }
    valid = false;
    return Gl::ONE;
}

int AlphaFunction(const std::string& rawName)
{
    static const char* const kNames[] = {"NEVER", "LESS", "EQUAL", "LEQUAL", "GREATER", "NOTEQUAL", "GEQUAL", "ALWAYS"};
    std::string name = Upper(Trim(rawName));
    for (int index = 0; index < 8; ++index)
    {
        if (name == kNames[index])
            return index + 1;
    }
    return 0;
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

std::string CanonicalSampler(const std::string& name)
{
    int colortex = ColorTexIndex(name);
    if (colortex >= 0)
        return "colortex" + std::to_string(colortex);
    if (name == "tex" || name == "gtexture")
        return "texture";
    if (name == "shadow" || name == "watershadow")
        return "shadowtex0";
    if (name == "shadowcolor")
        return "shadowcolor0";
    if (name == "gdepthtex")
        return "depthtex0";
    return name;
}

std::string ProgramStageName(const std::string& program)
{
    if (program.compare(0, 9, "gbuffers_") == 0 || program == "lce_fallback")
        return "gbuffers";
    if (program.compare(0, 5, "setup") == 0)
        return "setup";
    if (program.compare(0, 5, "begin") == 0)
        return "begin";
    if (program.compare(0, 10, "shadowcomp") == 0)
        return "shadowcomp";
    if (program.compare(0, 6, "shadow") == 0)
        return "shadow";
    if (program.compare(0, 7, "prepare") == 0)
        return "prepare";
    if (program.compare(0, 8, "deferred") == 0)
        return "deferred";
    return "composite";
}

double NowSeconds()
{
    using Clock = std::chrono::steady_clock;
    return std::chrono::duration<double>(Clock::now().time_since_epoch()).count();
}

float Halflife(float current, float target, float seconds, float halflife)
{
    if (halflife <= 0.0f)
        return target;
    return target + (current - target) * std::pow(0.5f, seconds / halflife);
}
}

ShaderPipeline& ShaderPipeline::Get()
{
    static ShaderPipeline instance;
    return instance;
}

ShaderPipeline::ShaderPipeline()
{
    for (int& slot : mParameterSlots)
        slot = -1;
}

void ShaderPipeline::SetBlockMap(const std::vector<int>& map, const std::vector<int>& lights)
{
    mPendingBlockMap.assign(map.size() * 2, 0);
    for (size_t element = 0; element < map.size(); ++element)
    {
        mPendingBlockMap[element * 2] = map[element];
        mPendingBlockMap[element * 2 + 1] = element < lights.size() ? lights[element] : 0;
    }
    mBlockMapDirty = true;
}

void ShaderPipeline::SetBiomeConstants(const std::map<std::string, float>& constants)
{
    mBiomeConstants = constants;
}

bool ShaderPipeline::WrittenBefore(int buffer, const std::string& program) const
{
    if (buffer < 0)
        return false;
    bool written = false;
    for (int family : {ShaderPass_Prepare, ShaderPass_Deferred, ShaderPass_Composite, ShaderPass_Final})
    {
        for (const ShaderProgramSource* pass : mPrograms.Passes(family))
        {
            if (pass->name == program)
                return written;
            if (std::find(pass->drawBuffers.begin(), pass->drawBuffers.end(), buffer) != pass->drawBuffers.end())
                written = true;
        }
    }
    return false;
}

int ShaderPipeline::SamplerLookup(const std::string& name, const std::string& program, GLenum type, void* user)
{
    ShaderPipeline* pipeline = static_cast<ShaderPipeline*>(user);
    std::string stage = ProgramStageName(program);
    std::string canonical = CanonicalSampler(name);
    // LegacyEX: a stage's texture is bound to the sampler of its own kind (OptiFine's texture.stage.name.N lines
    // give one name a 2D and a 3D texture, each seen by the programs that declare that kind of sampler)
    const GLenum wanted = type == Gl::SAMPLER_3D || type == Gl::INT_SAMPLER_3D || type == Gl::UNSIGNED_INT_SAMPLER_3D
                              ? Gl::TEXTURE_3D
                          : type == Gl::SAMPLER_1D || type == Gl::INT_SAMPLER_1D || type == Gl::UNSIGNED_INT_SAMPLER_1D
                              ? Gl::TEXTURE_1D
                              : Gl::TEXTURE_2D;
    for (size_t index = 0; index < pipeline->mCustomTextures.size(); ++index)
    {
        const CustomTexture& texture = pipeline->mCustomTextures[index];
        if (texture.texture == 0 && texture.resource.empty() && texture.image < 0)
            continue;
        if (texture.image >= 0 || texture.named)
        {
            if (texture.name == name)
                return static_cast<int>(index);
            continue;
        }
        if (texture.stage.empty() && texture.name == "noise" && canonical == "noisetex")
            return static_cast<int>(index);
        if (texture.stage == stage && CanonicalSampler(texture.name) == canonical
            && (texture.target == 0 || texture.target == wanted)
            && !pipeline->WrittenBefore(ColorTexIndex(texture.name), program))
            return static_cast<int>(index);
    }
    return -1;
}

int ShaderPipeline::ImageLookup(const std::string& name, void* user)
{
    ShaderPipeline* pipeline = static_cast<ShaderPipeline*>(user);
    for (size_t index = 0; index < pipeline->mImages.size(); ++index)
    {
        if (pipeline->mImages[index].name == name)
            return static_cast<int>(index);
    }
    return -1;
}

int ShaderPipeline::UniformLookup(const std::string& name, void* user)
{
    ShaderPipeline* pipeline = static_cast<ShaderPipeline*>(user);
    for (size_t index = 0; index < pipeline->mCustomUniforms.size(); ++index)
    {
        if (pipeline->mCustomUniforms[index].uniform && pipeline->mCustomUniforms[index].name == name)
            return static_cast<int>(index);
    }
    return -1;
}

void ShaderPipeline::Unload()
{
    mAlbedo.Release();
    mFxaa.Release();
    DestroyTargets();
    DestroyShadowTargets();
    mPrograms.Destroy();
    if (mFallback)
    {
        for (ShaderLinkedProgram& linked : mFallback->linked)
        {
            if (linked.name != 0)
                Gl::DeleteProgram(linked.name);
        }
        mFallback.reset();
    }
    for (CustomTexture& texture : mCustomTextures)
    {
        if (texture.texture != 0 && texture.resource.empty() && texture.image < 0)
            Gl::DeleteTextures(1, &texture.texture);
    }
    for (CustomImage& image : mImages)
    {
        if (image.texture != 0)
            Gl::DeleteTextures(1, &image.texture);
    }
    mImages.clear();
    for (BufferObject& object : mBufferObjects)
    {
        if (object.buffer != 0)
            Gl::DeleteBuffers(1, &object.buffer);
        object = BufferObject();
    }
    mCustomTextures.clear();
    mCustomUniforms.clear();
    mExpressions = ShaderExpressionContext();
    GLuint* textures[] = {&mNoiseTexture, &mNormalsTexture, &mSpecularTexture, &mBlockMapTexture};
    for (GLuint* texture : textures)
    {
        if (*texture != 0)
            Gl::DeleteTextures(1, texture);
        *texture = 0;
    }
    if (mBlockMapBuffer != 0)
        Gl::DeleteBuffers(1, &mBlockMapBuffer);
    mBlockMapBuffer = 0;
    if (mDepthReadBuffer != 0)
        Gl::DeleteBuffers(1, &mDepthReadBuffer);
    mDepthReadBuffer = 0;
    mDepthReadPending = false;
    for (GLuint& sampler : mSamplers)
    {
        if (sampler != 0)
            Gl::DeleteSamplers(1, &sampler);
        sampler = 0;
    }
    if (mEmptyVertexArray != 0)
        Gl::DeleteVertexArrays(1, &mEmptyVertexArray);
    mEmptyVertexArray = 0;
    mBoundProgram = nullptr;
    mDrawProgram = nullptr;
    mLoaded = false;
    mBlockMapDirty = !mPendingBlockMap.empty();
}

void ShaderPipeline::Shutdown()
{
    if (mLoaded)
        Unload();
    mReloadRequested = true;
    mDimension = 0x7fffffff;
}

bool ShaderPipeline::Reload(int dimension)
{
    Unload();
    mErrors.clear();
    mDimension = dimension;

    ShaderPackManager& manager = ShaderPackManager::Get();
    ShaderPack* pack = manager.ActivePack();
    bool internal = manager.Settings().UsesInternal();
    if (!internal && (pack == nullptr || !manager.Settings().UsesPack()))
        return false;

    mWorld.clear();
    if (!internal)
    {
        std::string world = "world" + std::to_string(dimension);
        std::vector<std::string> folders = pack->WorldFolders();
        mWorld = std::find(folders.begin(), folders.end(), world) != folders.end() ? world : std::string();
    }

    const ShaderPackProperties& properties = manager.Properties();
    ShaderPackOptions& options = manager.Options();
    std::vector<std::string> disabled;
    std::string profileName = properties.ActiveProfile(options);
    if (const ShaderProfile* profile = properties.Profile(profileName))
        disabled = profile->disabledPrograms;

    auto started = std::chrono::steady_clock::now();
    int glVersion = manager.GlVersion();
    mComputeSupported = glVersion >= 430 && Gl::DispatchCompute != nullptr && Gl::MemoryBarrier != nullptr;
    mImagesSupported = glVersion >= 420 && Gl::BindImageTexture != nullptr && Gl::MemoryBarrier != nullptr;
    mBuffersSupported = glVersion >= 430 && Gl::BindBufferBase != nullptr;
    LoadCustomUniforms();
    LoadCustomTextures();
    LoadCustomImages();
    LoadBufferObjects();

    ShaderMacros macros = manager.StandardMacros();
    std::vector<std::string> required = Split(properties.Get("iris.features.required"));
    std::vector<std::string> optional = Split(properties.Get("iris.features.optional"));
    for (size_t index = 0; index < required.size() + optional.size(); ++index)
    {
        bool isRequired = index < required.size();
        std::string feature = Upper(isRequired ? required[index] : optional[index - required.size()]);
        bool supported = (feature == "COMPUTE_SHADERS" && mComputeSupported) || (feature == "SSBO" && mBuffersSupported)
                         || (feature == "CUSTOM_IMAGES" && mImagesSupported)
                         || feature == "SEPARATE_HARDWARE_SAMPLERS" || feature == "BLOCK_EMISSION_ATTRIBUTE";
        if (supported)
            macros["IRIS_FEATURE_" + feature] = std::string();
        else if (isRequired)
            mErrors.push_back("the shader pack needs the " + feature + " feature, which is not supported");
    }
    const ShaderSettings& settings = manager.Settings();
    mOldLighting = ResolveTristate(settings.oldLighting, properties.Get("oldLighting"));
    mOldHandLight = ResolveTristate(settings.oldHandLight, properties.Get("oldHandLight"));
    mSeparateAo = ParseBool(properties.Get("separateAo"), false);
    if (mOldLighting)
        macros["MC_OLD_LIGHTING"] = std::string();
    if (mOldHandLight)
        macros["MC_OLD_HAND_LIGHT"] = std::string();
    if (mSeparateAo)
        macros["LCE_SEPARATE_AO"] = std::string();
    mPrograms.SetGlslVersion(manager.GlslVersion());
    mPrograms.SetLookups(&ShaderPipeline::SamplerLookup, &ShaderPipeline::UniformLookup, this);
    mPrograms.SetImageLookup(&ShaderPipeline::ImageLookup);
    mPrograms.SetComputeSupported(mComputeSupported);
    if (!internal)
        mPrograms.Load(*pack, options, properties, macros, mWorld, disabled, mErrors);

    mFallback.reset(new ShaderProgramSource);
    mFallback->name = "lce_fallback";
    std::string error;
    if (!mPrograms.LoadInline(*mFallback, kFallbackVertex, kFallbackFragment, error))
        mErrors.push_back("fallback program: " + error);

    int linked = mPrograms.CompileAll(mErrors);
    for (const std::string& message : mErrors)
        LOG_WARN(sLog, "%s", message.c_str());

    LoadSettings();
    CreateDefaultTextures();
    CreateNoiseTexture();

    Gl::GenFramebuffers(1, &mGbufferFramebuffer);
    Gl::GenFramebuffers(1, &mCompositeFramebuffer);
    Gl::GenFramebuffers(1, &mCopyFramebuffer);
    Gl::GenVertexArrays(1, &mEmptyVertexArray);
    std::memset(mAttached, 0, sizeof(mAttached));

    for (int kind = 0; kind < SamplerKind_Count; ++kind)
    {
        GLuint& sampler = mSamplers[kind];
        Gl::GenSamplers(1, &sampler);
        bool linear = kind == SamplerKind_Linear || kind == SamplerKind_LinearMip || kind == SamplerKind_ShadowLinear
                      || kind == SamplerKind_Repeat;
        GLenum minFilter = linear ? Gl::LINEAR : Gl::NEAREST;
        if (kind == SamplerKind_LinearMip)
            minFilter = Gl::LINEAR_MIPMAP_LINEAR;
        else if (kind == SamplerKind_NearestMip)
            minFilter = Gl::NEAREST_MIPMAP_NEAREST;
        Gl::SamplerParameteri(sampler, Gl::TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
        Gl::SamplerParameteri(sampler, Gl::TEXTURE_MAG_FILTER, static_cast<GLint>(linear ? Gl::LINEAR : Gl::NEAREST));
        GLenum wrap = kind == SamplerKind_Repeat || kind == SamplerKind_NearestRepeat ? Gl::REPEAT : Gl::CLAMP_TO_EDGE;
        Gl::SamplerParameteri(sampler, Gl::TEXTURE_WRAP_S, static_cast<GLint>(wrap));
        Gl::SamplerParameteri(sampler, Gl::TEXTURE_WRAP_T, static_cast<GLint>(wrap));
        Gl::SamplerParameteri(sampler, Gl::TEXTURE_WRAP_R, static_cast<GLint>(wrap));
        if (kind == SamplerKind_ShadowLinear || kind == SamplerKind_ShadowNearest)
        {
            Gl::SamplerParameteri(sampler, Gl::TEXTURE_COMPARE_MODE, static_cast<GLint>(Gl::COMPARE_REF_TO_TEXTURE));
            Gl::SamplerParameteri(sampler, Gl::TEXTURE_COMPARE_FUNC, static_cast<GLint>(Gl::LEQUAL));
        }
    }

    if (ShadowEnabled())
        CreateShadowTargets();
    CreateImages(false);
    AllocateBufferObjects(false);
    mSetupPending = true;

    Gl::GenBuffers(1, &mDepthReadBuffer);
    Gl::BindBuffer(Gl::PIXEL_PACK_BUFFER, mDepthReadBuffer);
    Gl::BufferData(Gl::PIXEL_PACK_BUFFER, 4, nullptr, Gl::STREAM_READ);
    Gl::BindBuffer(Gl::PIXEL_PACK_BUFFER, 0);

    mWidth = 0;
    mHeight = 0;
    mHasPrevious = false;
    mLoaded = true;
    mBlockMapDirty = !mPendingBlockMap.empty();
    double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    LOG_INFO(sLog, "Loaded %s for %s: %d programs in %.0f ms, %zu problems",
             internal ? ShaderSettings::kPackInternal : pack->Name().c_str(),
             mWorld.empty() ? "all worlds" : mWorld.c_str(), linked, seconds * 1000.0, mErrors.size());
    return true;
}

std::string ShaderPipeline::ProgramProperty(const std::string& prefix, const std::string& program) const
{
    const ShaderPackProperties& properties = ShaderPackManager::Get().Properties();
    if (!mWorld.empty() && properties.Has(prefix + mWorld + "/" + program))
        return properties.Get(prefix + mWorld + "/" + program);
    return properties.Get(prefix + program);
}

GLenum ShaderPipeline::BufferFormat(int buffer) const
{
    static const char* const kLegacy[8] = {"gcolorFormat", "gdepthFormat", "gnormalFormat", "compositeFormat",
                                           "gaux1Format",  "gaux2Format",  "gaux3Format",   "gaux4Format"};
    std::string value = mPrograms.Directive("colortex" + std::to_string(buffer) + "Format");
    if (value.empty() && buffer < 8)
        value = mPrograms.Directive(kLegacy[buffer]);
    GLenum format = value.empty() ? 0 : FormatFromName(value);
    return format != 0 ? format : Gl::RGBA8;
}

void ShaderPipeline::LoadSettings()
{
    const ShaderPackProperties& properties = ShaderPackManager::Get().Properties();
    const ShaderSettings& settings = ShaderPackManager::Get().Settings();

    std::vector<ShaderProgramSource*> programs;
    for (int index = 0; index < ShaderGbuffer_Count; ++index)
    {
        if (ShaderProgramSource* source = mPrograms.ResolveGbuffer(index))
            programs.push_back(source);
    }
    for (int family = 0; family < ShaderPass_FamilyCount; ++family)
    {
        for (ShaderProgramSource* pass : mPrograms.Passes(family))
            programs.push_back(pass);
    }

    static const char* const kLegacyNames[8] = {"gcolor", "gdepth", "gnormal", "composite",
                                                "gaux1",  "gaux2",  "gaux3",   "gaux4"};
    for (int buffer = 0; buffer < 16; ++buffer)
    {
        ColorTarget& target = mColor[buffer];
        target = ColorTarget();
        target.format = BufferFormat(buffer);
        std::string name = "colortex" + std::to_string(buffer);
        std::string clear = mPrograms.Directive(name + "Clear");
        if (clear.empty() && buffer < 8)
            clear = mPrograms.Directive(std::string(kLegacyNames[buffer]) + "Clear");
        target.clear = ParseBool(clear, true);
        float defaultColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        if (buffer == 1)
            defaultColor[0] = defaultColor[1] = defaultColor[2] = defaultColor[3] = 1.0f;
        std::memcpy(target.clearColor, defaultColor, sizeof(defaultColor));
        std::string clearColor = mPrograms.Directive(name + "ClearColor");
        if (clearColor.empty() && buffer < 8)
            clearColor = mPrograms.Directive(std::string(kLegacyNames[buffer]) + "ClearColor");
        if (!clearColor.empty())
            ParseVec4(clearColor, target.clearColor);
        std::string mipmap = mPrograms.Directive(name + "MipmapEnabled");
        if (mipmap.empty() && buffer < 8)
            mipmap = mPrograms.Directive(std::string(kLegacyNames[buffer]) + "MipmapEnabled");
        target.mipmap = ParseBool(mipmap, false);

        std::string size = properties.Get("size.buffer." + name);
        if (size.empty() && buffer < 8)
            size = properties.Get(std::string("size.buffer.") + kLegacyNames[buffer]);
        std::vector<std::string> parts = Split(size);
        if (parts.size() >= 2)
        {
            target.absolute = parts[0].find('.') == std::string::npos && parts[1].find('.') == std::string::npos;
            target.sizeX = static_cast<float>(std::atof(parts[0].c_str()));
            target.sizeY = static_cast<float>(std::atof(parts[1].c_str()));
            if (target.sizeX <= 0.0f || target.sizeY <= 0.0f)
            {
                target.absolute = false;
                target.sizeX = target.sizeY = 1.0f;
            }
        }
    }
    mColor[0].used = true;

    auto markUsed = [&](const ShaderLinkedProgram& linked) {
        for (const ShaderLinkedProgram::Sampler& sampler : linked.samplers)
        {
            if (sampler.source >= ShaderSource_ColorTex0 && sampler.source < ShaderSource_ColorTex0 + 16)
                mColor[sampler.source - ShaderSource_ColorTex0].used = true;
        }
        for (const ShaderLinkedProgram::Image& image : linked.images)
        {
            if (image.source >= ShaderSource_ColorTex0 && image.source < ShaderSource_ColorTex0 + 16)
                mColor[image.source - ShaderSource_ColorTex0].used = true;
        }
    };
    for (ShaderProgramSource* program : programs)
    {
        if (program->present)
        {
            for (int buffer : program->drawBuffers)
                mColor[buffer].used = true;
        }
        for (const ShaderLinkedProgram& linked : program->linked)
            markUsed(linked);
        for (const ShaderComputeSource& compute : program->compute)
            markUsed(compute.linked);
    }

    for (int program = 0; program < ShaderGbuffer_Count; ++program)
    {
        mGbufferAlphaFunction[program] = 0;
        mGbufferAlphaReference[program] = 0.0f;
        ShaderProgramSource* source = mPrograms.ResolveGbuffer(program);
        std::string value = ProgramProperty("alphaTest.", ShaderProgramSet::GbufferName(program));
        if (value.empty() && source != nullptr)
            value = ProgramProperty("alphaTest.", source->name);
        std::vector<std::string> parts = Split(value);
        if (parts.size() == 1 && (parts[0] == "off" || parts[0] == "false"))
            mGbufferAlphaFunction[program] = -1;
        else if (parts.size() >= 2)
        {
            int function = AlphaFunction(parts[0]);
            if (function > 0)
            {
                mGbufferAlphaFunction[program] = function;
                mGbufferAlphaReference[program] = static_cast<float>(std::atof(parts[1].c_str()));
            }
        }
    }

    auto directiveFloat = [&](const char* name, float fallback) {
        std::string value = mPrograms.Directive(name);
        return value.empty() ? fallback : static_cast<float>(std::atof(value.c_str()));
    };
    auto directiveBool = [&](const char* name, bool fallback) { return ParseBool(mPrograms.Directive(name), fallback); };

    mWetnessHalflife = directiveFloat("wetnessHalflife", 600.0f);
    mDrynessHalflife = directiveFloat("drynessHalflife", 200.0f);
    mEyeBrightnessHalflife = directiveFloat("eyeBrightnessHalflife", 10.0f);
    mCenterDepthHalflife = directiveFloat("centerDepthHalflife", 1.0f);
    mSunPathRotation = directiveFloat("sunPathRotation", 0.0f);
    mHandDepth = settings.handDepthMul;
    mRenderScale = settings.renderResMul;
    mAntialiasing = settings.antialiasingLevel;

    mShadowDistance = directiveFloat("shadowDistance", 160.0f);
    mShadowDistanceRenderMul = directiveFloat("shadowDistanceRenderMul", -1.0f);
    mShadowIntervalSize = directiveFloat("shadowIntervalSize", 2.0f);
    int resolution = static_cast<int>(directiveFloat("shadowMapResolution", 1024.0f) * settings.shadowResMul);
    mShadowResolution = std::max(16, std::min(resolution, 16384));
    bool hardware = directiveBool("shadowHardwareFiltering", false);
    mShadowHardware[0] = directiveBool("shadowHardwareFiltering0", hardware);
    mShadowHardware[1] = directiveBool("shadowHardwareFiltering1", hardware);
    bool shadowMip = directiveBool("generateShadowMipmap", false);
    mShadowMipmap[0] = directiveBool("shadowtex0Mipmap", directiveBool("shadowtexMipmap", shadowMip));
    mShadowMipmap[1] = directiveBool("shadowtex1Mipmap", shadowMip);
    bool colorMip = directiveBool("generateShadowColorMipmap", false);
    mShadowColorMipmap[0] = directiveBool("shadowcolor0Mipmap", directiveBool("shadowColor0Mipmap", colorMip));
    mShadowColorMipmap[1] = directiveBool("shadowcolor1Mipmap", directiveBool("shadowColor1Mipmap", colorMip));
    mShadowNearest[0] = directiveBool("shadowtex0Nearest", directiveBool("shadowtexNearest",
                                                                         directiveBool("shadow0MinMagNearest", false)));
    mShadowNearest[1] = directiveBool("shadowtex1Nearest", directiveBool("shadow1MinMagNearest", false));
    mShadowColorNearest[0] = directiveBool("shadowcolor0Nearest",
                                           directiveBool("shadowColor0Nearest",
                                                         directiveBool("shadowColor0MinMagNearest", false)));
    mShadowColorNearest[1] = directiveBool("shadowcolor1Nearest",
                                           directiveBool("shadowColor1Nearest",
                                                         directiveBool("shadowColor1MinMagNearest", false)));
    for (int index = 0; index < 2; ++index)
    {
        std::string name = "shadowcolor" + std::to_string(index);
        std::string format = mPrograms.Directive(name + "Format");
        GLenum parsed = format.empty() ? 0 : FormatFromName(format);
        mShadowColorFormat[index] = parsed != 0 ? parsed : Gl::RGBA8;
        mShadowColorClear[index] = ParseBool(mPrograms.Directive(name + "Clear"), true);
        float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        std::memcpy(mShadowColorClearColor[index], white, sizeof(white));
        std::string clearColor = mPrograms.Directive(name + "ClearColor");
        if (!clearColor.empty())
            ParseVec4(clearColor, mShadowColorClearColor[index]);
        mShadowColorUsed[index] = false;
    }
    if (ShaderProgramSource* shadow = mPrograms.ResolveGbuffer(ShaderGbuffer_Shadow))
    {
        for (int buffer : shadow->drawBuffers)
        {
            if (buffer < 2)
                mShadowColorUsed[buffer] = true;
        }
    }
    for (ShaderProgramSource* program : programs)
    {
        std::vector<const ShaderLinkedProgram*> linkedPrograms;
        for (const ShaderLinkedProgram& linked : program->linked)
            linkedPrograms.push_back(&linked);
        for (const ShaderComputeSource& compute : program->compute)
            linkedPrograms.push_back(&compute.linked);
        for (const ShaderLinkedProgram* linked : linkedPrograms)
        {
            for (const ShaderLinkedProgram::Sampler& sampler : linked->samplers)
            {
                if (sampler.source == ShaderSource_ShadowColor0)
                    mShadowColorUsed[0] = true;
                if (sampler.source == ShaderSource_ShadowColor1)
                    mShadowColorUsed[1] = true;
            }
            for (const ShaderLinkedProgram::Image& image : linked->images)
            {
                if (image.source == ShaderSource_ShadowColor0)
                    mShadowColorUsed[0] = true;
                if (image.source == ShaderSource_ShadowColor1)
                    mShadowColorUsed[1] = true;
            }
        }
    }
    mShadowEntities = ParseBool(properties.Get("shadowEntities"), true);
    mShadowBlockEntities = ParseBool(properties.Get("shadowBlockEntities"), true);
    mShadowTerrain = ParseBool(properties.Get("shadowTerrain"), true);
    mShadowTranslucent = ParseBool(properties.Get("shadowTranslucent"), true);
    mShadowPlayer = ParseBool(properties.Get("shadowPlayer"), false);
    std::string clouds = properties.Get("clouds");
    mCloudMode = clouds == "off" ? 0 : clouds == "fast" ? 1 : clouds == "fancy" ? 2 : -1;
    mSky = ParseBool(properties.Get("sky"), true);
    mSun = ParseBool(properties.Get("sun"), true);
    mMoon = ParseBool(properties.Get("moon"), true);
    mUnderwaterOverlay = ParseBool(properties.Get("underwaterOverlay"), true);
    mVignette = ParseBool(properties.Get("vignette"), true);
    mRainDepth = ParseBool(properties.Get("rain.depth"), false);
    mBeaconBeamDepth = ParseBool(properties.Get("beacon.beam.depth"), false);
    mDynamicHandLight = ParseBool(properties.Get("dynamicHandLight"), true);
    if (properties.Has("shadow.enabled") && !ParseBool(properties.Get("shadow.enabled"), true))
        mShadowResolution = 0;
}

void ShaderPipeline::LoadCustomUniforms()
{
    mExpressions = ShaderExpressionContext();
    mMatrixSlots.clear();
    for (int id = 0; id < ShaderUniform_Count; ++id)
    {
        const ShaderUniformInfo& info = ShaderUniformCatalogue(id);
        mParameterSlots[id] = -1;
        if (std::strncmp(info.name, "lce_", 4) == 0)
            continue;
        if (info.type == ShaderUniformType_Mat4)
        {
            int first = -1;
            for (int element = 0; element < 16; ++element)
            {
                std::string name = std::string(info.name) + "." + std::to_string(element / 4) + "."
                                   + std::to_string(element % 4);
                int slot = mExpressions.Define(name, 1);
                if (element == 0)
                    first = slot;
            }
            mMatrixSlots.push_back(std::make_pair(id, first));
            continue;
        }
        int size = 1;
        switch (info.type)
        {
        case ShaderUniformType_Vec2:
        case ShaderUniformType_IVec2:
            size = 2;
            break;
        case ShaderUniformType_Vec3:
        case ShaderUniformType_IVec3:
            size = 3;
            break;
        case ShaderUniformType_Vec4:
        case ShaderUniformType_IVec4:
            size = 4;
            break;
        default:
            break;
        }
        mParameterSlots[id] = mExpressions.Define(info.name, size);
    }
    for (const auto& constant : mBiomeConstants)
        mExpressions.DefineConstant(constant.first, constant.second);
    mExpressions.DefineConstant("PPT_NONE", 0.0f);
    mExpressions.DefineConstant("PPT_RAIN", 1.0f);
    mExpressions.DefineConstant("PPT_SNOW", 2.0f);
    // Iris's biome categories, the values biome_category takes (BiomeCategory in PcShaderPacks.cpp)
    static const char* const kCategories[] = {"CAT_NONE",   "CAT_TAIGA",  "CAT_EXTREME_HILLS", "CAT_JUNGLE",
                                              "CAT_MESA",   "CAT_PLAINS", "CAT_SAVANNA",       "CAT_ICY",
                                              "CAT_THE_END", "CAT_BEACH", "CAT_FOREST",        "CAT_OCEAN",
                                              "CAT_DESERT", "CAT_RIVER",  "CAT_SWAMP",         "CAT_MUSHROOM",
                                              "CAT_NETHER", "CAT_MOUNTAIN", "CAT_UNDERGROUND"};
    for (int index = 0; index < static_cast<int>(sizeof(kCategories) / sizeof(kCategories[0])); ++index)
        mExpressions.DefineConstant(kCategories[index], static_cast<float>(index));

    mCustomUniforms.clear();
    const ShaderPackProperties& properties = ShaderPackManager::Get().Properties();
    for (const auto& entry : properties.Entries())
    {
        const std::string& key = entry.first;
        bool uniform = key.compare(0, 8, "uniform.") == 0;
        bool variable = key.compare(0, 9, "variable.") == 0;
        if (!uniform && !variable)
            continue;
        std::string rest = key.substr(uniform ? 8 : 9);
        size_t dot = rest.find('.');
        if (dot == std::string::npos)
            continue;
        std::string type = rest.substr(0, dot);
        std::string name = rest.substr(dot + 1);
        int size = type == "vec2" ? 2 : type == "vec3" ? 3 : type == "vec4" ? 4 : 1;
        int kind = type == "float" ? 0 : type == "int" ? 1 : type == "bool" ? 2 : size == 1 ? -1 : 3;
        if (kind < 0)
        {
            mErrors.push_back("custom " + std::string(uniform ? "uniform" : "variable") + " " + name
                              + " has an unknown type " + type);
            continue;
        }
        CustomUniform custom;
        custom.name = name;
        custom.type = kind == 3 ? 2 + size : kind;
        custom.uniform = uniform;
        std::string error;
        if (!custom.expression.Parse(entry.second, mExpressions, error))
        {
            mErrors.push_back("custom " + std::string(uniform ? "uniform" : "variable") + " " + name + ": " + error);
            continue;
        }
        custom.slot = mExpressions.Define(name, size);
        mCustomUniforms.push_back(std::move(custom));
    }
}

void ShaderPipeline::ReadTextureMeta(ShaderPack& pack, const std::string& path, CustomTexture& texture)
{
    std::string meta;
    if (!pack.ReadFile(path + ".mcmeta", meta))
        return;
    auto flag = [&](const char* key) {
        size_t found = meta.find(key);
        if (found == std::string::npos)
            return false;
        size_t end = meta.find_first_of(",}", found);
        size_t value = meta.find("true", found);
        return value != std::string::npos && value < end;
    };
    texture.linear = flag("\"blur\"");
    texture.clamp = flag("\"clamp\"");
}

void ShaderPipeline::LoadCustomTextures()
{
    ShaderPackManager& manager = ShaderPackManager::Get();
    ShaderPack* pack = manager.ActivePack();
    const ShaderPackProperties& properties = manager.Properties();
    for (const auto& entry : properties.Entries())
    {
        const std::string& key = entry.first;
        bool named = key.compare(0, 14, "customTexture.") == 0;
        if (!named && key.compare(0, 8, "texture.") != 0)
            continue;
        std::string rest = key.substr(named ? 14 : 8);
        CustomTexture texture;
        texture.named = named;
        size_t dot = rest.find('.');
        if (named)
            texture.name = rest;
        else if (rest == "noise")
            texture.name = "noise";
        else if (dot != std::string::npos)
        {
            texture.stage = rest.substr(0, dot);
            texture.name = rest.substr(dot + 1);
            // LegacyEX: texture.stage.name.N (N a number) is another texture for the same name
            size_t last = texture.name.rfind('.');
            if (last != std::string::npos && last + 1 < texture.name.size()
                && texture.name.find_first_not_of("0123456789", last + 1) == std::string::npos)
                texture.name.erase(last);
        }
        else
            continue;

        std::vector<std::string> parts = Split(entry.second);
        if (parts.empty())
            continue;
        std::string path = parts[0];
        size_t colon = path.find(':');
        if (colon != std::string::npos)
        {
            std::string location = path.substr(colon + 1);
            bool supported = path.compare(0, colon, "minecraft") == 0 && location.size() > 13
                             && location.compare(0, 9, "textures/") == 0
                             && location.compare(location.size() - 4, 4, ".png") == 0;
            if (!supported)
            {
                mErrors.push_back("texture " + rest + " uses a resource location (" + path + ") with no game texture");
                continue;
            }
            texture.resource = location.substr(9, location.size() - 13);
            texture.target = Gl::TEXTURE_2D;
            mCustomTextures.push_back(texture);
            continue;
        }
        if (!path.empty() && path[0] == '/')
            path = path.substr(1);
        path = ShaderPack::NormalisePath(path);
        std::string data;
        if (!pack->ReadFile(path, data))
        {
            mErrors.push_back("texture " + rest + ": " + path + " is missing");
            continue;
        }

        if (parts.size() >= 2)
        {
            std::string targetName = Upper(parts[1]);
            GLenum target = targetName == "TEXTURE_1D"   ? Gl::TEXTURE_1D
                            : targetName == "TEXTURE_2D" ? Gl::TEXTURE_2D
                            : targetName == "TEXTURE_3D" ? Gl::TEXTURE_3D
                                                         : 0;
            int dimensions = target == Gl::TEXTURE_1D ? 1 : target == Gl::TEXTURE_2D ? 2 : 3;
            if (target == 0 || parts.size() < static_cast<size_t>(5 + dimensions))
            {
                mErrors.push_back("texture " + rest + " has an unknown raw layout");
                continue;
            }
            GLenum internal = FormatFromName(parts[2]);
            int size[3] = {1, 1, 1};
            for (int axis = 0; axis < dimensions; ++axis)
                size[axis] = std::atoi(parts[3 + axis].c_str());
            GLenum format = PixelFormatFromName(parts[3 + dimensions]);
            GLenum type = PixelTypeFromName(parts[4 + dimensions]);
            size_t expected = PixelSize(format, type) * static_cast<size_t>(size[0]) * size[1] * size[2];
            if (internal == 0 || format == 0 || type == 0 || size[0] <= 0 || size[1] <= 0 || size[2] <= 0
                || data.size() < expected)
            {
                mErrors.push_back("texture " + rest + " has bad raw parameters or too little data");
                continue;
            }
            Gl::GenTextures(1, &texture.texture);
            Gl::ActiveTexture(Gl::TEXTURE0);
            Gl::BindTexture(target, texture.texture);
            Gl::PixelStorei(Gl::UNPACK_ALIGNMENT, 1);
            if (target == Gl::TEXTURE_1D)
                Gl::TexImage1D(target, 0, static_cast<GLint>(internal), size[0], 0, format, type, data.data());
            else if (target == Gl::TEXTURE_2D)
                Gl::TexImage2D(target, 0, static_cast<GLint>(internal), size[0], size[1], 0, format, type, data.data());
            else
                Gl::TexImage3D(target, 0, static_cast<GLint>(internal), size[0], size[1], size[2], 0, format, type,
                               data.data());
            Gl::PixelStorei(Gl::UNPACK_ALIGNMENT, 4);
            Gl::TexParameteri(target, Gl::TEXTURE_MAX_LEVEL, 0);
            Gl::BindTexture(target, 0);
            texture.target = target;
            ReadTextureMeta(*pack, path, texture);
            mCustomTextures.push_back(texture);
            continue;
        }

        int width = 0;
        int height = 0;
        std::vector<unsigned char> pixels;
        if (!DecodeImage(data, width, height, pixels))
        {
            mErrors.push_back("texture " + rest + ": " + path + " could not be decoded");
            continue;
        }
        ReadTextureMeta(*pack, path, texture);
        Gl::GenTextures(1, &texture.texture);
        Gl::ActiveTexture(Gl::TEXTURE0);
        Gl::BindTexture(Gl::TEXTURE_2D, texture.texture);
        Gl::TexImage2D(Gl::TEXTURE_2D, 0, static_cast<GLint>(Gl::RGBA8), width, height, 0, Gl::RGBA, Gl::UNSIGNED_BYTE,
                       pixels.data());
        Gl::TexParameteri(Gl::TEXTURE_2D, Gl::TEXTURE_MAX_LEVEL, 0);
        Gl::BindTexture(Gl::TEXTURE_2D, 0);
        texture.target = Gl::TEXTURE_2D;
        mCustomTextures.push_back(texture);
    }
}

void ShaderPipeline::LoadCustomImages()
{
    mImages.clear();
    if (!mImagesSupported)
        return;
    const ShaderPackProperties& properties = ShaderPackManager::Get().Properties();
    for (const auto& entry : properties.Entries())
    {
        if (entry.first.compare(0, 6, "image.") != 0 || mImages.size() >= 16)
            continue;
        std::string name = entry.first.substr(6);
        std::vector<std::string> parts = Split(entry.second);
        if (parts.size() < 7 || parts.size() > 9)
        {
            mErrors.push_back("image " + name + " has an unknown layout");
            continue;
        }
        CustomImage image;
        image.name = name;
        image.format = PixelFormatFromName(parts[1]);
        image.internal = FormatFromName(parts[2]);
        image.type = PixelTypeFromName(parts[3]);
        image.clear = Lower(parts[4]) == "true";
        image.relative = Lower(parts[5]) == "true";
        if (image.format == 0 || image.internal == 0 || image.type == 0)
        {
            mErrors.push_back("image " + name + " has an unknown format");
            continue;
        }
        if (image.relative)
        {
            if (parts.size() < 8)
            {
                mErrors.push_back("image " + name + " is relative but has no height");
                continue;
            }
            image.target = Gl::TEXTURE_2D;
            image.size[0] = static_cast<float>(std::atof(parts[6].c_str()));
            image.size[1] = static_cast<float>(std::atof(parts[7].c_str()));
        }
        else
        {
            image.target = parts.size() == 7 ? Gl::TEXTURE_1D : parts.size() == 8 ? Gl::TEXTURE_2D : Gl::TEXTURE_3D;
            for (size_t axis = 6; axis < parts.size(); ++axis)
                image.size[axis - 6] = static_cast<float>(std::atoi(parts[axis].c_str()));
        }
        mImages.push_back(image);
        if (parts[0] != "none")
        {
            CustomTexture texture;
            texture.name = parts[0];
            texture.image = static_cast<int>(mImages.size()) - 1;
            texture.target = image.target;
            texture.linear = !IsIntegerFormat(image.internal) && !IsUnsignedIntegerFormat(image.internal);
            texture.clamp = true;
            mCustomTextures.push_back(texture);
        }
    }
}

void ShaderPipeline::CreateImages(bool relativeOnly)
{
    for (CustomImage& image : mImages)
    {
        if (relativeOnly && !image.relative)
            continue;
        int extent[3] = {1, 1, 1};
        if (image.relative)
        {
            extent[0] = std::max(1, static_cast<int>(static_cast<float>(mWidth) * image.size[0]));
            extent[1] = std::max(1, static_cast<int>(static_cast<float>(mHeight) * image.size[1]));
        }
        else
        {
            for (int axis = 0; axis < 3; ++axis)
                extent[axis] = std::max(1, static_cast<int>(image.size[axis]));
        }
        if (image.texture != 0 && std::equal(extent, extent + 3, image.extent))
            continue;
        if (image.texture != 0)
            Gl::DeleteTextures(1, &image.texture);
        std::copy(extent, extent + 3, image.extent);
        while (Gl::GetError() != 0)
        {
        }
        Gl::GenTextures(1, &image.texture);
        Gl::ActiveTexture(Gl::TEXTURE0);
        Gl::BindTexture(image.target, image.texture);
        if (image.target == Gl::TEXTURE_1D)
            Gl::TexImage1D(image.target, 0, static_cast<GLint>(image.internal), extent[0], 0, image.format, image.type,
                           nullptr);
        else if (image.target == Gl::TEXTURE_2D)
            Gl::TexImage2D(image.target, 0, static_cast<GLint>(image.internal), extent[0], extent[1], 0, image.format,
                           image.type, nullptr);
        else
            Gl::TexImage3D(image.target, 0, static_cast<GLint>(image.internal), extent[0], extent[1], extent[2], 0,
                           image.format, image.type, nullptr);
        Gl::TexParameteri(image.target, Gl::TEXTURE_MAX_LEVEL, 0);
        Gl::BindTexture(image.target, 0);
        if (Gl::ClearTexImage != nullptr)
            Gl::ClearTexImage(image.texture, 0, image.format, image.type, nullptr);
        GLenum error = Gl::GetError();
        if (error != 0)
            LOG_ERROR(sLog, "image %s (%dx%dx%d) could not be created: GL error 0x%x", image.name.c_str(), extent[0],
                      extent[1], extent[2], error);
        ++mBindingsStamp;
    }
}

void ShaderPipeline::ClearImages(bool all)
{
    if (Gl::ClearTexImage == nullptr)
        return;
    for (const CustomImage& image : mImages)
    {
        if (image.texture != 0 && (all || image.clear))
            Gl::ClearTexImage(image.texture, 0, image.format, image.type, nullptr);
    }
}

void ShaderPipeline::LoadBufferObjects()
{
    for (BufferObject& object : mBufferObjects)
        object = BufferObject();
    if (!mBuffersSupported)
        return;
    const ShaderPackProperties& properties = ShaderPackManager::Get().Properties();
    for (int index = 0; index < 9; ++index)
    {
        std::string value = properties.Get("bufferObject." + std::to_string(index));
        std::vector<std::string> parts = Split(value);
        if (parts.empty())
            continue;
        BufferObject& object = mBufferObjects[index];
        object.bytes = static_cast<GLsizeiptr>(std::atoll(parts[0].c_str()));
        if (object.bytes < 1)
        {
            object = BufferObject();
            continue;
        }
        if (parts.size() == 2)
            object.file = parts[1];
        else if (parts.size() >= 4)
        {
            object.relative = Lower(parts[1]) == "true";
            object.scale[0] = static_cast<float>(std::atof(parts[2].c_str()));
            object.scale[1] = static_cast<float>(std::atof(parts[3].c_str()));
        }
    }
}

void ShaderPipeline::AllocateBufferObjects(bool relativeOnly)
{
    for (int index = 0; index < 9; ++index)
    {
        BufferObject& object = mBufferObjects[index];
        if (object.bytes < 1 || (relativeOnly && !object.relative))
            continue;
        GLsizeiptr size = object.bytes;
        if (object.relative)
            size = static_cast<GLsizeiptr>(static_cast<int>(static_cast<float>(mWidth) * object.scale[0]))
                   * static_cast<GLsizeiptr>(static_cast<int>(static_cast<float>(mHeight) * object.scale[1]))
                   * object.bytes;
        size = std::max<GLsizeiptr>(size, 4);
        if (object.buffer != 0 && size == object.allocated)
            continue;
        if (object.buffer == 0)
            Gl::GenBuffers(1, &object.buffer);
        Gl::BindBuffer(Gl::SHADER_STORAGE_BUFFER, object.buffer);
        Gl::BufferData(Gl::SHADER_STORAGE_BUFFER, size, nullptr, Gl::DYNAMIC_COPY);
        if (Gl::ClearBufferData != nullptr)
            Gl::ClearBufferData(Gl::SHADER_STORAGE_BUFFER, Gl::R8, Gl::RED, Gl::UNSIGNED_BYTE, nullptr);
        std::string data;
        ShaderPack* pack = ShaderPackManager::Get().ActivePack();
        if (!object.file.empty() && pack != nullptr)
        {
            std::string path = object.file;
            if (!path.empty() && path[0] == '/')
                path = path.substr(1);
            if (!pack->ReadFile(ShaderPack::NormalisePath(path), data))
                mErrors.push_back("bufferObject." + std::to_string(index) + ": " + object.file + " is missing");
            else if (static_cast<GLsizeiptr>(data.size()) > size)
                mErrors.push_back("bufferObject." + std::to_string(index) + ": " + object.file + " is too large");
            else
                Gl::BufferSubData(Gl::SHADER_STORAGE_BUFFER, 0, static_cast<GLsizeiptr>(data.size()), data.data());
        }
        Gl::BindBuffer(Gl::SHADER_STORAGE_BUFFER, 0);
        object.allocated = size;
    }
}

void ShaderPipeline::CreateDefaultTextures()
{
    const unsigned char normal[4] = {128, 128, 255, 255};
    const unsigned char specular[4] = {0, 0, 0, 0};
    GLuint* targets[2] = {&mNormalsTexture, &mSpecularTexture};
    const unsigned char* pixels[2] = {normal, specular};
    for (int index = 0; index < 2; ++index)
    {
        Gl::GenTextures(1, targets[index]);
        Gl::ActiveTexture(Gl::TEXTURE0);
        Gl::BindTexture(Gl::TEXTURE_2D, *targets[index]);
        Gl::TexImage2D(Gl::TEXTURE_2D, 0, static_cast<GLint>(Gl::RGBA8), 1, 1, 0, Gl::RGBA, Gl::UNSIGNED_BYTE,
                       pixels[index]);
        Gl::TexParameteri(Gl::TEXTURE_2D, Gl::TEXTURE_MAX_LEVEL, 0);
    }
    Gl::BindTexture(Gl::TEXTURE_2D, 0);

    Gl::GenBuffers(1, &mBlockMapBuffer);
    Gl::BindBuffer(Gl::TEXTURE_BUFFER, mBlockMapBuffer);
    std::vector<int> unmapped(65536 * 2, 0);
    for (size_t element = 0; element < 65536; ++element)
        unmapped[element * 2] = -1;
    Gl::BufferData(Gl::TEXTURE_BUFFER, static_cast<GLsizeiptr>(unmapped.size() * sizeof(int)), unmapped.data(),
                   Gl::STATIC_DRAW);
    Gl::GenTextures(1, &mBlockMapTexture);
    Gl::BindTexture(Gl::TEXTURE_BUFFER, mBlockMapTexture);
    Gl::TexBuffer(Gl::TEXTURE_BUFFER, Gl::RG32I, mBlockMapBuffer);
    Gl::BindTexture(Gl::TEXTURE_BUFFER, 0);
    Gl::BindBuffer(Gl::TEXTURE_BUFFER, 0);
}

void ShaderPipeline::CreateNoiseTexture()
{
    std::string value = mPrograms.Directive("noiseTextureResolution");
    int size = value.empty() ? 256 : std::max(1, std::min(std::atoi(value.c_str()), 4096));
    std::vector<unsigned char> pixels(static_cast<size_t>(size) * size * 3);
    std::mt19937 generator(0x5eedu);
    for (unsigned char& pixel : pixels)
        pixel = static_cast<unsigned char>(generator() & 0xFF);
    Gl::GenTextures(1, &mNoiseTexture);
    Gl::ActiveTexture(Gl::TEXTURE0);
    Gl::BindTexture(Gl::TEXTURE_2D, mNoiseTexture);
    Gl::PixelStorei(Gl::UNPACK_ALIGNMENT, 1);
    Gl::TexImage2D(Gl::TEXTURE_2D, 0, static_cast<GLint>(Gl::RGB8), size, size, 0, Gl::RGB, Gl::UNSIGNED_BYTE,
                   pixels.data());
    Gl::PixelStorei(Gl::UNPACK_ALIGNMENT, 4);
    Gl::TexParameteri(Gl::TEXTURE_2D, Gl::TEXTURE_MAX_LEVEL, 0);
    Gl::BindTexture(Gl::TEXTURE_2D, 0);
}

void ShaderPipeline::DestroyTargets()
{
    for (ColorTarget& target : mColor)
    {
        for (GLuint& texture : target.textures)
        {
            if (texture != 0)
                Gl::DeleteTextures(1, &texture);
            texture = 0;
        }
        target.width = target.height = 0;
    }
    for (GLuint& texture : mDepthTextures)
    {
        if (texture != 0)
            Gl::DeleteTextures(1, &texture);
        texture = 0;
    }
    GLuint* framebuffers[3] = {&mGbufferFramebuffer, &mCompositeFramebuffer, &mCopyFramebuffer};
    for (GLuint* framebuffer : framebuffers)
    {
        if (*framebuffer != 0)
            Gl::DeleteFramebuffers(1, framebuffer);
        *framebuffer = 0;
    }
    std::memset(mAttached, 0, sizeof(mAttached));
    mWidth = mHeight = 0;
}

void ShaderPipeline::ResizeTargets(int width, int height)
{
    int scaledWidth = std::max(1, static_cast<int>(std::lround(width * mRenderScale)));
    int scaledHeight = std::max(1, static_cast<int>(std::lround(height * mRenderScale)));
    mScreenWidth = width;
    mScreenHeight = height;
    if (scaledWidth == mWidth && scaledHeight == mHeight)
        return;
    mWidth = scaledWidth;
    mHeight = scaledHeight;

    for (int buffer = 0; buffer < 16; ++buffer)
    {
        ColorTarget& target = mColor[buffer];
        for (GLuint& texture : target.textures)
        {
            if (texture != 0)
                Gl::DeleteTextures(1, &texture);
            texture = 0;
        }
        if (!target.used)
            continue;
        target.width = target.absolute ? static_cast<int>(target.sizeX)
                                       : std::max(1, static_cast<int>(std::lround(mWidth * target.sizeX)));
        target.height = target.absolute ? static_cast<int>(target.sizeY)
                                        : std::max(1, static_cast<int>(std::lround(mHeight * target.sizeY)));
        int levels = 1;
        if (target.mipmap)
        {
            int largest = std::max(target.width, target.height);
            while (largest > 1)
            {
                largest >>= 1;
                ++levels;
            }
        }
        GLenum format;
        GLenum type;
        TransferFormat(target.format, format, type);
        for (GLuint& texture : target.textures)
        {
            Gl::GenTextures(1, &texture);
            Gl::ActiveTexture(Gl::TEXTURE0);
            Gl::BindTexture(Gl::TEXTURE_2D, texture);
            for (int level = 0; level < levels; ++level)
            {
                Gl::TexImage2D(Gl::TEXTURE_2D, level, static_cast<GLint>(target.format),
                               std::max(1, target.width >> level), std::max(1, target.height >> level), 0, format,
                               type, nullptr);
            }
            Gl::TexParameteri(Gl::TEXTURE_2D, Gl::TEXTURE_MAX_LEVEL, levels - 1);
        }
        target.current = 0;
        target.mipDirty[0] = target.mipDirty[1] = true;

        for (int index = 0; index < 2; ++index)
        {
            Gl::BindFramebuffer(Gl::FRAMEBUFFER, mCopyFramebuffer);
            Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0, Gl::TEXTURE_2D, target.textures[index], 0);
            GLenum drawBuffer = Gl::COLOR_ATTACHMENT0;
            Gl::DrawBuffers(1, &drawBuffer);
            if (IsIntegerFormat(target.format))
            {
                GLint zero[4] = {0, 0, 0, 0};
                Gl::ClearBufferiv(Gl::COLOR, 0, zero);
            }
            else if (IsUnsignedIntegerFormat(target.format))
            {
                GLuint zero[4] = {0, 0, 0, 0};
                Gl::ClearBufferuiv(Gl::COLOR, 0, zero);
            }
            else
            {
                Gl::ClearBufferfv(Gl::COLOR, 0, target.clearColor);
            }
        }
    }
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, mCopyFramebuffer);
    Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0, Gl::TEXTURE_2D, 0, 0);

    for (GLuint& texture : mDepthTextures)
    {
        if (texture != 0)
            Gl::DeleteTextures(1, &texture);
        Gl::GenTextures(1, &texture);
        Gl::ActiveTexture(Gl::TEXTURE0);
        Gl::BindTexture(Gl::TEXTURE_2D, texture);
        Gl::TexImage2D(Gl::TEXTURE_2D, 0, static_cast<GLint>(Gl::DEPTH24_STENCIL8), mWidth, mHeight, 0,
                       Gl::DEPTH_STENCIL, Gl::UNSIGNED_INT_24_8, nullptr);
        Gl::TexParameteri(Gl::TEXTURE_2D, Gl::TEXTURE_MAX_LEVEL, 0);
    }
    Gl::BindTexture(Gl::TEXTURE_2D, 0);

    Gl::BindFramebuffer(Gl::FRAMEBUFFER, mGbufferFramebuffer);
    Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::DEPTH_STENCIL_ATTACHMENT, Gl::TEXTURE_2D, mDepthTextures[0], 0);
    for (int attachment = 0; attachment < 8; ++attachment)
        Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0 + attachment, Gl::TEXTURE_2D, 0, 0);
    std::memset(mAttached, 0, sizeof(mAttached));
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
    CreateImages(true);
    AllocateBufferObjects(true);
    mSetupPending = true;
    mBindingsStamp++;
    LOG_INFO(sLog, "Shader buffers resized to %dx%d", mWidth, mHeight);
}

void ShaderPipeline::CreateShadowTargets()
{
    DestroyShadowTargets();
    int size = mShadowResolution;
    int levels = 1;
    if (mShadowMipmap[0] || mShadowMipmap[1] || mShadowColorMipmap[0] || mShadowColorMipmap[1])
    {
        int largest = size;
        while (largest > 1)
        {
            largest >>= 1;
            ++levels;
        }
    }
    for (int index = 0; index < 2; ++index)
    {
        Gl::GenTextures(1, &mShadowDepth[index]);
        Gl::ActiveTexture(Gl::TEXTURE0);
        Gl::BindTexture(Gl::TEXTURE_2D, mShadowDepth[index]);
        int depthLevels = mShadowMipmap[index] ? levels : 1;
        for (int level = 0; level < depthLevels; ++level)
        {
            Gl::TexImage2D(Gl::TEXTURE_2D, level, static_cast<GLint>(Gl::DEPTH24_STENCIL8), std::max(1, size >> level),
                           std::max(1, size >> level), 0, Gl::DEPTH_STENCIL, Gl::UNSIGNED_INT_24_8, nullptr);
        }
        Gl::TexParameteri(Gl::TEXTURE_2D, Gl::TEXTURE_MAX_LEVEL, depthLevels - 1);
        if (!mShadowColorUsed[index])
            continue;
        Gl::GenTextures(1, &mShadowColor[index]);
        Gl::BindTexture(Gl::TEXTURE_2D, mShadowColor[index]);
        GLenum format;
        GLenum type;
        TransferFormat(mShadowColorFormat[index], format, type);
        int colorLevels = mShadowColorMipmap[index] ? levels : 1;
        for (int level = 0; level < colorLevels; ++level)
        {
            Gl::TexImage2D(Gl::TEXTURE_2D, level, static_cast<GLint>(mShadowColorFormat[index]),
                           std::max(1, size >> level), std::max(1, size >> level), 0, format, type, nullptr);
        }
        Gl::TexParameteri(Gl::TEXTURE_2D, Gl::TEXTURE_MAX_LEVEL, colorLevels - 1);
    }
    Gl::BindTexture(Gl::TEXTURE_2D, 0);
    Gl::GenFramebuffers(1, &mShadowFramebuffer);
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, mShadowFramebuffer);
    Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::DEPTH_STENCIL_ATTACHMENT, Gl::TEXTURE_2D, mShadowDepth[0], 0);
    for (int index = 0; index < 2; ++index)
        Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0 + index, Gl::TEXTURE_2D, mShadowColor[index], 0);
    if (Gl::CheckFramebufferStatus(Gl::FRAMEBUFFER) != Gl::FRAMEBUFFER_COMPLETE)
        mErrors.push_back("the shadow framebuffer is incomplete");
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
}

void ShaderPipeline::DestroyShadowTargets()
{
    for (GLuint* texture : {&mShadowDepth[0], &mShadowDepth[1], &mShadowColor[0], &mShadowColor[1]})
    {
        if (*texture != 0)
            Gl::DeleteTextures(1, texture);
        *texture = 0;
    }
    if (mShadowFramebuffer != 0)
        Gl::DeleteFramebuffers(1, &mShadowFramebuffer);
    mShadowFramebuffer = 0;
}

bool ShaderPipeline::ShadowEnabled() const
{
    return mShadowResolution > 0 && mPrograms.ResolveGbuffer(ShaderGbuffer_Shadow) != nullptr;
}

float ShaderPipeline::ShadowRenderDistance() const
{
    return mShadowDistanceRenderMul > 0.0f ? mShadowDistance * mShadowDistanceRenderMul : -1.0f;
}

void ShaderPipeline::ShadowMatrices(float* view, float* projection) const
{
    std::memcpy(view, mShadowDrawMatrix, sizeof(mShadowDrawMatrix));
    std::memcpy(projection, mShadowProjection, sizeof(mShadowProjection));
}

void ShaderPipeline::UpdateFrameUniforms(const ShaderWorldState& state, int screenWidth, int screenHeight)
{
    double now = NowSeconds();
    float seconds = mLastFrameSeconds < 0.0 ? 0.0f : static_cast<float>(now - mLastFrameSeconds);
    seconds = std::max(0.0f, std::min(seconds, 1.0f));
    mLastFrameSeconds = now;
    mFrameTimeCounter = std::fmod(mFrameTimeCounter + seconds, 3600.0);
    mExpressions.mFrameSeconds = seconds;

    auto setInt = [&](int id, int value) { mFrameValues[id][0] = static_cast<float>(value); };
    auto setFloat = [&](int id, float value) { mFrameValues[id][0] = value; };
    auto setVec = [&](int id, float x, float y, float z, float w = 0.0f) {
        mFrameValues[id][0] = x;
        mFrameValues[id][1] = y;
        mFrameValues[id][2] = z;
        mFrameValues[id][3] = w;
    };
    auto setMatrix = [&](int id, const float* matrix) { std::memcpy(mFrameValues[id], matrix, sizeof(float) * 16); };

    float drawViewInverse[16];
    Invert(state.view, drawViewInverse);
    float eyeOffset[3] = {drawViewInverse[12], drawViewInverse[13], drawViewInverse[14]};
    // LegacyEX: the camera's offset from where the world is drawn from: the eye in first person (Iris's
    // cameraPosition), where the view matrix puts it otherwise. What the view matrix moves beyond that (a small
    // forward nudge, view bobbing) stays in gbufferModelView, as in Iris
    float camOff[3] = {eyeOffset[0], eyeOffset[1], eyeOffset[2]};
    if (state.eyeHeight >= 0.0f)
    {
        camOff[0] = 0.0f;
        camOff[1] = state.eyeHeight;
        camOff[2] = 0.0f;
    }
    float view[16];
    std::memcpy(view, state.view, sizeof(view));
    Translate(view, camOff[0], camOff[1], camOff[2]);
    float viewInverse[16];
    Invert(view, viewInverse);
    double camera[3] = {state.cameraX + camOff[0], state.cameraY + camOff[1], state.cameraZ + camOff[2]};
    float projectionInverse[16];
    Invert(state.projection, projectionInverse);

    setInt(ShaderUniform_HeldItemId, state.heldItemId);
    int heldLight = mDynamicHandLight ? state.heldBlockLight : 0;
    int heldLight2 = mDynamicHandLight ? state.heldBlockLight2 : 0;
    if (mOldHandLight)
        heldLight = std::max(heldLight, heldLight2);
    setInt(ShaderUniform_HeldBlockLightValue, heldLight);
    setInt(ShaderUniform_HeldItemId2, state.heldItemId2);
    setInt(ShaderUniform_HeldBlockLightValue2, heldLight2);
    setVec(ShaderUniform_SkyColor, state.skyColor[0], state.skyColor[1], state.skyColor[2]);
    setVec(ShaderUniform_FogColor, state.fogColor[0], state.fogColor[1], state.fogColor[2]);
    setInt(ShaderUniform_WorldTime, static_cast<int>(state.worldTime % 24000));
    setInt(ShaderUniform_WorldDay, static_cast<int>(state.worldTime / 24000));
    setInt(ShaderUniform_MoonPhase, state.moonPhase);
    setInt(ShaderUniform_FrameCounter, static_cast<int>(mFrame % 720720));
    setFloat(ShaderUniform_FrameTime, seconds);
    setFloat(ShaderUniform_FrameTimeCounter, static_cast<float>(mFrameTimeCounter));

    float celestial = state.celestialAngle;
    float sunAngle = celestial < 0.75f ? celestial + 0.25f : celestial - 0.75f;
    setFloat(ShaderUniform_SunAngle, sunAngle);
    setFloat(ShaderUniform_ShadowAngle, sunAngle <= 0.5f ? sunAngle : sunAngle - 0.5f);
    setFloat(ShaderUniform_RainStrength, state.rainStrength);
    setFloat(ShaderUniform_ThunderStrength, state.thunderStrength);
    setFloat(ShaderUniform_AspectRatio, static_cast<float>(mWidth) / static_cast<float>(std::max(1, mHeight)));
    setFloat(ShaderUniform_ViewWidth, static_cast<float>(mWidth));
    setFloat(ShaderUniform_ViewHeight, static_cast<float>(mHeight));
    setFloat(ShaderUniform_Near, state.nearPlane);
    setFloat(ShaderUniform_Far, state.farPlane);

    float celestialMatrix[16];
    std::memcpy(celestialMatrix, view, sizeof(view));
    Rotate(celestialMatrix, -90.0f, 0.0f, 1.0f, 0.0f);
    Rotate(celestialMatrix, mSunPathRotation, 0.0f, 0.0f, 1.0f);
    Rotate(celestialMatrix, celestial * 360.0f, 1.0f, 0.0f, 0.0f);
    float sunVector[4] = {0.0f, 100.0f, 0.0f, 0.0f};
    float moonVector[4] = {0.0f, -100.0f, 0.0f, 0.0f};
    float sun[4];
    float moon[4];
    float up[4];
    Transform(celestialMatrix, sunVector, sun);
    Transform(celestialMatrix, moonVector, moon);
    Transform(view, sunVector, up);
    setVec(ShaderUniform_SunPosition, sun[0], sun[1], sun[2]);
    setVec(ShaderUniform_MoonPosition, moon[0], moon[1], moon[2]);
    if (sunAngle <= 0.5f)
        setVec(ShaderUniform_ShadowLightPosition, sun[0], sun[1], sun[2]);
    else
        setVec(ShaderUniform_ShadowLightPosition, moon[0], moon[1], moon[2]);
    setVec(ShaderUniform_UpPosition, up[0], up[1], up[2]);

    if (!mHasPrevious)
    {
        std::memcpy(mPreviousView, view, sizeof(view));
        std::memcpy(mPreviousProjection, state.projection, sizeof(mPreviousProjection));
        std::copy(camera, camera + 3, mPreviousCamera);
        mWetness = state.rainStrength;
        mEyeBrightnessSmooth[0] = static_cast<float>(state.eyeBlockLight * 16);
        mEyeBrightnessSmooth[1] = static_cast<float>(state.eyeSkyLight * 16);
    }
    auto setPosition = [&](int id, int intId, int fractId, const double* position) {
        double whole[3] = {std::floor(position[0]), std::floor(position[1]), std::floor(position[2])};
        setVec(id, static_cast<float>(position[0]), static_cast<float>(position[1]), static_cast<float>(position[2]));
        setVec(intId, static_cast<float>(whole[0]), static_cast<float>(whole[1]), static_cast<float>(whole[2]));
        setVec(fractId, static_cast<float>(position[0] - whole[0]), static_cast<float>(position[1] - whole[1]),
               static_cast<float>(position[2] - whole[2]));
    };
    setPosition(ShaderUniform_CameraPosition, ShaderUniform_CameraPositionInt, ShaderUniform_CameraPositionFract,
                camera);
    setPosition(ShaderUniform_PreviousCameraPosition, ShaderUniform_PreviousCameraPositionInt,
                ShaderUniform_PreviousCameraPositionFract, mPreviousCamera);
    setMatrix(ShaderUniform_GbufferModelView, view);
    setMatrix(ShaderUniform_GbufferModelViewInverse, viewInverse);
    setMatrix(ShaderUniform_GbufferPreviousModelView, mPreviousView);
    setMatrix(ShaderUniform_GbufferProjection, state.projection);
    setMatrix(ShaderUniform_GbufferProjectionInverse, projectionInverse);
    setMatrix(ShaderUniform_GbufferPreviousProjection, mPreviousProjection);
    setMatrix(ShaderUniform_CameraView, state.view);
    setMatrix(ShaderUniform_CameraViewInverse, drawViewInverse);

    setVec(ShaderUniform_EyePosition, static_cast<float>(state.eyeX), static_cast<float>(state.eyeY),
           static_cast<float>(state.eyeZ));
    setVec(ShaderUniform_RelativeEyePosition, static_cast<float>(camera[0] - state.eyeX),
           static_cast<float>(camera[1] - state.eyeY), static_cast<float>(camera[2] - state.eyeZ));
    setVec(ShaderUniform_PlayerLookVector, state.lookVector[0], state.lookVector[1], state.lookVector[2]);
    setVec(ShaderUniform_PlayerBodyVector, state.bodyVector[0], state.bodyVector[1], state.bodyVector[2]);
    bool survival = state.health >= 0.0f;
    setFloat(ShaderUniform_CurrentPlayerHealth, survival ? state.health / std::max(state.maxHealth, 1.0f) : -1.0f);
    setFloat(ShaderUniform_MaxPlayerHealth, survival ? state.maxHealth : -1.0f);
    setFloat(ShaderUniform_CurrentPlayerHunger, survival ? state.hunger / 20.0f : -1.0f);
    setFloat(ShaderUniform_MaxPlayerHunger, survival ? 20.0f : -1.0f);
    setFloat(ShaderUniform_CurrentPlayerAir, survival ? state.air / std::max(state.maxAir, 1.0f) : -1.0f);
    setFloat(ShaderUniform_MaxPlayerAir, survival ? state.maxAir : -1.0f);
    setFloat(ShaderUniform_CurrentPlayerArmor, survival ? state.armor / 50.0f : -1.0f);
    setFloat(ShaderUniform_MaxPlayerArmor, survival ? 50.0f : -1.0f);
    setInt(ShaderUniform_IsSpectator, state.spectator ? 1 : 0);
    setInt(ShaderUniform_FirstPersonCamera, state.firstPerson ? 1 : 0);
    setInt(ShaderUniform_IsRightHanded, state.rightHanded ? 1 : 0);
    setInt(ShaderUniform_IsElytraFlying, state.elytraFlying ? 1 : 0);
    setVec(ShaderUniform_LightningBoltPosition, state.lightning[0] - static_cast<float>(camera[0] - state.cameraX),
           state.lightning[1] - static_cast<float>(camera[1] - state.cameraY),
           state.lightning[2] - static_cast<float>(camera[2] - state.cameraZ), state.lightning[3]);
    if (state.lightning[3] == 0.0f)
        setVec(ShaderUniform_LightningBoltPosition, 0.0f, 0.0f, 0.0f, 0.0f);
    setFloat(ShaderUniform_EndFlashIntensity, 0.0f);
    setFloat(ShaderUniform_PreviousEndFlashIntensity, 0.0f);
    setVec(ShaderUniform_EndFlashPosition, 0.0f, 0.0f, 0.0f);
    setFloat(ShaderUniform_CloudHeight, state.cloudHeight);
    setInt(ShaderUniform_HeightLimit, state.heightLimit);
    setInt(ShaderUniform_BedrockLevel, 0);
    setInt(ShaderUniform_LogicalHeightLimit, state.logicalHeightLimit);
    setInt(ShaderUniform_Biome, state.biome);
    setInt(ShaderUniform_BiomeCategory, state.biomeCategory);
    setInt(ShaderUniform_BiomePrecipitation, state.biomePrecipitation);
    setFloat(ShaderUniform_Temperature, state.temperature);
    setFloat(ShaderUniform_Rainfall, state.rainfall);
    for (int flag = 0; flag < ShaderEntity_Count; ++flag)
        setInt(ShaderUniform_IsAlive + flag, state.entityFlags[flag] ? 1 : 0);

    float halfPlane = mShadowDistance;
    std::string fov = mPrograms.Directive("shadowMapFov");
    if (!fov.empty() && std::atof(fov.c_str()) > 0.0)
        Perspective(mShadowProjection, static_cast<float>(std::atof(fov.c_str())), 1.0f, 0.05f, 256.0f);
    else
        Ortho(mShadowProjection, -halfPlane, halfPlane, -halfPlane, halfPlane, 0.05f, 256.0f);
    Identity(mShadowMatrix);
    Translate(mShadowMatrix, 0.0f, 0.0f, -100.0f);
    Rotate(mShadowMatrix, 90.0f, 1.0f, 0.0f, 0.0f);
    float angle = celestial * -360.0f;
    if (sunAngle <= 0.5f)
        Rotate(mShadowMatrix, angle, 0.0f, 0.0f, 1.0f);
    else
        Rotate(mShadowMatrix, angle + 180.0f, 0.0f, 0.0f, 1.0f);
    Rotate(mShadowMatrix, mSunPathRotation, 1.0f, 0.0f, 0.0f);
    if (fov.empty() || std::atof(fov.c_str()) <= 0.0)
    {
        float interval = mShadowIntervalSize;
        if (interval > 0.0f)
        {
            float half = interval * 0.5f;
            Translate(mShadowMatrix, static_cast<float>(std::fmod(camera[0], static_cast<double>(interval))) - half,
                      static_cast<float>(std::fmod(camera[1], static_cast<double>(interval))) - half,
                      static_cast<float>(std::fmod(camera[2], static_cast<double>(interval))) - half);
        }
    }
    float shadowInverse[16];
    float shadowProjectionInverse[16];
    Invert(mShadowMatrix, shadowInverse);
    Invert(mShadowProjection, shadowProjectionInverse);
    std::memcpy(mShadowDrawMatrix, mShadowMatrix, sizeof(mShadowMatrix));
    Translate(mShadowDrawMatrix, -camOff[0], -camOff[1], -camOff[2]);
    setMatrix(ShaderUniform_ShadowModelView, mShadowMatrix);
    setMatrix(ShaderUniform_ShadowModelViewInverse, shadowInverse);
    setMatrix(ShaderUniform_ShadowProjection, mShadowProjection);
    setMatrix(ShaderUniform_ShadowProjectionInverse, shadowProjectionInverse);

    float rain = state.rainStrength;
    mWetness = Halflife(mWetness, rain, seconds, rain > mWetness ? mWetnessHalflife : mDrynessHalflife);
    setFloat(ShaderUniform_Wetness, mWetness);
    setFloat(ShaderUniform_EyeAltitude, state.eyeAltitude);
    float eyeBlock = static_cast<float>(state.eyeBlockLight * 16);
    float eyeSky = static_cast<float>(state.eyeSkyLight * 16);
    mEyeBrightnessSmooth[0] = Halflife(mEyeBrightnessSmooth[0], eyeBlock, seconds, mEyeBrightnessHalflife);
    mEyeBrightnessSmooth[1] = Halflife(mEyeBrightnessSmooth[1], eyeSky, seconds, mEyeBrightnessHalflife);
    setVec(ShaderUniform_EyeBrightness, eyeBlock, eyeSky, 0.0f);
    setVec(ShaderUniform_EyeBrightnessSmooth, std::round(mEyeBrightnessSmooth[0]), std::round(mEyeBrightnessSmooth[1]),
           0.0f);
    setVec(ShaderUniform_TerrainTextureSize, static_cast<float>(state.atlasWidth), static_cast<float>(state.atlasHeight),
           0.0f);
    setInt(ShaderUniform_TerrainIconSize, 16);
    setVec(ShaderUniform_AtlasSize, static_cast<float>(state.atlasWidth), static_cast<float>(state.atlasHeight), 0.0f);
    setInt(ShaderUniform_IsEyeInWater, state.isEyeInWater);
    setFloat(ShaderUniform_NightVision, state.nightVision);
    setFloat(ShaderUniform_Blindness, state.blindness);
    setFloat(ShaderUniform_DarknessFactor, 0.0f);
    setFloat(ShaderUniform_DarknessLightFactor, 0.0f);
    setFloat(ShaderUniform_ScreenBrightness, state.screenBrightness);
    setInt(ShaderUniform_HideGUI, state.hideGui ? 1 : 0);
    mCenterDepthSmooth = Halflife(mCenterDepthSmooth, mCenterDepth, seconds, mCenterDepthHalflife);
    setFloat(ShaderUniform_CenterDepthSmooth, mCenterDepthSmooth);
    setFloat(ShaderUniform_PlayerMood, state.playerMood);
    setInt(ShaderUniform_BossBattle, state.bossBattle);

    std::copy(camera, camera + 3, mPreviousCamera);
    std::memcpy(mPreviousView, view, sizeof(view));
    std::memcpy(mPreviousProjection, state.projection, sizeof(mPreviousProjection));
    mHasPrevious = true;
    (void)screenWidth;
    (void)screenHeight;
}

void ShaderPipeline::EvaluateCustomUniforms()
{
    for (int id = 0; id < ShaderUniform_Count; ++id)
    {
        int slot = mParameterSlots[id];
        if (slot < 0)
            continue;
        ShaderValue& value = mExpressions.Slot(slot);
        for (int component = 0; component < 4; ++component)
            value.v[component] = mFrameValues[id][component];
    }
    for (const auto& matrix : mMatrixSlots)
    {
        for (int element = 0; element < 16; ++element)
            mExpressions.Set(matrix.second + element, mFrameValues[matrix.first][element]);
    }
    for (CustomUniform& custom : mCustomUniforms)
    {
        ShaderValue value = custom.expression.Evaluate(mExpressions);
        ShaderValue& slot = mExpressions.Slot(custom.slot);
        int size = slot.size;
        if (custom.type == 1)
            value.v[0] = static_cast<float>(static_cast<int>(value.v[0]));
        else if (custom.type == 2)
            value.v[0] = value.v[0] != 0.0f ? 1.0f : 0.0f;
        for (int component = 0; component < 4; ++component)
            slot.v[component] = component < size ? value.v[value.size == 1 ? 0 : component] : 0.0f;
    }
}

bool ShaderPipeline::BeginWorld(const ShaderWorldState& state, int screenWidth, int screenHeight)
{
    if (mReloadRequested || state.dimension != mDimension)
    {
        mReloadRequested = false;
        Reload(state.dimension);
    }
    if (!mLoaded || screenWidth <= 0 || screenHeight <= 0)
        return false;

    if (mBlockMapDirty && mBlockMapBuffer != 0 && mPendingBlockMap.size() == 65536 * 2)
    {
        Gl::BindBuffer(Gl::TEXTURE_BUFFER, mBlockMapBuffer);
        Gl::BufferSubData(Gl::TEXTURE_BUFFER, 0, static_cast<GLsizeiptr>(mPendingBlockMap.size() * sizeof(int)),
                          mPendingBlockMap.data());
        Gl::BindBuffer(Gl::TEXTURE_BUFFER, 0);
        mBlockMapDirty = false;
    }

    ResizeTargets(screenWidth, screenHeight);
    ++mFrame;
    ++mBindingsStamp;
    ReadCenterDepth();
    UpdateFrameUniforms(state, screenWidth, screenHeight);
    EvaluateCustomUniforms();

    Gl::Disable(Gl::SCISSOR_TEST);
    Gl::ColorMask(1, 1, 1, 1);
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, mCopyFramebuffer);
    GLenum drawBuffer = Gl::COLOR_ATTACHMENT0;
    for (int buffer = 0; buffer < 16; ++buffer)
    {
        ColorTarget& target = mColor[buffer];
        if (!target.used || !target.clear)
            continue;
        // LegacyEX: both of the buffer's textures, as Iris does: a prepare pass that draws nothing still flips the
        // buffer, and the world is then drawn into the other one (Visual Vibrance's prepare2 left the sky under
        // the horizon black)
        for (int side = 0; side < 2; ++side)
        {
            GLuint texture = target.textures[side == 0 ? target.current : 1 - target.current];
            if (texture == 0)
                continue;
            Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0, Gl::TEXTURE_2D, texture, 0);
            Gl::DrawBuffers(1, &drawBuffer);
            Gl::Viewport(0, 0, target.width, target.height);
            float color[4];
            std::memcpy(color, target.clearColor, sizeof(color));
            if (buffer == 0 && mPrograms.Directive("colortex0ClearColor").empty()
                && mPrograms.Directive("gcolorClearColor").empty())
            {
                color[0] = state.fogColor[0];
                color[1] = state.fogColor[1];
                color[2] = state.fogColor[2];
                color[3] = 1.0f;
            }
            if (IsIntegerFormat(target.format))
            {
                GLint values[4] = {static_cast<GLint>(color[0]), static_cast<GLint>(color[1]), static_cast<GLint>(color[2]),
                                   static_cast<GLint>(color[3])};
                Gl::ClearBufferiv(Gl::COLOR, 0, values);
            }
            else if (IsUnsignedIntegerFormat(target.format))
            {
                GLuint values[4] = {static_cast<GLuint>(color[0]), static_cast<GLuint>(color[1]),
                                    static_cast<GLuint>(color[2]), static_cast<GLuint>(color[3])};
                Gl::ClearBufferuiv(Gl::COLOR, 0, values);
            }
            else
            {
                Gl::ClearBufferfv(Gl::COLOR, 0, color);
            }
        }
        target.mipDirty[0] = target.mipDirty[1] = true;
    }
    Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0, Gl::TEXTURE_2D, 0, 0);

    Gl::BindFramebuffer(Gl::FRAMEBUFFER, mGbufferFramebuffer);
    Gl::DepthMask(1);
    Gl::StencilMask(0xFF);
    Gl::ClearBufferfi(Gl::DEPTH_STENCIL, 0, 1.0f, 0);
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);

    mInWorld = true;
    mInShadow = false;
    mInHand = false;
    mTranslucent = false;
    mStage = ShaderRenderStage_None;
    mEntityId = -1;
    mBlockEntityId = -1;
    mBoundProgram = nullptr;
    mPrepareDone = false;
    mDepth2Copied = false;

    for (int index = 0; index < 9; ++index)
    {
        if (mBufferObjects[index].buffer != 0)
            Gl::BindBufferBase(Gl::SHADER_STORAGE_BUFFER, static_cast<GLuint>(index), mBufferObjects[index].buffer);
    }
    ClearImages(false);
    if (mSetupPending)
    {
        mSetupPending = false;
        RunPasses(ShaderPass_Setup);
    }
    RunPasses(ShaderPass_Begin);
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
    return true;
}

void ShaderPipeline::AbortWorld()
{
    mInWorld = false;
    mInShadow = false;
    mInHand = false;
    mBoundProgram = nullptr;
}

void ShaderPipeline::SetStage(int stage)
{
    if (!mInWorld)
        return;
    if (!mPrepareDone && !mInShadow)
    {
        mPrepareDone = true;
        RunPasses(ShaderPass_Prepare);
    }
    mStage = stage;
    mBoundProgram = nullptr;
}

void ShaderPipeline::ClearTargets(bool colour, bool depth)
{
    if (!mInWorld)
        return;
    GLuint framebuffer = mInShadow ? mShadowFramebuffer : mGbufferFramebuffer;
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, framebuffer);
    Gl::Disable(Gl::SCISSOR_TEST);
    if (depth)
    {
        Gl::DepthMask(1);
        Gl::StencilMask(0xFF);
        Gl::ClearBufferfi(Gl::DEPTH_STENCIL, 0, 1.0f, 0);
    }
    if (colour && !mInShadow)
    {
        float black[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        for (int attachment = 0; attachment < 8; ++attachment)
        {
            if (mAttached[attachment] != 0)
                Gl::ClearBufferfv(Gl::COLOR, attachment, black);
        }
    }
}

void ShaderPipeline::CopyDepth(GLuint destination)
{
    Gl::BindFramebuffer(Gl::READ_FRAMEBUFFER, mGbufferFramebuffer);
    Gl::BindFramebuffer(Gl::DRAW_FRAMEBUFFER, mCopyFramebuffer);
    Gl::FramebufferTexture2D(Gl::DRAW_FRAMEBUFFER, Gl::DEPTH_STENCIL_ATTACHMENT, Gl::TEXTURE_2D, destination, 0);
    Gl::Disable(Gl::SCISSOR_TEST);
    Gl::BlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, Gl::DEPTH_BUFFER_BIT | Gl::STENCIL_BUFFER_BIT,
                        Gl::NEAREST);
    Gl::FramebufferTexture2D(Gl::DRAW_FRAMEBUFFER, Gl::DEPTH_STENCIL_ATTACHMENT, Gl::TEXTURE_2D, 0, 0);
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
}

void ShaderPipeline::BeginTranslucent()
{
    if (!mInWorld || mTranslucent)
        return;
    if (!mPrepareDone)
    {
        mPrepareDone = true;
        RunPasses(ShaderPass_Prepare);
    }
    CopyDepth(mDepthTextures[1]);
    if (!mDepth2Copied)
        CopyDepth(mDepthTextures[2]);
    mDepth2Copied = true;
    Barrier();
    RunPasses(ShaderPass_Deferred);
    mTranslucent = true;
    mBoundProgram = nullptr;
}

void ShaderPipeline::BeginHand()
{
    if (!mInWorld)
        return;
    if (!mPrepareDone)
    {
        mPrepareDone = true;
        RunPasses(ShaderPass_Prepare);
    }
    if (!mDepth2Copied)
        CopyDepth(mDepthTextures[2]);
    mDepth2Copied = true;
    mInHand = true;
    mStage = mTranslucent ? ShaderRenderStage_HandTranslucent : ShaderRenderStage_HandSolid;
    mBoundProgram = nullptr;
}

void ShaderPipeline::EndHand()
{
    mInHand = false;
    mBoundProgram = nullptr;
}

bool ShaderPipeline::BeginShadow()
{
    if (!mInWorld || !ShadowEnabled() || mShadowFramebuffer == 0)
        return false;
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, mShadowFramebuffer);
    Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::DEPTH_STENCIL_ATTACHMENT, Gl::TEXTURE_2D, mShadowDepth[0], 0);
    Gl::Disable(Gl::SCISSOR_TEST);
    Gl::ColorMask(1, 1, 1, 1);
    Gl::DepthMask(1);
    Gl::StencilMask(0xFF);
    GLenum buffers[2] = {Gl::COLOR_ATTACHMENT0, Gl::COLOR_ATTACHMENT0 + 1};
    Gl::DrawBuffers(2, buffers);
    Gl::Viewport(0, 0, mShadowResolution, mShadowResolution);
    Gl::ClearBufferfi(Gl::DEPTH_STENCIL, 0, 1.0f, 0);
    for (int index = 0; index < 2; ++index)
    {
        if (mShadowColor[index] != 0 && mShadowColorClear[index])
            Gl::ClearBufferfv(Gl::COLOR, index, mShadowColorClearColor[index]);
    }
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
    mInShadow = true;
    mStage = ShaderRenderStage_None;
    mBoundProgram = nullptr;
    return true;
}

void ShaderPipeline::EndShadowOpaque()
{
    if (!mInShadow)
        return;
    Gl::BindFramebuffer(Gl::READ_FRAMEBUFFER, mShadowFramebuffer);
    Gl::BindFramebuffer(Gl::DRAW_FRAMEBUFFER, mCopyFramebuffer);
    Gl::FramebufferTexture2D(Gl::DRAW_FRAMEBUFFER, Gl::DEPTH_STENCIL_ATTACHMENT, Gl::TEXTURE_2D, mShadowDepth[1], 0);
    Gl::Disable(Gl::SCISSOR_TEST);
    Gl::BlitFramebuffer(0, 0, mShadowResolution, mShadowResolution, 0, 0, mShadowResolution, mShadowResolution,
                        Gl::DEPTH_BUFFER_BIT | Gl::STENCIL_BUFFER_BIT, Gl::NEAREST);
    Gl::FramebufferTexture2D(Gl::DRAW_FRAMEBUFFER, Gl::DEPTH_STENCIL_ATTACHMENT, Gl::TEXTURE_2D, 0, 0);
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
    mShadowOpaqueCopied = true;
}

void ShaderPipeline::EndShadow()
{
    if (!mInShadow)
        return;
    if (!mShadowOpaqueCopied)
        EndShadowOpaque();
    mShadowOpaqueCopied = false;
    mInShadow = false;
    for (int index = 0; index < 2; ++index)
    {
        Gl::ActiveTexture(Gl::TEXTURE0);
        if (mShadowMipmap[index] && mShadowDepth[index] != 0)
        {
            Gl::BindTexture(Gl::TEXTURE_2D, mShadowDepth[index]);
            Gl::GenerateMipmap(Gl::TEXTURE_2D);
        }
        if (mShadowColorMipmap[index] && mShadowColor[index] != 0)
        {
            Gl::BindTexture(Gl::TEXTURE_2D, mShadowColor[index]);
            Gl::GenerateMipmap(Gl::TEXTURE_2D);
        }
    }
    Gl::BindTexture(Gl::TEXTURE_2D, 0);
    Barrier();
    RunPasses(ShaderPass_ShadowComp);
    mPrepareDone = true;
    RunPasses(ShaderPass_Prepare);
    mBoundProgram = nullptr;
}

int ShaderPipeline::GbufferForStage(int vanillaPixelShader, ShaderAdapter adapter) const
{
    bool emissive = vanillaPixelShader == ePcPixelShader_EntityEmissiveAlpha
                    || vanillaPixelShader == ePcPixelShader_EntityEmissiveAlphaOnly;
    bool glint = adapter == ShaderAdapter_TexGen;
    switch (mStage)
    {
    case ShaderRenderStage_Sky:
    case ShaderRenderStage_Sunset:
    case ShaderRenderStage_Stars:
    case ShaderRenderStage_Void:
        return ShaderGbuffer_SkyBasic;
    case ShaderRenderStage_Sun:
    case ShaderRenderStage_Moon:
    case ShaderRenderStage_CustomSky:
        return ShaderGbuffer_SkyTextured;
    case ShaderRenderStage_TerrainSolid:
        return ShaderGbuffer_Terrain;
    case ShaderRenderStage_TerrainCutoutMipped:
        return ShaderGbuffer_TerrainCutoutMip;
    case ShaderRenderStage_TerrainCutout:
        return ShaderGbuffer_TerrainCutout;
    case ShaderRenderStage_TerrainTranslucent:
    case ShaderRenderStage_Tripwire:
        return ShaderGbuffer_Water;
    case ShaderRenderStage_Entities:
        return glint ? ShaderGbuffer_ArmorGlint : emissive ? ShaderGbuffer_SpiderEyes : ShaderGbuffer_Entities;
    case ShaderRenderStage_BlockEntities:
        return glint ? ShaderGbuffer_ArmorGlint : ShaderGbuffer_Block;
    case ShaderRenderStage_Destroy:
        return ShaderGbuffer_DamagedBlock;
    case ShaderRenderStage_Outline:
    case ShaderRenderStage_Debug:
    case ShaderRenderStage_WorldBorder:
        return ShaderGbuffer_Basic;
    case ShaderRenderStage_HandSolid:
        return glint ? ShaderGbuffer_ArmorGlint : ShaderGbuffer_Hand;
    case ShaderRenderStage_HandTranslucent:
        return glint ? ShaderGbuffer_ArmorGlint : ShaderGbuffer_HandWater;
    case ShaderRenderStage_Particles:
        return ShaderGbuffer_TexturedLit;
    case ShaderRenderStage_Clouds:
        return ShaderGbuffer_Clouds;
    case ShaderRenderStage_RainSnow:
        return ShaderGbuffer_Weather;
    default:
        return glint ? ShaderGbuffer_ArmorGlint : ShaderGbuffer_Textured;
    }
}

const ShaderLinkedProgram* ShaderPipeline::SelectProgram(ShaderAdapter adapter, int vanillaPixelShader)
{
    mCurrentSource = nullptr;
    mCurrentGbuffer = -1;
    if (!mInWorld)
        return nullptr;
    ShaderProgramSource* source = nullptr;
    if (mInShadow)
    {
        mCurrentGbuffer = ShaderGbuffer_Shadow;
        source = mPrograms.ResolveGbuffer(ShaderGbuffer_Shadow);
        if (source == nullptr)
            return nullptr;
    }
    else
    {
        mCurrentGbuffer = GbufferForStage(vanillaPixelShader, adapter);
        source = mPrograms.ResolveGbuffer(mCurrentGbuffer);
    }

    const ShaderLinkedProgram* linked = nullptr;
    if (source != nullptr)
    {
        std::string error;
        bool attempted = source->attempted[adapter];
        linked = mPrograms.Link(*source, adapter, error);
        if (linked == nullptr && !attempted && !error.empty())
        {
            LOG_WARN(sLog, "%s", error.c_str());
            mErrors.push_back(error);
        }
    }
    if (linked == nullptr && !mInShadow && mFallback)
    {
        std::string error;
        bool attempted = mFallback->attempted[adapter];
        source = mFallback.get();
        linked = mPrograms.Link(*source, adapter, error);
        if (linked == nullptr && !attempted && !error.empty())
            LOG_ERROR(sLog, "%s", error.c_str());
    }
    if (linked != nullptr)
        mCurrentSource = source;
    return linked;
}

bool ShaderPipeline::PrepareTargets(const ShaderLinkedProgram* program, ShaderDrawTarget& target)
{
    if (program == nullptr || mCurrentSource == nullptr)
        return false;
    const std::vector<int>& drawBuffers = mCurrentSource->drawBuffers;
    GLenum buffers[8];
    int count = 0;

    if (mInShadow)
    {
        target.framebuffer = mShadowFramebuffer;
        target.width = target.height = mShadowResolution;
        Gl::BindFramebuffer(Gl::FRAMEBUFFER, mShadowFramebuffer);
        for (int buffer : drawBuffers)
        {
            if (count >= 8)
                break;
            buffers[count++] = buffer < 2 && mShadowColor[buffer] != 0 ? Gl::COLOR_ATTACHMENT0 + buffer : Gl::NONE;
        }
        Gl::DrawBuffers(count, buffers);
    }
    else
    {
        target.framebuffer = mGbufferFramebuffer;
        target.width = mWidth;
        target.height = mHeight;
        Gl::BindFramebuffer(Gl::FRAMEBUFFER, mGbufferFramebuffer);
        for (int buffer : drawBuffers)
        {
            if (count >= 8)
                break;
            const ColorTarget& color = mColor[buffer];
            GLuint texture = color.textures[color.current];
            if (color.width != mWidth || color.height != mHeight)
                texture = 0;
            if (mAttached[count] != texture)
            {
                Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0 + count, Gl::TEXTURE_2D, texture, 0);
                mAttached[count] = texture;
            }
            buffers[count] = texture != 0 ? Gl::COLOR_ATTACHMENT0 + count : Gl::NONE;
            mColor[buffer].mipDirty[color.current] = true;
            ++count;
        }
        Gl::DrawBuffers(count, buffers);
    }

    target.blendOverride = false;
    std::string blend = ProgramProperty("blend.", mCurrentSource->name);
    std::vector<std::string> parts = Split(blend);
    if (parts.size() == 1 && (parts[0] == "off" || parts[0] == "false"))
    {
        target.blendOverride = true;
        target.blendEnable = false;
    }
    else if (parts.size() >= 4)
    {
        bool valid = true;
        for (int index = 0; index < 4; ++index)
        {
            bool factorValid = false;
            target.blend[index] = BlendFactor(parts[index], factorValid);
            valid = valid && factorValid;
        }
        if (valid)
        {
            target.blendOverride = true;
            target.blendEnable = true;
        }
    }
    target.bufferBlendOff = 0;
    for (int index = 0; index < count && index < static_cast<int>(drawBuffers.size()); ++index)
    {
        std::string name = mInShadow ? "shadowcolor" + std::to_string(drawBuffers[index])
                                     : "colortex" + std::to_string(drawBuffers[index]);
        std::string value = ProgramProperty("blend.", mCurrentSource->name + "." + name);
        if (value == "off" || value == "false")
            target.bufferBlendOff |= 1u << index;
    }
    return true;
}

void ShaderPipeline::BindTerrainData(GLuint vertexTexture, GLuint quadTexture)
{
    Gl::ActiveTexture(Gl::TEXTURE0 + ShaderTextureUnit_TerrainVertices);
    Gl::BindTexture(Gl::TEXTURE_BUFFER, vertexTexture);
    Gl::BindSampler(ShaderTextureUnit_TerrainVertices, 0);
    Gl::ActiveTexture(Gl::TEXTURE0 + ShaderTextureUnit_QuadData);
    Gl::BindTexture(Gl::TEXTURE_BUFFER, quadTexture);
    Gl::BindSampler(ShaderTextureUnit_QuadData, 0);
    Gl::ActiveTexture(Gl::TEXTURE0);
}

GLuint ShaderPipeline::SamplerFor(int source, bool shadow)
{
    if (source >= ShaderSource_ShadowTex0 && source <= ShaderSource_ShadowTex1)
    {
        int index = source - ShaderSource_ShadowTex0;
        bool nearest = mShadowNearest[index];
        if (shadow || mShadowHardware[index])
            return shadow ? mSamplers[nearest ? SamplerKind_ShadowNearest : SamplerKind_ShadowLinear]
                          : mSamplers[nearest ? SamplerKind_Nearest : SamplerKind_Linear];
        if (mShadowMipmap[index])
            return mSamplers[nearest ? SamplerKind_NearestMip : SamplerKind_LinearMip];
        return mSamplers[nearest ? SamplerKind_Nearest : SamplerKind_Linear];
    }
    if (source >= ShaderSource_ShadowColor0 && source <= ShaderSource_ShadowColor1)
    {
        int index = source - ShaderSource_ShadowColor0;
        bool nearest = mShadowColorNearest[index];
        if (mShadowColorMipmap[index])
            return mSamplers[nearest ? SamplerKind_NearestMip : SamplerKind_LinearMip];
        return mSamplers[nearest ? SamplerKind_Nearest : SamplerKind_Linear];
    }
    if (source >= ShaderSource_ColorTex0 && source < ShaderSource_ColorTex0 + 16)
    {
        const ColorTarget& target = mColor[source - ShaderSource_ColorTex0];
        if (IsIntegerFormat(target.format) || IsUnsignedIntegerFormat(target.format))
            return mSamplers[SamplerKind_Nearest];
        return mSamplers[target.mipmap ? SamplerKind_LinearMip : SamplerKind_Linear];
    }
    if (source >= ShaderSource_DepthTex0 && source <= ShaderSource_DepthTex2)
        return mSamplers[shadow ? SamplerKind_ShadowNearest : SamplerKind_Nearest];
    if (source == ShaderSource_Noise)
        return mSamplers[SamplerKind_Repeat];
    if (source == ShaderSource_Normals || source == ShaderSource_Specular)
        return mSamplers[SamplerKind_Nearest];
    if (source == ShaderSource_Lightmap)
        return mSamplers[SamplerKind_Linear];
    return 0;
}

GLuint ShaderPipeline::SourceTexture(int source, int custom, bool composite, GLenum& target)
{
    target = Gl::TEXTURE_2D;
    if (source == ShaderSource_Custom && custom >= 0 && custom < static_cast<int>(mCustomTextures.size()))
    {
        CustomTexture& texture = mCustomTextures[custom];
        target = texture.target;
        if (texture.image >= 0 && texture.image < static_cast<int>(mImages.size()))
        {
            target = mImages[texture.image].target;
            return mImages[texture.image].texture;
        }
        if (!texture.resource.empty())
            texture.texture = mResourceResolver != nullptr ? mResourceResolver(texture.resource) : 0;
        return texture.texture;
    }
    if (source >= ShaderSource_ColorTex0 && source < ShaderSource_ColorTex0 + 16)
    {
        ColorTarget& color = mColor[source - ShaderSource_ColorTex0];
        GLuint texture = color.textures[color.current];
        if (texture != 0 && color.mipmap && color.mipDirty[color.current] && composite)
        {
            Gl::ActiveTexture(Gl::TEXTURE0);
            Gl::BindTexture(Gl::TEXTURE_2D, texture);
            Gl::GenerateMipmap(Gl::TEXTURE_2D);
            color.mipDirty[color.current] = false;
        }
        return texture;
    }
    switch (source)
    {
    case ShaderSource_Normals:
        return mNormalsTexture;
    case ShaderSource_Specular:
        return mSpecularTexture;
    case ShaderSource_BlockMap:
        target = Gl::TEXTURE_BUFFER;
        return mBlockMapTexture;
    case ShaderSource_Lightmap:
        return mLightmapTexture;
    case ShaderSource_DepthTex0:
        return mDepthTextures[0];
    case ShaderSource_DepthTex1:
        return mDepthTextures[1];
    case ShaderSource_DepthTex2:
        return mDepthTextures[2];
    case ShaderSource_ShadowTex0:
        return mShadowDepth[0];
    case ShaderSource_ShadowTex1:
        return mShadowDepth[1];
    case ShaderSource_ShadowColor0:
        return mShadowColor[0];
    case ShaderSource_ShadowColor1:
        return mShadowColor[1];
    case ShaderSource_Noise:
        return mNoiseTexture;
    default:
        return 0;
    }
}

void ShaderPipeline::BindSamplers(const ShaderLinkedProgram& program, bool composite)
{
    for (const ShaderLinkedProgram::Sampler& sampler : program.samplers)
    {
        // LegacyEX: the renderer binds the terrain buffer textures itself, per buffer
        if (sampler.source == ShaderSource_TerrainVertices || sampler.source == ShaderSource_QuadData
            || sampler.source == ShaderSource_ChunkOffsets)
            continue;
        GLenum target = Gl::TEXTURE_2D;
        GLuint texture = SourceTexture(sampler.source, sampler.custom, composite, target);
        GLuint samplerObject = 0;
        if (sampler.source == ShaderSource_Custom)
        {
            const CustomTexture& custom = mCustomTextures[sampler.custom];
            if (custom.clamp)
                samplerObject = mSamplers[custom.linear ? SamplerKind_Linear : SamplerKind_Nearest];
            else
                samplerObject = mSamplers[custom.linear ? SamplerKind_Repeat : SamplerKind_NearestRepeat];
        }
        else if (target != Gl::TEXTURE_BUFFER)
        {
            samplerObject = SamplerFor(sampler.source, sampler.shadow);
        }
        Gl::ActiveTexture(Gl::TEXTURE0 + static_cast<GLenum>(sampler.unit));
        Gl::BindTexture(target, texture);
        Gl::BindSampler(static_cast<GLuint>(sampler.unit), samplerObject);
    }
    Gl::ActiveTexture(Gl::TEXTURE0);
    BindImages(program);
}

void ShaderPipeline::BindImages(const ShaderLinkedProgram& program)
{
    if (program.images.empty() || Gl::BindImageTexture == nullptr)
        return;
    for (const ShaderLinkedProgram::Image& image : program.images)
    {
        GLuint texture = 0;
        GLenum format = 0;
        GLboolean layered = Gl::FALSE_VALUE;
        if (image.source >= ShaderSource_ColorTex0 && image.source < ShaderSource_ColorTex0 + 16)
        {
            const ColorTarget& color = mColor[image.source - ShaderSource_ColorTex0];
            texture = color.textures[color.current];
            format = color.format;
        }
        else if (image.source == ShaderSource_ShadowColor0 || image.source == ShaderSource_ShadowColor1)
        {
            int index = image.source - ShaderSource_ShadowColor0;
            texture = mShadowColor[index];
            format = mShadowColorFormat[index];
        }
        else if (image.source == ShaderSource_Custom && image.custom >= 0
                 && image.custom < static_cast<int>(mImages.size()))
        {
            const CustomImage& custom = mImages[image.custom];
            texture = custom.texture;
            format = custom.internal;
            layered = custom.target == Gl::TEXTURE_3D ? Gl::TRUE_VALUE : Gl::FALSE_VALUE;
        }
        if (texture == 0 || format == 0)
            continue;
        Gl::BindImageTexture(image.unit, texture, 0, layered, 0, Gl::READ_WRITE, format);
    }
}

void ShaderPipeline::Barrier()
{
    if (Gl::MemoryBarrier != nullptr && (mComputeSupported || mImagesSupported || mBuffersSupported))
        Gl::MemoryBarrier(Gl::ALL_BARRIER_BITS);
}

void ShaderPipeline::ApplyPassBlend(const ShaderProgramSource& pass, const std::vector<int>& written)
{
    Gl::Disable(Gl::BLEND);
    std::vector<std::string> parts = Split(ProgramProperty("blend.", pass.name));
    if (parts.size() < 4)
        return;
    GLenum factors[4];
    for (int index = 0; index < 4; ++index)
    {
        bool valid = false;
        factors[index] = BlendFactor(parts[index], valid);
        if (!valid)
            return;
    }
    Gl::Enable(Gl::BLEND);
    Gl::BlendFuncSeparate(factors[0], factors[1], factors[2], factors[3]);
    for (size_t index = 0; index < written.size(); ++index)
    {
        std::string value = ProgramProperty("blend.", pass.name + ".colortex" + std::to_string(written[index]));
        std::vector<std::string> bufferParts = Split(value);
        if (bufferParts.size() == 1 && (bufferParts[0] == "off" || bufferParts[0] == "false"))
        {
            Gl::Disablei(Gl::BLEND, static_cast<GLuint>(index));
            continue;
        }
        if (bufferParts.size() < 4 || Gl::BlendFuncSeparatei == nullptr)
            continue;
        GLenum bufferFactors[4];
        bool valid = true;
        for (int part = 0; part < 4 && valid; ++part)
            bufferFactors[part] = BlendFactor(bufferParts[part], valid);
        if (valid)
            Gl::BlendFuncSeparatei(static_cast<GLuint>(index), bufferFactors[0], bufferFactors[1], bufferFactors[2],
                                   bufferFactors[3]);
    }
}

void ShaderPipeline::RunCompute(ShaderProgramSource& pass, ShaderComputeSource& compute)
{
    bool attempted = compute.attempted;
    std::string error;
    const ShaderLinkedProgram* program = mPrograms.LinkCompute(pass, compute, error);
    if (program == nullptr)
    {
        if (!attempted && !error.empty())
        {
            LOG_WARN(sLog, "%s", error.c_str());
            mErrors.push_back(error);
        }
        return;
    }
    Gl::UseProgram(program->name);
    ShaderLinkedProgram& mutableProgram = const_cast<ShaderLinkedProgram&>(*program);
    if (program->frameStamp != mFrame)
    {
        UploadFrameUniforms(*program);
        mutableProgram.frameStamp = mFrame;
    }
    // LegacyEX: the draw uniforms too (fogColor and the like), as a composite pass gets them; without them Visual
    // Vibrance's sunlight, worked out from fogColor in a compute shader, was black
    ShaderDrawValues values = mLastDrawValues;
    values.quadData = false;
    int stage = mStage;
    mStage = ShaderRenderStage_None;
    UploadDrawUniforms(*program, values);
    mStage = stage;
    BindSamplers(*program, true);
    mBoundProgram = nullptr;

    GLuint groups[3] = {1, 1, 1};
    if (compute.absolute)
    {
        for (int axis = 0; axis < 3; ++axis)
            groups[axis] = static_cast<GLuint>(std::max(1, compute.workGroups[axis]));
    }
    else
    {
        float width = static_cast<float>(mWidth) * compute.workGroupsRender[0];
        float height = static_cast<float>(mHeight) * compute.workGroupsRender[1];
        groups[0] = static_cast<GLuint>(std::max(1.0f, std::ceil(width / static_cast<float>(compute.localSize[0]))));
        groups[1] = static_cast<GLuint>(std::max(1.0f, std::ceil(height / static_cast<float>(compute.localSize[1]))));
    }
    Gl::DispatchCompute(groups[0], groups[1], groups[2]);
    Barrier();
}

void ShaderPipeline::UploadFrameUniforms(const ShaderLinkedProgram& program)
{
    for (const ShaderLinkedProgram::Uniform& uniform : program.frameUniforms)
    {
        const float* value = mFrameValues[uniform.id];
        switch (ShaderUniformCatalogue(uniform.id).type)
        {
        case ShaderUniformType_Int:
            Gl::Uniform1i(uniform.location, static_cast<GLint>(value[0]));
            break;
        case ShaderUniformType_Float:
            Gl::Uniform1f(uniform.location, value[0]);
            break;
        case ShaderUniformType_Vec2:
            Gl::Uniform2f(uniform.location, value[0], value[1]);
            break;
        case ShaderUniformType_Vec3:
            Gl::Uniform3f(uniform.location, value[0], value[1], value[2]);
            break;
        case ShaderUniformType_Vec4:
            Gl::Uniform4f(uniform.location, value[0], value[1], value[2], value[3]);
            break;
        case ShaderUniformType_IVec2:
            Gl::Uniform2i(uniform.location, static_cast<GLint>(value[0]), static_cast<GLint>(value[1]));
            break;
        case ShaderUniformType_IVec3:
            Gl::Uniform3i(uniform.location, static_cast<GLint>(value[0]), static_cast<GLint>(value[1]),
                          static_cast<GLint>(value[2]));
            break;
        case ShaderUniformType_IVec4:
            Gl::Uniform4i(uniform.location, static_cast<GLint>(value[0]), static_cast<GLint>(value[1]),
                          static_cast<GLint>(value[2]), static_cast<GLint>(value[3]));
            break;
        case ShaderUniformType_Mat4:
            Gl::UniformMatrix4fv(uniform.location, 1, 0, value);
            break;
        }
    }
    for (const auto& custom : program.customUniforms)
    {
        const CustomUniform& definition = mCustomUniforms[static_cast<size_t>(custom.first)];
        const ShaderValue& value = mExpressions.Slot(definition.slot);
        switch (definition.type)
        {
        case 0:
            Gl::Uniform1f(custom.second, value.v[0]);
            break;
        case 1:
        case 2:
            Gl::Uniform1i(custom.second, static_cast<GLint>(value.v[0]));
            break;
        case 4:
            Gl::Uniform2f(custom.second, value.v[0], value.v[1]);
            break;
        case 5:
            Gl::Uniform3f(custom.second, value.v[0], value.v[1], value.v[2]);
            break;
        case 6:
            Gl::Uniform4f(custom.second, value.v[0], value.v[1], value.v[2], value.v[3]);
            break;
        default:
            break;
        }
    }
}

void ShaderPipeline::UploadDrawUniforms(const ShaderLinkedProgram& program, const ShaderDrawValues& values)
{
    ShaderLinkedProgram& mutableProgram = const_cast<ShaderLinkedProgram&>(program);
    for (size_t index = 0; index < program.drawUniforms.size(); ++index)
    {
        const ShaderLinkedProgram::Uniform& uniform = program.drawUniforms[index];
        if (uniform.id == ShaderUniform_FirstVertex)
            continue;
        float value[16] = {};
        switch (uniform.id)
        {
        case ShaderUniform_FogMode:
            value[0] = static_cast<float>(values.fogMode);
            break;
        case ShaderUniform_FogStart:
            value[0] = values.fogStart;
            break;
        case ShaderUniform_FogEnd:
            value[0] = values.fogEnd;
            break;
        case ShaderUniform_FogDensity:
            value[0] = values.fogDensity;
            break;
        case ShaderUniform_FogColor:
            std::memcpy(value, values.fogColor, sizeof(values.fogColor));
            break;
        case ShaderUniform_RenderStage:
            value[0] = static_cast<float>(mStage);
            break;
        case ShaderUniform_AlphaTestRef:
            value[0] = values.alphaRef;
            break;
        case ShaderUniform_EntityColor:
            std::memcpy(value, values.entityColor, sizeof(values.entityColor));
            break;
        case ShaderUniform_EntityId:
            value[0] = static_cast<float>(mEntityId);
            break;
        case ShaderUniform_BlockEntityId:
            value[0] = static_cast<float>(mBlockEntityId);
            break;
        case ShaderUniform_BlendFunc:
            for (int component = 0; component < 4; ++component)
                value[component] = static_cast<float>(values.blendFunc[component]);
            break;
        case ShaderUniform_InstanceId:
            value[0] = 0.0f;
            break;
        case ShaderUniform_ProjectionFix:
            Identity(value);
            if (mInHand)
                value[10] = mHandDepth;
            break;
        case ShaderUniform_AlphaTest:
            if (mCurrentGbuffer >= 0 && !mInShadow)
            {
                value[0] = static_cast<float>(mGbufferAlphaFunction[mCurrentGbuffer]);
                value[1] = mGbufferAlphaFunction[mCurrentGbuffer] == 0 ? values.alphaRef
                                                                       : mGbufferAlphaReference[mCurrentGbuffer];
            }
            else
            {
                // LegacyEX: the shadow pass keeps the game's own alpha test (leaves cast their leaves' shapes)
                value[1] = values.alphaRef;
            }
            break;
        case ShaderUniform_FogColorValue:
            std::memcpy(value, values.fogColor, sizeof(values.fogColor));
            value[3] = 1.0f;
            break;
        case ShaderUniform_FogValues:
            value[0] = values.fogDensity;
            value[1] = values.fogStart;
            value[2] = values.fogEnd;
            value[3] = values.fogEnd > values.fogStart ? 1.0f / (values.fogEnd - values.fogStart) : 0.0f;
            break;
        case ShaderUniform_FlipY:
            value[0] = 1.0f;
            break;
        case ShaderUniform_QuadDataMode:
            value[0] = values.quadData ? 1.0f : 0.0f;
            break;
        case ShaderUniform_Light0Position:
            std::memcpy(value, values.lights[0], sizeof(float) * 4);
            break;
        case ShaderUniform_Light1Position:
            std::memcpy(value, values.lights[1], sizeof(float) * 4);
            break;
        case ShaderUniform_Light0Diffuse:
            std::memcpy(value, values.lights[2], sizeof(float) * 4);
            break;
        case ShaderUniform_Light1Diffuse:
            std::memcpy(value, values.lights[3], sizeof(float) * 4);
            break;
        case ShaderUniform_LightAmbient:
            std::memcpy(value, values.lights[4], sizeof(float) * 4);
            break;
        case ShaderUniform_ColourScale:
            std::memcpy(value, values.colourScale, sizeof(values.colourScale));
            break;
        case ShaderUniform_QuadVertexMode:
            value[0] = values.quadVertices ? 1.0f : 0.0f;
            break;
        default:
            break;
        }
        ShaderUniformType type = ShaderUniformCatalogue(uniform.id).type;
        size_t count = type == ShaderUniformType_Mat4 ? 16 : 4;
        float* cache = &mutableProgram.drawCache[index * 16];
        if (std::memcmp(cache, value, count * sizeof(float)) == 0)
            continue;
        std::memcpy(cache, value, count * sizeof(float));
        switch (type)
        {
        case ShaderUniformType_Int:
            Gl::Uniform1i(uniform.location, static_cast<GLint>(value[0]));
            break;
        case ShaderUniformType_Float:
            Gl::Uniform1f(uniform.location, value[0]);
            break;
        case ShaderUniformType_Vec2:
            Gl::Uniform2f(uniform.location, value[0], value[1]);
            break;
        case ShaderUniformType_Vec3:
            Gl::Uniform3f(uniform.location, value[0], value[1], value[2]);
            break;
        case ShaderUniformType_Vec4:
            Gl::Uniform4f(uniform.location, value[0], value[1], value[2], value[3]);
            break;
        case ShaderUniformType_IVec2:
            Gl::Uniform2i(uniform.location, static_cast<GLint>(value[0]), static_cast<GLint>(value[1]));
            break;
        case ShaderUniformType_IVec3:
            Gl::Uniform3i(uniform.location, static_cast<GLint>(value[0]), static_cast<GLint>(value[1]),
                          static_cast<GLint>(value[2]));
            break;
        case ShaderUniformType_IVec4:
            Gl::Uniform4i(uniform.location, static_cast<GLint>(value[0]), static_cast<GLint>(value[1]),
                          static_cast<GLint>(value[2]), static_cast<GLint>(value[3]));
            break;
        case ShaderUniformType_Mat4:
            Gl::UniformMatrix4fv(uniform.location, 1, 0, value);
            break;
        }
    }
}

void ShaderPipeline::ApplyProgram(const ShaderLinkedProgram* program, const ShaderDrawValues& values)
{
    if (program == nullptr)
        return;
    ShaderLinkedProgram& mutableProgram = const_cast<ShaderLinkedProgram&>(*program);
    if (program->frameStamp != mFrame)
    {
        UploadFrameUniforms(*program);
        mutableProgram.frameStamp = mFrame;
    }
    UploadDrawUniforms(*program, values);
    mLastDrawValues = values;
    mDrawProgram = program;
    if (mBoundProgram != program || mBoundStamp != mBindingsStamp)
    {
        BindSamplers(*program, false);
        mBoundProgram = program;
        mBoundStamp = mBindingsStamp;
    }
}

void ShaderPipeline::SetChunkOffsetMode(int mode)
{
    if (mDrawProgram == nullptr)
        return;
    ShaderLinkedProgram& program = const_cast<ShaderLinkedProgram&>(*mDrawProgram);
    for (size_t index = 0; index < program.drawUniforms.size(); ++index)
    {
        const ShaderLinkedProgram::Uniform& uniform = program.drawUniforms[index];
        if (uniform.id != ShaderUniform_ChunkOffsetMode)
            continue;
        float* cache = &program.drawCache[index * 16];
        const float value = static_cast<float>(mode);
        if (cache[0] != value)
        {
            cache[0] = value;
            Gl::Uniform1i(uniform.location, mode);
        }
        return;
    }
}

void ShaderPipeline::SetFirstVertex(int firstVertex)
{
    if (mDrawProgram == nullptr)
        return;
    ShaderLinkedProgram& program = const_cast<ShaderLinkedProgram&>(*mDrawProgram);
    for (size_t index = 0; index < program.drawUniforms.size(); ++index)
    {
        const ShaderLinkedProgram::Uniform& uniform = program.drawUniforms[index];
        if (uniform.id != ShaderUniform_FirstVertex)
            continue;
        float* cache = &program.drawCache[index * 16];
        float value = static_cast<float>(firstVertex);
        if (cache[0] != value)
        {
            cache[0] = value;
            Gl::Uniform1i(uniform.location, firstVertex);
        }
        return;
    }
}

void ShaderPipeline::RunPasses(int family)
{
    for (ShaderProgramSource* pass : mPrograms.Passes(family))
        RunPass(*pass, false, 0, 0, 0);
}

void ShaderPipeline::RunPass(ShaderProgramSource& pass, bool final, GLuint mainFramebuffer, int screenWidth,
                             int screenHeight)
{
    if (!pass.compute.empty())
    {
        Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
        for (ShaderComputeSource& compute : pass.compute)
            RunCompute(pass, compute);
    }
    if (!pass.present)
        return;
    std::string error;
    bool attempted = pass.attempted[ShaderAdapter_Quad];
    const ShaderLinkedProgram* program = mPrograms.Link(pass, ShaderAdapter_Quad, error);
    if (program == nullptr)
    {
        if (!attempted && !error.empty())
        {
            LOG_WARN(sLog, "%s", error.c_str());
            mErrors.push_back(error);
        }
        return;
    }

    Gl::Disable(Gl::SCISSOR_TEST);
    Gl::Disable(Gl::DEPTH_TEST);
    Gl::Disable(Gl::STENCIL_TEST);
    Gl::Disable(Gl::CULL_FACE);
    Gl::Disable(Gl::BLEND);
    Gl::Disable(Gl::POLYGON_OFFSET_FILL);
    Gl::ColorMask(1, 1, 1, 1);
    Gl::DepthMask(0);

    std::vector<int> written;
    if (final)
    {
        Gl::BindFramebuffer(Gl::FRAMEBUFFER, mainFramebuffer);
        GLenum buffer = Gl::COLOR_ATTACHMENT0;
        Gl::DrawBuffers(1, &buffer);
        Gl::Viewport(0, 0, screenWidth, screenHeight);
    }
    else
    {
        Gl::BindFramebuffer(Gl::FRAMEBUFFER, mCompositeFramebuffer);
        GLenum buffers[8];
        int count = 0;
        int width = mWidth;
        int height = mHeight;
        for (int buffer : pass.drawBuffers)
        {
            if (count >= 8)
                break;
            ColorTarget& target = mColor[buffer];
            GLuint texture = target.textures[1 - target.current];
            Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0 + count, Gl::TEXTURE_2D, texture, 0);
            buffers[count] = texture != 0 ? Gl::COLOR_ATTACHMENT0 + count : Gl::NONE;
            if (count == 0 && texture != 0)
            {
                width = target.width;
                height = target.height;
            }
            written.push_back(buffer);
            ++count;
        }
        for (int attachment = count; attachment < 8; ++attachment)
            Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0 + attachment, Gl::TEXTURE_2D, 0, 0);
        Gl::DrawBuffers(count, buffers);
        float scale = 1.0f;
        std::string value = ProgramProperty("scale.", pass.name);
        if (!value.empty())
            scale = std::max(0.0f, std::min(1.0f, static_cast<float>(std::atof(Split(value)[0].c_str()))));
        Gl::Viewport(0, 0, std::max(1, static_cast<int>(width * scale)), std::max(1, static_cast<int>(height * scale)));
    }

    Gl::UseProgram(program->name);
    ShaderLinkedProgram& mutableProgram = const_cast<ShaderLinkedProgram&>(*program);
    if (program->frameStamp != mFrame)
    {
        UploadFrameUniforms(*program);
        mutableProgram.frameStamp = mFrame;
    }
    ShaderDrawValues values = mLastDrawValues;
    values.quadData = false;
    int stage = mStage;
    mStage = ShaderRenderStage_None;
    UploadDrawUniforms(*program, values);
    mStage = stage;
    for (size_t index = 0; index < program->drawUniforms.size(); ++index)
    {
        if (program->drawUniforms[index].id == ShaderUniform_FlipY)
        {
            float flip = final && mFlipFinal ? -1.0f : 1.0f;
            Gl::Uniform1f(program->drawUniforms[index].location, flip);
            mutableProgram.drawCache[index * 16] = flip;
        }
    }
    BindSamplers(*program, true);
    mBoundProgram = nullptr;

    ApplyPassBlend(pass, written);
    Gl::BindVertexArray(mEmptyVertexArray);
    Gl::DrawArrays(Gl::TRIANGLE_STRIP, 0, 4);
    Gl::Disable(Gl::BLEND);
    if (!program->images.empty())
        Barrier();
    if (final)
    {
        // LegacyEX: the finished image is opaque whatever alpha the pack wrote (Solas writes only gl_FragColor.rgb);
        // the renderer keeps alpha in the main framebuffer, and an undefined one hid the image
        const float opaque[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        Gl::ColorMask(0, 0, 0, 1);
        Gl::ClearBufferfv(Gl::COLOR, 0, opaque);
        Gl::ColorMask(1, 1, 1, 1);
    }

    if (!final)
    {
        for (int buffer : written)
        {
            std::string flip = ProgramProperty("flip.", pass.name + ".colortex" + std::to_string(buffer));
            if (flip.empty() && buffer < 8)
            {
                static const char* const kLegacy[8] = {"gcolor", "gdepth", "gnormal", "composite",
                                                       "gaux1",  "gaux2",  "gaux3",   "gaux4"};
                flip = ProgramProperty("flip.", pass.name + "." + kLegacy[buffer]);
            }
            ColorTarget& target = mColor[buffer];
            target.mipDirty[1 - target.current] = true;
            if (ParseBool(flip, true))
                target.current = 1 - target.current;
        }
        for (int attachment = 0; attachment < 8; ++attachment)
            Gl::FramebufferTexture2D(Gl::FRAMEBUFFER, Gl::COLOR_ATTACHMENT0 + attachment, Gl::TEXTURE_2D, 0, 0);
        ++mBindingsStamp;
    }
    Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
}

void ShaderPipeline::ReadCenterDepth()
{
    if (!mDepthReadPending || mDepthReadBuffer == 0)
        return;
    Gl::BindBuffer(Gl::PIXEL_PACK_BUFFER, mDepthReadBuffer);
    void* mapped = Gl::MapBufferRange(Gl::PIXEL_PACK_BUFFER, 0, 4, Gl::MAP_READ_BIT);
    if (mapped != nullptr)
    {
        float depth = 1.0f;
        std::memcpy(&depth, mapped, sizeof(depth));
        Gl::UnmapBuffer(Gl::PIXEL_PACK_BUFFER);
        if (std::isfinite(depth))
            mCenterDepth = std::max(0.0f, std::min(depth, 1.0f));
    }
    Gl::BindBuffer(Gl::PIXEL_PACK_BUFFER, 0);
    mDepthReadPending = false;
}

void ShaderPipeline::EndWorld(GLuint mainFramebuffer, int screenWidth, int screenHeight)
{
    if (!mInWorld)
        return;
    if (mInShadow)
        EndShadow();
    BeginTranslucent();
    mInHand = false;

    Barrier();
    RunPasses(ShaderPass_Composite);
    GLuint finalTarget = mFxaa.Target(screenWidth, screenHeight);
    bool offscreen = finalTarget != 0;
    // our renderer draws the world the GL way up, as the packs do, so the final pass needs no flip
    mFlipFinal = false;
    if (!offscreen)
        finalTarget = mainFramebuffer;
    const std::vector<ShaderProgramSource*>& finals = mPrograms.Passes(ShaderPass_Final);
    if (!finals.empty() && mPrograms.Link(*finals[0], ShaderAdapter_Quad, mLastError) != nullptr)
    {
        RunPass(*finals[0], true, finalTarget, screenWidth, screenHeight);
    }
    else
    {
        if (!mFinalCopy)
        {
            mFinalCopy.reset(new ShaderProgramSource);
            mFinalCopy->name = "lce_final";
            std::string error;
            if (!mPrograms.LoadInline(*mFinalCopy, kFinalVertex, kFinalFragment, error))
                LOG_ERROR(sLog, "final copy program: %s", error.c_str());
        }
        RunPass(*mFinalCopy, true, finalTarget, screenWidth, screenHeight);
    }
    if (offscreen)
        mFxaa.Apply(mainFramebuffer, screenWidth, screenHeight, mAntialiasing,
                    Gl::TEXTURE0 + static_cast<GLenum>(ShaderTextureUnit_FirstPack));

    if (mDepthReadBuffer != 0)
    {
        Gl::BindFramebuffer(Gl::READ_FRAMEBUFFER, mGbufferFramebuffer);
        Gl::BindBuffer(Gl::PIXEL_PACK_BUFFER, mDepthReadBuffer);
        Gl::ReadPixels(mWidth / 2, mHeight / 2, 1, 1, Gl::DEPTH_COMPONENT, Gl::FLOAT, nullptr);
        Gl::BindBuffer(Gl::PIXEL_PACK_BUFFER, 0);
        Gl::BindFramebuffer(Gl::FRAMEBUFFER, 0);
        mDepthReadPending = true;
    }

    for (int unit = ShaderTextureUnit_Normals; unit < ShaderTextureUnit_FirstPack + 24; ++unit)
    {
        Gl::ActiveTexture(Gl::TEXTURE0 + static_cast<GLenum>(unit));
        Gl::BindTexture(Gl::TEXTURE_2D, 0);
        Gl::BindSampler(static_cast<GLuint>(unit), 0);
    }
    Gl::ActiveTexture(Gl::TEXTURE0);
    Gl::DepthMask(1);
    mAlbedo.Trim(mFrame);
    mInWorld = false;
    mStage = ShaderRenderStage_None;
    mBoundProgram = nullptr;
}
