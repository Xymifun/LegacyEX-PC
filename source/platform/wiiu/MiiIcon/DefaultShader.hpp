#ifndef PLATFORM_WIIU_MIIICON_DEFAULTSHADER_HPP
#define PLATFORM_WIIU_MIIICON_DEFAULTSHADER_HPP

#include <nn/ffl/FFLResult.h>

FFLResult InitDefaultShader();
void SetupDefaultShaderForCreateModel();
void SetupDefaultShaderForDrawTexture(unsigned short width, unsigned short height);
void FinalizeDefaultShader();

#endif
