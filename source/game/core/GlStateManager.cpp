#include "game/core/GlStateManager.hpp"

#include <string.h>

#include "game/client/render/Renderer.hpp"
#include "game/core/Buffer.hpp"
#include "game/core/ByteBuffer.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/IntBuffer.hpp"
#include "game/core/Matrix4f.hpp"

const int GL_OBJECT_PLANE = 0;
const int GL_EYE_PLANE = 1;
const int GL_EYE_LINEAR = 1;
const int GL_TEXTURE = 2;
const int GL_GREATER = 4;
const int GL_MODELVIEW = 0;
const int GL_MODELVIEW_MATRIX = 0;
const int GL_PROJECTION_MATRIX = 1;

enum GlStateCapability
{
    STATE_NONE = 0,
    STATE_TEXTURE_2D = 1,
    STATE_BLEND = 2,
    STATE_CULL_FACE = 3,
    STATE_ALPHA_TEST = 4,
    STATE_DEPTH_TEST = 5,
    STATE_FOG = 6,
    STATE_LIGHTING = 7,
    STATE_LIGHT0 = 8,
    STATE_LIGHT1 = 9
};

enum TexGenCapability
{
    TEX_GEN_CAP_S = 0,
    TEX_GEN_CAP_T = 1,
    TEX_GEN_CAP_Q = 2,
    TEX_GEN_CAP_R = 3
};

static const int MAX_LIGHTS = 8;
static const int MAX_TEXTURE_UNITS = 8;

static int activeTextureUnit;
static int shadeMode;
static GlStateManager::BooleanState* lights[MAX_LIGHTS];
static GlStateManager::TextureState* textures[MAX_TEXTURE_UNITS];

static GlStateManager::AlphaState* alphaState = new GlStateManager::AlphaState();
static GlStateManager::BooleanState* lightingState = new GlStateManager::BooleanState(STATE_LIGHTING);
static GlStateManager::ColorMaterialState* colorMaterialState = new GlStateManager::ColorMaterialState();
static GlStateManager::BlendState* blendState = new GlStateManager::BlendState();
static GlStateManager::DepthState* depthState = new GlStateManager::DepthState();
static GlStateManager::FogState* fogState = new GlStateManager::FogState();
static GlStateManager::CullState* cullState = new GlStateManager::CullState();
static GlStateManager::PolygonOffsetState* polygonOffsetState = new GlStateManager::PolygonOffsetState();
static GlStateManager::ColorLogicState* colorLogicState = new GlStateManager::ColorLogicState();
static GlStateManager::TexGenState* texGenState = new GlStateManager::TexGenState();
static GlStateManager::ClearState* clearState = new GlStateManager::ClearState();
static GlStateManager::StencilState* stencilState = new GlStateManager::StencilState();
static GlStateManager::BooleanState* normalizeState = new GlStateManager::BooleanState(STATE_NONE);
static GlStateManager::BooleanState* rescaleNormalState = new GlStateManager::BooleanState(STATE_NONE);
static GlStateManager::ColorMask* colorMaskState = new GlStateManager::ColorMask();
static GlStateManager::Color* color = new GlStateManager::Color();

GlStateManager::BooleanState::BooleanState(int capability)
{
    this->capability = capability;
    state = false;
}

void GlStateManager::BooleanState::setEnabled(bool enabled)
{
    state = enabled;

    if (enabled)
    {
        GlStateManager::enableState(capability);
    }
    else
    {
        GlStateManager::disableState(capability);
    }
}

void GlStateManager::BooleanState::enable()
{
    setEnabled(true);
}

void GlStateManager::BooleanState::disable()
{
    setEnabled(false);
}

GlStateManager::Color::Color()
{
    red = 1.0f;
    green = 1.0f;
    blue = 1.0f;
    alpha = 1.0f;
}

GlStateManager::Color::Color(float red, float green, float blue, float alpha)
{
    this->red = red;
    this->green = green;
    this->blue = blue;
    this->alpha = alpha;
}

GlStateManager::AlphaState::AlphaState()
{
    alphaTest = new BooleanState(STATE_ALPHA_TEST);
    func = 7;
    ref = -1.0f;
}

GlStateManager::BlendState::BlendState()
{
    blend = new BooleanState(STATE_BLEND);
    srcFactor = 1;
    dstFactor = 0;
    srcFactorAlpha = 1;
    dstFactorAlpha = 0;
}

GlStateManager::ClearState::ClearState()
{
    depth = 1.0;
    color = new Color(0.0f, 0.0f, 0.0f, 0.0f);
    stencil = 0;
}

