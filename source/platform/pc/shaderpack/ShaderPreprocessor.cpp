// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPreprocessor.cpp
#include "platform/pc/shaderpack/ShaderPreprocessor.hpp"

#include <cctype>
#include <cstdlib>
#include <sstream>
#include <vector>

namespace
{
class ExpressionParser
{
public:
    ExpressionParser(const std::string& text, const ShaderMacros& macros, int depth)
        : mText(text), mMacros(macros), mDepth(depth), mPosition(0)
    {
    }

    double Parse()
    {
        double value = ParseConditional();
        return value;
    }

private:
    void SkipSpace()
    {
        while (mPosition < mText.size() && std::isspace(static_cast<unsigned char>(mText[mPosition])))
            ++mPosition;
    }

    bool Match(const char* token)
    {
        SkipSpace();
        size_t length = std::char_traits<char>::length(token);
        if (mText.compare(mPosition, length, token) != 0)
            return false;
        mPosition += length;
        return true;
    }

    bool Peek(const char* token)
    {
        SkipSpace();
        return mText.compare(mPosition, std::char_traits<char>::length(token), token) == 0;
    }

    double ParseConditional()
    {
        double condition = ParseOr();
        if (Match("?"))
        {
            double whenTrue = ParseConditional();
            Match(":");
            double whenFalse = ParseConditional();
            return condition != 0.0 ? whenTrue : whenFalse;
        }
        return condition;
    }

    double ParseOr()
    {
        double value = ParseAnd();
        while (Match("||"))
        {
            double right = ParseAnd();
            value = (value != 0.0 || right != 0.0) ? 1.0 : 0.0;
        }
        return value;
    }

    double ParseAnd()
    {
        double value = ParseEquality();
        while (Match("&&"))
        {
            double right = ParseEquality();
            value = (value != 0.0 && right != 0.0) ? 1.0 : 0.0;
        }
        return value;
    }

    double ParseEquality()
    {
        double value = ParseRelational();
        for (;;)
        {
            if (Match("=="))
                value = value == ParseRelational() ? 1.0 : 0.0;
            else if (Match("!="))
                value = value != ParseRelational() ? 1.0 : 0.0;
            else
                return value;
        }
    }

    double ParseRelational()
    {
        double value = ParseAdditive();
        for (;;)
        {
            if (Match("<="))
                value = value <= ParseAdditive() ? 1.0 : 0.0;
            else if (Match(">="))
                value = value >= ParseAdditive() ? 1.0 : 0.0;
            else if (Peek("<") && !Peek("<<") && Match("<"))
                value = value < ParseAdditive() ? 1.0 : 0.0;
            else if (Peek(">") && !Peek(">>") && Match(">"))
                value = value > ParseAdditive() ? 1.0 : 0.0;
            else
                return value;
        }
    }

    double ParseAdditive()
    {
        double value = ParseMultiplicative();
        for (;;)
        {
            if (Match("+"))
                value += ParseMultiplicative();
            else if (Peek("-") && Match("-"))
                value -= ParseMultiplicative();
            else
                return value;
        }
    }

    double ParseMultiplicative()
    {
        double value = ParseUnary();
        for (;;)
        {
            if (Match("*"))
            {
                value *= ParseUnary();
            }
            else if (Match("/"))
            {
                double divisor = ParseUnary();
                value = divisor != 0.0 ? value / divisor : 0.0;
            }
            else if (Match("%"))
            {
                long long divisor = static_cast<long long>(ParseUnary());
                value = divisor != 0 ? static_cast<double>(static_cast<long long>(value) % divisor) : 0.0;
            }
            else
            {
                return value;
            }
        }
    }

    double ParseUnary()
    {
        if (Match("!"))
            return ParseUnary() == 0.0 ? 1.0 : 0.0;
        if (Match("-"))
            return -ParseUnary();
        if (Match("+"))
            return ParseUnary();
        return ParsePrimary();
    }

    double ParsePrimary()
    {
        SkipSpace();
        if (Match("("))
        {
            double value = ParseConditional();
            Match(")");
            return value;
        }
        if (mPosition >= mText.size())
            return 0.0;

        char character = mText[mPosition];
        if (std::isdigit(static_cast<unsigned char>(character)) || character == '.')
        {
            const char* begin = mText.c_str() + mPosition;
            char* end = nullptr;
            double value = std::strtod(begin, &end);
            mPosition += static_cast<size_t>(end - begin);
            while (mPosition < mText.size() && std::isalpha(static_cast<unsigned char>(mText[mPosition])))
                ++mPosition;
            return value;
        }
        if (std::isalpha(static_cast<unsigned char>(character)) || character == '_')
        {
            std::string name = ReadIdentifier();
            if (name == "defined")
            {
                bool parenthesised = Match("(");
                SkipSpace();
                std::string target = ReadIdentifier();
                if (parenthesised)
                    Match(")");
                return mMacros.count(target) != 0 ? 1.0 : 0.0;
            }
            if (name == "true")
                return 1.0;
            if (name == "false")
                return 0.0;
            auto found = mMacros.find(name);
            if (found == mMacros.end() || found->second.empty() || mDepth > 16)
                return 0.0;
            return ExpressionParser(found->second, mMacros, mDepth + 1).Parse();
        }
        ++mPosition;
        return 0.0;
    }

