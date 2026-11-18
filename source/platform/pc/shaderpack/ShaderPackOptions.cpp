// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPackOptions.cpp
#include "platform/pc/shaderpack/ShaderPackOptions.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <utility>

#include "platform/pc/shaderpack/ShaderPack.hpp"

namespace
{
const char* const kConstOptionNames[] = {
    "shadowMapResolution", "shadowDistance", "shadowDistanceRenderMul", "shadowIntervalSize", "generateShadowMipmap",
    "generateShadowColorMipmap", "shadowHardwareFiltering", "shadowHardwareFiltering0", "shadowHardwareFiltering1",
    "shadowtex0Mipmap", "shadowtexMipmap", "shadowtex1Mipmap", "shadowcolor0Mipmap", "shadowColor0Mipmap",
    "shadowcolor1Mipmap", "shadowColor1Mipmap", "shadowtex0Nearest", "shadowtexNearest", "shadow0MinMagNearest",
    "shadowtex1Nearest", "shadow1MinMagNearest", "shadowcolor0Nearest", "shadowColor0Nearest",
    "shadowColor0MinMagNearest", "shadowcolor1Nearest", "shadowColor1Nearest", "shadowColor1MinMagNearest",
    "wetnessHalflife", "drynessHalflife", "eyeBrightnessHalflife", "centerDepthHalflife", "sunPathRotation",
    "ambientOcclusionLevel", "superSamplingLevel", "noiseTextureResolution"};

enum LineKind
{
    Line_None,
    Line_Boolean,
    Line_Value,
    Line_Const
};

struct ParsedLine
{
    LineKind kind;
    bool enabled;
    std::string name;
    std::string value;
    std::string constType;
    std::string comment;
    bool hasComment;
};

bool IsIdentifierStart(char character)
{
    return std::isalpha(static_cast<unsigned char>(character)) || character == '_';
}

bool IsIdentifierChar(char character)
{
    return std::isalnum(static_cast<unsigned char>(character)) || character == '_';
}

size_t SkipSpace(const std::string& text, size_t position)
{
    while (position < text.size() && (text[position] == ' ' || text[position] == '\t'))
        ++position;
    return position;
}

std::string Trim(const std::string& text)
{
    size_t start = text.find_first_not_of(" \t\r");
    if (start == std::string::npos)
        return std::string();
    return text.substr(start, text.find_last_not_of(" \t\r") - start + 1);
}

std::string ReadIdentifier(const std::string& text, size_t& position)
{
    size_t start = position;
    if (position < text.size() && IsIdentifierStart(text[position]))
    {
        while (position < text.size() && IsIdentifierChar(text[position]))
            ++position;
    }
    return text.substr(start, position - start);
}

bool ReadComment(const std::string& text, size_t position, std::string& comment)
{
    position = SkipSpace(text, position);
    if (position == text.size())
    {
        comment.clear();
        return true;
    }
    if (text.compare(position, 2, "//") != 0)
        return false;
    comment = Trim(text.substr(position + 2));
    return true;
}

bool IsConstOptionName(const std::string& name)
{
    for (const char* known : kConstOptionNames)
    {
        if (name == known)
            return true;
    }
    return false;
}

ParsedLine ParseLine(const std::string& raw)
{
    ParsedLine parsed = {Line_None, true, std::string(), std::string(), std::string(), std::string(), false};
    std::string line = raw;
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    size_t position = SkipSpace(line, 0);

    bool commented = false;
    if (line.compare(position, 2, "//") == 0)
    {
        commented = true;
        position = SkipSpace(line, position + 2);
    }

    if (line.compare(position, 7, "#define") == 0)
    {
        position += 7;
        if (position >= line.size() || (line[position] != ' ' && line[position] != '\t'))
            return parsed;
        position = SkipSpace(line, position);
        std::string name = ReadIdentifier(line, position);
        if (name.empty() || (position < line.size() && line[position] == '('))
            return parsed;
        size_t afterName = SkipSpace(line, position);
        std::string comment;
        if (ReadComment(line, afterName, comment))
        {
            parsed.kind = Line_Boolean;
            parsed.enabled = !commented;
            parsed.name = name;
            parsed.comment = comment;
            parsed.hasComment = afterName < line.size();
            return parsed;
        }
        if (commented)
            return parsed;
        size_t valueEnd = afterName;
        while (valueEnd < line.size() && line[valueEnd] != ' ' && line[valueEnd] != '\t' &&
               line.compare(valueEnd, 2, "//") != 0)
            ++valueEnd;
        if (!ReadComment(line, valueEnd, comment) || comment.find('[') == std::string::npos)
            return parsed;
        parsed.kind = Line_Value;
        parsed.name = name;
        parsed.value = line.substr(afterName, valueEnd - afterName);
        parsed.comment = comment;
        parsed.hasComment = true;
        return parsed;
    }

    if (commented || line.compare(position, 5, "const") != 0)
        return parsed;
    position = SkipSpace(line, position + 5);
    std::string type = ReadIdentifier(line, position);
    if (type != "int" && type != "float" && type != "bool")
        return parsed;
    position = SkipSpace(line, position);
    std::string name = ReadIdentifier(line, position);
    if (!IsConstOptionName(name))
        return parsed;
    position = SkipSpace(line, position);
    if (position < line.size() && line[position] == '=')
        position = SkipSpace(line, position + 1);
    size_t semicolon = line.find(';', position);
    if (semicolon == std::string::npos)
        return parsed;
    std::string value = Trim(line.substr(position, semicolon - position));
    std::string comment;
    if (value.empty() || !ReadComment(line, semicolon + 1, comment))
        return parsed;
    parsed.kind = Line_Const;
    parsed.name = name;
    parsed.value = value;
    parsed.constType = type;
    parsed.comment = comment;
    parsed.hasComment = SkipSpace(line, semicolon + 1) < line.size();
    return parsed;
}

void SplitComment(const std::string& comment, std::string& description, std::vector<std::string>& values, bool& hasList)
{
    hasList = false;
    values.clear();
    size_t open = comment.find('[');
    size_t close = open == std::string::npos ? std::string::npos : comment.find(']', open);
    if (open == std::string::npos || close == std::string::npos)
    {
        description = comment;
        return;
    }
    hasList = true;
    std::string list = comment.substr(open + 1, close - open - 1);
    size_t start = 0;
    while (start < list.size())
    {
        size_t tokenStart = list.find_first_not_of(" \t", start);
        if (tokenStart == std::string::npos)
            break;
        size_t tokenEnd = list.find_first_of(" \t", tokenStart);
        if (tokenEnd == std::string::npos)
            tokenEnd = list.size();
        values.push_back(list.substr(tokenStart, tokenEnd - tokenStart));
        start = tokenEnd;
    }
    description = Trim(comment.substr(0, open) + " " + comment.substr(close + 1));
}

std::set<std::string> ReferencedNames(const std::string& source)
{
    std::set<std::string> names;
    size_t start = 0;
    while (start < source.size())
    {
        size_t end = source.find('\n', start);
        if (end == std::string::npos)
            end = source.size();
        std::string line = source.substr(start, end - start);
        start = end + 1;

        size_t position = SkipSpace(line, 0);
        if (position >= line.size() || line[position] != '#')
            continue;
        position = SkipSpace(line, position + 1);
        std::string directive = ReadIdentifier(line, position);
        if (directive == "ifdef" || directive == "ifndef")
        {
            position = SkipSpace(line, position);
            std::string name = ReadIdentifier(line, position);
            if (!name.empty())
                names.insert(name);
        }
        else if (directive == "if" || directive == "elif")
        {
            size_t search = position;
            while ((search = line.find("defined", search)) != std::string::npos)
            {
                search += 7;
                size_t cursor = SkipSpace(line, search);
                if (cursor < line.size() && line[cursor] == '(')
                    cursor = SkipSpace(line, cursor + 1);
                std::string name = ReadIdentifier(line, cursor);
                if (!name.empty())
                    names.insert(name);
            }
        }
    }
    return names;
}

template <typename Function> void ForEachLine(const std::string& source, Function function)
{
    size_t start = 0;
    while (start < source.size())
    {
        size_t end = source.find('\n', start);
        if (end == std::string::npos)
            end = source.size();
        function(source.substr(start, end - start), end == source.size());
        start = end + 1;
    }
}

bool MakeOption(const ParsedLine& parsed, ShaderOption& option)
{
    if (parsed.kind == Line_None)
        return false;
    option.name = parsed.name;
    option.ambiguous = false;
    option.hasValueList = false;
    if (parsed.kind == Line_Boolean)
    {
        option.kind = ShaderOption::Boolean;
        option.defaultValue = parsed.enabled ? "true" : "false";
        option.values = {"true", "false"};
        option.comment = parsed.comment;
        option.hasValueList = true;
    }
    else if (parsed.kind == Line_Const && parsed.constType == "bool")
    {
        option.kind = ShaderOption::ConstBoolean;
        option.defaultValue = parsed.value;
        option.values = {"true", "false"};
        option.comment = parsed.comment;
        option.hasValueList = true;
    }
    else
    {
        option.kind = parsed.kind == Line_Value ? ShaderOption::Value : ShaderOption::ConstValue;
        option.defaultValue = parsed.value;
        SplitComment(parsed.comment, option.comment, option.values, option.hasValueList);
        if (std::find(option.values.begin(), option.values.end(), option.defaultValue) == option.values.end())
            option.values.insert(option.values.begin(), option.defaultValue);
    }
    return true;
}

struct FileScan
{
    bool loaded;
    std::vector<ShaderOption> candidates;
    std::set<std::string> referenced;
    std::vector<std::string> includes;
};

void ScanText(const std::string& text, const std::string& path, FileScan& scan)
{
    scan.referenced = ReferencedNames(text);
    ForEachLine(text, [&](const std::string& line, bool) {
        std::string include;
        if (!path.empty() && ShaderPack::ParseIncludeLine(line, path, include))
        {
            scan.includes.push_back(include);
            return;
        }
        ShaderOption option;
        if (MakeOption(ParseLine(line), option))
            scan.candidates.push_back(option);
    });
}
}

