#ifndef GAME_WORLD_GENERATION_LAYERFILLRULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_LAYERFILLRULEDEFINITION_HPP

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/world/generation/LayerAttributeRuleDefinition.hpp"

class LayerFill;

class LayerFillRuleDefinition : public LayerAttributeRuleDefinition<LayerFill*>
{
public:
    LayerFillRuleDefinition();

private:
    static LayerAttributeChildRuleDefinition<LayerFill*>* CreateFuncImpl(ConsoleGameRules::EGameRuleType type);
};

#endif
