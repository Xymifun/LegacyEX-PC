#include "game/minigame/StarShapeRuleDefinition.hpp"

#include "game/core/StarShape.hpp"

StarShapeRuleDefinition::StarShapeRuleDefinition() : mPoints(5)
{
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_points, &mPoints);
}

ConsoleGameRules::EGameRuleType StarShapeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_StarShape;
}

LayerShape* StarShapeRuleDefinition::Generate() const
{
    return new StarShape(mPoints);
}
