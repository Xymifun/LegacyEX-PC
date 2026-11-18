// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPackOptions.hpp
#ifndef PLATFORM_PC_SHADERPACK_SHADERPACKOPTIONS_HPP
#define PLATFORM_PC_SHADERPACK_SHADERPACKOPTIONS_HPP

#include <map>
#include <string>
#include <vector>

#include "platform/pc/shaderpack/ShaderPreprocessor.hpp"

class ShaderPack;

struct ShaderOption
{
    enum Kind
    {
        Boolean,
        Value,
        ConstValue,
        ConstBoolean
    };

    std::string name;
    Kind kind;
    std::string defaultValue;
    std::vector<std::string> values;
    std::string comment;
    bool ambiguous;
    bool hasValueList;

    bool IsBoolean() const { return kind == Boolean || kind == ConstBoolean; }
};

class ShaderPackOptions
{
public:
    void Discover(const ShaderPack& pack, std::vector<std::string>& errors);
    void DiscoverSource(const std::string& source);

    const std::vector<ShaderOption>& Options() const { return mOptions; }
    const ShaderOption* Find(const std::string& name) const;
    std::string Value(const std::string& name) const;
    bool SetValue(const std::string& name, const std::string& value);
    void ResetAll();
    void CycleValue(const std::string& name, int direction);

    void LoadValues(const std::string& text);
    std::string SaveValues() const;

    ShaderMacros Macros() const;
    std::string ApplyToSource(const std::string& source) const;

private:
    void AddOption(const ShaderOption& option);

    std::vector<ShaderOption> mOptions;
    std::map<std::string, size_t> mIndex;
    std::map<std::string, std::string> mValues;
};

#endif
