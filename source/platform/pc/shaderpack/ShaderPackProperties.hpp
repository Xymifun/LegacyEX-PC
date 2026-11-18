// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPackProperties.hpp
#ifndef PLATFORM_PC_SHADERPACK_SHADERPACKPROPERTIES_HPP
#define PLATFORM_PC_SHADERPACK_SHADERPACKPROPERTIES_HPP

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "platform/pc/shaderpack/ShaderPreprocessor.hpp"

class ShaderPackOptions;

typedef std::vector<std::pair<std::string, std::string>> ShaderPropertyList;

ShaderPropertyList ShaderParseProperties(const std::string& text);
std::vector<std::string> ShaderSplitList(const std::string& text);

struct ShaderScreen
{
    std::string name;
    int columns;
    std::vector<std::string> items;
};

struct ShaderProfile
{
    std::string name;
    std::vector<std::pair<std::string, std::string>> values;
    std::vector<std::string> disabledPrograms;
};

class ShaderPackProperties
{
public:
    void Parse(const std::string& text, const ShaderMacros& standardMacros, const ShaderMacros& optionMacros,
               const ShaderPackOptions& options);

    bool Has(const std::string& key) const { return mValues.count(key) != 0; }
    std::string Get(const std::string& key, const std::string& fallback = std::string()) const;
    const ShaderPropertyList& Entries() const { return mEntries; }

    const ShaderScreen* Screen(const std::string& name) const;
    const std::vector<ShaderScreen>& Screens() const { return mScreens; }
    const std::vector<ShaderProfile>& Profiles() const { return mProfiles; }
    const ShaderProfile* Profile(const std::string& name) const;
    bool IsSlider(const std::string& option) const { return mSliders.count(option) != 0; }

    static bool ProfileMatches(const ShaderProfile& profile, const ShaderPackOptions& options);
    static void ApplyProfile(const ShaderProfile& profile, ShaderPackOptions& options);
    std::string ActiveProfile(const ShaderPackOptions& options) const;

private:
    void ResolveProfile(const std::string& name, const std::map<std::string, std::string>& raw,
                        const ShaderPackOptions& options, ShaderProfile& profile, std::vector<std::string>& stack) const;

    ShaderPropertyList mEntries;
    std::map<std::string, std::string> mValues;
    std::vector<ShaderScreen> mScreens;
    std::vector<ShaderProfile> mProfiles;
    std::set<std::string> mSliders;
};

#endif
