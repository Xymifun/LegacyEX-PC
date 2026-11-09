#include <nn/ffl/FFLDrawParam.h>

#include <nn/ffl/FFLiDefaultParams.h>
#include <nn/ffl/FFLiDefaultShader.h>
#include <nn/ffl/FFLiManager.h>
#include <nn/ffl/FFLiShaderCallback.h>
#include <nn/ffl/FFLiUtil.h>
#include <nn/ffl/FFLiUtilShaderAllocator.h>

#include <nn/ffl/detail/FFLiBufferAllocator.h>

#include <string.h>

namespace
{

const u32 NUM_ATTRIBUTES = 5;

const FFLiUtilShader::AttributeStreamDesc ATTRIBUTE_STREAM_DESC[NUM_ATTRIBUTES] = {
    {"a_position", FFL_ATTRIBUTE_BUFFER_TYPE_POSITION, 0, GX2_ATTRIB_FORMAT_32_32_32_FLOAT},
    {"a_texCoord", FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD, 0, GX2_ATTRIB_FORMAT_32_32_FLOAT},
    {"a_normal", FFL_ATTRIBUTE_BUFFER_TYPE_NORMAL, 0, GX2_ATTRIB_FORMAT_10_10_10_2_SNORM},
    {"a_tangent", FFL_ATTRIBUTE_BUFFER_TYPE_TANGENT, 0, GX2_ATTRIB_FORMAT_8_8_8_8_SNORM},
    {"a_color", FFL_ATTRIBUTE_BUFFER_TYPE_COLOR, 0, GX2_ATTRIB_FORMAT_8_8_8_8_UNORM}};

GX2Boolean s_EnablePolygonMode = GX2_FALSE;
GX2PolygonMode s_PolygonModeFront = GX2_POLYGON_MODE_TRIANGLE;
GX2PolygonMode s_PolygonModeBack = GX2_POLYGON_MODE_TRIANGLE;

void SetCullingModeWraper(GX2FrontFaceMode frontFaceMode, GX2Boolean cullFront, GX2Boolean cullBack)
{
    GX2SetPolygonControl(frontFaceMode, cullFront, cullBack, s_EnablePolygonMode, s_PolygonModeFront, s_PolygonModeBack,
                         GX2_FALSE, GX2_FALSE, GX2_FALSE);
}

void SetupCulling(FFLCullMode cullMode, const FFLModulateParam& modulateParam, BOOL cullBack)
{
    bool reverse = modulateParam.type < FFL_MODULATE_TYPE_SHAPE_MAX && cullBack;

    switch (cullMode)
    {
    case FFL_CULL_MODE_NONE:
        SetCullingModeWraper(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_FALSE);
        break;
    case FFL_CULL_MODE_BACK:
        if (reverse)
            SetCullingModeWraper(GX2_FRONT_FACE_CCW, GX2_TRUE, GX2_FALSE);
        else
            SetCullingModeWraper(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_TRUE);
        break;
    case FFL_CULL_MODE_FRONT:
        if (!reverse)
            SetCullingModeWraper(GX2_FRONT_FACE_CCW, GX2_TRUE, GX2_FALSE);
        else
            SetCullingModeWraper(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_TRUE);
        break;
    }
}

void BindTexture(FFLiDefaultShader* pShader, const FFLModulateParam& modulateParam)
{
    if (modulateParam.pGX2Texture != NULL)
    {
        const GX2Sampler* pSampler = &pShader->sampler;
        if (pShader->useEyebrowSampler && modulateParam.type == FFL_MODULATE_TYPE_EYEBROW)
            pSampler = &pShader->eyebrowSampler;

        GX2SetPixelTexture(modulateParam.pGX2Texture, pShader->textureLocation);
        GX2SetPixelSampler(pSampler, pShader->textureLocation);
    }
}

void SetupConstColor(u32 location, const FFLColor& color)
{
    GX2SetPixelUniformReg(location, 4, &color);
}

s32 ModulateModeToValue(FFLModulateMode mode)
{
    switch (mode)
    {
    case FFL_MODULATE_MODE_0:
        return 0;
    case FFL_MODULATE_MODE_1:
        return 1;
    case FFL_MODULATE_MODE_2:
        return 2;
    case FFL_MODULATE_MODE_3:
        return 3;
    case FFL_MODULATE_MODE_4:
        return 4;
    case FFL_MODULATE_MODE_5:
        return 5;
    default:
        return 0;
    }
}

void SetupModulateMode(FFLiDefaultShader* pShader, FFLModulateMode mode)
{
    s32 values[4] = {0, 0, 0, 0};
    values[0] = ModulateModeToValue(mode);
    GX2SetPixelUniformReg(pShader->modeLocation, 4, values);
}

void SetupModulate(FFLiDefaultShader* pShader, const FFLModulateParam& modulateParam)
{
    BindTexture(pShader, modulateParam);

    switch (modulateParam.mode)
    {
    case FFL_MODULATE_MODE_0:
    case FFL_MODULATE_MODE_3:
    case FFL_MODULATE_MODE_4:
    case FFL_MODULATE_MODE_5:
        SetupConstColor(pShader->const1Location, *modulateParam.pColorR);
        break;
    case FFL_MODULATE_MODE_2:
        SetupConstColor(pShader->const1Location, *modulateParam.pColorR);
        SetupConstColor(pShader->const2Location, *modulateParam.pColorG);
        SetupConstColor(pShader->const3Location, *modulateParam.pColorB);
        break;
    case FFL_MODULATE_MODE_1:
        break;
    }

    SetupModulateMode(pShader, modulateParam.mode);
}

FFLiDefaultShaderLight* GetLightParam(FFLiDefaultShaderParam* pParam)
{
    return &pParam->light;
}

FFLiDefaultShaderMaterial* GetMaterialParam(FFLiDefaultShaderParam* pParam, FFLDrawType type)
{
    return &pParam->material[type];
}

void SetupMaterial(const FFLiDefaultShader* pShader, const FFLDrawParam& drawParam)
{
    FFLModulateType type = drawParam.modulateParam.type;
    if (type >= FFL_MODULATE_TYPE_MUSTACHE && type <= FFL_MODULATE_TYPE_FILL)
        return;

    bool noTangent = drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_TANGENT].ptr == NULL;
    const FFLiDefaultShaderMaterial& material = pShader->param.material[type];

