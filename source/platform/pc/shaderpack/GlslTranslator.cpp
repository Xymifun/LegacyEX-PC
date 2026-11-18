// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/GlslTranslator.cpp
#include "platform/pc/shaderpack/GlslTranslator.hpp"

#include <cctype>
#include <cstdlib>
#include <map>

namespace
{
const char* const kPackAttributes[] = {"mc_Entity", "mc_midTexCoord", "at_tangent", "at_velocity", "at_midBlock"};

const char* const kDroppedExtensions[] = {"GL_EXT_gpu_shader4", "GL_ARB_shader_texture_lod",
                                          "GL_EXT_shader_texture_lod"};

const char* const kReservedSince130[] = {"flat",   "smooth",    "noperspective", "layout",     "uint",
                                         "uvec2",  "uvec3",     "uvec4",         "sample",     "patch",
                                         "subroutine", "common", "partition",    "active",     "filter",
                                         "resource", "buffer",  "shared",        "precise",    "coherent",
                                         "restrict", "readonly", "writeonly",    "invariant"};

const char* const kGpuShader4Keywords[] = {"flat", "noperspective", "uint", "uvec2", "uvec3", "uvec4"};

const char* const kDoubleTypes[] = {"double", "dvec2",   "dvec3",   "dvec4",   "dmat2",   "dmat3",
                                    "dmat4",  "dmat2x2", "dmat2x3", "dmat2x4", "dmat3x2", "dmat3x3",
                                    "dmat3x4", "dmat4x2", "dmat4x3", "dmat4x4"};

const char* const kLenientKeywords[] = {"flat",  "smooth", "noperspective", "invariant", "layout",
                                        "uint",  "uvec2",  "uvec3",         "uvec4"};

const char* const kUniformBuiltins[] = {"gl_ModelViewMatrix",
                                        "gl_ModelViewMatrixInverse",
                                        "gl_ModelViewMatrixTranspose",
                                        "gl_ModelViewMatrixInverseTranspose",
                                        "gl_ProjectionMatrix",
                                        "gl_ProjectionMatrixInverse",
                                        "gl_ProjectionMatrixTranspose",
                                        "gl_ProjectionMatrixInverseTranspose",
                                        "gl_ModelViewProjectionMatrix",
                                        "gl_ModelViewProjectionMatrixInverse",
                                        "gl_ModelViewProjectionMatrixTranspose",
                                        "gl_ModelViewProjectionMatrixInverseTranspose",
                                        "gl_NormalMatrix",
                                        "gl_TextureMatrix",
                                        "gl_TextureMatrixInverse",
                                        "gl_TextureMatrixTranspose",
                                        "gl_TextureMatrixInverseTranspose",
                                        "gl_Fog",
                                        "gl_LightSource",
                                        "gl_LightModel",
                                        "gl_FrontMaterial",
                                        "gl_BackMaterial",
                                        "gl_ClipVertex",
                                        "gl_MaxTextureCoords"};

const char* const kVertexInputs[] = {"gl_Vertex",         "gl_Color",          "gl_SecondaryColor",
                                     "gl_Normal",         "gl_FogCoord",       "gl_MultiTexCoord0",
                                     "gl_MultiTexCoord1", "gl_MultiTexCoord2", "gl_MultiTexCoord3",
                                     "gl_MultiTexCoord4", "gl_MultiTexCoord5", "gl_MultiTexCoord6",
                                     "gl_MultiTexCoord7"};

const std::map<std::string, std::string>& FunctionRenames()
{
    static const std::map<std::string, std::string> renames = {
        {"texture1D", "texture"},
        {"texture2D", "texture"},
        {"texture3D", "texture"},
        {"textureCube", "texture"},
        {"texture2DRect", "texture"},
        {"texture1DLod", "textureLod"},
        {"texture2DLod", "textureLod"},
        {"texture3DLod", "textureLod"},
        {"textureCubeLod", "textureLod"},
        {"texture2DLodEXT", "textureLod"},
        {"texture2DLodARB", "textureLod"},
        {"texture3DLodARB", "textureLod"},
        {"textureCubeLodEXT", "textureLod"},
        {"textureCubeLodARB", "textureLod"},
        {"texture2DGrad", "textureGrad"},
        {"texture2DGradARB", "textureGrad"},
        {"texture2DGradEXT", "textureGrad"},
        {"texture3DGrad", "textureGrad"},
        {"texture3DGradARB", "textureGrad"},
        {"textureCubeGrad", "textureGrad"},
        {"textureCubeGradARB", "textureGrad"},
        {"textureCubeGradEXT", "textureGrad"},
        {"texture1DProj", "textureProj"},
        {"texture2DProj", "textureProj"},
        {"texture3DProj", "textureProj"},
        {"texture2DProjLod", "textureProjLod"},
        {"texture3DProjLod", "textureProjLod"},
        {"texture2DProjLodEXT", "textureProjLod"},
        {"texture2DProjLodARB", "textureProjLod"},
        {"texture2DProjGradARB", "textureProjGrad"},
        {"texelFetch1D", "texelFetch"},
        {"texelFetch2D", "texelFetch"},
        {"texelFetch3D", "texelFetch"},
        {"texelFetch2DRect", "texelFetch"},
        {"textureSize1D", "textureSize"},
        {"textureSize2D", "textureSize"},
        {"textureSize3D", "textureSize"},
        {"textureSizeCube", "textureSize"},
        {"textureSize2DRect", "textureSize"},
        {"shadow1D", "lce_shadow2D"},
        {"shadow2D", "lce_shadow2D"},
        {"shadow2DLod", "lce_shadow2DLod"},
        {"shadow2DLodEXT", "lce_shadow2DLod"},
        {"shadow2DLodARB", "lce_shadow2DLod"},
        {"shadow2DProj", "lce_shadow2DProj"},
        {"shadow2DGradARB", "lce_shadow2DGrad"},
    };
    return renames;
}

template <size_t N> bool Contains(const char* const (&list)[N], const std::string& value)
{
    for (const char* entry : list)
    {
        if (value == entry)
            return true;
    }
    return false;
}

bool IsIdentifierStart(char character)
{
    return std::isalpha(static_cast<unsigned char>(character)) || character == '_';
}

bool IsIdentifierChar(char character)
{
    return std::isalnum(static_cast<unsigned char>(character)) || character == '_';
}

std::string ReplaceGlPrefix(const std::string& name)
{
    return "lce_" + name.substr(3);
}
}

