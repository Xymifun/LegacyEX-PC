// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderFxaa.hpp
#ifndef PLATFORM_PC_SHADERPACK_SHADERFXAA_HPP
#define PLATFORM_PC_SHADERPACK_SHADERFXAA_HPP

#include "platform/pc/shaderpack/GlApi.hpp"

class ShaderFxaa
{
public:
    GLuint Target(int width, int height);
    void Apply(GLuint destination, int width, int height, int level, GLenum textureUnit);
    void Release();

private:
    bool EnsureProgram();

    GLuint mProgram = 0;
    GLuint mFramebuffer = 0;
    GLuint mTexture = 0;
    GLuint mVertexArray = 0;
    GLuint mSampler = 0;
    GLint mInverseSizeLocation = -1;
    GLint mLongSearchLocation = -1;
    GLint mSourceLocation = -1;
    int mWidth = 0;
    int mHeight = 0;
    bool mFailed = false;
};

#endif
