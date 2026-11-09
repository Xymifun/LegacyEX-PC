#ifndef GAME_WORLD_GENERATION_BASICLAYERFILLRULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_BASICLAYERFILLRULEDEFINITION_HPP

#include <string>

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"

class LayerFill;

class BasicLayerFillRuleDefinition : public LayerAttributeChildRuleDefinition<LayerFill*>
{
public:
    BasicLayerFillRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();

    virtual LayerFill* Generate() const;

private:
    std::wstring mType;
};

#endif