void GlslTranslator::Tokenize(const std::string& text, bool directives, std::vector<Token>& tokens)
{
    size_t length = text.size();
    size_t index = 0;
    bool lineStart = true;

    while (index < length)
    {
        char character = text[index];
        if (character == '\n')
        {
            tokens.push_back({Token::Newline, "\n"});
            lineStart = true;
            ++index;
            continue;
        }
        if (character == '\\' && index + 1 < length && (text[index + 1] == '\n' || text[index + 1] == '\r'))
        {
            size_t end = index + 1;
            if (text[end] == '\r')
                ++end;
            if (end < length && text[end] == '\n')
                ++end;
            tokens.push_back({Token::Space, text.substr(index, end - index)});
            index = end;
            continue;
        }
        if (character == ' ' || character == '\t' || character == '\r' || character == '\f' || character == '\v')
        {
            size_t end = index;
            while (end < length && (text[end] == ' ' || text[end] == '\t' || text[end] == '\r' || text[end] == '\f'
                                    || text[end] == '\v'))
                ++end;
            tokens.push_back({Token::Space, text.substr(index, end - index)});
            index = end;
            continue;
        }
        if (directives && lineStart && character == '#')
        {
            size_t end = index;
            while (end < length && text[end] != '\n')
            {
                if (text[end] == '\\' && end + 1 < length && (text[end + 1] == '\n' || text[end + 1] == '\r'))
                {
                    end += text[end + 1] == '\r' && end + 2 < length && text[end + 2] == '\n' ? 3 : 2;
                    continue;
                }
                if (text[end] == '/' && end + 1 < length && text[end + 1] == '*')
                {
                    size_t close = text.find("*/", end + 2);
                    end = close == std::string::npos ? length : close + 2;
                    continue;
                }
                ++end;
            }
            tokens.push_back({Token::Directive, text.substr(index, end - index)});
            index = end;
            lineStart = false;
            continue;
        }
        lineStart = false;
        if (character == '/' && index + 1 < length && text[index + 1] == '/')
        {
            size_t end = text.find('\n', index);
            if (end == std::string::npos)
                end = length;
            tokens.push_back({Token::Comment, text.substr(index, end - index)});
            index = end;
            continue;
        }
        if (character == '/' && index + 1 < length && text[index + 1] == '*')
        {
            size_t close = text.find("*/", index + 2);
            size_t end = close == std::string::npos ? length : close + 2;
            tokens.push_back({Token::Comment, text.substr(index, end - index)});
            index = end;
            continue;
        }
        if (IsIdentifierStart(character))
        {
            size_t end = index;
            while (end < length && IsIdentifierChar(text[end]))
                ++end;
            tokens.push_back({Token::Identifier, text.substr(index, end - index)});
            index = end;
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(character))
            || (character == '.' && index + 1 < length && std::isdigit(static_cast<unsigned char>(text[index + 1]))))
        {
            size_t end = index;
            bool hex = character == '0' && index + 1 < length && (text[index + 1] == 'x' || text[index + 1] == 'X');
            if (hex)
                end += 2;
            while (end < length)
            {
                char current = text[end];
                if (IsIdentifierChar(current) || current == '.')
                {
                    ++end;
                    continue;
                }
                if (!hex && (current == '+' || current == '-') && (text[end - 1] == 'e' || text[end - 1] == 'E'))
                {
                    ++end;
                    continue;
                }
                break;
            }
            tokens.push_back({Token::Number, text.substr(index, end - index)});
            index = end;
            continue;
        }
        tokens.push_back({Token::Punct, std::string(1, character)});
        ++index;
    }
}

