#ifndef GAME_WORLD_GENERATION_LAYERTHEMERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_LAYERTHEMERULEDEFINITION_HPP

#include <string>

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/world/generation/LayerAttributeRuleDefinition.hpp"
#include "game/world/generation/LayerTheme.hpp"

class LayerThemeRuleDefinition : public LayerAttributeRuleDefinition<LayerTheme*>
{
public:
    LayerThemeRuleDefinition();

    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);

    unsigned int GetPassId() const;

    static LayerAttributeChildRuleDefinition<LayerTheme*>* CreateFuncImpl(ConsoleGameRules::EGameRuleType type);

private:
    unsigned int mPassId;
};

#endif
