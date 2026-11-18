// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderExpression.cpp
#include "platform/pc/shaderpack/ShaderExpression.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <random>

namespace
{
const float kPi = 3.14159265358979f;

bool IsNameStart(char character)
{
    return std::isalpha(static_cast<unsigned char>(character)) || character == '_';
}

bool IsNameChar(char character)
{
    return std::isalnum(static_cast<unsigned char>(character)) || character == '_';
}

int ComponentIndex(const std::string& name)
{
    if (name == "x" || name == "r" || name == "s")
        return 0;
    if (name == "y" || name == "g" || name == "t")
        return 1;
    if (name == "z" || name == "b" || name == "p")
        return 2;
    if (name == "w" || name == "a" || name == "q")
        return 3;
    return -1;
}

struct FunctionInfo
{
    const char* name;
    int minArgs;
    int maxArgs;
};

const FunctionInfo kFunctions[] = {
    {"sin", 1, 1},    {"cos", 1, 1},     {"asin", 1, 1},    {"acos", 1, 1},   {"tan", 1, 1},
    {"atan", 1, 1},   {"atan2", 2, 2},   {"torad", 1, 1},   {"todeg", 1, 1},  {"min", 1, 64},
    {"max", 1, 64},   {"clamp", 3, 3},   {"abs", 1, 1},     {"floor", 1, 1},  {"ceil", 1, 1},
    {"exp", 1, 1},    {"exp2", 1, 1},    {"frac", 1, 1},    {"log", 1, 1},    {"log2", 1, 1},
    {"pow", 2, 2},    {"random", 0, 0},  {"round", 1, 1},   {"signum", 1, 1}, {"sqrt", 1, 1},
    {"fmod", 2, 2},   {"if", 3, 63},     {"smooth", 1, 4},  {"between", 3, 3}, {"equals", 3, 3},
    {"in", 2, 64},    {"vec2", 2, 2},    {"vec3", 3, 3},    {"vec4", 4, 4},
};

const FunctionInfo* FindFunction(const std::string& name)
{
    for (const FunctionInfo& info : kFunctions)
    {
        if (name == info.name)
            return &info;
    }
    return nullptr;
}

float RandomFloat()
{
    static std::mt19937 generator(12345u);
    return std::uniform_real_distribution<float>(0.0f, 1.0f)(generator);
}
}

int ShaderExpressionContext::Define(const std::string& name, int size)
{
    auto found = mNames.find(name);
    if (found != mNames.end())
    {
        mSlots[found->second].size = size;
        return found->second;
    }
    ShaderValue value;
    value.size = size;
    mSlots.push_back(value);
    int index = static_cast<int>(mSlots.size()) - 1;
    mNames[name] = index;
    return index;
}

int ShaderExpressionContext::Find(const std::string& name) const
{
    auto found = mNames.find(name);
    return found == mNames.end() ? -1 : found->second;
}

void ShaderExpressionContext::Set(int index, float x, float y, float z, float w)
{
    ShaderValue& value = mSlots[index];
    value.v[0] = x;
    value.v[1] = y;
    value.v[2] = z;
    value.v[3] = w;
}

void ShaderExpressionContext::DefineConstant(const std::string& name, float value)
{
    mConstants[name] = value;
}

bool ShaderExpressionContext::FindConstant(const std::string& name, float& value) const
{
    auto found = mConstants.find(name);
    if (found == mConstants.end())
        return false;
    value = found->second;
    return true;
}

int ShaderExpression::Add(const Node& node)
{
    mNodes.push_back(node);
    return static_cast<int>(mNodes.size()) - 1;
}

void ShaderExpression::SkipSpace()
{
    while (mPosition < mText.size() && std::isspace(static_cast<unsigned char>(mText[mPosition])))
        ++mPosition;
}

bool ShaderExpression::Accept(const char* text)
{
    SkipSpace();
    size_t length = std::char_traits<char>::length(text);
    if (mText.compare(mPosition, length, text) != 0)
        return false;
    if ((text[0] == '<' || text[0] == '>' || text[0] == '=' || text[0] == '!') && length == 1
        && mPosition + 1 < mText.size() && mText[mPosition + 1] == '=')
        return false;
    if ((text[0] == '&' || text[0] == '|') && length == 1)
        return false;
    mPosition += length;
    return true;
}

bool ShaderExpression::Parse(const std::string& text, const ShaderExpressionContext& context, std::string& error)
{
    mNodes.clear();
    mText = text;
    mPosition = 0;
    mError.clear();
    mContext = &context;
    mRoot = ParseOr();
    SkipSpace();
    if (mError.empty() && mPosition != mText.size())
        mError = "unexpected '" + mText.substr(mPosition) + "'";
    mContext = nullptr;
    if (!mError.empty())
    {
        error = mError;
        mRoot = -1;
        return false;
    }
    return true;
}