size_t GlslTranslator::NextSignificant(const std::vector<Token>& tokens, size_t index) const
{
    for (size_t next = index + 1; next < tokens.size(); ++next)
    {
        Token::Type type = tokens[next].type;
        if (type != Token::Space && type != Token::Newline && type != Token::Comment)
            return next;
    }
    return std::string::npos;
}

size_t GlslTranslator::PreviousSignificant(const std::vector<Token>& tokens, size_t index) const
{
    while (index-- > 0)
    {
        Token::Type type = tokens[index].type;
        if (type != Token::Space && type != Token::Newline && type != Token::Comment)
            return index;
    }
    return std::string::npos;
}

bool GlslTranslator::Parse(ShaderStage stage, const std::string& source, std::string& error)
{
    mStage = stage;
    mVersion = 110;
    mRenameTexture = false;
    mSeenCode = false;
    mUserOutputs = 0;
    mLeadingExtensions.clear();
    mTokens.clear();
    mUsage = GlslUsage();

    Tokenize(source, true, mTokens);

    bool gpuShader4 = false;
    mUsesDoubles = false;
    for (size_t index = 0; index < mTokens.size(); ++index)
    {
        const Token& token = mTokens[index];
        if (token.type == Token::Directive && token.text.find("GL_EXT_gpu_shader4") != std::string::npos)
            gpuShader4 = true;
        if (token.type == Token::Identifier && Contains(kDoubleTypes, token.text))
            mUsesDoubles = true;
        if (token.type == Token::Number && token.text.size() > 2
            && (token.text.compare(token.text.size() - 2, 2, "lf") == 0
                || token.text.compare(token.text.size() - 2, 2, "LF") == 0))
            mUsesDoubles = true;
        if (token.type != Token::Identifier || token.text != "texture")
            continue;
        size_t type = PreviousSignificant(mTokens, index);
        if (type == std::string::npos || mTokens[type].text != "sampler2D")
            continue;
        size_t qualifier = PreviousSignificant(mTokens, type);
        for (int step = 0; step < 3 && qualifier != std::string::npos; ++step)
        {
            if (mTokens[qualifier].text == "uniform")
            {
                mRenameTexture = true;
                break;
            }
            qualifier = PreviousSignificant(mTokens, qualifier);
        }
    }
    mGpuShader4 = gpuShader4;

    mExplicitOutputs.clear();
    if (stage == ShaderStage_Fragment)
    {
        for (size_t index = 0; index < mTokens.size(); ++index)
        {
            if (mTokens[index].type != Token::Identifier || mTokens[index].text != "out")
                continue;
            size_t scan = PreviousSignificant(mTokens, index);
            while (scan != std::string::npos && IsOutputQualifier(mTokens[scan].text))
                scan = PreviousSignificant(mTokens, scan);
            if (scan == std::string::npos || mTokens[scan].text != ")")
                continue;
            int location = LayoutLocation(mTokens, scan);
            if (location >= 0)
                mExplicitOutputs.insert(location);
        }
    }

    return Rewrite(mTokens, false, error);
}

bool GlslTranslator::IsOutputQualifier(const std::string& text)
{
    return text == "flat" || text == "smooth" || text == "centroid" || text == "invariant" || text == "highp"
           || text == "mediump" || text == "lowp" || text == "precise";
}