    std::string ReadIdentifier()
    {
        size_t start = mPosition;
        while (mPosition < mText.size() &&
               (std::isalnum(static_cast<unsigned char>(mText[mPosition])) || mText[mPosition] == '_'))
            ++mPosition;
        return mText.substr(start, mPosition - start);
    }

    const std::string& mText;
    const ShaderMacros& mMacros;
    int mDepth;
    size_t mPosition;
};

std::string Trim(const std::string& text)
{
    size_t start = text.find_first_not_of(" \t\r");
    if (start == std::string::npos)
        return std::string();
    size_t end = text.find_last_not_of(" \t\r");
    return text.substr(start, end - start + 1);
}

std::string StripLineComment(const std::string& text)
{
    size_t comment = text.find("//");
    return comment == std::string::npos ? text : text.substr(0, comment);
}

struct Branch
{
    bool parentActive;
    bool taken;
    bool active;
};
}

double ShaderEvaluateExpression(const std::string& expression, const ShaderMacros& macros)
{
    return ExpressionParser(expression, macros, 0).Parse();
}

std::string ShaderPreprocessConditionals(const std::string& text, ShaderMacros macros, bool keepDefines)
{
    std::string output;
    std::vector<Branch> branches;
    long lineNumber = 0;
    std::string lineFile;
    size_t start = 0;
    // the lines a directive continued onto (a backslash at the end of a line), kept as empty lines after it so the
    // lines below keep their numbers
    size_t continuedLines = 0;
    while (start < text.size())
    {
        output.append(continuedLines, '\n');
        continuedLines = 0;
        size_t end = text.find('\n', start);
        if (end == std::string::npos)
            end = text.size();
        std::string line = text.substr(start, end - start);
        start = end + 1;
        long currentLine = ++lineNumber;

        std::string trimmed = Trim(line);
        if (!trimmed.empty() && trimmed[0] == '#')
        {
            // a directive over several lines, as in Photon's "#if defined A \" + "    && defined B": one line
            for (;;)
            {
                size_t last = line.find_last_not_of(" \t\r");
                if (last == std::string::npos || line[last] != '\\' || start >= text.size())
                    break;
                end = text.find('\n', start);
                if (end == std::string::npos)
                    end = text.size();
                line = line.substr(0, last) + " " + text.substr(start, end - start);
                start = end + 1;
                ++lineNumber;
                ++continuedLines;
            }
            trimmed = Trim(line);
        }
        bool active = branches.empty() || branches.back().active;
        if (trimmed.size() > 1 && trimmed[0] == '#')
        {
            std::string body = Trim(trimmed.substr(1));
            size_t split = body.find_first_of(" \t(");
            std::string directive = split == std::string::npos ? body : body.substr(0, split);
            std::string argument = split == std::string::npos ? std::string() : Trim(StripLineComment(body.substr(split)));

            if (directive == "ifdef" || directive == "ifndef" || directive == "if")
            {
                bool condition;
                if (directive == "if")
                    condition = active && ShaderEvaluateExpression(argument, macros) != 0.0;
                else
                    condition = active && (macros.count(Trim(argument)) != 0) == (directive == "ifdef");
                branches.push_back(Branch{active, condition, condition});
                output += '\n';
                continue;
            }
            if (directive == "elif" && !branches.empty())
            {
                Branch& branch = branches.back();
                bool condition = branch.parentActive && !branch.taken &&
                                 ShaderEvaluateExpression(argument, macros) != 0.0;
                branch.active = condition;
                branch.taken = branch.taken || condition;
                output += '\n';
                continue;
            }
            if (directive == "else" && !branches.empty())
            {
                Branch& branch = branches.back();
                branch.active = branch.parentActive && !branch.taken;
                branch.taken = true;
                output += '\n';
                continue;
            }
            if (directive == "endif" && !branches.empty())
            {
                branches.pop_back();
                output += '\n';
                continue;
            }
            if (directive == "define" || directive == "undef")
            {
                if (active)
                {
                    std::string definition = Trim(argument);
                    size_t nameEnd = definition.find_first_of(" \t(");
                    std::string name = nameEnd == std::string::npos ? definition : definition.substr(0, nameEnd);
                    std::string value = nameEnd == std::string::npos ? std::string() : Trim(definition.substr(nameEnd));
                    bool redefined = false;
                    if (directive == "undef")
                        macros.erase(name);
                    else if (!name.empty())
                    {
                        auto existing = macros.find(name);
                        redefined = existing != macros.end() && existing->second != value;
                        macros[name] = value;
                    }
                    if (keepDefines && redefined && !lineFile.empty())
                        output += "#undef " + name + "\n#line " + std::to_string(currentLine) + " " + lineFile + "\n";
                    if (keepDefines)
                        output += line;
                }
                output += '\n';
                continue;
            }
            if (directive == "line")
            {
                std::istringstream parts(argument);
                long number = 0;
                std::string file;
                if (parts >> number)
                {
                    lineNumber = number - 1;
                    if (parts >> file)
                        lineFile = file;
                }
            }
            if (directive == "line" && keepDefines)
            {
                output += line;
                output += '\n';
                continue;
            }
        }
        if (active)
            output += line;
        output += '\n';
    }
    output.append(continuedLines, '\n');
    return output;
}
