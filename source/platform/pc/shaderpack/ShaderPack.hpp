// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPack.hpp
#ifndef PLATFORM_PC_SHADERPACK_SHADERPACK_HPP
#define PLATFORM_PC_SHADERPACK_SHADERPACK_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "platform/pc/shaderpack/ZipArchive.hpp"

class ShaderPack
{
public:
    struct Source
    {
        std::string text;
        std::vector<std::string> files;
    };

    static std::unique_ptr<ShaderPack> Open(const std::string& path, std::string& error);
    static bool IsShaderPack(const std::string& path);

    const std::string& Name() const { return mName; }
    const std::vector<std::string>& Files() const { return mFiles; }
    bool HasFile(const std::string& path) const;
    bool ReadFile(const std::string& path, std::string& out) const;
    bool LoadSource(const std::string& path, Source& out, bool lineDirectives, std::string& error) const;
    std::vector<std::string> ProgramFiles() const;
    std::vector<std::string> WorldFolders() const;

    static std::string NormalisePath(const std::string& path);
    static bool ParseIncludeLine(const std::string& line, const std::string& fromFile, std::string& resolved);

private:
    bool AppendSource(const std::string& path, Source& out, bool lineDirectives, std::vector<std::string>& stack,
                      std::string& error) const;

    std::string mName;
    std::string mFolder;
    std::unique_ptr<ZipArchive> mZip;
    std::string mZipRoot;
    std::vector<std::string> mFiles;
    mutable std::unordered_map<std::string, std::string> mCache;
};

#endif