GlStateManager::ColorLogicState::ColorLogicState()
{
    colorLogicOp = new BooleanState(STATE_NONE);
    opcode = 0;
}

GlStateManager::ColorMask::ColorMask()
{
    red = true;
    green = true;
    blue = true;
    alpha = true;
}

GlStateManager::ColorMaterialState::ColorMaterialState()
{
    colorMaterial = new BooleanState(STATE_NONE);
    face = 2;
    mode = 0;
}

GlStateManager::CullState::CullState()
{
    cullFace = new BooleanState(STATE_CULL_FACE);
    mode = 1;
}

GlStateManager::DepthState::DepthState()
{
    depthTest = new BooleanState(STATE_DEPTH_TEST);
    maskEnabled = true;
    depthFunc = 4;
}

GlStateManager::FogState::FogState()
{
    fog = new BooleanState(STATE_FOG);
    mode = 2;
    density = 1.0f;
    start = 0.0f;
    end = 1.0f;
}

GlStateManager::PolygonOffsetState::PolygonOffsetState()
{
    polyOffsetFill = new BooleanState(STATE_NONE);
    polyOffsetLine = new BooleanState(STATE_TEXTURE_2D);
    factor = 0.0f;
    units = 0.0f;
}

GlStateManager::StencilFunc::StencilFunc()
{
    func = 7;
    ref = 0;
    mask = -1;
}

GlStateManager::StencilState::StencilState()
{
    func = new StencilFunc();
    mask = -1;
    fail = 0;
    zfail = 0;
    zpass = 0;
}

GlStateManager::TexGenCoord::TexGenCoord(int coord, int capability)
{
    this->coord = coord;
    textureGen = new BooleanState(capability);
    mode = -1;
}

GlStateManager::TexGenState::TexGenState()
{
    s = new TexGenCoord(S, TEX_GEN_CAP_S);
    t = new TexGenCoord(T, TEX_GEN_CAP_T);
    r = new TexGenCoord(R, TEX_GEN_CAP_R);
    q = new TexGenCoord(Q, TEX_GEN_CAP_Q);
}

GlStateManager::TextureState::TextureState()
{
    texture2DState = new BooleanState(STATE_TEXTURE_2D);
    textureName = 0;
}

void GlStateManager::staticCtor()
{
    for (int light = 0; light < MAX_LIGHTS; ++light)
    {
        lights[light] = new BooleanState(STATE_LIGHT0 + light);
    }

    for (int unit = 0; unit < MAX_TEXTURE_UNITS; ++unit)
    {
        textures[unit] = new TextureState();
    }
}

void GlStateManager::enableState(int capability)
{
    switch (capability)
    {
    case STATE_NONE:
        break;
    case STATE_BLEND:
        gRenderer->StateSetBlendEnable(true);
        break;
    case STATE_CULL_FACE:
        gRenderer->StateSetFaceCull(true);
        break;
    case STATE_DEPTH_TEST:
        gRenderer->StateSetDepthTestEnable(true);
        break;
    case STATE_ALPHA_TEST:
        gRenderer->StateSetAlphaTestEnable(true);
        break;
    case STATE_FOG:
        gRenderer->StateSetFogEnable(true);
        break;
    case STATE_LIGHTING:
        gRenderer->StateSetLightingEnable(true);
        break;
    case STATE_LIGHT0:
        gRenderer->StateSetLightEnable(0, true);
        break;
    case STATE_LIGHT1:
        gRenderer->StateSetLightEnable(1, true);
        break;
    default:
        break;
    }
}

void GlStateManager::disableState(int capability)
{
    switch (capability)
    {
    case STATE_NONE:
        break;
    case STATE_TEXTURE_2D:
        gRenderer->TextureBind(-1);
        break;
    case STATE_BLEND:
        gRenderer->StateSetBlendEnable(false);
        break;
    case STATE_CULL_FACE:
        gRenderer->StateSetFaceCull(false);
        break;
    case STATE_DEPTH_TEST:
        gRenderer->StateSetDepthTestEnable(false);
        break;
    case STATE_ALPHA_TEST:
        gRenderer->StateSetAlphaTestEnable(false);
        break;
    case STATE_FOG:
        gRenderer->StateSetFogEnable(false);
        break;
    case STATE_LIGHTING:
        gRenderer->StateSetLightingEnable(false);
        break;
    case STATE_LIGHT0:
        gRenderer->StateSetLightEnable(0, false);
        break;
    case STATE_LIGHT1:
        gRenderer->StateSetLightEnable(1, false);
        break;
    default:
        break;
    }
}

void GlStateManager::pushMatrix()
{
    gRenderer->MatrixPush();
}