    FFLColor specular = material.specular;

    GX2SetPixelUniformReg(pShader->materialAmbientLocation, 4, &material.ambient);
    GX2SetPixelUniformReg(pShader->materialDiffuseLocation, 4, &material.diffuse);
    GX2SetPixelUniformReg(pShader->materialSpecularLocation, 4, &specular);

    f32 specularPower[4] = {material.specularPower, 0.0f, 0.0f, 0.0f};
    GX2SetPixelUniformReg(pShader->materialSpecularPowerLocation, 4, specularPower);

    u32 specularMode[4] = {static_cast<u32>(material.specularMode), 0, 0, 0};
    if (noTangent)
        specularMode[0] = FFL_SPECULAR_MODE_BLINN;
    GX2SetPixelUniformReg(pShader->materialSpecularModeLocation, 4, specularMode);
}

void ShaderCallbackDraw(void* pObj, const FFLDrawParam* pDrawParam)
{
    FFLiDefaultShader* pShader = static_cast<FFLiDefaultShader*>(pObj);
    const FFLDrawParam& drawParam = *pDrawParam;

    SetupCulling(drawParam.cullMode, drawParam.modulateParam, pShader->param.cullBack);
    SetupModulate(pShader, drawParam.modulateParam);
    SetupMaterial(pShader, drawParam);

    if (drawParam.primitiveParam.pIndexBuffer != NULL)
    {
        GX2SetAttribBuffer(FFL_ATTRIBUTE_BUFFER_TYPE_POSITION,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION].size,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION].stride,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION].ptr);
        GX2SetAttribBuffer(FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD].size,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD].stride,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD].ptr);
        GX2SetAttribBuffer(FFL_ATTRIBUTE_BUFFER_TYPE_NORMAL,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_NORMAL].size,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_NORMAL].stride,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_NORMAL].ptr);
        GX2SetAttribBuffer(FFL_ATTRIBUTE_BUFFER_TYPE_TANGENT,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_TANGENT].size,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_TANGENT].stride,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_TANGENT].ptr);
        GX2SetAttribBuffer(FFL_ATTRIBUTE_BUFFER_TYPE_COLOR,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_COLOR].size,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_COLOR].stride,
                           drawParam.attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_COLOR].ptr);

        GX2DrawIndexedEx(drawParam.primitiveParam.primitiveType, drawParam.primitiveParam.indexCount,
                         drawParam.primitiveParam.indexFormat, drawParam.primitiveParam.pIndexBuffer, 0, 1);
    }
}

