#include "platform/pc/PcRendererShaders.hpp"

namespace
{
const char COMMON_HEADER[] = "#version 330 core\n"
                             "layout(std140) uniform VertexRegisters { vec4 vr[41]; };\n"
                             "layout(std140) uniform BoneRegisters { vec4 br[215]; };\n"
                             "layout(std140) uniform PixelRegisters { vec4 pr[32]; };\n";

const char VERTEX_COMMON[] =
    "out vec4 vColour;\n"
    "out vec4 vTex;\n"
    "uniform sampler2D uVertexTexture;\n"
    "mat4 RegisterMatrix(int base) { return mat4(vr[base], vr[base + 1], vr[base + 2], vr[base + 3]); }\n"
    "vec4 ViewPosition(vec4 position) { return RegisterMatrix(0) * (RegisterMatrix(4) * position); }\n"
    "vec4 ClipPosition(vec4 view) { return RegisterMatrix(8) * view; }\n"
    "float FogFactor(vec4 view)\n"
    "{\n"
    "    vec4 fog = vr[17];\n"
    "    if (fog.z == 1.0)\n"
    "    {\n"
    "        float farDistance = abs(fog.x);\n"
    "        float nearDistance = abs(fog.y);\n"
    "        return clamp((farDistance - length(view.xyz)) / (farDistance - nearDistance), 0.0, 1.0);\n"
    "    }\n"
    "    if (fog.z == 2.0)\n"
    "        return clamp(exp2(view.z * 1.442695 * fog.x), 0.0, 1.0);\n"
    "    if (fog.z == 3.0)\n"
    "        return clamp(exp2(view.z * 1.442695 * (view.z * -fog.x * fog.x)), 0.0, 1.0);\n"
    "    return 1.0;\n"
    "}\n"
    "vec3 LightmapColour(int light)\n"
    "{\n"
    "    vec2 coordinates = vec2(float(light >> 16), float((light << 16) >> 16)) * 0.00390625;\n"
    "    coordinates = fract(max(coordinates, vr[16].xy));\n"
    "    return textureLod(uVertexTexture, coordinates, 0.0).rgb;\n"
    "}\n"
    "vec4 DirectionalLight(vec4 normal)\n"
    "{\n"
    "    normal = normal * inversesqrt(max(dot(normal, normal), 1e-20));\n"
    "    vec4 light = vr[22] + max(dot(normal, vr[18]), 0.0) * vr[20] + max(dot(normal, vr[19]), 0.0) * vr[21];\n"
    "    return clamp(light, 0.0, 1.0);\n"
    "}\n";

const char VERTEX_STANDARD[] =
    "layout(location = 0) in vec3 aPosition;\n"
    "layout(location = 1) in vec4 aColour;\n"
    "layout(location = 2) in vec4 aNormal;\n"
    "layout(location = 3) in vec2 aTexture;\n"
    "layout(location = 4) in int aLight;\n"
    "void main()\n"
    "{\n"
    "    vec4 view = ViewPosition(vec4(aPosition, 1.0));\n"
    "    gl_Position = ClipPosition(view);\n"
    "    vec4 colour = aColour.wzyx;\n"
    "    vColour = vec4(colour.rgb * LightmapColour(aLight), colour.a);\n"
    "    vTex = vec4((RegisterMatrix(12) * vec4(aTexture, 0.0, 1.0)).xy, FogFactor(view), 1.0);\n"
    "}\n";

const char VERTEX_STANDARD_LIGHTING[] =
    "layout(location = 0) in vec3 aPosition;\n"
    "layout(location = 1) in vec4 aColour;\n"
    "layout(location = 2) in vec4 aNormal;\n"
    "layout(location = 3) in vec2 aTexture;\n"
    "layout(location = 4) in int aLight;\n"
    "void main()\n"
    "{\n"
    "    vec4 view = ViewPosition(vec4(aPosition, 1.0));\n"
    "    gl_Position = ClipPosition(view);\n"
    "    vec4 colour = aColour.wzyx;\n"
    "    vec4 light = DirectionalLight(RegisterMatrix(0) * (RegisterMatrix(4) * aNormal));\n"
    "    vColour = vec4(colour.rgb * light.rgb * LightmapColour(aLight), colour.a * light.a);\n"
    "    vTex = vec4((RegisterMatrix(12) * vec4(aTexture, 0.0, 1.0)).xy, FogFactor(view), 1.0);\n"
    "}\n";

const char VERTEX_TEX_GEN[] =
    "layout(location = 0) in vec3 aPosition;\n"
    "layout(location = 1) in vec4 aColour;\n"
    "layout(location = 4) in int aLight;\n"
    "void main()\n"
    "{\n"
    "    vec4 object = RegisterMatrix(4) * vec4(aPosition, 1.0);\n"
    "    vec4 view = RegisterMatrix(0) * object;\n"
    "    gl_Position = ClipPosition(view);\n"
    "    vec4 colour = aColour.wzyx;\n"
    "    vColour = vec4(colour.rgb * LightmapColour(aLight), colour.a);\n"
    "    vec4 generated = RegisterMatrix(23) * view + RegisterMatrix(27) * object;\n"
    "    vTex = vec4((RegisterMatrix(12) * generated).xy, FogFactor(view), 1.0);\n"
    "}\n";

const char VERTEX_TERRAIN[] =
    "layout(location = 0) in ivec4 aPosition;\n"
    "layout(location = 1) in ivec4 aTexture;\n"
    "layout(location = 5) in vec4 aChunkOffset;\n"
    "void main()\n"
    "{\n"
    "    vec3 position = vec3(aPosition.xyz) * 0.0009765625;\n"
    "    position.x += float(aTexture.w);\n"
    "    position.yz += aChunkOffset.yz;\n"
    "    vec4 view = RegisterMatrix(0) * vec4(position, 1.0);\n"
    "    gl_Position = ClipPosition(view);\n"
    "    float packedColour = float(aPosition.w) + 32768.0;\n"
    "    vec3 colour = vec3(fract(packedColour * 3.0517578e-05), fract(packedColour * 0.0009765625), fract(packedColour * 0.03125));\n"
    "    float alpha = 1.0;\n"
    "    if (fract(packedColour * 1.5258789e-05) >= 0.5)\n"
    "    {\n"
    "        float distance = length(view.xyz);\n"
    "        float alphaDistance = abs(vr[17].w);\n"
    "        float fade = alphaDistance > 0.0 ? clamp((alphaDistance - distance) / alphaDistance, 0.0, 1.0) : 0.0;\n"
    "        alpha = (1.0 - vr[40].x) * (1.0 - fade) + vr[40].x;\n"
    "    }\n"
    "    float light = float(aTexture.z);\n"
    "    vec3 lightmap = textureLod(uVertexTexture, vec2(fract(light * 1.5258789e-05), fract(light * 0.00390625)), 0.0).rgb;\n"
    "    vColour = vec4(colour * lightmap, alpha);\n"
    "    vTex = vec4(vec2(aTexture.xy) * 0.00012207031, FogFactor(view), 1.0);\n"
    "}\n";

const char VERTEX_UI[] = "layout(location = 0) in vec2 aPosition;\n"
                         "void main()\n"
                         "{\n"
                         "    gl_Position = ClipPosition(ViewPosition(vec4(aPosition, 0.0, 1.0)));\n"
                         "    vColour = vec4(1.0);\n"
                         "    vTex = vec4(aPosition * vr[33].xy + vr[33].zw, 0.0, 0.0);\n"
                         "}\n";

const char VERTEX_SCALING_GRID_UI[] =
    "layout(location = 0) in vec2 aPosition;\n"
    "float GridAxis(float value, vec4 edges, float scale)\n"
    "{\n"
    "    float middle = (edges.y + edges.z) * 0.5;\n"
    "    if (edges.y >= value)\n"
    "        return min((value - edges.x) / scale + edges.x, middle);\n"
    "    if (value >= edges.z)\n"
    "        return max((value - edges.w) / scale + edges.w, middle);\n"
    "    return value;\n"
    "}\n"
    "void main()\n"
    "{\n"
    "    vec2 position = vec2(GridAxis(aPosition.x, vr[34], vr[36].x), GridAxis(aPosition.y, vr[35], vr[36].y));\n"
    "    gl_Position = ClipPosition(ViewPosition(vec4(position, 0.0, 1.0)));\n"
    "    vColour = vec4(1.0);\n"
    "    vTex = vec4(aPosition * vr[33].xy + vr[33].zw, 0.0, 0.0);\n"
    "}\n";

const char VERTEX_COLOURED_UI[] = "layout(location = 0) in vec2 aPosition;\n"
                                 "layout(location = 1) in vec2 aTexture;\n"
                                 "layout(location = 2) in vec4 aColour;\n"
                                 "void main()\n"
                                 "{\n"
                                 "    gl_Position = ClipPosition(ViewPosition(vec4(aPosition, 0.0, 1.0)));\n"
                                 "    vColour = aColour.wzyx;\n"
                                 "    vTex = vec4(aTexture, 0.0, 0.0);\n"
                                 "}\n";

const char VERTEX_ENTITY[] =
    "layout(location = 0) in vec3 aPosition;\n"
    "layout(location = 1) in vec4 aColour;\n"
    "layout(location = 2) in vec4 aNormal;\n"
    "layout(location = 3) in vec2 aTexture;\n"
    "layout(location = 4) in int aLight;\n"
    "uniform bool uLighting;\n"
    "void main()\n"
    "{\n"
    "    int bone = int(aNormal.w * 127.0 + 0.5);\n"
    "    mat4 boneMatrix = mat4(br[bone * 4], br[bone * 4 + 1], br[bone * 4 + 2], br[bone * 4 + 3]);\n"
    "    vec4 view = RegisterMatrix(4) * (boneMatrix * vec4(aPosition, 1.0));\n"
    "    gl_Position = ClipPosition(view);\n"
    "    vec4 colour = aColour.wzyx;\n"
    "    vec4 light = vec4(1.0);\n"
    "    if (uLighting)\n"
    "        light = DirectionalLight(RegisterMatrix(4) * (boneMatrix * vec4(aNormal.xyz, 0.0)));\n"
    "    vColour = vec4(colour.rgb * light.rgb * LightmapColour(aLight), colour.a * light.a);\n"
    "    vTex = vec4((RegisterMatrix(12) * vec4(aTexture, 0.0, 1.0)).xy, FogFactor(view), 1.0);\n"
    "}\n";

const char PIXEL_COMMON[] = "in vec4 vColour;\n"
                            "in vec4 vTex;\n"
                            "out vec4 oColour;\n"
                            "uniform sampler2D uTexture0;\n"
                            "uniform sampler2D uTexture1;\n"
                            "uniform sampler2D uTexture2;\n"
                            "uniform sampler2D uTexture3;\n"
                            "uniform float uAlphaReference;\n"
                            "vec4 SampleDiffuse(sampler2D sampler, vec2 coordinates)\n"
                            "{\n"
                            "    if (coordinates.x > 1.0)\n"
                            "        return textureLod(sampler, coordinates, 0.0);\n"
                            "    return texture(sampler, coordinates);\n"
                            "}\n"
                            "vec3 ApplyFog(vec3 colour) { return mix(pr[1].rgb, colour, vTex.z); }\n"
                            "void Finish(vec4 colour)\n"
                            "{\n"
                            "    if (colour.a < uAlphaReference)\n"
                            "        discard;\n"
                            "    oColour = colour;\n"
                            "}\n";

const char PIXEL_DEFAULT[] = "uniform int uLayers;\n"
                             "uniform bool uForceLod;\n"
                             "vec4 OverLayer(vec4 below, vec4 above)\n"
                             "{\n"
                             "    return vec4(mix(below.rgb, above.rgb, above.a), above.a + below.a * (1.0 - above.a));\n"
                             "}\n"
                             "void main()\n"
                             "{\n"
                             "    vec4 colour = (uForceLod ? textureLod(uTexture0, vTex.xy, pr[8].x)\n"
                             "                             : SampleDiffuse(uTexture0, vTex.xy)) * pr[4];\n"
                             "    if (uLayers > 1) colour = OverLayer(colour, texture(uTexture1, vTex.xy) * pr[5]);\n"
                             "    if (uLayers > 2) colour = OverLayer(colour, texture(uTexture2, vTex.xy) * pr[6]);\n"
                             "    if (uLayers > 3) colour = OverLayer(colour, texture(uTexture3, vTex.xy) * pr[7]);\n"
                             "    colour.rgb = mix(colour.rgb, pr[2].rgb, pr[2].a);\n"
                             "    colour *= vColour * pr[0];\n"
                             "    Finish(vec4(ApplyFog(colour.rgb), colour.a));\n"
                             "}\n";

const char PIXEL_TERRAIN[] =
    "void main()\n"
    "{\n"
    "    vec4 texel = SampleDiffuse(uTexture0, vTex.xy);\n"
    "    if (texel.r + texel.g + texel.b + texel.a == 0.0)\n"
    "        discard;\n"
    "    vec4 tint = (vColour - 1.0) * texel.a + 1.0;\n"
    "    vec3 colour = texel.rgb * pr[4].rgb * pr[0].rgb * tint.rgb;\n"
    "    Finish(vec4(ApplyFog(colour), texel.a * tint.a * pr[0].a));\n"
    "}\n";

const char PIXEL_LAYERED_COLOUR_MASK[] =
    "void main()\n"
    "{\n"
    "    vec4 colour = SampleDiffuse(uTexture0, vTex.xy);\n"
    "    ivec2 atlasSize = ivec2(pr[9].xy);\n"
    "    ivec2 pixel = ivec2(floor(vTex.xy * vec2(textureSize(uTexture0, 0))));\n"
    "    vec3 tint = vec3(1.0);\n"
    "    for (int layer = 0; layer < 7; ++layer)\n"
    "    {\n"
    "        float cellCode = pr[10 + layer].a;\n"
    "        ivec2 cell = ivec2(int(floor(cellCode)), int(floor(fract(cellCode) * pr[9].y + 0.5)));\n"
    "        ivec2 at = clamp(cell + pixel, ivec2(0), atlasSize - 1);\n"
    "        float mask = texelFetch(uTexture1, at, 0).r;\n"
    "        tint = mix(tint, pr[10 + layer].rgb, mask);\n"
    "    }\n"
    "    colour.rgb *= tint;\n"
    "    colour *= vColour * pr[0];\n"
    "    Finish(vec4(ApplyFog(colour.rgb), colour.a));\n"
    "}\n";

const char PIXEL_UI_COLOUR_ADD[] = "void main()\n"
                                 "{\n"
                                 "    vec4 colour = texture(uTexture0, vTex.xy);\n"
                                 "    colour.rgb *= pr[4].rgb;\n"
                                 "    colour = colour * pr[0] + pr[3];\n"
                                 "    Finish(vec4(colour.rgb * colour.a, colour.a));\n"
                                 "}\n";

const char PIXEL_UI[] = "void main()\n"
                        "{\n"
                        "    vec4 texel = texture(uTexture0, vTex.xy);\n"
                        "    float alpha = vColour.a * texel.a * pr[0].a;\n"
                        "    vec3 colour = vColour.rgb * texel.rgb * pr[4].rgb * pr[0].rgb;\n"
                        "    Finish(vec4(colour * alpha, alpha));\n"
                        "}\n";

const char PIXEL_ENTITY_CHANGE_COLOUR[] = "void main()\n"
                                        "{\n"
                                        "    vec4 colour = SampleDiffuse(uTexture0, vTex.xy) * pr[4];\n"
                                        "    colour.rgb = mix(colour.rgb, pr[2].rgb, pr[2].a);\n"
                                        "    colour *= vColour * pr[0];\n"
                                        "    Finish(vec4(ApplyFog(colour.rgb), colour.a));\n"
                                        "}\n";

const char PIXEL_ENTITY_EMISSIVE_ALPHA[] = "void main()\n"
                                         "{\n"
                                         "    vec4 texel = SampleDiffuse(uTexture0, vTex.xy);\n"
                                         "    if (texel.a <= 0.0)\n"
                                         "        discard;\n"
                                         "    vec3 light = mix(vColour.rgb, vec3(1.0), 1.0 - texel.a);\n"
                                         "    vec3 colour = mix(texel.rgb * pr[4].rgb, pr[2].rgb, pr[2].a);\n"
                                         "    colour *= light * pr[0].rgb;\n"
                                         "    Finish(vec4(ApplyFog(colour), vColour.a * pr[4].a * pr[0].a));\n"
                                         "}\n";

const char PIXEL_ENTITY_EMISSIVE_ALPHA_ONLY[] = "void main()\n"
                                             "{\n"
                                             "    vec4 texel = SampleDiffuse(uTexture0, vTex.xy);\n"
                                             "    float glow = texel.a > 0.0 ? 1.0 - texel.a : 0.0;\n"
                                             "    vec3 colour = texel.rgb * pr[4].rgb * pr[0].rgb;\n"
                                             "    Finish(vec4(ApplyFog(colour), glow * pr[4].a * pr[0].a));\n"
                                             "}\n";

const char PRESENT_VERTEX[] = "#version 330 core\n"
                              "out vec2 vCoordinates;\n"
                              "void main()\n"
                              "{\n"
                              "    vec2 corner = vec2(float(gl_VertexID & 1), float(gl_VertexID >> 1));\n"
                              "    vCoordinates = corner;\n"
                              "    gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);\n"
                              "}\n";

const char CURSOR_PIXEL[] = "#version 330 core\n"
                            "in vec2 vCoordinates;\n"
                            "out vec4 oColour;\n"
                            "uniform sampler2D uImage;\n"
                            "void main()\n"
                            "{\n"
                            "    oColour = texture(uImage, vec2(vCoordinates.x, 1.0 - vCoordinates.y));\n"
                            "}\n";

const char PANEL_VERTEX[] = "#version 330 core\n"
                            "out vec2 vCoordinates;\n"
                            "uniform mat4 uMatrix;\n"
                            "uniform vec4 uSource;\n"
                            "void main()\n"
                            "{\n"
                            "    vec2 corner = vec2(float(gl_VertexID & 1), float(gl_VertexID >> 1));\n"
                            "    vCoordinates = uSource.xy + corner * uSource.zw;\n"
                            "    gl_Position = uMatrix * vec4(corner * 2.0 - 1.0, 0.0, 1.0);\n"
                            "}\n";

const char PANEL_PIXEL[] = "#version 330 core\n"
                           "in vec2 vCoordinates;\n"
                           "out vec4 oColour;\n"
                           "uniform sampler2D uImage;\n"
                           "uniform float uOpaque;\n"
                           "uniform vec4 uCuts[4];\n"
                           "uniform int uCutCount;\n"
                           "void main()\n"
                           "{\n"
                           "    for (int i = 0; i < uCutCount; ++i)\n"
                           "    {\n"
                           "        if (all(greaterThanEqual(vCoordinates, uCuts[i].xy)) && all(lessThan(vCoordinates, uCuts[i].zw)))\n"
                           "            discard;\n"
                           "    }\n"
                           "    vec4 colour = texture(uImage, vCoordinates);\n"
                           "    float alpha = uOpaque > 0.5 ? 1.0 : sqrt(colour.a);\n"
                           "    if (alpha < 0.02)\n"
                           "        discard;\n"
                           "    oColour = vec4(colour.rgb, alpha);\n"
                           "}\n";

const char BOX_VERTEX[] = "#version 330 core\n"
                          "out float vShade;\n"
                          "uniform mat4 uMatrix;\n"
                          "const vec3 kCorners[8] = vec3[8](vec3(-1,-1,-1), vec3(1,-1,-1), vec3(1,1,-1), vec3(-1,1,-1),\n"
                          "                                 vec3(-1,-1,1), vec3(1,-1,1), vec3(1,1,1), vec3(-1,1,1));\n"
                          "const int kFaces[36] = int[36](0,2,1, 0,3,2, 4,5,6, 4,6,7, 0,1,5, 0,5,4,\n"
                          "                               3,7,6, 3,6,2, 0,4,7, 0,7,3, 1,2,6, 1,6,5);\n"
                          "const float kShades[6] = float[6](0.8, 0.8, 0.6, 1.0, 0.7, 0.7);\n"
                          "void main()\n"
                          "{\n"
                          "    vShade = kShades[gl_VertexID / 6];\n"
                          "    gl_Position = uMatrix * vec4(kCorners[kFaces[gl_VertexID]], 1.0);\n"
                          "}\n";

const char BOX_PIXEL[] = "#version 330 core\n"
                         "in float vShade;\n"
                         "out vec4 oColour;\n"
                         "uniform vec4 uColour;\n"
                         "void main()\n"
                         "{\n"
                         "    oColour = vec4(uColour.rgb * vShade * uColour.a, uColour.a);\n"
                         "}\n";

const char PRESENT_OVER_PIXEL[] = "#version 330 core\n"
                                 "in vec2 vCoordinates;\n"
                                 "out vec4 oColour;\n"
                                 "uniform sampler2D uScreen;\n"
                                 "uniform float uGamma;\n"
                                 "void main()\n"
                                 "{\n"
                                 "    vec4 colour = texture(uScreen, vCoordinates);\n"
                                 "    oColour = vec4(pow(max(colour.rgb, vec3(0.0)), vec3(uGamma)), sqrt(colour.a));\n"
                                 "}\n";

const char PRESENT_PIXEL[] = "#version 330 core\n"
                             "in vec2 vCoordinates;\n"
                             "out vec4 oColour;\n"
                             "uniform sampler2D uScreen;\n"
                             "uniform float uGamma;\n"
                             "void main()\n"
                             "{\n"
                             "    vec3 colour = texture(uScreen, vCoordinates).rgb;\n"
                             "    oColour = vec4(pow(max(colour, vec3(0.0)), vec3(uGamma)), 1.0);\n"
                             "}\n";
}

