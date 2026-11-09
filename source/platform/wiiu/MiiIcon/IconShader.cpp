#include "platform/wiiu/MiiIcon/IconShader.hpp"

#include "platform/wiiu/MiiIcon/DefaultShader.hpp"
#include "platform/wiiu/MiiIcon/ExtShader.hpp"
#include "platform/wiiu/MiiIcon/MiiIconAssert.hpp"

namespace
{

IconShaderType s_IconShaderType;

}

void InitIconShader(IconShaderType type)
{
    s_IconShaderType = type;

    switch (type)
    {
    case ICON_SHADER_TYPE_DEFAULT:
        InitDefaultShader();
        break;
    case ICON_SHADER_TYPE_PLAIN:
        InitExtShader();
        break;
    default:
        break;
    }
}

void SetupIconShaderForCreateModel()
{
    switch (s_IconShaderType)
    {
    case ICON_SHADER_TYPE_DEFAULT:
        SetupDefaultShaderForCreateModel();
        break;
    case ICON_SHADER_TYPE_PLAIN:
        SetupExtShaderForCreateModel();
        break;
    default:
        break;
    }
}

void SetupIconShaderForDrawTexture(unsigned short width, unsigned short height)
{
    switch (s_IconShaderType)
    {
    case ICON_SHADER_TYPE_DEFAULT:
        SetupDefaultShaderForDrawTexture(width, height);
        break;
    case ICON_SHADER_TYPE_PLAIN:
        SetupExtShaderForDrawTexture(width, height);
        break;
    default:
        MII_ICON_INVALID_CONDITION(s_IconShaderType);
        break;
    }
}

void FinalizeIconShader()
{
    switch (s_IconShaderType)
    {
    case ICON_SHADER_TYPE_DEFAULT:
        FinalizeDefaultShader();
        break;
    case ICON_SHADER_TYPE_PLAIN:
        FinalizeExtShader();
        break;
    default:
        MII_ICON_INVALID_CONDITION(s_IconShaderType);
        break;
    }
}
