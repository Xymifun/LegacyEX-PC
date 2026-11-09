#ifndef GAME_WORLD_GENERATION_UNIFORMLAYERSIZERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_UNIFORMLAYERSIZERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"

class LayerSize;

class UniformLayerSizeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerSize*>
{
public:
    UniformLayerSizeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerSize* Generate() const;

private:
    int mSize;
};

#endif
