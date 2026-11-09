#ifndef GAME_WORLD_GENERATION_FUNCTIONPATCHESLAYERTHEMERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_FUNCTIONPATCHESLAYERTHEMERULEDEFINITION_HPP

#include "game/world/generation/FunctionPatchesLayerTheme.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"

class FunctionPatchesLayerThemeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerTheme*>
{
public:
    FunctionPatchesLayerThemeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    virtual LayerTheme* Generate() const;

private:
    int mFunction;
    double mNoiseScale;
};

#endif