int ShaderExpression::ParseOr()
{
    int left = ParseAnd();
    while (mError.empty() && Accept("||"))
    {
        Node node;
        node.kind = Node::Binary;
        node.op = "||";
        node.children = {left, ParseAnd()};
        left = Add(node);
    }
    return left;
}

int ShaderExpression::ParseAnd()
{
    int left = ParseComparison();
    while (mError.empty() && Accept("&&"))
    {
        Node node;
        node.kind = Node::Binary;
        node.op = "&&";
        node.children = {left, ParseComparison()};
        left = Add(node);
    }
    return left;
}

int ShaderExpression::ParseComparison()
{
    int left = ParseAdditive();
    while (mError.empty())
    {
        static const char* const kOperators[] = {"==", "!=", "<=", ">=", "<", ">"};
        const char* matched = nullptr;
        for (const char* op : kOperators)
        {
            if (Accept(op))
            {
                matched = op;
                break;
            }
        }
        if (matched == nullptr)
            break;
        Node node;
        node.kind = Node::Binary;
        node.op = matched;
        node.children = {left, ParseAdditive()};
        left = Add(node);
    }
    return left;
}

int ShaderExpression::ParseAdditive()
{
    int left = ParseMultiplicative();
    while (mError.empty())
    {
        const char* op = Accept("+") ? "+" : Accept("-") ? "-" : nullptr;
        if (op == nullptr)
            break;
        Node node;
        node.kind = Node::Binary;
        node.op = op;
        node.children = {left, ParseMultiplicative()};
        left = Add(node);
    }
    return left;
}

int ShaderExpression::ParseMultiplicative()
{
    int left = ParseUnary();
    while (mError.empty())
    {
        const char* op = Accept("*") ? "*" : Accept("/") ? "/" : Accept("%") ? "%" : nullptr;
        if (op == nullptr)
            break;
        Node node;
        node.kind = Node::Binary;
        node.op = op;
        node.children = {left, ParseUnary()};
        left = Add(node);
    }
    return left;
}

int ShaderExpression::ParseUnary()
{
    if (Accept("!"))
    {
        Node node;
        node.kind = Node::Unary;
        node.op = "!";
        node.children = {ParseUnary()};
        return Add(node);
    }
    if (Accept("-"))
    {
        Node node;
        node.kind = Node::Unary;
        node.op = "-";
        node.children = {ParseUnary()};
        return Add(node);
    }
    if (Accept("+"))
        return ParseUnary();
    return ParsePostfix();
}

int ShaderExpression::ParsePostfix()
{
    int value = ParsePrimary();
    while (mError.empty())
    {
        SkipSpace();
        if (mPosition >= mText.size() || mText[mPosition] != '.')
            break;
        size_t start = ++mPosition;
        while (mPosition < mText.size() && IsNameChar(mText[mPosition]))
            ++mPosition;
        int component = ComponentIndex(mText.substr(start, mPosition - start));
        if (component < 0)
        {
            mError = "unknown component '" + mText.substr(start, mPosition - start) + "'";
            break;
        }
        Node node;
        node.kind = Node::Component;
        node.component = component;
        node.children = {value};
        value = Add(node);
    }
    return value;
}

