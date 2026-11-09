#include "game/world/chunk/ActiveChunkAreaRuleDefinition.hpp"

ActiveChunkAreaRuleDefinition::ActiveChunkAreaRuleDefinition()
{
}

ActiveChunkAreaRuleDefinition::~ActiveChunkAreaRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType ActiveChunkAreaRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_ActiveChunkArea;
}