void ShaderPackOptions::Discover(const ShaderPack& pack, std::vector<std::string>& errors)
{
    mOptions.clear();
    mIndex.clear();
    std::map<std::string, FileScan> scans;
    std::set<std::pair<std::string, size_t>> accepted;

    for (const std::string& program : pack.ProgramFiles())
    {
        std::vector<std::string> order;
        std::set<std::string> visited;
        std::vector<std::string> pending(1, program);
        while (!pending.empty())
        {
            std::string path = pending.back();
            pending.pop_back();
            if (!visited.insert(path).second)
                continue;
            auto found = scans.find(path);
            if (found == scans.end())
            {
                FileScan scan;
                std::string text;
                scan.loaded = pack.ReadFile(path, text);
                if (scan.loaded)
                    ScanText(text, path, scan);
                found = scans.emplace(path, std::move(scan)).first;
            }
            if (!found->second.loaded)
            {
                errors.push_back(program + ": missing " + path);
                continue;
            }
            order.push_back(path);
            for (auto include = found->second.includes.rbegin(); include != found->second.includes.rend(); ++include)
                pending.push_back(*include);
        }

        std::set<std::string> referenced;
        for (const std::string& path : order)
        {
            const std::set<std::string>& names = scans[path].referenced;
            referenced.insert(names.begin(), names.end());
        }
        for (const std::string& path : order)
        {
            const std::vector<ShaderOption>& candidates = scans[path].candidates;
            for (size_t i = 0; i < candidates.size(); ++i)
            {
                if (accepted.count(std::make_pair(path, i)) != 0)
                    continue;
                if (candidates[i].kind == ShaderOption::Boolean && referenced.count(candidates[i].name) == 0)
                    continue;
                accepted.insert(std::make_pair(path, i));
                AddOption(candidates[i]);
            }
        }
    }
    for (auto it = mValues.begin(); it != mValues.end();)
    {
        const ShaderOption* option = Find(it->first);
        if (option == nullptr || option->ambiguous || it->second == option->defaultValue)
            it = mValues.erase(it);
        else
            ++it;
    }
}