const char* PcShaderCommonHeader()
{
    return COMMON_HEADER;
}

const char* PcShaderVertexCommon()
{
    return VERTEX_COMMON;
}

const char* PcShaderPixelCommon()
{
    return PIXEL_COMMON;
}

const char* PcShaderVertexSource(PcVertexShader sh)
{
    switch (sh)
    {
    case ePcVertexShader_Standard:
        return VERTEX_STANDARD;
    case ePcVertexShader_Terrain:
        return VERTEX_TERRAIN;
    case ePcVertexShader_StandardLighting:
        return VERTEX_STANDARD_LIGHTING;
    case ePcVertexShader_TexGen:
        return VERTEX_TEX_GEN;
    case ePcVertexShader_Ui:
    case ePcVertexShader_TiledUi:
        return VERTEX_UI;
    case ePcVertexShader_ScalingGridUi:
        return VERTEX_SCALING_GRID_UI;
    case ePcVertexShader_ColouredUi:
        return VERTEX_COLOURED_UI;
    case ePcVertexShader_Entity:
    case ePcVertexShader_EntityLighting:
        return VERTEX_ENTITY;
    default:
        return VERTEX_STANDARD;
    }
}

const char* PcShaderPixelSource(PcPixelShader sh)
{
    switch (sh)
    {
    case ePcPixelShader_Default:
    case ePcPixelShader_OrderIndependentAlpha:
    case ePcPixelShader_TerrainForceLod:
        return PIXEL_DEFAULT;
    case ePcPixelShader_Terrain:
        return PIXEL_TERRAIN;
    case ePcPixelShader_LayeredColourMask:
        return PIXEL_LAYERED_COLOUR_MASK;
    case ePcPixelShader_UiColourAdd:
        return PIXEL_UI_COLOUR_ADD;
    case ePcPixelShader_Ui:
    case ePcPixelShader_UiAlphaTest:
        return PIXEL_UI;
    case ePcPixelShader_EntityChangeColour:
        return PIXEL_ENTITY_CHANGE_COLOUR;
    case ePcPixelShader_EntityEmissiveAlpha:
        return PIXEL_ENTITY_EMISSIVE_ALPHA;
    case ePcPixelShader_EntityEmissiveAlphaOnly:
        return PIXEL_ENTITY_EMISSIVE_ALPHA_ONLY;
    default:
        return PIXEL_DEFAULT;
    }
}

const char* PcShaderPresentVertex()
{
    return PRESENT_VERTEX;
}

const char* PcShaderCursorPixel()
{
    return CURSOR_PIXEL;
}

const char* PcShaderPresentPixel()
{
    return PRESENT_PIXEL;
}

const char* PcShaderPresentOverPixel()
{
    return PRESENT_OVER_PIXEL;
}

const char* PcShaderPanelVertex()
{
    return PANEL_VERTEX;
}

const char* PcShaderPanelPixel()
{
    return PANEL_PIXEL;
}

const char* PcShaderBoxVertex()
{
    return BOX_VERTEX;
}

const char* PcShaderBoxPixel()
{
    return BOX_PIXEL;
}