int GlslTranslator::LayoutLocation(const std::vector<Token>& tokens, size_t close) const
{
    size_t open = close;
    while (open != std::string::npos && tokens[open].text != "(")
        open = PreviousSignificant(tokens, open);
    int location = -1;
    for (size_t inner = open == std::string::npos ? close : open; inner < close; ++inner)
    {
        if (tokens[inner].text != "location")
            continue;
        size_t equals = NextSignificant(tokens, inner);
        size_t value = equals == std::string::npos ? equals : NextSignificant(tokens, equals);
        if (value != std::string::npos && tokens[equals].text == "=" && tokens[value].type == Token::Number)
            location = std::atoi(tokens[value].text.c_str());
    }
    return location;
}

bool GlslTranslator::RewriteDirective(Token& token, std::string& error)
{
    const std::string& text = token.text;
    size_t position = 1;
    while (position < text.size() && (text[position] == ' ' || text[position] == '\t'))
        ++position;
    size_t nameEnd = position;
    while (nameEnd < text.size() && IsIdentifierChar(text[nameEnd]))
        ++nameEnd;
    std::string name = text.substr(position, nameEnd - position);

    if (name == "version")
    {
        mVersion = std::atoi(text.c_str() + nameEnd);
        token.text.clear();
        return true;
    }

    if (name == "extension")
    {
        size_t start = nameEnd;
        while (start < text.size() && (text[start] == ' ' || text[start] == '\t'))
            ++start;
        size_t end = start;
        while (end < text.size() && IsIdentifierChar(text[end]))
            ++end;
        std::string extension = text.substr(start, end - start);
        if (Contains(kDroppedExtensions, extension))
        {
            token.text.clear();
            return true;
        }
        mLeadingExtensions.push_back(text);
        token.text.clear();
        return true;
    }

    if (name != "define")
        return true;

    size_t macroStart = nameEnd;
    while (macroStart < text.size() && (text[macroStart] == ' ' || text[macroStart] == '\t'))
        ++macroStart;
    size_t macroEnd = macroStart;
    while (macroEnd < text.size() && IsIdentifierChar(text[macroEnd]))
        ++macroEnd;
    if (macroEnd < text.size() && text[macroEnd] == '(')
    {
        size_t close = text.find(')', macroEnd);
        macroEnd = close == std::string::npos ? text.size() : close + 1;
    }

    std::vector<Token> body;
    Tokenize(text.substr(macroEnd), false, body);
    if (!Rewrite(body, true, error))
        return false;

    std::string rewritten = text.substr(0, macroEnd);
    for (const Token& part : body)
        rewritten += part.text;
    token.text = rewritten;
    return true;
}

