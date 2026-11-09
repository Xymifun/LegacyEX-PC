#include "platform/wiiu/MiiIcon/MakeIcon.hpp"

#include "platform/wiiu/MiiIcon/MiiIconAssert.hpp"

#include <nn/ffl/FFLCharModelBuffer.h>
#include <nn/ffl/FFLCharModelDesc.h>
#include <nn/ffl/FFLCharModelSource.h>
#include <nn/ffl/FFLExpression.h>
#include <nn/ffl/FFLIcon.h>
#include <nn/ffl/FFLModelType.h>

#include <stdlib.h>

namespace
{

const FFLIconSetting ICON_SETTING = {
    64,   64, FFL_ICON_BACKGROUND_DIRECT, {0.0f, 0.0f, 0.0f, 0.0f}, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
    FALSE};

}

void* CreateModel(FFLCharModel* model, FFLDataSource dataSource, FFLStoreData* storeData, int index)
{
    MII_ICON_ASSERT_POINTER(model);

    FFLCharModelDesc desc;
    desc.expressionFlag = 1 << FFL_EXPRESSION_NORMAL;
    desc.resolution = FFL_RESOLUTION_TEX_256;
    desc.resourceType = FFL_RESOURCE_TYPE_MIDDLE;
    desc.modelFlag = 1 << FFL_MODEL_TYPE_0;
    desc.compressTexture = FALSE;

    FFLCharModelSource source;
    source.pBuffer = storeData;
    source.dataSource = dataSource;
    source.index = index;

    FFLCharModelBuffer buffer;
    buffer.bufferSize = FFLGetBufferSizeCharModel(&desc);
    buffer.tempBufferSize = FFLGetTempBufferSizeCharModel(&desc);
    buffer.pBuffer = malloc(buffer.bufferSize);
    buffer.pTempBuffer = malloc(buffer.tempBufferSize);

    FFLResult result = FFLInitCharModelCPUStep(model, &source, &desc, &buffer);
    MII_ICON_ASSERT_MESSAGE(result == FFL_RESULT_OK, "FFLCharModel cannot be set up. (Result: %d)", result);

    SetupIconShaderForCreateModel();
    FFLInitCharModelGPUStep(model);
    GX2DrawDone();

    free(buffer.pTempBuffer);
    return buffer.pBuffer;
}

void* CreateIconTexture(GX2Texture* texture, const FFLCharModel* model)
{
    MII_ICON_ASSERT_POINTER(texture);
    MII_ICON_ASSERT_POINTER(model);

    size_t bufferSize = FFLGetIconBufferSize(&ICON_SETTING);
    size_t tempBufferSize = FFLGetIconTempBufferSize(&ICON_SETTING);
    void* buffer = malloc(bufferSize);
    void* tempBuffer = malloc(tempBufferSize);

    SetupIconShaderForDrawTexture(ICON_SETTING.width, ICON_SETTING.height);

    FFLResult result =
        FFLMakeIconEx(texture, model, FFL_EXPRESSION_NORMAL, &ICON_SETTING, buffer, tempBuffer);
    MII_ICON_ASSERT_MESSAGE(result == FFL_RESULT_OK, "FFLInitCharModel Failed (FFLResult: %d)", result);

    GX2DrawDone();
    free(tempBuffer);
    return buffer;
}

void* MakeIcon(GX2Texture* texture, IconShaderType shaderType, FFLDataSource dataSource,
               FFLStoreData* storeData, int index)
{
    MII_ICON_ASSERT_POINTER(texture);
    MII_ICON_ASSERT(ICON_SHADER_TYPE_BEGIN <= shaderType && shaderType < ICON_SHADER_TYPE_END);

    InitIconShader(shaderType);

    FFLCharModel model;
    void* modelBuffer = CreateModel(&model, dataSource, storeData, index);

    GX2SetContextState(NULL);

    void* iconBuffer = CreateIconTexture(texture, &model);

    FFLDeleteCharModel(&model);
    free(modelBuffer);

    FinalizeIconShader();
    return iconBuffer;
}
