#ifndef GAME_MINIGAME_RINGSHAPERULEDEFINITION_HPP
#define GAME_MINIGAME_RINGSHAPERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerShape.hpp"

class RingShapeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerShape*>
{
public:
    RingShapeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual LayerShape* Generate() const;

private:
    double mScale;
    LayerAttributeChildRuleDefinition<LayerShape*>* mChild;
};

#endif