void ShaderPackOptions::DiscoverSource(const std::string& source)
{
    FileScan scan;
    ScanText(source, std::string(), scan);
    for (const ShaderOption& option : scan.candidates)
    {
        if (option.kind != ShaderOption::Boolean || scan.referenced.count(option.name) != 0)
            AddOption(option);
    }
}

void ShaderPackOptions::AddOption(const ShaderOption& option)
{
    auto found = mIndex.find(option.name);
    if (found == mIndex.end())
    {
        mIndex[option.name] = mOptions.size();
        mOptions.push_back(option);
        return;
    }
    ShaderOption& existing = mOptions[found->second];
    if (existing.kind != option.kind || existing.defaultValue != option.defaultValue)
        existing.ambiguous = true;
    if (existing.comment.empty())
        existing.comment = option.comment;
    for (const std::string& value : option.values)
    {
        if (std::find(existing.values.begin(), existing.values.end(), value) == existing.values.end())
            existing.values.push_back(value);
    }
    existing.hasValueList = existing.hasValueList || option.hasValueList;
}

const ShaderOption* ShaderPackOptions::Find(const std::string& name) const
{
    auto found = mIndex.find(name);
    return found == mIndex.end() ? nullptr : &mOptions[found->second];
}

std::string ShaderPackOptions::Value(const std::string& name) const
{
    auto found = mValues.find(name);
    if (found != mValues.end())
        return found->second;
    const ShaderOption* option = Find(name);
    return option ? option->defaultValue : std::string();
}

