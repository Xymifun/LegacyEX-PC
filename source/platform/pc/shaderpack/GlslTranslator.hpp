// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/GlslTranslator.hpp
#ifndef PLATFORM_PC_SHADERPACK_GLSLTRANSLATOR_HPP
#define PLATFORM_PC_SHADERPACK_GLSLTRANSLATOR_HPP

#include <set>
#include <string>
#include <utility>
#include <vector>

enum ShaderStage
{
    ShaderStage_Vertex,
    ShaderStage_Geometry,
    ShaderStage_Fragment,
    ShaderStage_Count
};

struct GlslUsage
{
    std::set<std::string> builtins;
    std::vector<std::pair<std::string, std::string>> attributes;
    std::set<int> fragData;
    std::string output0;
    int outputLocations = 0;
    int texCoordCount = 0;
};

class GlslTranslator
{
public:
    static const int kHeaderSourceIndex = 999;
    static const int kFooterSourceIndex = 998;

    bool Parse(ShaderStage stage, const std::string& source, std::string& error);

    ShaderStage Stage() const { return mStage; }
    int Version() const { return mVersion; }
    bool UsesDoubles() const { return mUsesDoubles; }
    const GlslUsage& Usage() const { return mUsage; }
    bool Uses(const std::string& builtin) const { return mUsage.builtins.count(builtin) != 0; }

    std::string Emit(int version, const std::string& header, const std::string& footer,
                     const GlslUsage& linked) const;

    static std::string VersionLine(int version);
    static std::string StripUnusedFunctions(const std::string& source);
    static std::set<std::string> DeclaredNames(const std::string& source, const char* qualifier);
    static std::string RequalifyDeclarations(const std::string& source, const char* from, const char* to,
                                             const std::set<std::string>& names);

    struct Token
    {
        enum Type
        {
            Space,
            Newline,
            Comment,
            Identifier,
            Number,
            Directive,
            Punct
        };

        Type type;
        std::string text;
    };

private:

    static void Tokenize(const std::string& text, bool directives, std::vector<Token>& tokens);
    bool Rewrite(std::vector<Token>& tokens, bool macroBody, std::string& error);
    bool RewriteDirective(Token& token, std::string& error);
    size_t NextSignificant(const std::vector<Token>& tokens, size_t index) const;
    size_t PreviousSignificant(const std::vector<Token>& tokens, size_t index) const;
    int LayoutLocation(const std::vector<Token>& tokens, size_t close) const;
    static bool IsOutputQualifier(const std::string& text);
    void Use(const std::string& name) { mUsage.builtins.insert(name); }

    ShaderStage mStage = ShaderStage_Vertex;
    int mVersion = 110;
    bool mRenameTexture = false;
    bool mSeenCode = false;
    bool mGpuShader4 = false;
    bool mUsesDoubles = false;
    int mUserOutputs = 0;
    std::set<int> mExplicitOutputs;
    std::vector<std::string> mLeadingExtensions;
    std::vector<Token> mTokens;
    GlslUsage mUsage;
};

#endif