void GlStateManager::popMatrix()
{
    gRenderer->MatrixPop();
}

void GlStateManager::loadIdentity()
{
    gRenderer->MatrixSetIdentity();
}

void GlStateManager::matrixMode(int mode)
{
    gRenderer->MatrixMode(mode);
}

void GlStateManager::translatef(float x, float y, float z)
{
    gRenderer->MatrixTranslate(x, y, z);
}

void GlStateManager::translated(double x, double y, double z)
{
    gRenderer->MatrixTranslate((float)x, (float)y, (float)z);
}

void GlStateManager::scalef(float x, float y, float z)
{
    gRenderer->MatrixScale(x, y, z);
}

void GlStateManager::scaled(double x, double y, double z)
{
    gRenderer->MatrixScale((float)x, (float)y, (float)z);
}

void GlStateManager::rotatef(float angle, float x, float y, float z)
{
    gRenderer->MatrixRotate(angle * 0.017453292f, x, y, z);
}

void GlStateManager::rotated(double angle, double x, double y, double z)
{
    gRenderer->MatrixRotate((float)angle * 0.017453292f, (float)x, (float)y, (float)z);
}

void GlStateManager::perspective(float fieldOfView, float aspect, float nearDistance, float farDistance)
{
    gRenderer->MatrixPerspective(fieldOfView, aspect, nearDistance, farDistance);
}

void GlStateManager::ortho(double left, double right, double bottom, double top, double nearDistance,
                           double farDistance)
{
    gRenderer->MatrixOrthogonal((float)left, (float)right, (float)bottom, (float)top, (float)nearDistance,
                                (float)farDistance);
}

void GlStateManager::viewport(int x, int y, int width, int height)
{
}

void GlStateManager::getMatrix(int mode, FloatBuffer* buffer)
{
    float* matrix = gRenderer->MatrixGet(mode);
    memcpy(buffer->mBuffer, matrix, 16 * sizeof(float));
}

void GlStateManager::multMatrix(FloatBuffer* buffer)
{
    gRenderer->MatrixMult(buffer->mBuffer);
}

void GlStateManager::multMatrix(Matrix4f* matrix)
{
    gRenderer->MatrixMult(matrix->m);
}

void GlStateManager::glMultMatrixf(float* matrix)
{
    gRenderer->MatrixMult(matrix);
}

void GlStateManager::enableLighting()
{
    lightingState->enable();
}

void GlStateManager::disableLighting()
{
    lightingState->disable();
}

void GlStateManager::enableLight(int light)
{
    lights[(unsigned int)(light - STATE_LIGHT0)]->enable();
}

void GlStateManager::disableLight(int light)
{
    lights[(unsigned int)(light - STATE_LIGHT0)]->disable();
}

void GlStateManager::light(int light, int parameter, FloatBuffer* buffer)
{
    int index;
    float* values = buffer->mBuffer;

    if (light == STATE_LIGHT0)
    {
        index = 0;
    }
    else if (light == STATE_LIGHT1)
    {
        index = 1;
    }
    else
    {
        return;
    }

    if (parameter == 1)
    {
        gRenderer->StateSetLightDirection(index, values[0], values[1], values[2]);
    }
    else if (parameter == 3)
    {
        gRenderer->StateSetLightColour(index, values[0], values[1], values[2]);
    }
}

void GlStateManager::lightModel(int parameter, FloatBuffer* buffer)
{
    float* values = buffer->mBuffer;

    if (parameter != 1)
    {
        return;
    }

    gRenderer->StateSetLightAmbientColour(values[0], values[1], values[2]);
}

void GlStateManager::enableColorMaterial()
{
    colorMaterialState->colorMaterial->enable();
}

void GlStateManager::disableColorMaterial()
{
    colorMaterialState->colorMaterial->disable();
}

void GlStateManager::colorMaterial(int face, int mode)
{
    colorMaterialState->face = face;
    colorMaterialState->mode = mode;
}

void GlStateManager::enableDepthTest()
{
    depthState->depthTest->enable();
}

void GlStateManager::disableDepthTest()
{
    depthState->depthTest->disable();
}

void GlStateManager::depthFunc(int func)
{
    depthState->depthFunc = func;
    gRenderer->StateSetDepthFunc(func);
}

void GlStateManager::depthMask(bool flag)
{
    depthState->maskEnabled = flag;
    gRenderer->StateSetDepthMask(flag);
}

void GlStateManager::enableBlend()
{
    blendState->blend->enable();
}

void GlStateManager::disableBlend()
{
    blendState->blend->disable();
}