void ShaderCallbackSetMatrix(void* pObj, const Mat44* pMatrix)
{
    const FFLiDefaultShader* pShader = static_cast<const FFLiDefaultShader*>(pObj);

    Mat44 matrix;
    MAT44Identity(&matrix);
    GX2SetVertexUniformReg(pShader->modelViewLocation, 16, &matrix);
    GX2SetVertexUniformReg(pShader->inverseTransposeLocation, 12, &matrix);

    MAT44Transpose(pMatrix, &matrix);
    GX2SetVertexUniformReg(pShader->projectionLocation, 16, &matrix);
}

void Identity(f32 matrix[][4])
{
    memset(matrix, 0, sizeof(f32) * 3 * 4);
    matrix[0][0] = 1.0f;
    matrix[1][1] = 1.0f;
    matrix[2][2] = 1.0f;
}

void InitLightParam(FFLiDefaultShaderLight* pLight)
{
    const FFLiDefaultParams::Light& light = FFLiDefaultParams::GetLight();

    pLight->enable = light.enable;
    pLight->direction = light.direction;
    pLight->ambient = light.ambient;
    pLight->diffuse = light.diffuse;
    pLight->specular = light.specular;
}

void InitMaterialParam(FFLiDefaultShaderMaterial* pMaterial, FFLDrawType type)
{
    const FFLiDefaultParams::Material& material = FFLiDefaultParams::GetMaterial(type);

    pMaterial->ambient = material.ambient;
    pMaterial->diffuse = material.diffuse;
    pMaterial->specular = material.specular;
    pMaterial->specularPower = material.specularPower;
    pMaterial->specularMode = material.specularMode;
}

void InitParam(FFLiDefaultShaderParam* pParam)
{
    MAT44Identity(&pParam->projection);
    MAT44Identity(&pParam->modelView);
    Identity(pParam->inverseTranspose);

    InitLightParam(&pParam->light);

    for (u32 i = 0; i < FFL_DRAW_TYPE_MAX; i++)
        InitMaterialParam(&pParam->material[i], FFLDrawType(i));

    const FFLiDefaultParams::Rim& rim = FFLiDefaultParams::GetRim();
    pParam->rimColor = rim.color;
    pParam->rimPower = rim.power;
    pParam->cullBack = FALSE;
}

void SetUniformLightDisable(const FFLiDefaultShader* pShader)
{
    u32 values[4] = {0, 0, 0, 0};
    GX2SetPixelUniformReg(pShader->lightEnableLocation, 4, values);
}

void SetupUniformLight(const FFLiDefaultShader* pShader)
{
    const FFLiDefaultShaderLight& light = pShader->param.light;

    f32 direction[4] = {light.direction.x, light.direction.y, light.direction.z, 0.0f};
    u32 enable[4] = {light.enable != FALSE, 0, 0, 0};

    GX2SetPixelUniformReg(pShader->lightDirectionLocation, 4, direction);
    GX2SetPixelUniformReg(pShader->lightEnableLocation, 4, enable);
    GX2SetPixelUniformReg(pShader->lightAmbientLocation, 4, &light.ambient);
    GX2SetPixelUniformReg(pShader->lightDiffuseLocation, 4, &light.diffuse);
    GX2SetPixelUniformReg(pShader->lightSpecularLocation, 4, &light.specular);
}

void SetupUniformParam(const FFLiDefaultShader* pShader)
{
    const FFLiDefaultShaderParam& param = pShader->param;

    GX2SetVertexUniformReg(pShader->projectionLocation, 16, &param.projection);
    GX2SetVertexUniformReg(pShader->modelViewLocation, 16, &param.modelView);
    GX2SetVertexUniformReg(pShader->inverseTransposeLocation, 12, param.inverseTranspose);

    SetupUniformLight(pShader);

    f32 rimPower[4] = {param.rimPower, 0.0f, 0.0f, 0.0f};
    GX2SetPixelUniformReg(pShader->rimColorLocation, 4, &param.rimColor);
    GX2SetPixelUniformReg(pShader->rimPowerLocation, 4, rimPower);
}

