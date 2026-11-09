#include "game/world/generation/FunctionPatchesLayerThemeRuleDefinition.hpp"

#include <string>

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

template <typename T>
std::wstring _toString(T value);

template <typename T>
T _fromString(const std::wstring& value);

static const std::wstring sFunctionNames[5] = {L"GrassPatches", L"Hellfire", L"Snowlayer", L"TntTraps", L"CarpetTrap"};

static const FunctionPatchesLayerTheme::BlockFunction sFunctions[] = {
    FunctionPatchesLayerTheme::GRASS_PATCHES, FunctionPatchesLayerTheme::HELLFIRE_PATCHES,
    FunctionPatchesLayerTheme::SNOWLAYER_PATCHES, FunctionPatchesLayerTheme::TNT_TRAPS,
    FunctionPatchesLayerTheme::CARPET_TRAP};

FunctionPatchesLayerThemeRuleDefinition::FunctionPatchesLayerThemeRuleDefinition() : mFunction(-1), mNoiseScale(20.0)
{
}

ConsoleGameRules::EGameRuleType FunctionPatchesLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_FunctionPatchesTheme;
}

void FunctionPatchesLayerThemeRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface,
                                                              unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 2);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_func, _toString(mFunction));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_scale, _toString(mNoiseScale));
}

void FunctionPatchesLayerThemeRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"func") == 0)
    {
        if (sFunctionNames[0].compare(value) == 0)
        {
            mFunction = 0;
        }

        if (sFunctionNames[1].compare(value) == 0)
        {
            mFunction = 1;
        }

        if (sFunctionNames[2].compare(value) == 0)
        {
            mFunction = 2;
        }

        if (sFunctionNames[3].compare(value) == 0)
        {
            mFunction = 3;
        }

        if (sFunctionNames[4].compare(value) == 0)
        {
            mFunction = 4;
        }
    }
    else if (name.compare(L"scale") == 0)
    {
        mNoiseScale = _fromString<double>(value);
    }
    else
    {
        GameRuleDefinition::addAttribute(name, value);
    }
}

LayerTheme* FunctionPatchesLayerThemeRuleDefinition::Generate() const
{
    return new FunctionPatchesLayerTheme(5, mNoiseScale, sFunctions[mFunction]);
}
