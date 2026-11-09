#ifndef PLATFORM_WIIU_MIIICON_ICONSHADER_HPP
#define PLATFORM_WIIU_MIIICON_ICONSHADER_HPP

enum IconShaderType
{
    ICON_SHADER_TYPE_DEFAULT = 0,
    ICON_SHADER_TYPE_PLAIN = 1,
    ICON_SHADER_TYPE_BEGIN = ICON_SHADER_TYPE_DEFAULT,
    ICON_SHADER_TYPE_END = 2
};

void InitIconShader(IconShaderType type);
void SetupIconShaderForCreateModel();
void SetupIconShaderForDrawTexture(unsigned short width, unsigned short height);
void FinalizeIconShader();

#endif