void Mat44to34(Mat34* pDst, const Mat44* pSrc)
{
    for (u32 i = 0; i < 3; i++)
    {
        for (u32 j = 0; j < 4; j++)
            pDst->mtx[i][j] = pSrc->mtx[i][j];
    }
}

void StoreColumnMajor(Mat44* pDst, const Mat44& src)
{
    for (u32 i = 0; i < 4; i++)
    {
        for (u32 j = 0; j < 4; j++)
            pDst->mtx[j][i] = src.mtx[i][j];
    }
}

void StoreColumnMajor(f32 dst[][4], const Mat44& src)
{
    for (u32 i = 0; i < 3; i++)
    {
        for (u32 j = 0; j < 3; j++)
            dst[j][i] = src.mtx[i][j];
    }
}

}

u32 FFLiGetBufferSizeDefaultShader(const void* pShaderData)
{
    return FFLiUtilShader::GetBufferSize(NUM_ATTRIBUTES, pShaderData, 0) + sizeof(GX2ContextState);
}

FFLResult FFLiInitDefaultShader(FFLiDefaultShader* pShader, void* pBuffer, const void* pShaderData)
{
    if (!FFLiCheckAlignPtr(pBuffer, GX2_CONTEXT_STATE_ALIGNMENT))
        return FFL_RESULT_ERROR;

    memset(pShader, 0, sizeof(FFLiDefaultShader));

    FFLiBufferAllocator allocator;
    FFLiInitBufferAllocator(&allocator, pBuffer, FFLiGetBufferSizeDefaultShader(pShaderData));

    pShader->callback.pObj = pShader;

    GX2ContextState* pContextState = static_cast<GX2ContextState*>(
        FFLiAllocateBufferAllocator(&allocator, sizeof(GX2ContextState), GX2_CONTEXT_STATE_ALIGNMENT));
    pShader->callback.pContextState = pContextState;
    pShader->callback.pDrawFunc = ShaderCallbackDraw;
    pShader->callback.pSetMatrixFunc = ShaderCallbackSetMatrix;
    GX2SetupContextState(pContextState);

    FFLiUtilShaderAllocator shaderAllocator(&allocator, NUM_ATTRIBUTES, pShaderData, 0);

    if (!FFLiUtilShader::BuildShader(&pShader->pVertexShader, &shaderAllocator, pShaderData, 0, true))
        return FFL_RESULT_ERROR;

    if (!FFLiUtilShader::BuildShader(&pShader->pPixelShader, &shaderAllocator, pShaderData, 0, true))
        return FFL_RESULT_ERROR;

    if (!FFLiUtilShader::BuildShader(&pShader->fetchShader, pShader->pVertexShader, &shaderAllocator,
                                     ATTRIBUTE_STREAM_DESC, NUM_ATTRIBUTES, true))
        return FFL_RESULT_ERROR;

    GX2InitSampler(&pShader->sampler, GX2_TEX_CLAMP_MIRROR, GX2_TEX_XY_FILTER_BILINEAR);
    GX2InitSampler(&pShader->eyebrowSampler, GX2_TEX_CLAMP_MIRROR, GX2_TEX_XY_FILTER_BILINEAR);
    GX2InitSamplerLOD(&pShader->eyebrowSampler, 0.0f, 15.999f, 0.5f);
    pShader->useEyebrowSampler = false;

    if (!FFLiUtilShader::GetUniformLocation(&pShader->projectionLocation, pShader->pVertexShader, "u_proj")
        || !FFLiUtilShader::GetUniformLocation(&pShader->modelViewLocation, pShader->pVertexShader, "u_mv")
        || !FFLiUtilShader::GetUniformLocation(&pShader->inverseTransposeLocation, pShader->pVertexShader, "u_it")
        || !FFLiUtilShader::GetSamplerLocation(&pShader->textureLocation, pShader->pPixelShader, "s_texture")
        || !FFLiUtilShader::GetUniformLocation(&pShader->modeLocation, pShader->pPixelShader, "u_mode")
        || !FFLiUtilShader::GetUniformLocation(&pShader->const1Location, pShader->pPixelShader, "u_const1")
        || !FFLiUtilShader::GetUniformLocation(&pShader->const2Location, pShader->pPixelShader, "u_const2")
        || !FFLiUtilShader::GetUniformLocation(&pShader->const3Location, pShader->pPixelShader, "u_const3")
        || !FFLiUtilShader::GetUniformLocation(&pShader->lightEnableLocation, pShader->pPixelShader, "u_light_enable")
        || !FFLiUtilShader::GetUniformLocation(&pShader->lightAmbientLocation, pShader->pPixelShader, "u_light_ambient")
        || !FFLiUtilShader::GetUniformLocation(&pShader->lightDiffuseLocation, pShader->pPixelShader, "u_light_diffuse")
        || !FFLiUtilShader::GetUniformLocation(&pShader->lightSpecularLocation, pShader->pPixelShader,
                                               "u_light_specular")
        || !FFLiUtilShader::GetUniformLocation(&pShader->lightDirectionLocation, pShader->pPixelShader, "u_light_dir")
        || !FFLiUtilShader::GetUniformLocation(&pShader->materialAmbientLocation, pShader->pPixelShader,
                                               "u_material_ambient")
        || !FFLiUtilShader::GetUniformLocation(&pShader->materialDiffuseLocation, pShader->pPixelShader,
                                               "u_material_diffuse")
        || !FFLiUtilShader::GetUniformLocation(&pShader->materialSpecularLocation, pShader->pPixelShader,
                                               "u_material_specular")
        || !FFLiUtilShader::GetUniformLocation(&pShader->materialSpecularPowerLocation, pShader->pPixelShader,
                                               "u_material_specular_power")
        || !FFLiUtilShader::GetUniformLocation(&pShader->materialSpecularModeLocation, pShader->pPixelShader,
                                               "u_material_specular_mode")
        || !FFLiUtilShader::GetUniformLocation(&pShader->rimColorLocation, pShader->pPixelShader, "u_rim_color")
        || !FFLiUtilShader::GetUniformLocation(&pShader->rimPowerLocation, pShader->pPixelShader, "u_rim_power"))
        return FFL_RESULT_ERROR;

    InitParam(&pShader->param);
    return FFL_RESULT_OK;
}

