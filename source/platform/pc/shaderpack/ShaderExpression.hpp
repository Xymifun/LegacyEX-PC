// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderExpression.hpp
#ifndef PLATFORM_PC_SHADERPACK_SHADEREXPRESSION_HPP
#define PLATFORM_PC_SHADERPACK_SHADEREXPRESSION_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

struct ShaderValue
{
    int size = 1;
    float v[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    static ShaderValue Scalar(float value)
    {
        ShaderValue result;
        result.v[0] = value;
        return result;
    }
};

class ShaderExpressionContext
{
public:
    int Define(const std::string& name, int size);
    int Find(const std::string& name) const;
    ShaderValue& Slot(int index) { return mSlots[index]; }
    const ShaderValue& Slot(int index) const { return mSlots[index]; }
    void Set(int index, float value) { mSlots[index].v[0] = value; }
    void Set(int index, float x, float y, float z, float w = 0.0f);
    void DefineConstant(const std::string& name, float value);
    bool FindConstant(const std::string& name, float& value) const;

    float mFrameSeconds = 0.0f;

private:
    std::vector<ShaderValue> mSlots;
    std::map<std::string, int> mNames;
    std::map<std::string, float> mConstants;
};

class ShaderExpression
{
public:
    bool Parse(const std::string& text, const ShaderExpressionContext& context, std::string& error);
    ShaderValue Evaluate(const ShaderExpressionContext& context);

private:
    struct Node
    {
        enum Kind
        {
            Constant,
            Slot,
            Component,
            Unary,
            Binary,
            Function
        };

        Kind kind = Constant;
        float constant = 0.0f;
        int slot = -1;
        int component = 0;
        std::string op;
        std::vector<int> children;
        float state = 0.0f;
        bool hasState = false;
    };

    int ParseOr();
    int ParseAnd();
    int ParseComparison();
    int ParseAdditive();
    int ParseMultiplicative();
    int ParseUnary();
    int ParsePostfix();
    int ParsePrimary();
    int Add(const Node& node);
    void SkipSpace();
    bool Accept(const char* text);
    ShaderValue EvaluateNode(int index, const ShaderExpressionContext& context);

    std::vector<Node> mNodes;
    int mRoot = -1;
    std::string mText;
    size_t mPosition = 0;
    std::string mError;
    const ShaderExpressionContext* mContext = nullptr;
};

#endif
