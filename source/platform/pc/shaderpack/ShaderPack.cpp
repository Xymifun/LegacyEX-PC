// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPack.cpp
#include "platform/pc/shaderpack/ShaderPack.hpp"

#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <iterator>
#include <set>

namespace
{
const size_t kMaxIncludeDepth = 64;

bool IsDirectory(const std::string& path)
{
    struct stat info;
    return stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}

bool IsRegularFile(const std::string& path)
{
    struct stat info;
    return stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode);
}

// Every file below folder, as paths relative to it joined with '/'.
void ListFilesRecursive(const std::string& folder, const std::string& prefix, std::vector<std::string>& files)
{
    DIR* directory = opendir(folder.c_str());
    if (directory == nullptr)
        return;
    while (struct dirent* entry = readdir(directory))
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;
        std::string full = folder + "/" + name;
        std::string relative = prefix.empty() ? name : prefix + "/" + name;
        if (IsDirectory(full))
            ListFilesRecursive(full, relative, files);
        else
            files.push_back(relative);
    }
    closedir(directory);
}

bool EndsWith(const std::string& text, const char* suffix)
{
    size_t length = std::char_traits<char>::length(suffix);
    return text.size() >= length && text.compare(text.size() - length, length, suffix) == 0;
}

std::string ParentFolder(const std::string& path)
{
    size_t slash = path.rfind('/');
    return slash == std::string::npos ? std::string() : path.substr(0, slash + 1);
}

bool ParseInclude(const std::string& line, std::string& target)
{
    size_t position = line.find_first_not_of(" \t");
    if (position == std::string::npos || line.compare(position, 8, "#include") != 0)
        return false;
    position = line.find_first_not_of(" \t", position + 8);
    if (position == std::string::npos)
        return false;
    char open = line[position];
    char close = open == '"' ? '"' : open == '<' ? '>' : 0;
    if (close == 0)
        return false;
    size_t end = line.find(close, position + 1);
    if (end == std::string::npos)
        return false;
    target = line.substr(position + 1, end - position - 1);
    return !target.empty();
}
}

std::string ShaderPack::NormalisePath(const std::string& path)
{
    std::vector<std::string> parts;
    size_t start = 0;
    std::string unified = path;
    std::replace(unified.begin(), unified.end(), '\\', '/');
    while (start <= unified.size())
    {
        size_t end = unified.find('/', start);
        if (end == std::string::npos)
            end = unified.size();
        std::string part = unified.substr(start, end - start);
        if (part == "..")
        {
            if (!parts.empty())
                parts.pop_back();
        }
        else if (!part.empty() && part != ".")
        {
            parts.push_back(part);
        }
        start = end + 1;
    }
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i)
    {
        if (i != 0)
            result += '/';
        result += parts[i];
    }
    return result;
}

bool ShaderPack::IsShaderPack(const std::string& path)
{
    if (IsDirectory(path))
        return IsDirectory(path + "/shaders");
    if (!IsRegularFile(path))
        return false;
    std::string extension = path.size() >= 4 ? path.substr(path.size() - 4) : std::string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".zip";
}

std::unique_ptr<ShaderPack> ShaderPack::Open(const std::string& path, std::string& error)
{
    std::unique_ptr<ShaderPack> pack(new ShaderPack());
    size_t slash = path.find_last_of("/\\");
    pack->mName = slash == std::string::npos ? path : path.substr(slash + 1);

    if (IsDirectory(path))
    {
        std::string shaders = path + "/shaders";
        if (!IsDirectory(shaders))
        {
            error = "the folder has no shaders folder";
            return nullptr;
        }
        pack->mFolder = shaders;
        ListFilesRecursive(shaders, std::string(), pack->mFiles);
        for (std::string& file : pack->mFiles)
            file = NormalisePath(file);
    }
    else
    {
        pack->mZip.reset(new ZipArchive());
        if (!pack->mZip->Open(path, error))
            return nullptr;
        for (const std::string& name : pack->mZip->Names())
        {
            if (name.compare(0, 8, "shaders/") == 0)
                pack->mFiles.push_back(NormalisePath(name.substr(8)));
        }
        if (pack->mFiles.empty())
        {
            error = "the archive has no shaders folder";
            return nullptr;
        }
        pack->mZipRoot = "shaders/";
    }

    std::sort(pack->mFiles.begin(), pack->mFiles.end());
    return pack;
}