bool GlslTranslator::Rewrite(std::vector<Token>& tokens, bool macroBody, std::string& error)
{
    const std::map<std::string, std::string>& renames = FunctionRenames();

    for (size_t index = 0; index < tokens.size(); ++index)
    {
        Token& token = tokens[index];

        if (token.type == Token::Directive)
        {
            if (!RewriteDirective(token, error))
                return false;
            continue;
        }
        if (token.type == Token::Number || token.type == Token::Punct)
        {
            if (!macroBody)
                mSeenCode = true;
            continue;
        }
        if (token.type != Token::Identifier)
            continue;
        if (!macroBody)
            mSeenCode = true;

        const std::string name = token.text;
        size_t previous = PreviousSignificant(tokens, index);
        bool member = previous != std::string::npos && tokens[previous].text == ".";
        if (member)
            continue;

        if (name == "main")
        {
            token.text = "lce_pack_main";
            continue;
        }

        auto rename = renames.find(name);
        if (rename != renames.end())
        {
            token.text = rename->second;
            if (rename->second.compare(0, 4, "lce_") == 0)
                Use(rename->second);
            continue;
        }

        if (name == "texture")
        {
            size_t next = NextSignificant(tokens, index);
            if (mRenameTexture && (next == std::string::npos || tokens[next].text != "("))
                token.text = "gtexture";
            continue;
        }

        if (mVersion < 130 && Contains(kReservedSince130, name)
            && !(mGpuShader4 && Contains(kGpuShader4Keywords, name)))
        {
            size_t next = NextSignificant(tokens, index);
            bool keywordUse = Contains(kLenientKeywords, name) && next != std::string::npos
                              && (tokens[next].type == Token::Identifier || tokens[next].text == "(");
            if (!keywordUse)
                token.text = "lce_kw_" + name;
            continue;
        }

        if (Contains(kUniformBuiltins, name))
        {
            token.text = ReplaceGlPrefix(name);
            Use(token.text);
            continue;
        }

        if (name == "gl_TexCoord")
        {
            token.text = "lce_TexCoord";
            Use(token.text);
            size_t open = NextSignificant(tokens, index);
            size_t value = open == std::string::npos ? open : NextSignificant(tokens, open);
            size_t close = value == std::string::npos ? value : NextSignificant(tokens, value);
            int count = 8;
            if (close != std::string::npos && tokens[open].text == "[" && tokens[value].type == Token::Number
                && tokens[close].text == "]")
                count = std::atoi(tokens[value].text.c_str()) + 1;
            if (count > mUsage.texCoordCount)
                mUsage.texCoordCount = count > 8 ? 8 : count;
            continue;
        }

        if (name == "gl_FogFragCoord")
        {
            token.text = "lce_FogFragCoord";
            Use(token.text);
            continue;
        }

        if (mStage == ShaderStage_Vertex)
        {
            if (name == "attribute")
            {
                token.text = "in";
                continue;
            }
            if (name == "varying")
            {
                token.text = "out";
                continue;
            }
            if (name == "gl_FrontColor" || name == "gl_BackColor")
            {
                token.text = "lce_FrontColor";
                Use(token.text);
                continue;
            }
            if (name == "gl_FrontSecondaryColor" || name == "gl_BackSecondaryColor")
            {
                token.text = "lce_FrontSecondaryColor";
                Use(token.text);
                continue;
            }
            if (Contains(kVertexInputs, name))
            {
                token.text = ReplaceGlPrefix(name);
                Use(token.text);
                continue;
            }
            if (name == "ftransform")
            {
                token.text = "lce_ftransform";
                Use(token.text);
                continue;
            }
            if (Contains(kPackAttributes, name) && !macroBody)
            {
                size_t type = previous;
                size_t qualifier = type == std::string::npos ? type : PreviousSignificant(tokens, type);
                size_t next = NextSignificant(tokens, index);
                if (qualifier != std::string::npos && tokens[type].type == Token::Identifier
                    && (tokens[qualifier].text == "in" || tokens[qualifier].text == "attribute")
                    && next != std::string::npos && tokens[next].text == ";")
                {
                    tokens[qualifier].text.clear();
                    size_t before = PreviousSignificant(tokens, qualifier);
                    if (before != std::string::npos && tokens[before].text == ")")
                    {
                        size_t open = before;
                        while (open != std::string::npos && tokens[open].text != "(")
                            open = PreviousSignificant(tokens, open);
                        size_t layout = open == std::string::npos ? open : PreviousSignificant(tokens, open);
                        if (layout != std::string::npos && tokens[layout].text == "layout")
                        {
                            for (size_t clear = layout; clear <= before; ++clear)
                                tokens[clear].text.clear();
                        }
                    }
                    bool known = false;
                    for (const auto& attribute : mUsage.attributes)
                        known = known || attribute.second == name;
                    if (!known)
                        mUsage.attributes.push_back(std::make_pair(tokens[type].text, name));
                }
                continue;
            }
        }
        else if (mStage == ShaderStage_Fragment)
        {
            if (name == "varying")
            {
                token.text = "in";
                continue;
            }
            if (name == "gl_Color")
            {
                token.text = "lce_FrontColor";
                Use(token.text);
                continue;
            }
            if (name == "gl_SecondaryColor")
            {
                token.text = "lce_FrontSecondaryColor";
                Use(token.text);
                continue;
            }
            if (name == "gl_FragColor")
            {
                token.text = "lce_FragData0";
                mUsage.fragData.insert(0);
                mUsage.output0 = "lce_FragData0";
                if (mUsage.outputLocations < 1)
                    mUsage.outputLocations = 1;
                continue;
            }
            if (name == "gl_FragData")
            {
                size_t open = NextSignificant(tokens, index);
                size_t value = open == std::string::npos ? open : NextSignificant(tokens, open);
                size_t close = value == std::string::npos ? value : NextSignificant(tokens, value);
                if (close == std::string::npos || tokens[open].text != "[" || tokens[value].type != Token::Number
                    || tokens[close].text != "]")
                {
                    error = "gl_FragData must be indexed with a constant number";
                    return false;
                }
                int buffer = std::atoi(tokens[value].text.c_str());
                if (buffer < 0 || buffer > 15)
                {
                    error = "gl_FragData index " + tokens[value].text + " is out of range";
                    return false;
                }
                token.text = "lce_FragData" + std::to_string(buffer);
                for (size_t clear = index + 1; clear <= close; ++clear)
                {
                    if (tokens[clear].type != Token::Newline)
                        tokens[clear].text.clear();
                }
                mUsage.fragData.insert(buffer);
                if (buffer + 1 > mUsage.outputLocations)
                    mUsage.outputLocations = buffer + 1;
                if (buffer == 0)
                    mUsage.output0 = "lce_FragData0";
                continue;
            }
            if (name == "out" && !macroBody)
            {
                size_t scan = previous;
                while (scan != std::string::npos && IsOutputQualifier(tokens[scan].text))
                    scan = PreviousSignificant(tokens, scan);
                int location = -1;
                if (scan != std::string::npos && tokens[scan].text == ")")
                {
                    location = LayoutLocation(tokens, scan);
                    if (location < 0)
                        continue;
                }
                else
                {
                    bool statementStart = scan == std::string::npos || tokens[scan].text == ";"
                                          || tokens[scan].text == "}" || tokens[scan].type == Token::Directive;
                    if (!statementStart)
                        continue;
                    while (mExplicitOutputs.count(mUserOutputs) != 0)
                        ++mUserOutputs;
                    location = mUserOutputs++;
                    token.text = "layout(location = " + std::to_string(location) + ") out";
                }
                size_t type = NextSignificant(tokens, index);
                while (type != std::string::npos
                       && (tokens[type].text == "highp" || tokens[type].text == "mediump" || tokens[type].text == "lowp"))
                    type = NextSignificant(tokens, type);
                size_t variable = type == std::string::npos ? type : NextSignificant(tokens, type);
                if (location == 0 && variable != std::string::npos && tokens[variable].type == Token::Identifier)
                    mUsage.output0 = tokens[variable].text;
                if (location + 1 > mUsage.outputLocations)
                    mUsage.outputLocations = location + 1;
                continue;
            }
        }
    }
    return true;
}

