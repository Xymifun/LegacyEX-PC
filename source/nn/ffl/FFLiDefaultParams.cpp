#include <nn/ffl/FFLiDefaultParams.h>

namespace
{

const FFLiDefaultParams::Light LIGHT = {
    true, {0.0f, 0.0f, 1.0f}, {0.73f, 0.73f, 0.73f, 1.0f}, {0.6f, 0.6f, 0.6f, 1.0f}, {0.7f, 0.7f, 0.7f, 1.0f}};

const FFLiDefaultParams::Rim RIM = {{0.3f, 0.3f, 0.3f, 1.0f}, 2.0f};

const FFLiDefaultParams::Material MATERIALS[FFL_DRAW_TYPE_MAX] = {
    {{0.85f, 0.75f, 0.75f, 1.0f}, {0.75f, 0.75f, 0.75f, 1.0f}, {0.3f, 0.3f, 0.3f, 1.0f}, 1.2f, FFL_SPECULAR_MODE_BLINN},
    {{1.0f, 1.0f, 1.0f, 1.0f},
     {0.7f, 0.7f, 0.7f, 1.0f},
     {0.0f, 0.0f, 0.0f, 1.0f},
     40.0f,
     FFL_SPECULAR_MODE_ANISOTROPIC},
    {{0.9f, 0.85f, 0.85f, 1.0f},
     {0.75f, 0.75f, 0.75f, 1.0f},
     {0.22f, 0.22f, 0.22f, 1.0f},
     1.5f,
     FFL_SPECULAR_MODE_BLINN},
    {{0.85f, 0.75f, 0.75f, 1.0f}, {0.75f, 0.75f, 0.75f, 1.0f}, {0.3f, 0.3f, 0.3f, 1.0f}, 1.2f, FFL_SPECULAR_MODE_BLINN},
    {{1.0f, 1.0f, 1.0f, 1.0f},
     {0.7f, 0.7f, 0.7f, 1.0f},
     {0.35f, 0.35f, 0.35f, 1.0f},
     10.0f,
     FFL_SPECULAR_MODE_ANISOTROPIC},
    {{0.75f, 0.75f, 0.75f, 1.0f}, {0.72f, 0.72f, 0.72f, 1.0f}, {0.3f, 0.3f, 0.3f, 1.0f}, 1.5f, FFL_SPECULAR_MODE_BLINN},
    {{1.0f, 1.0f, 1.0f, 1.0f},
     {0.7f, 0.7f, 0.7f, 1.0f},
     {0.0f, 0.0f, 0.0f, 1.0f},
     40.0f,
     FFL_SPECULAR_MODE_ANISOTROPIC},
    {{1.0f, 1.0f, 1.0f, 1.0f},
     {0.7f, 0.7f, 0.7f, 1.0f},
     {0.0f, 0.0f, 0.0f, 1.0f},
     40.0f,
     FFL_SPECULAR_MODE_ANISOTROPIC},
    {{1.0f, 1.0f, 1.0f, 1.0f},
     {0.7f, 0.7f, 0.7f, 1.0f},
     {0.0f, 0.0f, 0.0f, 1.0f},
     40.0f,
     FFL_SPECULAR_MODE_ANISOTROPIC}};

}

const FFLiDefaultParams::Material& FFLiDefaultParams::GetMaterial(FFLDrawType type)
{
    return MATERIALS[type];
}

const FFLiDefaultParams::Light& FFLiDefaultParams::GetLight()
{
    return LIGHT;
}

const FFLiDefaultParams::Rim& FFLiDefaultParams::GetRim()
{
    return RIM;
}
