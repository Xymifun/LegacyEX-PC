#ifndef PLATFORM_PC_PCRENDERERSHADERS_HPP
#define PLATFORM_PC_PCRENDERERSHADERS_HPP

enum PcVertexShader
{
    ePcVertexShader_Standard,
    ePcVertexShader_Terrain,
    ePcVertexShader_StandardLighting,
    ePcVertexShader_TexGen,
    ePcVertexShader_Ui,
    ePcVertexShader_ScalingGridUi,
    ePcVertexShader_TiledUi,
    ePcVertexShader_ColouredUi,
    ePcVertexShader_Entity,
    ePcVertexShader_EntityLighting,
    ePcVertexShader_Count
};

enum PcPixelShader
{
    ePcPixelShader_Default,
    ePcPixelShader_DefaultTwoLayers,
    ePcPixelShader_DefaultThreeLayers,
    ePcPixelShader_DefaultFourLayers,
    ePcPixelShader_Terrain,
    ePcPixelShader_TerrainForceLod,
    ePcPixelShader_OrderIndependentAlpha,
    ePcPixelShader_LayeredColourMask,
    ePcPixelShader_UiColourAdd,
    ePcPixelShader_Ui,
    ePcPixelShader_UiAlphaTest,
    ePcPixelShader_EntityChangeColour,
    ePcPixelShader_EntityEmissiveAlpha,
    ePcPixelShader_EntityEmissiveAlphaOnly,
    ePcPixelShader_Count
};

const char* PcShaderCommonHeader();
const char* PcShaderVertexCommon();
const char* PcShaderPixelCommon();
const char* PcShaderVertexSource(PcVertexShader shader);
const char* PcShaderPixelSource(PcPixelShader shader);
const char* PcShaderPresentVertex();
const char* PcShaderPresentPixel();
const char* PcShaderPresentOverPixel();
const char* PcShaderPanelVertex();
const char* PcShaderPanelPixel();
const char* PcShaderBoxVertex();
const char* PcShaderBoxPixel();
const char* PcShaderCursorPixel();

#endif