std::string GlslTranslator::StripUnusedFunctions(const std::string& source)
{
    std::vector<Token> tokens;
    Tokenize(source, true, tokens);
    auto next = [&](size_t index) {
        for (++index; index < tokens.size(); ++index)
        {
            Token::Type type = tokens[index].type;
            if (type != Token::Space && type != Token::Newline && type != Token::Comment && type != Token::Directive)
                return index;
        }
        return tokens.size();
    };
    auto match = [&](size_t index, const char* open, const char* close) {
        int depth = 0;
        for (; index < tokens.size(); ++index)
        {
            if (tokens[index].type != Token::Punct)
                continue;
            if (tokens[index].text == open)
                ++depth;
            else if (tokens[index].text == close && --depth == 0)
                return index;
        }
        return tokens.size();
    };

    struct Definition
    {
        std::string name;
        size_t start;
        size_t end;
    };
    std::vector<Definition> definitions;
    std::vector<bool> inside(tokens.size(), false);
    size_t statement = 0;
    int depth = 0;
    for (size_t index = 0; index < tokens.size(); ++index)
    {
        const Token& token = tokens[index];
        if (token.type == Token::Directive)
        {
            if (depth == 0)
                statement = index + 1;
            continue;
        }
        if (token.type == Token::Punct)
        {
            if (token.text == "{")
                ++depth;
            else if (token.text == "}" && depth > 0 && --depth == 0)
                statement = index + 1;
            else if (token.text == ";" && depth == 0)
                statement = index + 1;
            continue;
        }
        if (depth != 0 || token.type != Token::Identifier)
            continue;
        size_t open = next(index);
        if (open >= tokens.size() || tokens[open].text != "(")
            continue;
        size_t close = match(open, "(", ")");
        if (close >= tokens.size())
            break;
        size_t body = next(close);
        if (body >= tokens.size() || tokens[body].text != "{")
        {
            index = close;
            continue;
        }
        size_t end = match(body, "{", "}");
        if (end >= tokens.size())
            break;
        definitions.push_back({token.text, statement, end});
        for (size_t covered = statement; covered <= end; ++covered)
            inside[covered] = true;
        index = end;
        statement = end + 1;
    }
    if (definitions.empty())
        return source;

    std::multimap<std::string, size_t> byName;
    for (size_t definition = 0; definition < definitions.size(); ++definition)
        byName.insert(std::make_pair(definitions[definition].name, definition));
    std::set<std::string> reached;
    std::vector<std::string> pending;
    auto reach = [&](const std::string& name) {
        if (byName.count(name) != 0 && reached.insert(name).second)
            pending.push_back(name);
    };
    auto reachToken = [&](const Token& token) {
        if (token.type == Token::Identifier)
        {
            reach(token.text);
            return;
        }
        if (token.type != Token::Directive)
            return;
        const std::string& text = token.text;
        for (size_t position = 0; position < text.size();)
        {
            if (!IsIdentifierStart(text[position]))
            {
                ++position;
                continue;
            }
            size_t end = position;
            while (end < text.size() && IsIdentifierChar(text[end]))
                ++end;
            reach(text.substr(position, end - position));
            position = end;
        }
    };
    reach("main");
    for (size_t index = 0; index < tokens.size(); ++index)
    {
        if (!inside[index] || tokens[index].type == Token::Directive)
            reachToken(tokens[index]);
    }
    while (!pending.empty())
    {
        std::string name = pending.back();
        pending.pop_back();
        auto range = byName.equal_range(name);
        for (auto entry = range.first; entry != range.second; ++entry)
        {
            const Definition& definition = definitions[entry->second];
            for (size_t index = definition.start; index <= definition.end; ++index)
                reachToken(tokens[index]);
        }
    }

    bool removed = false;
    for (const Definition& definition : definitions)
    {
        if (reached.count(definition.name) != 0)
            continue;
        removed = true;
        for (size_t index = definition.start; index <= definition.end; ++index)
        {
            if (tokens[index].type != Token::Newline && tokens[index].type != Token::Directive)
                tokens[index].text.clear();
        }
    }
    if (!removed)
        return source;
    std::string out;
    out.reserve(source.size());
    for (const Token& token : tokens)
        out += token.text;
    return out;
}

