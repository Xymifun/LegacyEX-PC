#include "game/minigame/SpiralShapeRuleDefinition.hpp"

#include "game/core/GameRules_DoubleAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/core/SpiralShape.hpp"

SpiralShapeRuleDefinition::SpiralShapeRuleDefinition()
{
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_minCount, &mMinRadius)->setMin(1)->setMax(24);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_maxCount, &mMaxRadius)->setMin(1)->setMax(24);

    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_rot, &mAngle)->setMin(0.0)->setMax(360.0);
}

ConsoleGameRules::EGameRuleType SpiralShapeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_SpiralShape;
}

LayerShape* SpiralShapeRuleDefinition::Generate() const
{
    return new SpiralShape(mMinRadius, mMaxRadius, mAngle * 3.1459 / 360.0);
}
