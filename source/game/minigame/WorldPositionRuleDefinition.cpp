#include "game/minigame/WorldPositionRuleDefinition.hpp"

#include "game/core/GameRules_IntAttribute.hpp"

WorldPositionRuleDefinition::WorldPositionRuleDefinition()
{
    mZ = 0;
    mY = 0;
    mX = 0;

    addIntAttribute(ConsoleGameRules::eGameRuleAttr_x, &mX);

    GameRules_IntAttribute* height = addIntAttribute(ConsoleGameRules::eGameRuleAttr_y, &mY);
    height->setMin(0);

    addIntAttribute(ConsoleGameRules::eGameRuleAttr_z, &mZ);
}

ConsoleGameRules::EGameRuleType WorldPositionRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_WorldPosition;
}