int ShaderExpression::ParsePrimary()
{
    SkipSpace();
    if (mPosition >= mText.size())
    {
        mError = "unexpected end of expression";
        return -1;
    }

    char character = mText[mPosition];
    if (character == '(')
    {
        ++mPosition;
        int inner = ParseOr();
        if (mError.empty() && !Accept(")"))
            mError = "missing ')'";
        return inner;
    }

    if (std::isdigit(static_cast<unsigned char>(character)) || character == '.')
    {
        const char* begin = mText.c_str() + mPosition;
        char* end = nullptr;
        float number = std::strtof(begin, &end);
        if (end == begin)
        {
            mError = "bad number";
            return -1;
        }
        mPosition += static_cast<size_t>(end - begin);
        if (mPosition < mText.size() && (mText[mPosition] == 'f' || mText[mPosition] == 'F'))
            ++mPosition;
        Node node;
        node.kind = Node::Constant;
        node.constant = number;
        return Add(node);
    }

    if (!IsNameStart(character))
    {
        mError = std::string("unexpected '") + character + "'";
        return -1;
    }

    size_t start = mPosition;
    while (mPosition < mText.size() && IsNameChar(mText[mPosition]))
        ++mPosition;
    std::string name = mText.substr(start, mPosition - start);

    SkipSpace();
    if (mPosition < mText.size() && mText[mPosition] == '(')
    {
        const FunctionInfo* info = FindFunction(name);
        if (info == nullptr)
        {
            mError = "unknown function '" + name + "'";
            return -1;
        }
        ++mPosition;
        Node node;
        node.kind = Node::Function;
        node.op = name;
        if (!Accept(")"))
        {
            do
            {
                node.children.push_back(ParseOr());
            } while (mError.empty() && Accept(","));
            if (mError.empty() && !Accept(")"))
                mError = "missing ')' after arguments of " + name;
        }
        int count = static_cast<int>(node.children.size());
        if (mError.empty() && (count < info->minArgs || count > info->maxArgs))
            mError = "wrong number of arguments for " + name;
        if (mError.empty() && name == "if" && count % 2 == 0)
            mError = "if() needs an odd number of arguments";
        return Add(node);
    }

    Node node;
    if (name == "pi")
    {
        node.constant = kPi;
        return Add(node);
    }
    if (name == "true" || name == "false")
    {
        node.constant = name == "true" ? 1.0f : 0.0f;
        return Add(node);
    }
    float constant = 0.0f;
    if (mContext->FindConstant(name, constant))
    {
        node.constant = constant;
        return Add(node);
    }
    int slot = mContext->Find(name);
    if (slot < 0 && mPosition + 3 < mText.size() && mText[mPosition] == '.'
        && std::isdigit(static_cast<unsigned char>(mText[mPosition + 1])) && mText[mPosition + 2] == '.'
        && std::isdigit(static_cast<unsigned char>(mText[mPosition + 3])))
    {
        slot = mContext->Find(name + mText.substr(mPosition, 4));
        if (slot >= 0)
            mPosition += 4;
    }
    if (slot < 0 && name.compare(0, 6, "BIOME_") == 0)
    {
        node.constant = -1.0f;
        return Add(node);
    }
    if (slot < 0)
    {
        mError = "unknown name '" + name + "'";
        return -1;
    }
    node.kind = Node::Slot;
    node.slot = slot;
    return Add(node);
}

ShaderValue ShaderExpression::Evaluate(const ShaderExpressionContext& context)
{
    if (mRoot < 0)
        return ShaderValue();
    return EvaluateNode(mRoot, context);
}

