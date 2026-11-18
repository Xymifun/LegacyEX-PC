// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPackLang.hpp
#ifndef PLATFORM_PC_SHADERPACK_SHADERPACKLANG_HPP
#define PLATFORM_PC_SHADERPACK_SHADERPACKLANG_HPP

#include <map>
#include <string>

class ShaderPack;
struct ShaderOption;

class ShaderPackLang
{
public:
    void Load(const ShaderPack& pack, const std::string& locale);
    size_t Size() const { return mEntries.size(); }
    bool Has(const std::string& key) const { return mEntries.count(key) != 0; }
    std::string Get(const std::string& key, const std::string& fallback) const;

    std::string OptionLabel(const std::string& option) const;
    std::string OptionComment(const ShaderOption& option) const;
    std::string ValueLabel(const ShaderOption& option, const std::string& value) const;
    std::string ScreenLabel(const std::string& screen) const;
    std::string ScreenComment(const std::string& screen) const;
    std::string ProfileLabel(const std::string& profile) const;
    std::string ProfileComment(const std::string& profile) const;

    static std::string StripFormatting(const std::string& text);

private:
    void LoadFile(const ShaderPack& pack, const std::string& locale);

    std::map<std::string, std::string> mEntries;
};

#endif
