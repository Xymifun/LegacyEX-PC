// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPackProperties.cpp
#include "platform/pc/shaderpack/ShaderPackProperties.hpp"

#include <algorithm>
#include <cstdlib>

#include "platform/pc/shaderpack/ShaderPackOptions.hpp"

namespace
{
bool IsSpace(char character)
{
    return character == ' ' || character == '\t' || character == '\f';
}

void AppendUtf8(std::string& out, unsigned int codePoint)
{
    if (codePoint < 0x80)
    {
        out += static_cast<char>(codePoint);
    }
    else if (codePoint < 0x800)
    {
        out += static_cast<char>(0xC0 | (codePoint >> 6));
        out += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
    else
    {
        out += static_cast<char>(0xE0 | (codePoint >> 12));
        out += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
}

std::string Unescape(const std::string& text)
{
    std::string out;
    for (size_t i = 0; i < text.size(); ++i)
    {
        char character = text[i];
        if (character != '\\' || i + 1 >= text.size())
        {
            out += character;
            continue;
        }
        char next = text[++i];
        switch (next)
        {
        case 't':
            out += '\t';
            break;
        case 'n':
            out += '\n';
            break;
        case 'r':
            out += '\r';
            break;
        case 'f':
            out += '\f';
            break;
        case 'u':
            if (i + 4 < text.size())
            {
                unsigned int codePoint = static_cast<unsigned int>(std::strtoul(text.substr(i + 1, 4).c_str(), nullptr, 16));
                AppendUtf8(out, codePoint);
                i += 4;
            }
            break;
        default:
            out += next;
            break;
        }
    }
    return out;
}

bool EndsWithOddBackslashes(const std::string& line)
{
    size_t count = 0;
    for (size_t i = line.size(); i > 0 && line[i - 1] == '\\'; --i)
        ++count;
    return (count % 2) == 1;
}

bool IsUnprocessedKey(const std::string& key)
{
    return key == "screen" || key.compare(0, 7, "screen.") == 0 || key == "sliders" ||
           key.compare(0, 8, "profile.") == 0 || key.compare(0, 8, "version.") == 0 || key == "oldLighting" ||
           key == "separateAo";
}
}

ShaderPropertyList ShaderParseProperties(const std::string& text)
{
    ShaderPropertyList entries;
    std::string logical;
    bool continuing = false;
    size_t start = 0;
    std::string source = text;
    if (source.size() >= 3 && static_cast<unsigned char>(source[0]) == 0xEF &&
        static_cast<unsigned char>(source[1]) == 0xBB && static_cast<unsigned char>(source[2]) == 0xBF)
        source.erase(0, 3);

    while (start <= source.size())
    {
        size_t end = source.find('\n', start);
        if (end == std::string::npos)
            end = source.size();
        std::string line = source.substr(start, end - start);
        start = end + 1;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        size_t first = 0;
        while (first < line.size() && IsSpace(line[first]))
            ++first;
        line = line.substr(first);
        if (!continuing && (line.empty() || line[0] == '#' || line[0] == '!'))
        {
            if (end == source.size())
                break;
            continue;
        }

        if (EndsWithOddBackslashes(line))
        {
            logical += line.substr(0, line.size() - 1);
            continuing = true;
            if (end == source.size())
                break;
            continue;
        }
        logical += line;
        continuing = false;

        size_t position = 0;
        std::string key;
        while (position < logical.size())
        {
            char character = logical[position];
            if (character == '\\' && position + 1 < logical.size())
            {
                key += logical.substr(position, 2);
                position += 2;
                continue;
            }
            if (character == '=' || character == ':' || IsSpace(character))
                break;
            key += character;
            ++position;
        }
        while (position < logical.size() && IsSpace(logical[position]))
            ++position;
        if (position < logical.size() && (logical[position] == '=' || logical[position] == ':'))
            ++position;
        while (position < logical.size() && IsSpace(logical[position]))
            ++position;
        std::string value = logical.substr(position);
        while (!value.empty() && IsSpace(value.back()))
            value.pop_back();
        if (!key.empty())
            entries.emplace_back(Unescape(key), Unescape(value));
        logical.clear();
        if (end == source.size())
            break;
    }
    return entries;
}

std::vector<std::string> ShaderSplitList(const std::string& text)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    while (start < text.size())
    {
        size_t tokenStart = text.find_first_not_of(" \t\r\n", start);
        if (tokenStart == std::string::npos)
            break;
        size_t tokenEnd = text.find_first_of(" \t\r\n", tokenStart);
        if (tokenEnd == std::string::npos)
            tokenEnd = text.size();
        tokens.push_back(text.substr(tokenStart, tokenEnd - tokenStart));
        start = tokenEnd;
    }
    return tokens;
}

void ShaderPackProperties::Parse(const std::string& text, const ShaderMacros& standardMacros,
                                 const ShaderMacros& optionMacros, const ShaderPackOptions& options)
{
    mEntries.clear();
    mValues.clear();
    mScreens.clear();
    mProfiles.clear();
    mSliders.clear();

    ShaderMacros allMacros = standardMacros;
    for (const auto& macro : optionMacros)
        allMacros[macro.first] = macro.second;

    ShaderPropertyList unprocessed = ShaderParseProperties(ShaderPreprocessConditionals(text, standardMacros));
    ShaderPropertyList processed = ShaderParseProperties(ShaderPreprocessConditionals(text, allMacros));

    for (const auto& entry : unprocessed)
    {
        if (IsUnprocessedKey(entry.first))
            mEntries.push_back(entry);
    }
    for (const auto& entry : processed)
    {
        if (!IsUnprocessedKey(entry.first))
            mEntries.push_back(entry);
    }
    for (const auto& entry : mEntries)
        mValues[entry.first] = entry.second;

    std::vector<std::string> screenOrder;
    std::map<std::string, std::string> rawProfiles;
    std::vector<std::string> profileOrder;
    for (const auto& entry : unprocessed)
    {
        const std::string& key = entry.first;
        if ((key == "screen" || key.compare(0, 7, "screen.") == 0) &&
            (key.size() < 8 || key.compare(key.size() - 8, 8, ".columns") != 0))
        {
            std::string name = key == "screen" ? std::string() : key.substr(7);
            if (std::find(screenOrder.begin(), screenOrder.end(), name) == screenOrder.end())
                screenOrder.push_back(name);
        }
        else if (key.compare(0, 8, "profile.") == 0)
        {
            std::string name = key.substr(8);
            if (rawProfiles.count(name) == 0)
                profileOrder.push_back(name);
            rawProfiles[name] = entry.second;
        }
    }

    for (const std::string& name : screenOrder)
    {
        ShaderScreen screen;
        screen.name = name;
        std::string key = name.empty() ? std::string("screen") : "screen." + name;
        screen.items = ShaderSplitList(Get(key));
        screen.columns = std::max(1, std::atoi(Get(key + ".columns", "2").c_str()));
        mScreens.push_back(screen);
    }

    for (const std::string& token : ShaderSplitList(Get("sliders")))
        mSliders.insert(token);

    for (const std::string& name : profileOrder)
    {
        ShaderProfile profile;
        profile.name = name;
        std::vector<std::string> stack;
        ResolveProfile(name, rawProfiles, options, profile, stack);
        mProfiles.push_back(profile);
    }
}

void ShaderPackProperties::ResolveProfile(const std::string& name, const std::map<std::string, std::string>& raw,
                                          const ShaderPackOptions& options, ShaderProfile& profile,
                                          std::vector<std::string>& stack) const
{
    auto found = raw.find(name);
    if (found == raw.end() || std::find(stack.begin(), stack.end(), name) != stack.end())
        return;
    stack.push_back(name);

    auto setValue = [&profile](const std::string& option, const std::string& value) {
        for (auto& existing : profile.values)
        {
            if (existing.first == option)
            {
                existing.second = value;
                return;
            }
        }
        profile.values.emplace_back(option, value);
    };

    for (const std::string& token : ShaderSplitList(found->second))
    {
        if (token.compare(0, 8, "profile.") == 0)
        {
            ResolveProfile(token.substr(8), raw, options, profile, stack);
        }
        else if (token.compare(0, 9, "!program.") == 0)
        {
            profile.disabledPrograms.push_back(token.substr(9));
        }
        else if (token[0] == '!')
        {
            setValue(token.substr(1), "false");
        }
        else
        {
            size_t separator = token.find_first_of(":=");
            if (separator == std::string::npos)
                setValue(token, "true");
            else
                setValue(token.substr(0, separator), token.substr(separator + 1));
        }
    }
    stack.pop_back();
}

std::string ShaderPackProperties::Get(const std::string& key, const std::string& fallback) const
{
    auto found = mValues.find(key);
    return found == mValues.end() ? fallback : found->second;
}

const ShaderScreen* ShaderPackProperties::Screen(const std::string& name) const
{
    for (const ShaderScreen& screen : mScreens)
    {
        if (screen.name == name)
            return &screen;
    }
    return nullptr;
}

const ShaderProfile* ShaderPackProperties::Profile(const std::string& name) const
{
    for (const ShaderProfile& profile : mProfiles)
    {
        if (profile.name == name)
            return &profile;
    }
    return nullptr;
}

bool ShaderPackProperties::ProfileMatches(const ShaderProfile& profile, const ShaderPackOptions& options)
{
    for (const auto& entry : profile.values)
    {
        const ShaderOption* option = options.Find(entry.first);
        if (option != nullptr && options.Value(entry.first) != entry.second)
            return false;
    }
    return true;
}

void ShaderPackProperties::ApplyProfile(const ShaderProfile& profile, ShaderPackOptions& options)
{
    for (const auto& entry : profile.values)
        options.SetValue(entry.first, entry.second);
}

std::string ShaderPackProperties::ActiveProfile(const ShaderPackOptions& options) const
{
    for (const ShaderProfile& profile : mProfiles)
    {
        if (ProfileMatches(profile, options))
            return profile.name;
    }
    return std::string();
}
