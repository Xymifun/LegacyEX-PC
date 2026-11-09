#ifndef GAME_WORLD_GENERATION_LAYERSHAPERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_LAYERSHAPERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeRuleDefinition.hpp"
#include "game/world/generation/LayerShape.hpp"

class LayerShapeRuleDefinition : public LayerAttributeRuleDefinition<LayerShape*>
{
public:
    LayerShapeRuleDefinition();

    static LayerAttributeChildRuleDefinition<LayerShape*>* CreateFuncImpl(ConsoleGameRules::EGameRuleType type);
};

#endif