void GlStateManager::blendFunc(int srcFactor, int dstFactor)
{
    blendState->srcFactor = srcFactor;
    blendState->dstFactor = dstFactor;
    gRenderer->StateSetBlendFunc(srcFactor, dstFactor);
}

void GlStateManager::blendFuncSeparate(int srcFactor, int dstFactor, int srcFactorAlpha, int dstFactorAlpha)
{
    blendState->srcFactor = srcFactor;
    blendState->dstFactor = dstFactor;
    blendState->srcFactorAlpha = srcFactorAlpha;
    blendState->dstFactorAlpha = dstFactorAlpha;
}

void GlStateManager::enableFog()
{
    fogState->fog->enable();
}

void GlStateManager::disableFog()
{
    fogState->fog->disable();
}

void GlStateManager::fogMode(int mode)
{
    fogState->mode = mode;
    gRenderer->StateSetFogMode(mode);
}

void GlStateManager::fogDensity(float density)
{
    fogState->density = density;
    gRenderer->StateSetFogDensity(density);
}

void GlStateManager::fogStart(float start)
{
    fogState->start = start;
    gRenderer->StateSetFogNearDistance(start);
}

void GlStateManager::fogEnd(float end)
{
    fogState->end = end;
    gRenderer->StateSetFogFarDistance(end);
}

float GlStateManager::getFogStart()
{
    return fogState->start;
}

float GlStateManager::getFogEnd()
{
    return fogState->end;
}

void GlStateManager::glFog(int parameter, FloatBuffer* buffer)
{
    float* values = buffer->mBuffer;
    gRenderer->StateSetFogColour(values[0], values[1], values[2]);
}

void GlStateManager::enableCull()
{
    cullState->cullFace->enable();
}

void GlStateManager::disableCull()
{
    cullState->cullFace->disable();
}

void GlStateManager::cullFace(int mode)
{
    cullState->mode = mode;
    gRenderer->StateSetFaceCullCW(mode == 1);
}

void GlStateManager::enablePolygonOffset()
{
    polygonOffsetState->polyOffsetFill->enable();
}

void GlStateManager::disablePolygonOffset()
{
    polygonOffsetState->polyOffsetFill->disable();
}

void GlStateManager::polygonOffset(float factor, float units)
{
    polygonOffsetState->factor = factor;
    polygonOffsetState->units = units;
    gRenderer->StateSetDepthSlopeAndBias(factor, units);
}

GlStateManager::TexGenCoord* GlStateManager::getTexGen(TexGen coord)
{
    switch (coord)
    {
    case S:
        return texGenState->s;
    case T:
        return texGenState->t;
    case R:
        return texGenState->r;
    case Q:
        return texGenState->q;
    default:
        return texGenState->s;
    }
}

void GlStateManager::enableTexGen(TexGen coord)
{
    getTexGen(coord)->textureGen->enable();
}

void GlStateManager::disableTexGen(TexGen coord)
{
    getTexGen(coord)->textureGen->disable();
}

void GlStateManager::texGenMode(TexGen coord, int mode)
{
    TexGenCoord* texGenCoord = getTexGen(coord);

    if (texGenCoord->mode != mode)
    {
        texGenCoord->mode = mode;
    }
}

void GlStateManager::texGenParam(TexGen coord, int parameter, FloatBuffer* buffer)
{
    gRenderer->StateSetTexGenCol(getTexGen(coord)->coord, buffer->get(0), buffer->get(1), buffer->get(2),
                                 buffer->get(3), parameter == 1);
}

void GlStateManager::enableNormalize()
{
    normalizeState->enable();
}

void GlStateManager::disableNormalize()
{
    normalizeState->disable();
}

void GlStateManager::enableRescaleNormal()
{
    rescaleNormalState->enable();
}

void GlStateManager::disableRescaleNormal()
{
    rescaleNormalState->disable();
}

void GlStateManager::enableAlphaTest()
{
    alphaState->alphaTest->enable();
}

void GlStateManager::disableAlphaTest()
{
    alphaState->alphaTest->disable();
}

void GlStateManager::alphaFunc(int func, float ref)
{
    alphaState->func = func;
    alphaState->ref = ref;
    gRenderer->StateSetAlphaFunc(func, ref);
}

void GlStateManager::colorMask(bool red, bool green, bool blue, bool alpha)
{
    ColorMask* mask = colorMaskState;
    mask->red = red;
    mask->green = green;
    mask->blue = blue;
    mask->alpha = alpha;
    gRenderer->StateSetWriteEnable(red, green, blue, alpha);
}

void GlStateManager::clearDepth(double depth)
{
    clearState->depth = depth;
}

