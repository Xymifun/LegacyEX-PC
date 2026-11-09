#ifndef GAME_WORLD_GENERATION_LAYERSIZERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_LAYERSIZERULEDEFINITION_HPP

#include "game/core/LayerSize.hpp"
#include "game/world/generation/LayerAttributeRuleDefinition.hpp"

class LayerSizeRuleDefinition : public LayerAttributeRuleDefinition<LayerSize*>
{
public:
    LayerSizeRuleDefinition();

private:
    static LayerAttributeChildRuleDefinition<LayerSize*>* CreateFuncImpl(ConsoleGameRules::EGameRuleType type);
};

#endif