void FFLiDestroyDefaultShader(FFLiDefaultShader* pShader)
{
}

const FFLShaderCallback* FFLiGetCallbackDefaultShader(const FFLiDefaultShader* pShader)
{
    return &pShader->callback;
}

void FFLiSetDefaultShader(const FFLiDefaultShader* pShader)
{
    GX2VertexShader* pVertexShader = pShader->pVertexShader;
    GX2PixelShader* pPixelShader = pShader->pPixelShader;

    GX2SetFetchShader(&pShader->fetchShader.fetchShader);
    GX2SetVertexShader(pVertexShader);
    GX2SetPixelShader(pPixelShader);
}

void FFLiSetupInitCharModelDefaultShaderNoSetCallback(const FFLiDefaultShader* pShader)
{
    FFLiShaderCallback callback;
    callback.Set(&pShader->callback);
    callback.CallSetContextState();

    FFLiSetDefaultShader(pShader);
    SetUniformLightDisable(pShader);
}

void FFLiSetupInitCharModelDefaultShader(const FFLiDefaultShader* pShader)
{
    if (FFLiManager::IsConstruct())
    {
        FFLiShaderCallback& callback = FFLiManager::GetInstance()->GetShaderCallback();
        callback.Set(FFLiGetCallbackDefaultShader(pShader));
        FFLiSetupInitCharModelDefaultShaderNoSetCallback(pShader);
    }
}

void FFLiSetupDrawDefaultShaderNoSetCallback(const FFLiDefaultShader* pShader)
{
    GX2VertexShader* pVertexShader = pShader->pVertexShader;
    GX2PixelShader* pPixelShader = pShader->pPixelShader;

    GX2SetFetchShader(&pShader->fetchShader.fetchShader);
    GX2SetVertexShader(pVertexShader);
    GX2SetPixelShader(pPixelShader);
    SetupUniformParam(pShader);
}

void FFLiSetupDrawDefaultShader(const FFLiDefaultShader* pShader)
{
    if (FFLiManager::IsConstruct())
    {
        FFLiShaderCallback& callback = FFLiManager::GetInstance()->GetShaderCallback();
        callback.Set(FFLiGetCallbackDefaultShader(pShader));
        FFLiSetupDrawDefaultShaderNoSetCallback(pShader);
    }
}