namespace
{
template <typename Visit> void VisitDeclarations(std::vector<GlslTranslator::Token>& tokens, const char* qualifier,
                                                 Visit visit)
{
    int depth = 0;
    bool statementStart = true;
    size_t layout = std::string::npos;
    for (size_t index = 0; index < tokens.size(); ++index)
    {
        GlslTranslator::Token& token = tokens[index];
        if (token.type == GlslTranslator::Token::Space || token.type == GlslTranslator::Token::Newline
            || token.type == GlslTranslator::Token::Comment || token.type == GlslTranslator::Token::Directive)
            continue;
        if (token.type == GlslTranslator::Token::Punct)
        {
            if (token.text == "{" || token.text == "(")
                ++depth;
            else if ((token.text == "}" || token.text == ")") && depth > 0)
                --depth;
            if (depth == 0 && (token.text == ";" || token.text == "}"))
            {
                statementStart = true;
                layout = std::string::npos;
            }
            continue;
        }
        if (depth != 0 || token.type != GlslTranslator::Token::Identifier)
            continue;
        if (token.text == "layout" && statementStart)
            layout = index;
        if (statementStart && token.text == qualifier)
        {
            std::string name;
            int bracket = 0;
            for (size_t next = index + 1; next < tokens.size(); ++next)
            {
                const GlslTranslator::Token& part = tokens[next];
                if (part.type == GlslTranslator::Token::Punct)
                {
                    if (part.text == "[")
                        ++bracket;
                    else if (part.text == "]")
                        --bracket;
                    else if (bracket == 0 && (part.text == ";" || part.text == ","))
                        break;
                    continue;
                }
                if (part.type == GlslTranslator::Token::Identifier && bracket == 0)
                    name = part.text;
            }
            if (!name.empty())
                visit(index, name, layout == std::string::npos ? index : layout);
        }
        if (token.text != "flat" && token.text != "smooth" && token.text != "noperspective" && token.text != "centroid"
            && token.text != "invariant" && token.text != "layout")
            statementStart = false;
    }
}
}

std::set<std::string> GlslTranslator::DeclaredNames(const std::string& source, const char* qualifier)
{
    std::vector<Token> tokens;
    Tokenize(source, true, tokens);
    std::set<std::string> names;
    VisitDeclarations(tokens, qualifier,
                      [&](size_t, const std::string& name, size_t) { names.insert(name); });
    return names;
}

