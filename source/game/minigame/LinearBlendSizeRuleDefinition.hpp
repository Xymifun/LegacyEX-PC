#ifndef GAME_MINIGAME_LINEARBLENDSIZERULEDEFINITION_HPP
#define GAME_MINIGAME_LINEARBLENDSIZERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"

class LayerSize;

class LinearBlendSizeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerSize*>
{
public:
    LinearBlendSizeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerSize* Generate() const;

private:
    int mMaxSize;
    int mMinSize;
};

#endif