void FFLiDrawDefaultShader(FFLiDefaultShader* pShader, const FFLDrawParam* pDrawParam)
{
    ShaderCallbackDraw(pShader, pDrawParam);
}

GX2ContextState* FFLiGetContextStateDefaultShader(FFLiDefaultShader* pShader)
{
    return const_cast<GX2ContextState*>(pShader->callback.pContextState);
}

GX2Sampler* FFLiGetSamplerDefaultShader(FFLiDefaultShader* pShader)
{
    return &pShader->sampler;
}

void FFLiSetMatrixExDefaultShader(FFLiDefaultShader* pShader, const Mat44* pProjection, const Mat34* pModelView,
                                  const Mat34* pInverseTranspose)
{
    MAT44Transpose(pProjection, &pShader->param.projection);

    Mat44 matrix;
    MAT34To44(pModelView, &matrix);
    StoreColumnMajor(&pShader->param.modelView, matrix);

    MAT34To44(pInverseTranspose, &matrix);
    StoreColumnMajor(pShader->param.inverseTranspose, matrix);
}

void FFLiSetMatrixDefaultShader(FFLiDefaultShader* pShader, const Mat44* pProjection, const Mat34* pModelView)
{
    Mat44 inverse;
    MAT34To44(pModelView, &inverse);
    C_MTX44Inverse(inverse.mtx, inverse.mtx);
    MAT44Transpose(&inverse, &inverse);

    Mat34 inverseTranspose;
    Mat44to34(&inverseTranspose, &inverse);

    FFLiSetMatrixExDefaultShader(pShader, pProjection, pModelView, &inverseTranspose);
}

void FFLiSetCullBackDefaultShader(FFLiDefaultShader* pShader, BOOL cullBack)
{
    pShader->param.cullBack = cullBack;
}

void FFLiSetLightEnableDefaultShader(FFLiDefaultShader* pShader, bool enable)
{
    GetLightParam(&pShader->param)->enable = enable;
}

void FFLiSetLightDirectionDefaultShader(FFLiDefaultShader* pShader, const FFLVec3& direction)
{
    GetLightParam(&pShader->param)->direction = direction;
}

void FFLiSetLightAmbientDefaultShader(FFLiDefaultShader* pShader, const FFLColor& color)
{
    GetLightParam(&pShader->param)->ambient = color;
}

void FFLiSetLightDiffuseDefaultShader(FFLiDefaultShader* pShader, const FFLColor& color)
{
    GetLightParam(&pShader->param)->diffuse = color;
}

void FFLiSetLightSpecularDefaultShader(FFLiDefaultShader* pShader, const FFLColor& color)
{
    GetLightParam(&pShader->param)->specular = color;
}

void FFLiSetMaterialAmbientDefaultShader(FFLiDefaultShader* pShader, FFLDrawType type, const FFLColor& color)
{
    GetMaterialParam(&pShader->param, type)->ambient = color;
}

void FFLiSetMaterialDiffuseDefaultShader(FFLiDefaultShader* pShader, FFLDrawType type, const FFLColor& color)
{
    GetMaterialParam(&pShader->param, type)->diffuse = color;
}

void FFLiSetMaterialSpecularDefaultShader(FFLiDefaultShader* pShader, FFLDrawType type, const FFLColor& color)
{
    GetMaterialParam(&pShader->param, type)->specular = color;
}

void FFLiSetMaterialSpecularPowerDefaultShader(FFLiDefaultShader* pShader, FFLDrawType type, f32 power)
{
    GetMaterialParam(&pShader->param, type)->specularPower = power;
}

void FFLiSetMaterialSpecularModeDefaultShader(FFLiDefaultShader* pShader, FFLDrawType type, FFLSpecularMode mode)
{
    GetMaterialParam(&pShader->param, type)->specularMode = mode;
}

void FFLiSetRimColorDefaultShader(FFLiDefaultShader* pShader, const FFLColor& color)
{
    pShader->param.rimColor = color;
}

void FFLiSetRimPowerDefaultShader(FFLiDefaultShader* pShader, f32 power)
{
    pShader->param.rimPower = power;
}

void FFLpSetPolygonModeDefaultShader(GX2Boolean enable, GX2PolygonMode frontMode, GX2PolygonMode backMode)
{
    s_EnablePolygonMode = enable;
    s_PolygonModeFront = frontMode;
    s_PolygonModeBack = backMode;
}
