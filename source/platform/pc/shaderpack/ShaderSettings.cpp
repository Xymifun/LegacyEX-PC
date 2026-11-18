// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderSettings.cpp
#include "platform/pc/shaderpack/ShaderSettings.hpp"

#include "platform/pc/PcOs.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <sstream>

#include "platform/pc/shaderpack/ShaderPackProperties.hpp"
#include "platform/pc/shaderpack/ShaderUniforms.hpp"

const char* const ShaderSettings::kPackOff = "OFF";
const char* const ShaderSettings::kPackInternal = "(internal)";
const float ShaderSettings::kQualityValues[5] = {0.5f, 0.70710677f, 1.0f, 1.4142135f, 2.0f};
const float ShaderSettings::kHandDepthValues[3] = {0.0625f, 0.125f, 0.25f};

namespace
{
const int kAntialiasingLevels[3] = {0, 2, 4};
const ShaderSettings::Tristate kTristates[3] = {ShaderSettings::Default, ShaderSettings::On, ShaderSettings::Off};
const char* const kAntialiasingLabels[3] = {"OFF", "FXAA 2x", "FXAA 4x"};
const char* const kSwitchLabels[2] = {"OFF", "ON"};
const char* const kQualityLabels[5] = {"0.5x", "0.7x", "1x", "1.4x", "2x"};
const char* const kHandDepthLabels[3] = {"0.5x", "1x", "2x"};
const char* const kTristateLabels[3] = {"Default", "ON", "OFF"};
const ShaderSettings::ChoiceInfo kChoices[ShaderSettings::Choice_Count] = {
    {"Antialiasing", "MC_FXAA_LEVEL", true, 3, kAntialiasingLabels},
    {"Normal Map", "MC_NORMAL_MAP", false, 2, kSwitchLabels},
    {"Specular Map", "MC_SPECULAR_MAP", false, 2, kSwitchLabels},
    {"Render Quality", "MC_RENDER_QUALITY", true, 5, kQualityLabels},
    {"Shadow Quality", "MC_SHADOW_QUALITY", true, 5, kQualityLabels},
    {"Hand Depth", "MC_HAND_DEPTH", true, 3, kHandDepthLabels},
    {"Old Hand Light", nullptr, false, 3, kTristateLabels},
    {"Old Lighting", nullptr, false, 3, kTristateLabels}};

template <size_t Count> int NearestIndex(const float (&values)[Count], float value)
{
    int best = 0;
    for (size_t index = 1; index < Count; ++index)
    {
        if (std::fabs(values[index] - value) < std::fabs(values[best] - value))
            best = static_cast<int>(index);
    }
    return best;
}

int TristateIndex(ShaderSettings::Tristate value)
{
    for (int index = 0; index < 3; ++index)
    {
        if (kTristates[index] == value)
            return index;
    }
    return 0;
}

std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

std::string FloatMacro(float value)
{
    std::ostringstream stream;
    stream.precision(8);
    stream << value;
    std::string text = stream.str();
    if (text.find('.') == std::string::npos)
        text += ".0";
    return text;
}

template <size_t Count> float Snap(float value, const float (&allowed)[Count], float fallback)
{
    if (!std::isfinite(value))
        return fallback;
    float best = allowed[0];
    for (float candidate : allowed)
    {
        if (std::fabs(candidate - value) < std::fabs(best - value))
            best = candidate;
    }
    return best;
}

ShaderSettings::Tristate ParseTristate(const std::string& text)
{
    if (text == "true")
        return ShaderSettings::On;
    if (text == "false")
        return ShaderSettings::Off;
    return ShaderSettings::Default;
}

const char* TristateText(ShaderSettings::Tristate value)
{
    return value == ShaderSettings::On ? "true" : value == ShaderSettings::Off ? "false" : "default";
}

std::string FloatText(float value)
{
    std::ostringstream stream;
    stream << value;
    std::string text = stream.str();
    if (text.find('.') == std::string::npos && text.find('e') == std::string::npos)
        text += ".0";
    return text;
}
}

void ShaderSettings::Load(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
        return;
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    for (const auto& entry : ShaderParseProperties(text))
    {
        const std::string& key = entry.first;
        const std::string& value = entry.second;
        if (key == "shaderPack")
            shaderPack = value.empty() ? std::string(kPackOff) : value;
        else if (key == "enableShaders")
            enableShaders = value != "false";
        else if (key == "antialiasingLevel")
        {
            int level = std::atoi(value.c_str());
            antialiasingLevel = level >= 4 ? 4 : level >= 2 ? 2 : 0;
        }
        else if (key == "normalMapEnabled")
            normalMapEnabled = value != "false";
        else if (key == "specularMapEnabled")
            specularMapEnabled = value != "false";
        else if (key == "renderResMul")
            renderResMul = Snap(static_cast<float>(std::atof(value.c_str())), kQualityValues, 1.0f);
        else if (key == "shadowResMul")
            shadowResMul = Snap(static_cast<float>(std::atof(value.c_str())), kQualityValues, 1.0f);
        else if (key == "handDepthMul")
            handDepthMul = Snap(static_cast<float>(std::atof(value.c_str())), kHandDepthValues, 0.125f);
        else if (key == "oldHandLight")
            oldHandLight = ParseTristate(value);
        else if (key == "oldLighting")
            oldLighting = ParseTristate(value);
    }
}

