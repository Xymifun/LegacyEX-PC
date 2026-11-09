#include "platform/wiiu/MiiIcon/ExtShader.hpp"

#include "platform/wiiu/MiiIcon/MiiIconAssert.hpp"
#include "platform/wiiu/MiiIcon/PlainShader.hpp"

#include <nn/types.h>

#include <math.h>

namespace
{

ALIGNVAR(4)
const u8 EXT_SHADER_DATA[] = {
#include "platform/wiiu/shaders/MiiIconExtShader.inc"
};

PlainShader s_ExtShader;

}

float RadianToDegree(float radian)
{
    return radian * 180.0f / 3.14159;
}

void GetIconMatrix(Mat44* projection, Mat34* view, unsigned short width, unsigned short height)
{
    MII_ICON_ASSERT_MESSAGE(width != 0, "Width must not be zero.");
    MII_ICON_ASSERT_MESSAGE(height != 0, "Height must not be zero.");
    MII_ICON_ASSERT_POINTER(projection);
    MII_ICON_ASSERT_POINTER(view);

    float aspect = float(width) / float(height);
    float scale;
    if (width < height)
        scale = aspect;
    else
        scale = 1.0f;

    float fovy = RadianToDegree(atan2f(43.2f / scale, 500.0f)) * 2.0f;
    MTXPerspective(projection->mtx, fovy, aspect, 500.0f, 700.0f);

    Vec cameraPosition = {0.0f, 34.5f, 600.0f};
    static const Vec cameraUp = {0.0f, 1.0f, 0.0f};
    static const Vec target = {0.0f, 34.5f, 0.0f};
    MTXLookAt(view->mtx, &cameraPosition, &cameraUp, &target);
}

void InitExtShader()
{
    InitPlainShader(&s_ExtShader, EXT_SHADER_DATA);
}

void SetupExtShaderForCreateModel()
{
    BindPlainShader(&s_ExtShader);
}

void SetupExtShaderForDrawTexture(unsigned short width, unsigned short height)
{
    Mat44 projection;
    Mat34 view;
    GetIconMatrix(&projection, &view, width, height);

    Mat44 modelViewProjection;
    MAT34To44(&view, &modelViewProjection);
    MAT44Concat(&projection, &modelViewProjection, &modelViewProjection);

    BindPlainShaderForIcon(&s_ExtShader);
    SetMatrixPlainShader(&s_ExtShader, &modelViewProjection);
}

void FinalizeExtShader()
{
    DestroyPlainShader(&s_ExtShader);
}