void GlStateManager::clearColor(float red, float green, float blue, float alpha)
{
    clearState->color->red = red;
    clearState->color->green = green;
    clearState->color->blue = blue;
    clearState->color->alpha = alpha;

    float colour[4];
    colour[0] = red;
    colour[1] = green;
    colour[2] = blue;
    colour[3] = alpha;
    gRenderer->SetClearColour(colour);
}

void GlStateManager::clear(int flags)
{
    gRenderer->Clear(flags);
}

void GlStateManager::activeTexture(int texture)
{
    activeTextureUnit = texture;
}

void GlStateManager::enableTexture()
{
    textures[activeTextureUnit]->texture2DState->enable();
}

void GlStateManager::disableTexture()
{
    textures[activeTextureUnit]->texture2DState->disable();
}

void GlStateManager::bindTexture(int texture)
{
    textures[activeTextureUnit]->textureName = texture;
    gRenderer->TextureBind(texture);
}

int GlStateManager::genTexture()
{
    return gRenderer->TextureCreate();
}

void GlStateManager::deleteTexture(int texture)
{
    gRenderer->TextureFree(texture);

    for (int unit = 0; unit < MAX_TEXTURE_UNITS; ++unit)
    {
        if (textures[unit]->textureName == texture)
        {
            textures[unit]->textureName = -1;
        }
    }
}

void GlStateManager::genTextures(IntBuffer* buffer)
{
    for (unsigned int index = 0; index < buffer->limit(); ++index)
    {
        buffer->put(gRenderer->TextureCreate());
    }
}

void GlStateManager::deleteTextures(IntBuffer* buffer)
{
    for (unsigned int index = 0; index < buffer->limit(); ++index)
    {
        gRenderer->TextureFree(buffer->get(index));
    }
}

void GlStateManager::texImage2D(int target, int level, int internalFormat, int width, int height, int border,
                                int format, int type, ByteBuffer* pixels)
{
    gRenderer->TextureData(width, height, pixels->getBuffer(), level, C4JRender::eTextureFormat_RGBA8);
}

void GlStateManager::texParameteri(int target, int parameter, int value)
{
    gRenderer->TextureSetParam(parameter, value);
}

void GlStateManager::texParameteri(int parameter, int value)
{
    gRenderer->TextureSetParam(parameter, value);
}

void GlStateManager::texLayerColor3f(int stage, float red, float green, float blue)
{
    gRenderer->StateSetTexColour(stage, red, green, blue, 1.0f);
}

void GlStateManager::multiTexCoord2f(float u, float v)
{
    if (u > 255.0f)
    {
        u = 255.0f;
    }

    if (v > 255.0f)
    {
        v = 255.0f;
    }

    gRenderer->StateSetVertexTextureUV(u * 0.00390625f, v * 0.00390625f);
}

void GlStateManager::setupSolidRenderingTextureCombine(int stage)
{
}

void GlStateManager::tearDownSolidRenderingTextureCombine()
{
}

void GlStateManager::color3f(float red, float green, float blue)
{
    color4f(red, green, blue, 1.0f);
}

void GlStateManager::color4f(float red, float green, float blue, float alpha)
{
    color->red = red;
    color->green = green;
    color->blue = blue;
    color->alpha = alpha;
    gRenderer->StateSetColour(red, green, blue, alpha);
}

void GlStateManager::glColor3f(float red, float green, float blue)
{
    gRenderer->StateSetColour(red, green, blue, 1.0f);
}

void GlStateManager::normal3f(float x, float y, float z)
{
}

void GlStateManager::lineWidth(float width)
{
    gRenderer->StateSetLineWidth(width);
}

void GlStateManager::shadeModel(int mode)
{
    if (mode == shadeMode)
    {
        return;
    }

    shadeMode = mode;
}

int GlStateManager::genLists(int count)
{
    return gRenderer->CBuffCreate(count);
}

void GlStateManager::newList(int list, int mode)
{
    gRenderer->CBuffStart(list, false);
}

void GlStateManager::endList(int list)
{
    gRenderer->CBuffEnd();
}

void GlStateManager::callList(int list)
{
    gRenderer->CBuffCall(list, true, 0xFF);
}

void GlStateManager::callLists(IntBuffer* buffer)
{
    for (unsigned int index = 0; index < buffer->limit(); ++index)
    {
        gRenderer->CBuffCall(buffer->get(index), true, 0xFF);
    }
}

void GlStateManager::deleteLists(int list, int range)
{
    gRenderer->CBuffDelete(list, range);
}

void GlStateManager::disableClientState(int capability)
{
}

void GlStateManager::flush()
{
}

void GlStateManager::setProfile()
{
}

void GlStateManager::unsetProfile()
{
}