ShaderValue ShaderExpression::EvaluateNode(int index, const ShaderExpressionContext& context)
{
    Node& node = mNodes[index];
    switch (node.kind)
    {
    case Node::Constant:
        return ShaderValue::Scalar(node.constant);
    case Node::Slot:
        return context.Slot(node.slot);
    case Node::Component:
    {
        ShaderValue value = EvaluateNode(node.children[0], context);
        return ShaderValue::Scalar(value.v[node.component]);
    }
    case Node::Unary:
    {
        ShaderValue value = EvaluateNode(node.children[0], context);
        for (int component = 0; component < value.size; ++component)
            value.v[component] = node.op == "!" ? (value.v[component] == 0.0f ? 1.0f : 0.0f) : -value.v[component];
        return value;
    }
    case Node::Binary:
    {
        const std::string& op = node.op;
        if (op == "&&" || op == "||")
        {
            bool left = EvaluateNode(node.children[0], context).v[0] != 0.0f;
            if (op == "&&" && !left)
                return ShaderValue::Scalar(0.0f);
            if (op == "||" && left)
                return ShaderValue::Scalar(1.0f);
            return ShaderValue::Scalar(EvaluateNode(node.children[1], context).v[0] != 0.0f ? 1.0f : 0.0f);
        }
        ShaderValue left = EvaluateNode(node.children[0], context);
        ShaderValue right = EvaluateNode(node.children[1], context);
        ShaderValue result;
        result.size = left.size > right.size ? left.size : right.size;
        for (int component = 0; component < result.size; ++component)
        {
            float a = left.v[left.size == 1 ? 0 : component];
            float b = right.v[right.size == 1 ? 0 : component];
            float value = 0.0f;
            if (op == "+")
                value = a + b;
            else if (op == "-")
                value = a - b;
            else if (op == "*")
                value = a * b;
            else if (op == "/")
                value = b == 0.0f ? 0.0f : a / b;
            else if (op == "%")
                value = b == 0.0f ? 0.0f : std::fmod(a, b);
            else if (op == "==")
                value = a == b ? 1.0f : 0.0f;
            else if (op == "!=")
                value = a != b ? 1.0f : 0.0f;
            else if (op == "<")
                value = a < b ? 1.0f : 0.0f;
            else if (op == ">")
                value = a > b ? 1.0f : 0.0f;
            else if (op == "<=")
                value = a <= b ? 1.0f : 0.0f;
            else if (op == ">=")
                value = a >= b ? 1.0f : 0.0f;
            result.v[component] = value;
        }
        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
        {
            bool all = true;
            for (int component = 0; component < result.size; ++component)
                all = all && result.v[component] != 0.0f;
            return ShaderValue::Scalar(all ? 1.0f : 0.0f);
        }
        return result;
    }
    case Node::Function:
        break;
    }

    const std::string& name = node.op;
    const std::vector<int>& children = node.children;
    auto arg = [&](size_t which) { return EvaluateNode(children[which], context).v[0]; };

    if (name == "if")
    {
        for (size_t pair = 0; pair + 1 < children.size(); pair += 2)
        {
            if (arg(pair) != 0.0f)
                return EvaluateNode(children[pair + 1], context);
        }
        return EvaluateNode(children.back(), context);
    }
    if (name == "smooth")
    {
        size_t valueIndex = children.size() == 4 ? 1 : 0;
        float value = arg(valueIndex);
        float fadeUp = 1.0f;
        float fadeDown = 1.0f;
        if (children.size() == 4)
        {
            fadeUp = arg(2);
            fadeDown = arg(3);
        }
        else if (children.size() == 3)
        {
            fadeUp = arg(1);
            fadeDown = arg(2);
        }
        else if (children.size() == 2)
        {
            fadeUp = arg(1);
            fadeDown = fadeUp;
        }
        if (!node.hasState)
        {
            node.state = value;
            node.hasState = true;
        }
        else
        {
            float fade = value > node.state ? fadeUp : fadeDown;
            if (fade <= 0.0f)
                node.state = value;
            else
                node.state = value + (node.state - value) * std::pow(0.01f, context.mFrameSeconds / fade);
        }
        return ShaderValue::Scalar(node.state);
    }
    if (name == "vec2" || name == "vec3" || name == "vec4")
    {
        ShaderValue result;
        result.size = static_cast<int>(children.size());
        for (size_t component = 0; component < children.size(); ++component)
            result.v[component] = arg(component);
        return result;
    }
    if (name == "min" || name == "max")
    {
        float result = arg(0);
        for (size_t which = 1; which < children.size(); ++which)
        {
            float value = arg(which);
            result = name == "min" ? (value < result ? value : result) : (value > result ? value : result);
        }
        return ShaderValue::Scalar(result);
    }
    if (name == "in")
    {
        float value = arg(0);
        for (size_t which = 1; which < children.size(); ++which)
        {
            if (arg(which) == value)
                return ShaderValue::Scalar(1.0f);
        }
        return ShaderValue::Scalar(0.0f);
    }
    if (name == "random")
        return ShaderValue::Scalar(RandomFloat());

    float a = children.empty() ? 0.0f : arg(0);
    float result = 0.0f;
    if (name == "sin")
        result = std::sin(a);
    else if (name == "cos")
        result = std::cos(a);
    else if (name == "asin")
        result = std::asin(a);
    else if (name == "acos")
        result = std::acos(a);
    else if (name == "tan")
        result = std::tan(a);
    else if (name == "atan")
        result = std::atan(a);
    else if (name == "atan2")
        result = std::atan2(a, arg(1));
    else if (name == "torad")
        result = a * kPi / 180.0f;
    else if (name == "todeg")
        result = a * 180.0f / kPi;
    else if (name == "clamp")
    {
        float low = arg(1);
        float high = arg(2);
        result = a < low ? low : (a > high ? high : a);
    }
    else if (name == "abs")
        result = std::fabs(a);
    else if (name == "floor")
        result = std::floor(a);
    else if (name == "ceil")
        result = std::ceil(a);
    else if (name == "exp")
        result = std::exp(a);
    else if (name == "exp2")
        result = std::exp2(a);
    else if (name == "frac")
        result = a - std::floor(a);
    else if (name == "log")
        result = std::log(a);
    else if (name == "log2")
        result = std::log2(a);
    else if (name == "pow")
        result = std::pow(a, arg(1));
    else if (name == "round")
        result = std::round(a);
    else if (name == "signum")
        result = a > 0.0f ? 1.0f : (a < 0.0f ? -1.0f : 0.0f);
    else if (name == "sqrt")
        result = std::sqrt(a);
    else if (name == "fmod")
    {
        float b = arg(1);
        result = b == 0.0f ? 0.0f : a - b * std::floor(a / b);
    }
    else if (name == "between")
        result = a >= arg(1) && a <= arg(2) ? 1.0f : 0.0f;
    else if (name == "equals")
        result = std::fabs(a - arg(1)) <= arg(2) ? 1.0f : 0.0f;
    return ShaderValue::Scalar(result);
}