bool ShaderSettings::Save(const std::string& path) const
{
    std::string temporary = path + ".tmp";
    {
        std::ofstream file(temporary.c_str(), std::ios::binary | std::ios::trunc);
        if (!file)
            return false;
        file << "shaderPack=" << shaderPack << "\n";
        file << "enableShaders=" << (enableShaders ? "true" : "false") << "\n";
        file << "antialiasingLevel=" << antialiasingLevel << "\n";
        file << "normalMapEnabled=" << (normalMapEnabled ? "true" : "false") << "\n";
        file << "specularMapEnabled=" << (specularMapEnabled ? "true" : "false") << "\n";
        file << "renderResMul=" << FloatText(renderResMul) << "\n";
        file << "shadowResMul=" << FloatText(shadowResMul) << "\n";
        file << "handDepthMul=" << FloatText(handDepthMul) << "\n";
        file << "oldHandLight=" << TristateText(oldHandLight) << "\n";
        file << "oldLighting=" << TristateText(oldLighting) << "\n";
        if (!file)
            return false;
    }
    return PcOsReplaceFile(temporary.c_str(), path.c_str());
}

const ShaderSettings::ChoiceInfo& ShaderSettings::Describe(int choice)
{
    return kChoices[choice >= 0 && choice < Choice_Count ? choice : 0];
}

int ShaderSettings::GetChoice(int choice) const
{
    switch (choice)
    {
    case Choice_Antialiasing:
        return antialiasingLevel >= 4 ? 2 : antialiasingLevel >= 2 ? 1 : 0;
    case Choice_NormalMap:
        return normalMapEnabled ? 1 : 0;
    case Choice_SpecularMap:
        return specularMapEnabled ? 1 : 0;
    case Choice_RenderQuality:
        return NearestIndex(kQualityValues, renderResMul);
    case Choice_ShadowQuality:
        return NearestIndex(kQualityValues, shadowResMul);
    case Choice_HandDepth:
        return NearestIndex(kHandDepthValues, handDepthMul);
    case Choice_OldHandLight:
        return TristateIndex(oldHandLight);
    case Choice_OldLighting:
        return TristateIndex(oldLighting);
    default:
        return 0;
    }
}

void ShaderSettings::SetChoice(int choice, int index)
{
    if (index < 0 || index >= Describe(choice).count)
        return;
    switch (choice)
    {
    case Choice_Antialiasing:
        antialiasingLevel = kAntialiasingLevels[index];
        break;
    case Choice_NormalMap:
        normalMapEnabled = index != 0;
        break;
    case Choice_SpecularMap:
        specularMapEnabled = index != 0;
        break;
    case Choice_RenderQuality:
        renderResMul = kQualityValues[index];
        break;
    case Choice_ShadowQuality:
        shadowResMul = kQualityValues[index];
        break;
    case Choice_HandDepth:
        handDepthMul = kHandDepthValues[index];
        break;
    case Choice_OldHandLight:
        oldHandLight = kTristates[index];
        break;
    case Choice_OldLighting:
        oldLighting = kTristates[index];
        break;
    default:
        break;
    }
}

bool ShaderSettings::operator==(const ShaderSettings& other) const
{
    return shaderPack == other.shaderPack && enableShaders == other.enableShaders
           && antialiasingLevel == other.antialiasingLevel && normalMapEnabled == other.normalMapEnabled
           && specularMapEnabled == other.specularMapEnabled && renderResMul == other.renderResMul
           && shadowResMul == other.shadowResMul && handDepthMul == other.handDepthMul
           && oldHandLight == other.oldHandLight && oldLighting == other.oldLighting;
}

ShaderMacros gShaderExtraMacros;

// Iris's biome categories and precipitation types, also as macros (Iris shaderpack/IrisDefines.java)
static const char* CATEGORY_NAMES[] = {"NONE",   "TAIGA",  "EXTREME_HILLS", "JUNGLE",   "MESA",   "PLAINS", "SAVANNA",
                                       "ICY",    "THE_END", "BEACH",        "FOREST",   "OCEAN",  "DESERT", "RIVER",
                                       "SWAMP",  "MUSHROOM", "NETHER",      "MOUNTAIN", "UNDERGROUND"};
static const int CATEGORY_COUNT = (int)(sizeof(CATEGORY_NAMES) / sizeof(CATEGORY_NAMES[0]));

