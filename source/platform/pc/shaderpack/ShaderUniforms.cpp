// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderUniforms.cpp
#include "platform/pc/shaderpack/ShaderUniforms.hpp"

#include <cstring>

namespace
{
const char* const kRenderStages[ShaderRenderStage_Count] = {
    "NONE",          "SKY",         "SUNSET",     "CUSTOM_SKY",    "SUN",
    "MOON",          "STARS",       "VOID",       "TERRAIN_SOLID", "TERRAIN_CUTOUT_MIPPED",
    "TERRAIN_CUTOUT", "ENTITIES",   "BLOCK_ENTITIES", "DESTROY",   "OUTLINE",
    "DEBUG",         "HAND_SOLID",  "TERRAIN_TRANSLUCENT", "TRIPWIRE", "PARTICLES",
    "CLOUDS",        "RAIN_SNOW",   "WORLD_BORDER", "HAND_TRANSLUCENT"};

const ShaderUniformInfo kUniforms[ShaderUniform_Count] = {
    {"heldItemId", ShaderUniformType_Int, false},
    {"heldBlockLightValue", ShaderUniformType_Int, false},
    {"heldItemId2", ShaderUniformType_Int, false},
    {"heldBlockLightValue2", ShaderUniformType_Int, false},
    {"fogMode", ShaderUniformType_Int, true},
    {"fogStart", ShaderUniformType_Float, true},
    {"fogEnd", ShaderUniformType_Float, true},
    {"fogDensity", ShaderUniformType_Float, true},
    {"fogColor", ShaderUniformType_Vec3, true},
    {"skyColor", ShaderUniformType_Vec3, false},
    {"worldTime", ShaderUniformType_Int, false},
    {"worldDay", ShaderUniformType_Int, false},
    {"moonPhase", ShaderUniformType_Int, false},
    {"frameCounter", ShaderUniformType_Int, false},
    {"frameTime", ShaderUniformType_Float, false},
    {"frameTimeCounter", ShaderUniformType_Float, false},
    {"sunAngle", ShaderUniformType_Float, false},
    {"shadowAngle", ShaderUniformType_Float, false},
    {"rainStrength", ShaderUniformType_Float, false},
    {"thunderStrength", ShaderUniformType_Float, false},
    {"aspectRatio", ShaderUniformType_Float, false},
    {"viewWidth", ShaderUniformType_Float, false},
    {"viewHeight", ShaderUniformType_Float, false},
    {"near", ShaderUniformType_Float, false},
    {"far", ShaderUniformType_Float, false},
    {"sunPosition", ShaderUniformType_Vec3, false},
    {"moonPosition", ShaderUniformType_Vec3, false},
    {"shadowLightPosition", ShaderUniformType_Vec3, false},
    {"upPosition", ShaderUniformType_Vec3, false},
    {"cameraPosition", ShaderUniformType_Vec3, false},
    {"previousCameraPosition", ShaderUniformType_Vec3, false},
    {"gbufferModelView", ShaderUniformType_Mat4, false},
    {"gbufferModelViewInverse", ShaderUniformType_Mat4, false},
    {"gbufferPreviousModelView", ShaderUniformType_Mat4, false},
    {"gbufferProjection", ShaderUniformType_Mat4, false},
    {"gbufferProjectionInverse", ShaderUniformType_Mat4, false},
    {"gbufferPreviousProjection", ShaderUniformType_Mat4, false},
    {"shadowProjection", ShaderUniformType_Mat4, false},
    {"shadowProjectionInverse", ShaderUniformType_Mat4, false},
    {"shadowModelView", ShaderUniformType_Mat4, false},
    {"shadowModelViewInverse", ShaderUniformType_Mat4, false},
    {"wetness", ShaderUniformType_Float, false},
    {"eyeAltitude", ShaderUniformType_Float, false},
    {"eyeBrightness", ShaderUniformType_IVec2, false},
    {"eyeBrightnessSmooth", ShaderUniformType_IVec2, false},
    {"terrainTextureSize", ShaderUniformType_IVec2, false},
    {"terrainIconSize", ShaderUniformType_Int, false},
    {"atlasSize", ShaderUniformType_IVec2, false},
    {"isEyeInWater", ShaderUniformType_Int, false},
    {"nightVision", ShaderUniformType_Float, false},
    {"blindness", ShaderUniformType_Float, false},
    {"darknessFactor", ShaderUniformType_Float, false},
    {"darknessLightFactor", ShaderUniformType_Float, false},
    {"screenBrightness", ShaderUniformType_Float, false},
    {"hideGUI", ShaderUniformType_Int, false},
    {"centerDepthSmooth", ShaderUniformType_Float, false},
    {"playerMood", ShaderUniformType_Float, false},
    {"bossBattle", ShaderUniformType_Int, false},
    {"cameraPositionInt", ShaderUniformType_IVec3, false},
    {"cameraPositionFract", ShaderUniformType_Vec3, false},
    {"previousCameraPositionInt", ShaderUniformType_IVec3, false},
    {"previousCameraPositionFract", ShaderUniformType_Vec3, false},
    {"eyePosition", ShaderUniformType_Vec3, false},
    {"relativeEyePosition", ShaderUniformType_Vec3, false},
    {"playerLookVector", ShaderUniformType_Vec3, false},
    {"playerBodyVector", ShaderUniformType_Vec3, false},
    {"currentPlayerHealth", ShaderUniformType_Float, false},
    {"maxPlayerHealth", ShaderUniformType_Float, false},
    {"currentPlayerHunger", ShaderUniformType_Float, false},
    {"maxPlayerHunger", ShaderUniformType_Float, false},
    {"currentPlayerAir", ShaderUniformType_Float, false},
    {"maxPlayerAir", ShaderUniformType_Float, false},
    {"currentPlayerArmor", ShaderUniformType_Float, false},
    {"maxPlayerArmor", ShaderUniformType_Float, false},
    {"isSpectator", ShaderUniformType_Int, false},
    {"firstPersonCamera", ShaderUniformType_Int, false},
    {"isRightHanded", ShaderUniformType_Int, false},
    {"isElytraFlying", ShaderUniformType_Int, false},
    {"lightningBoltPosition", ShaderUniformType_Vec4, false},
    {"endFlashIntensity", ShaderUniformType_Float, false},
    {"previousEndFlashIntensity", ShaderUniformType_Float, false},
    {"endFlashPosition", ShaderUniformType_Vec3, false},
    {"cloudHeight", ShaderUniformType_Float, false},
    {"heightLimit", ShaderUniformType_Int, false},
    {"bedrockLevel", ShaderUniformType_Int, false},
    {"logicalHeightLimit", ShaderUniformType_Int, false},
    {"biome", ShaderUniformType_Int, false},
    {"biome_category", ShaderUniformType_Int, false},
    {"biome_precipitation", ShaderUniformType_Int, false},
    {"temperature", ShaderUniformType_Float, false},
    {"rainfall", ShaderUniformType_Float, false},
    {"is_alive", ShaderUniformType_Int, false},
    {"is_burning", ShaderUniformType_Int, false},
    {"is_child", ShaderUniformType_Int, false},
    {"is_glowing", ShaderUniformType_Int, false},
    {"is_hurt", ShaderUniformType_Int, false},
    {"is_in_lava", ShaderUniformType_Int, false},
    {"is_in_water", ShaderUniformType_Int, false},
    {"is_invisible", ShaderUniformType_Int, false},
    {"is_on_ground", ShaderUniformType_Int, false},
    {"is_ridden", ShaderUniformType_Int, false},
    {"is_riding", ShaderUniformType_Int, false},
    {"is_sneaking", ShaderUniformType_Int, false},
    {"is_sprinting", ShaderUniformType_Int, false},
    {"is_wet", ShaderUniformType_Int, false},
    {"renderStage", ShaderUniformType_Int, true},
    {"alphaTestRef", ShaderUniformType_Float, true},
    {"entityColor", ShaderUniformType_Vec4, true},
    {"entityId", ShaderUniformType_Int, true},
    {"blockEntityId", ShaderUniformType_Int, true},
    {"blendFunc", ShaderUniformType_IVec4, true},
    {"instanceId", ShaderUniformType_Int, true},
    {"lce_ProjectionFix", ShaderUniformType_Mat4, true},
    {"lce_CameraView", ShaderUniformType_Mat4, false},
    {"lce_CameraViewInverse", ShaderUniformType_Mat4, false},
    {"lce_AlphaTest", ShaderUniformType_Vec2, true},
    {"lce_FlipY", ShaderUniformType_Float, true},
    {"lce_FogColorValue", ShaderUniformType_Vec4, true},
    {"lce_FogValues", ShaderUniformType_Vec4, true},
    {"lce_QuadDataMode", ShaderUniformType_Int, true},
    {"lce_LightSource[0].position", ShaderUniformType_Vec4, true},
    {"lce_LightSource[0].diffuse", ShaderUniformType_Vec4, true},
    {"lce_LightSource[1].position", ShaderUniformType_Vec4, true},
    {"lce_LightSource[1].diffuse", ShaderUniformType_Vec4, true},
    {"lce_LightModel.ambient", ShaderUniformType_Vec4, true},
    {"lce_ColourScale", ShaderUniformType_Vec4, true},
    {"lce_FirstVertex", ShaderUniformType_Int, true},
    {"lce_QuadVertexMode", ShaderUniformType_Int, true},
    // LegacyEX: 1 while chunk sections draw from the renderer's shared terrain buffers (PcTerrainArena)
    {"lce_ChunkOffsetMode", ShaderUniformType_Int, true},
};
}

const char* ShaderRenderStageName(int stage)
{
    return kRenderStages[stage];
}

const ShaderUniformInfo& ShaderUniformCatalogue(int id)
{
    return kUniforms[id];
}

int ShaderFindUniform(const char* name)
{
    for (int id = 0; id < ShaderUniform_Count; ++id)
    {
        if (std::strcmp(kUniforms[id].name, name) == 0)
            return id;
    }
    return -1;
}
