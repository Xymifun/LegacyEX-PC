#ifndef GAME_MINIGAME_STARSHAPERULEDEFINITION_HPP
#define GAME_MINIGAME_STARSHAPERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerShape.hpp"

class StarShapeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerShape*>
{
public:
    StarShapeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerShape* Generate() const;

protected:
    int mPoints;
};

#endif