ShaderMacros ShaderStandardMacros(const ShaderSettings& settings, int glVersion, int glslVersion,
                                  const std::string& glVendor, const std::string& glRenderer,
                                  const std::vector<std::string>& glExtensions)
{
    ShaderMacros macros;
    for (const std::string& extension : glExtensions)
    {
        if (extension.compare(0, 3, "GL_") == 0)
            macros["MC_" + extension] = std::string();
    }
    macros["MC_VERSION"] = std::to_string(ShaderSettings::kMinecraftVersion);
    macros["IS_IRIS"] = std::string();
    macros["IRIS_VERSION"] = std::to_string(ShaderSettings::kIrisVersion);
    macros["IRIS_FEATURE_CUSTOM_TEXTURE_NAME"] = std::string();
    macros["MAX_COLOR_BUFFERS"] = "16";
    macros["MC_GL_VERSION"] = std::to_string(glVersion);
    macros["MC_GLSL_VERSION"] = std::to_string(glslVersion);
#if defined(_WIN32)
    macros["MC_OS_WINDOWS"] = std::string();
#elif defined(__APPLE__)
    macros["MC_OS_MAC"] = std::string();
#elif defined(__linux__)
    macros["MC_OS_LINUX"] = std::string();
#else
    macros["MC_OS_OTHER"] = std::string();
#endif

    std::string vendor = Lower(glVendor);
    const char* vendorMacro = "MC_GL_VENDOR_OTHER";
    if (vendor.find("nvidia") != std::string::npos)
        vendorMacro = "MC_GL_VENDOR_NVIDIA";
    else if (vendor.find("ati") == 0)
        vendorMacro = "MC_GL_VENDOR_ATI";
    else if (vendor.find("amd") != std::string::npos)
        vendorMacro = "MC_GL_VENDOR_AMD";
    else if (vendor.find("intel") != std::string::npos)
        vendorMacro = "MC_GL_VENDOR_INTEL";
    else if (vendor.find("mesa") != std::string::npos)
        vendorMacro = "MC_GL_VENDOR_MESA";
    else if (vendor.find("x.org") != std::string::npos)
        vendorMacro = "MC_GL_VENDOR_XORG";
    macros[vendorMacro] = std::string();
    if (std::string(vendorMacro) == "MC_GL_VENDOR_NVIDIA")
    {
        static const char* const kSmallIntegers[][2] = {
            {"int8_t", "int"},     {"i8vec2", "ivec2"},   {"i8vec3", "ivec3"},   {"i8vec4", "ivec4"},
            {"uint8_t", "uint"},   {"u8vec2", "uvec2"},   {"u8vec3", "uvec3"},   {"u8vec4", "uvec4"},
            {"int16_t", "int"},    {"i16vec2", "ivec2"},  {"i16vec3", "ivec3"},  {"i16vec4", "ivec4"},
            {"uint16_t", "uint"},  {"u16vec2", "uvec2"},  {"u16vec3", "uvec3"},  {"u16vec4", "uvec4"}};
        for (const auto& type : kSmallIntegers)
            macros[type[0]] = type[1];
    }

    std::string renderer = Lower(glRenderer);
    const char* rendererMacro = "MC_GL_RENDERER_OTHER";
    if (renderer.find("radeon") != std::string::npos)
        rendererMacro = "MC_GL_RENDERER_RADEON";
    else if (renderer.find("geforce") != std::string::npos)
        rendererMacro = "MC_GL_RENDERER_GEFORCE";
    else if (renderer.find("quadro") != std::string::npos)
        rendererMacro = "MC_GL_RENDERER_QUADRO";
    else if (renderer.find("intel") != std::string::npos)
        rendererMacro = "MC_GL_RENDERER_INTEL";
    else if (renderer.find("gallium") != std::string::npos)
        rendererMacro = "MC_GL_RENDERER_GALLIUM";
    else if (renderer.find("mesa") != std::string::npos)
        rendererMacro = "MC_GL_RENDERER_MESA";
    macros[rendererMacro] = std::string();

    if (settings.antialiasingLevel > 0)
        macros["MC_FXAA_LEVEL"] = std::to_string(settings.antialiasingLevel);
    if (settings.normalMapEnabled)
        macros["MC_NORMAL_MAP"] = std::string();
    if (settings.specularMapEnabled)
        macros["MC_SPECULAR_MAP"] = std::string();
    macros["MC_RENDER_QUALITY"] = FloatMacro(settings.renderResMul);
    macros["MC_SHADOW_QUALITY"] = FloatMacro(settings.shadowResMul);
    macros["MC_HAND_DEPTH"] = FloatMacro(settings.handDepthMul);

    for (int stage = 0; stage < ShaderRenderStage_Count; ++stage)
        macros[std::string("MC_RENDER_STAGE_") + ShaderRenderStageName(stage)] = std::to_string(stage);

    // as Iris (gl/shader/StandardMacros.java): entities are drawn one by one, so entityId is right per draw
    macros["IRIS_REQUIRES_SEPARATE_ENTITY_DRAWS"] = "";
    for (int i = 0; i < CATEGORY_COUNT; i++)
        macros[std::string("CAT_") + CATEGORY_NAMES[i]] = std::to_string(i);
    macros["PPT_NONE"] = "0";
    macros["PPT_RAIN"] = "1";
    macros["PPT_SNOW"] = "2";
    for (ShaderMacros::const_iterator it = gShaderExtraMacros.begin(); it != gShaderExtraMacros.end(); ++it)
        macros[it->first] = it->second;
    return macros;
}
