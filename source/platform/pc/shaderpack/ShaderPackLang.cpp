// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPackLang.cpp
#include "platform/pc/shaderpack/ShaderPackLang.hpp"

#include <algorithm>
#include <cctype>

#include "platform/pc/shaderpack/ShaderPack.hpp"
#include "platform/pc/shaderpack/ShaderPackOptions.hpp"
#include "platform/pc/shaderpack/ShaderPackProperties.hpp"

namespace
{
std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}
}

void ShaderPackLang::Load(const ShaderPack& pack, const std::string& locale)
{
    mEntries.clear();
    LoadFile(pack, "en_us");
    if (Lower(locale) != "en_us")
        LoadFile(pack, locale);
}

void ShaderPackLang::LoadFile(const ShaderPack& pack, const std::string& locale)
{
    std::string wanted = "lang/" + Lower(locale) + ".lang";
    for (const std::string& file : pack.Files())
    {
        if (Lower(file) != wanted)
            continue;
        std::string text;
        if (!pack.ReadFile(file, text))
            return;
        for (const auto& entry : ShaderParseProperties(text))
            mEntries[entry.first] = entry.second;
        return;
    }
}

std::string ShaderPackLang::Get(const std::string& key, const std::string& fallback) const
{
    auto found = mEntries.find(key);
    return found == mEntries.end() ? fallback : found->second;
}

std::string ShaderPackLang::OptionLabel(const std::string& option) const
{
    return Get("option." + option, option);
}

std::string ShaderPackLang::OptionComment(const ShaderOption& option) const
{
    return Get("option." + option.name + ".comment", option.comment);
}

std::string ShaderPackLang::ValueLabel(const ShaderOption& option, const std::string& value) const
{
    std::string label;
    std::string key = "value." + option.name + "." + value;
    if (Has(key))
        label = Get(key, value);
    else if (option.IsBoolean())
        label = value == "true" ? "ON" : "OFF";
    else
        label = value;
    if (!option.IsBoolean())
        label = Get("prefix." + option.name, std::string()) + label + Get("suffix." + option.name, std::string());
    return label;
}

std::string ShaderPackLang::ScreenLabel(const std::string& screen) const
{
    return Get("screen." + screen, screen);
}

std::string ShaderPackLang::ScreenComment(const std::string& screen) const
{
    return Get("screen." + screen + ".comment", std::string());
}

std::string ShaderPackLang::ProfileLabel(const std::string& profile) const
{
    return Get("profile." + profile, profile);
}

std::string ShaderPackLang::ProfileComment(const std::string& profile) const
{
    return Get("profile." + profile + ".comment", std::string());
}

std::string ShaderPackLang::StripFormatting(const std::string& text)
{
    std::string out;
    for (size_t i = 0; i < text.size(); ++i)
    {
        if (static_cast<unsigned char>(text[i]) == 0xC2 && i + 2 < text.size() &&
            static_cast<unsigned char>(text[i + 1]) == 0xA7)
        {
            i += 2;
            continue;
        }
        out += text[i];
    }
    return out;
}
