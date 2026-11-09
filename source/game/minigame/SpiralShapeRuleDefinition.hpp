#ifndef GAME_MINIGAME_SPIRALSHAPERULEDEFINITION_HPP
#define GAME_MINIGAME_SPIRALSHAPERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerShape.hpp"

class SpiralShapeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerShape*>
{
public:
    SpiralShapeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerShape* Generate() const;

protected:
    int mMinRadius;
    int mMaxRadius;
    double mAngle;
};

#endif
