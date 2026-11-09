#ifndef PLATFORM_WIIU_MIIICON_EXTSHADER_HPP
#define PLATFORM_WIIU_MIIICON_EXTSHADER_HPP

#include <cafe/mat.h>

float RadianToDegree(float radian);
void GetIconMatrix(Mat44* projection, Mat34* view, unsigned short width, unsigned short height);

void InitExtShader();
void SetupExtShaderForCreateModel();
void SetupExtShaderForDrawTexture(unsigned short width, unsigned short height);
void FinalizeExtShader();

#endif
