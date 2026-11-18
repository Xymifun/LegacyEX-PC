// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderAlbedoBaker.hpp
#ifndef PLATFORM_PC_SHADERPACK_SHADERALBEDOBAKER_HPP
#define PLATFORM_PC_SHADERPACK_SHADERALBEDOBAKER_HPP

#include <vector>

#include "platform/pc/shaderpack/GlApi.hpp"

enum ShaderAlbedoMode
{
    ShaderAlbedoMode_Layered,
    ShaderAlbedoMode_ColourMask,
    ShaderAlbedoMode_ChangeColour,
    ShaderAlbedoMode_EmissiveAlpha,
    ShaderAlbedoMode_EmissiveAlphaOnly
};

struct ShaderAlbedoRequest
{
    int mode = ShaderAlbedoMode_Layered;
    int layers = 1;
    GLuint textures[4] = {};
    unsigned int versions[4] = {};
    int sizes[4][2] = {};
    float colours[4][4] = {};
    float diffuse[4] = {};
    float mask[8][4] = {};
};

class ShaderAlbedoBaker
{
public:
    GLuint Resolve(const ShaderAlbedoRequest& request, unsigned long long frame, bool& stateChanged);
    void Trim(unsigned long long frame);
    void Release();

private:
    struct Entry
    {
        ShaderAlbedoRequest request;
        GLuint texture = 0;
        int width = 0;
        int height = 0;
        unsigned long long lastUsed = 0;
    };

    bool EnsureResources();
    bool Bake(const ShaderAlbedoRequest& request, Entry& entry);

    GLuint mProgram = 0;
    GLuint mFramebuffer = 0;
    GLuint mVertexArray = 0;
    GLuint mSampler = 0;
    GLint mModeLocation = -1;
    GLint mLayersLocation = -1;
    GLint mColourLocations[4] = {-1, -1, -1, -1};
    GLint mDiffuseLocation = -1;
    GLint mMaskLocations[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    bool mFailed = false;
    std::vector<Entry> mEntries;
};

#endif
