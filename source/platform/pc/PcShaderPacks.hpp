#ifndef PLATFORM_PC_PCSHADERPACKS_HPP
#define PLATFORM_PC_PCSHADERPACKS_HPP

#include <string>
#include <vector>

#include "platform/pc/shaderpack/ShaderUniforms.hpp"

class BlockEntity;
class Entity;
class GameRenderer;
class LevelRenderer;

void PcShaderPacksRequestPack(const std::string& name);

std::vector<std::string> PcShaderPacksList();
std::string PcShaderPacksCurrent();
void PcShaderPacksChoose(const std::string& name);

struct PcShaderMenuEntry
{
    enum Kind
    {
        Toggle,
        Choice,
        Profile,
        Page,
        Info
    };

    Kind kind;
    std::string key;
    std::wstring label;
    std::wstring valueLabel;
    bool checked;
    int value;
    int count;
};

bool PcShaderPacksHasOptions();
std::vector<PcShaderMenuEntry> PcShaderPacksPage(const std::string& page);
void PcShaderPacksSetToggle(const std::string& option, bool on);
void PcShaderPacksSetChoice(const std::string& option, int position);
void PcShaderPacksSetProfile(int position);
void PcShaderPacksResetOptions();
void PcShaderPacksApplyOptions();
int PcShaderPacksOptionsVersion();

void PcShaderPacksBeginFrame(bool eligible);
bool PcShaderPacksBeginWorld(GameRenderer* gameRenderer, float partialTick);
void PcShaderPacksRenderShadows(LevelRenderer* levelRenderer, float partialTick);
void PcShaderPacksEndWorld();
bool PcShaderPacksActive();
bool PcShaderPacksDrewWorld();

void PcShaderPacksSetStage(int stage);
void PcShaderPacksBeginTranslucent();
void PcShaderPacksBeginHand();
void PcShaderPacksEndHand();

void PcShaderPacksSetEntity(Entity* entity);
void PcShaderPacksSetBlockEntity(BlockEntity* blockEntity);
void PcShaderPacksClearEntity();

bool PcShaderPacksRenderHorizon();
bool PcShaderPacksRenderSun();
bool PcShaderPacksRenderMoon();
int PcShaderPacksCloudMode();
bool PcShaderPacksWeatherDepth();
bool PcShaderPacksVignette();
bool PcShaderPacksUnderwaterOverlay();

#endif