std::string GlslTranslator::RequalifyDeclarations(const std::string& source, const char* from, const char* to,
                                                  const std::set<std::string>& names)
{
    std::vector<Token> tokens;
    Tokenize(source, true, tokens);
    bool changed = false;
    VisitDeclarations(tokens, from, [&](size_t qualifier, const std::string& name, size_t layout) {
        if (names.count(name) == 0)
            return;
        for (size_t index = layout; index < qualifier; ++index)
            tokens[index].text.clear();
        tokens[qualifier].text = to;
        changed = true;
    });
    if (!changed)
        return source;
    std::string out;
    out.reserve(source.size());
    for (const Token& token : tokens)
        out += token.text;
    return out;
}

std::string GlslTranslator::VersionLine(int version)
{
    if (version >= 150)
        return "#version " + std::to_string(version) + " core";
    return "#version " + std::to_string(version);
}

std::string GlslTranslator::Emit(int version, const std::string& header, const std::string& footer,
                                 const GlslUsage& linked) const
{
    std::string out = VersionLine(version) + "\n";
    for (const std::string& extension : mLeadingExtensions)
        out += extension + "\n";

    out += "#line 1 " + std::to_string(kHeaderSourceIndex) + "\n";

    const char* direction = mStage == ShaderStage_Vertex ? "out" : "in";
    const GlslUsage& varyings = mStage == ShaderStage_Vertex ? linked : mUsage;
    if (mStage != ShaderStage_Geometry)
    {
        if (varyings.builtins.count("lce_FrontColor"))
            out += std::string(direction) + " vec4 lce_FrontColor;\n";
        if (varyings.builtins.count("lce_FrontSecondaryColor"))
            out += std::string(direction) + " vec4 lce_FrontSecondaryColor;\n";
        if (varyings.texCoordCount > 0)
            out += std::string(direction) + " vec4 lce_TexCoord[" + std::to_string(varyings.texCoordCount) + "];\n";
        if (varyings.builtins.count("lce_FogFragCoord"))
            out += std::string(direction) + " float lce_FogFragCoord;\n";
    }

    for (int buffer : mUsage.fragData)
        out += "layout(location = " + std::to_string(buffer) + ") out vec4 lce_FragData" + std::to_string(buffer)
               + ";\n";

    if (mUsage.builtins.count("lce_shadow2D") || mUsage.builtins.count("lce_shadow2DLod")
        || mUsage.builtins.count("lce_shadow2DProj") || mUsage.builtins.count("lce_shadow2DGrad"))
    {
        out += "vec4 lce_shadow2D(sampler2DShadow s, vec3 c) { return vec4(texture(s, c)); }\n"
               "vec4 lce_shadow2D(sampler2D s, vec3 c) { return vec4(step(c.z, texture(s, c.xy).r)); }\n"
               "vec4 lce_shadow2DLod(sampler2DShadow s, vec3 c, float l) { return vec4(textureLod(s, c, l)); }\n"
               "vec4 lce_shadow2DLod(sampler2D s, vec3 c, float l) { return vec4(step(c.z, textureLod(s, c.xy, "
               "l).r)); }\n"
               "vec4 lce_shadow2DProj(sampler2DShadow s, vec4 c) { return vec4(textureProj(s, c)); }\n"
               "vec4 lce_shadow2DGrad(sampler2DShadow s, vec3 c, vec2 x, vec2 y) { return vec4(textureGrad(s, c, x, "
               "y)); }\n";
        if (mStage == ShaderStage_Fragment)
            out += "vec4 lce_shadow2D(sampler2DShadow s, vec3 c, float b) { return vec4(texture(s, c, b)); }\n"
                   "vec4 lce_shadow2D(sampler2D s, vec3 c, float b) { return vec4(step(c.z, texture(s, c.xy, "
                   "b).r)); }\n";
    }

    out += header;
    out += "#line 1 0\n";
    for (const Token& token : mTokens)
        out += token.text;
    out += "\n#line 1 " + std::to_string(kFooterSourceIndex) + "\n";

    if (mStage == ShaderStage_Vertex)
    {
        out += "void lce_AssignAttributes()\n{\n";
        for (const auto& attribute : mUsage.attributes)
        {
            std::string source = attribute.second == "mc_Entity"        ? "lce_EntityData"
                                 : attribute.second == "mc_midTexCoord" ? "lce_MidTexCoordData"
                                 : attribute.second == "at_tangent"     ? "lce_TangentData"
                                 : attribute.second == "at_velocity"    ? "lce_VelocityData"
                                                                        : "lce_MidBlockData";
            out += "    " + attribute.second + " = " + attribute.first + "(" + source + ");\n";
        }
        out += "}\n";
    }

    out += footer;
    return out;
}
