#include "game/minigame/ActiveViewAreaRuleDefinition.hpp"

ActiveViewAreaRuleDefinition::ActiveViewAreaRuleDefinition()
{
}

ActiveViewAreaRuleDefinition::~ActiveViewAreaRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType ActiveViewAreaRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_ActiveViewArea;
}