bool ShaderPackOptions::SetValue(const std::string& name, const std::string& value)
{
    const ShaderOption* option = Find(name);
    if (option == nullptr || option->ambiguous)
        return false;
    if (option->hasValueList && std::find(option->values.begin(), option->values.end(), value) == option->values.end())
        return false;
    if (value == option->defaultValue)
        mValues.erase(name);
    else
        mValues[name] = value;
    return true;
}

void ShaderPackOptions::ResetAll()
{
    mValues.clear();
}

void ShaderPackOptions::CycleValue(const std::string& name, int direction)
{
    const ShaderOption* option = Find(name);
    if (option == nullptr || option->ambiguous || option->values.empty())
        return;
    std::string current = Value(name);
    auto found = std::find(option->values.begin(), option->values.end(), current);
    int count = static_cast<int>(option->values.size());
    int index = found == option->values.end() ? 0 : static_cast<int>(found - option->values.begin());
    index = ((index + direction) % count + count) % count;
    SetValue(name, option->values[index]);
}

void ShaderPackOptions::LoadValues(const std::string& text)
{
    mValues.clear();
    ForEachLine(text, [&](const std::string& rawLine, bool) {
        std::string line = Trim(rawLine);
        if (line.empty() || line[0] == '#')
            return;
        size_t equals = line.find('=');
        if (equals == std::string::npos)
            return;
        std::string name = Trim(line.substr(0, equals));
        std::string value = Trim(line.substr(equals + 1));
        if (mOptions.empty())
            mValues[name] = value;
        else
            SetValue(name, value);
    });
}

std::string ShaderPackOptions::SaveValues() const
{
    std::string text;
    for (const auto& entry : mValues)
        text += entry.first + "=" + entry.second + "\n";
    return text;
}

ShaderMacros ShaderPackOptions::Macros() const
{
    ShaderMacros macros;
    for (const ShaderOption& option : mOptions)
    {
        std::string value = option.ambiguous ? option.defaultValue : Value(option.name);
        if (option.kind == ShaderOption::Boolean)
        {
            if (value == "true")
                macros[option.name] = std::string();
        }
        else if (option.kind == ShaderOption::Value)
        {
            macros[option.name] = value;
        }
    }
    return macros;
}

std::string ShaderPackOptions::ApplyToSource(const std::string& source) const
{
    std::set<std::string> referenced = ReferencedNames(source);
    std::string output;
    output.reserve(source.size() + 256);
    ForEachLine(source, [&](const std::string& line, bool last) {
        ParsedLine parsed = ParseLine(line);
        const ShaderOption* option = parsed.kind == Line_None ? nullptr : Find(parsed.name);
        bool rewrite = option != nullptr && !option->ambiguous &&
                       (parsed.kind != Line_Boolean || referenced.count(parsed.name) != 0);
        if (rewrite)
        {
            std::string value = Value(parsed.name);
            std::string comment = parsed.hasComment ? " // " + parsed.comment : std::string();
            if (parsed.kind == Line_Boolean)
                output += (value == "true" ? "#define " : "//#define ") + parsed.name + comment;
            else if (parsed.kind == Line_Value)
                output += "#define " + parsed.name + " " + value + comment;
            else
                output += "const " + parsed.constType + " " + parsed.name + " = " + value + ";" + comment;
        }
        else
        {
            output += line;
        }
        if (!last)
            output += '\n';
    });
    return output;
}
