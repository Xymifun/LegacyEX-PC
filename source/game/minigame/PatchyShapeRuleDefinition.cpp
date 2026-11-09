#include "game/minigame/PatchyShapeRuleDefinition.hpp"

#include "game/core/GameRules_DoubleAttribute.hpp"
#include "game/core/PatchyShape.hpp"

PatchyShapeRuleDefinition::PatchyShapeRuleDefinition() : mScale(5.0)
{
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_scale, &mScale)->setMin(1.0)->setMax(254.0);
}

ConsoleGameRules::EGameRuleType PatchyShapeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_PatchyShape;
}

LayerShape* PatchyShapeRuleDefinition::Generate() const
{
    return new PatchyShape(mScale);
}
