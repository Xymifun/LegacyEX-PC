#ifndef GAME_MINIGAME_RANDOMIZESIZERULEDEFINITION_HPP
#define GAME_MINIGAME_RANDOMIZESIZERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"

class LayerSize;

class RandomizeSizeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerSize*>
{
public:
    RandomizeSizeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();

    virtual LayerSize* Generate() const;

private:
    float mAmount;
    LayerAttributeChildRuleDefinition<LayerSize*>* mChild;
};

#endif