bool ShaderPack::HasFile(const std::string& path) const
{
    return std::binary_search(mFiles.begin(), mFiles.end(), NormalisePath(path));
}

bool ShaderPack::ReadFile(const std::string& path, std::string& out) const
{
    std::string normalised = NormalisePath(path);
    auto cached = mCache.find(normalised);
    if (cached != mCache.end())
    {
        out = cached->second;
        return true;
    }
    if (!std::binary_search(mFiles.begin(), mFiles.end(), normalised))
        return false;
    if (mZip)
    {
        std::string error;
        if (!mZip->Read(mZipRoot + normalised, out, error))
            return false;
    }
    else
    {
        std::ifstream file((mFolder + "/" + normalised).c_str(), std::ios::binary);
        if (!file)
            return false;
        out.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
    mCache[normalised] = out;
    return true;
}

bool ShaderPack::ParseIncludeLine(const std::string& line, const std::string& fromFile, std::string& resolved)
{
    std::string include;
    if (!ParseInclude(line, include))
        return false;
    resolved = include[0] == '/' ? NormalisePath(include) : NormalisePath(ParentFolder(fromFile) + include);
    return true;
}

bool ShaderPack::LoadSource(const std::string& path, Source& out, bool lineDirectives, std::string& error) const
{
    out.text.clear();
    out.files.clear();
    std::vector<std::string> stack;
    return AppendSource(NormalisePath(path), out, lineDirectives, stack, error);
}

bool ShaderPack::AppendSource(const std::string& path, Source& out, bool lineDirectives,
                              std::vector<std::string>& stack, std::string& error) const
{
    if (stack.size() >= kMaxIncludeDepth || std::find(stack.begin(), stack.end(), path) != stack.end())
    {
        error = "include loop at " + path;
        return false;
    }
    std::string text;
    if (!ReadFile(path, text))
    {
        error = stack.empty() ? "missing " + path : "missing include " + path + " (from " + stack.back() + ")";
        return false;
    }
    if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF &&
        static_cast<unsigned char>(text[1]) == 0xBB && static_cast<unsigned char>(text[2]) == 0xBF)
        text.erase(0, 3);
    std::replace(text.begin(), text.end(), '\0', ' ');

    size_t fileIndex = out.files.size();
    out.files.push_back(path);
    stack.push_back(path);

    size_t lineNumber = 0;
    size_t start = 0;
    while (start < text.size())
    {
        size_t end = text.find('\n', start);
        if (end == std::string::npos)
            end = text.size();
        std::string line = text.substr(start, end - start);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        ++lineNumber;

        std::string target;
        if (ParseIncludeLine(line, path, target))
        {
            if (lineDirectives)
                out.text += "#line 1 " + std::to_string(out.files.size()) + "\n";
            if (!AppendSource(target, out, lineDirectives, stack, error))
                return false;
            if (lineDirectives)
                out.text += "#line " + std::to_string(lineNumber + 1) + " " + std::to_string(fileIndex) + "\n";
        }
        else
        {
            out.text += line;
            out.text += '\n';
        }
        start = end + 1;
    }

    stack.pop_back();
    return true;
}

std::vector<std::string> ShaderPack::ProgramFiles() const
{
    std::vector<std::string> programs;
    for (const std::string& file : mFiles)
    {
        if (EndsWith(file, ".vsh") || EndsWith(file, ".fsh") || EndsWith(file, ".gsh") || EndsWith(file, ".csh"))
            programs.push_back(file);
    }
    return programs;
}

std::vector<std::string> ShaderPack::WorldFolders() const
{
    std::set<std::string> folders;
    for (const std::string& file : mFiles)
    {
        if (file.compare(0, 5, "world") == 0)
        {
            size_t slash = file.find('/');
            if (slash != std::string::npos)
                folders.insert(file.substr(0, slash));
        }
    }
    return std::vector<std::string>(folders.begin(), folders.end());
}
