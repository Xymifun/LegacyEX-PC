#include "platform/wiiu/MiiIcon/DefaultShader.hpp"

#include "platform/wiiu/MiiIcon/MiiIconAssert.hpp"

#include <cafe/gx2.h>
#include <cafe/mat.h>
#include <nn/ffl/FFLDefaultShader.h>
#include <nn/ffl/FFLIcon.h>
#include <nn/types.h>

#include <stdint.h>
#include <stdlib.h>

namespace
{

ALIGNVAR(4)
const u8 DEFAULT_SHADER_DATA[] = {
#include "platform/wiiu/shaders/MiiIconDefaultShader.inc"
};

void* s_DefaultShaderBuffer = NULL;
FFLDefaultShader s_DefaultShader;

}

FFLResult InitDefaultShader()
{
    if (s_DefaultShaderBuffer == NULL)
    {
        s_DefaultShaderBuffer =
            malloc(FFLGetBufferSizeDefaultShader(DEFAULT_SHADER_DATA) + GX2_SHADER_ALIGNMENT);
        MII_ICON_ASSERT_POINTER(s_DefaultShaderBuffer);
    }

    void* buffer = reinterpret_cast<void*>(
        (reinterpret_cast<uintptr_t>(s_DefaultShaderBuffer) + GX2_SHADER_ALIGNMENT - 1) &
        ~uintptr_t(GX2_SHADER_ALIGNMENT - 1));
    MII_ICON_ASSERT_POINTER(buffer);

    FFLResult result = FFLInitDefaultShader(&s_DefaultShader, buffer, DEFAULT_SHADER_DATA);
    MII_ICON_ASSERT_MESSAGE(result == FFL_RESULT_OK, "A default shader cannot be set up.");

    FFLSetDefaultShader(&s_DefaultShader);
    return result;
}

void SetupDefaultShaderForCreateModel()
{
    FFLSetupInitCharModelDefaultShader(&s_DefaultShader);
}

void SetupDefaultShaderForDrawTexture(unsigned short width, unsigned short height)
{
    Mat44 projection;
    Mat34 view;
    FFLGetIconMatrix(&projection, &view, width, height);
    FFLSetMatrixDefaultShader(&s_DefaultShader, &projection, &view);

    GX2SetContextState(FFLGetContextStateDefaultShader(&s_DefaultShader));
    FFLSetupDrawDefaultShader(&s_DefaultShader);
}

void FinalizeDefaultShader()
{
    FFLDestroyDefaultShader(&s_DefaultShader);
}
