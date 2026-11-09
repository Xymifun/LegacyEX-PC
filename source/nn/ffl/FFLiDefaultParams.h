#ifndef FFLI_DEFAULT_PARAMS_H_
#define FFLI_DEFAULT_PARAMS_H_

#include <nn/ffl/FFLColor.h>
#include <nn/ffl/FFLDefaultShader.h>
#include <nn/ffl/FFLVec.h>

class FFLiDefaultParams
{
public:
    struct Light
    {
        bool enable;
        FFLVec3 direction;
        FFLColor ambient;
        FFLColor diffuse;
        FFLColor specular;
    };
    NN_STATIC_ASSERT(sizeof(Light) == 0x40);

    struct Material
    {
        FFLColor ambient;
        FFLColor diffuse;
        FFLColor specular;
        f32 specularPower;
        FFLSpecularMode specularMode;
    };
    NN_STATIC_ASSERT(sizeof(Material) == 0x38);

    struct Rim
    {
        FFLColor color;
        f32 power;
    };
    NN_STATIC_ASSERT(sizeof(Rim) == 0x14);

public:
    static const Material& GetMaterial(FFLDrawType type);
    static const Light& GetLight();
    static const Rim& GetRim();
};

#endif // FFLI_DEFAULT_PARAMS_H_
