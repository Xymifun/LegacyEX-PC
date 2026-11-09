#include "game/core/SpawnPositionSet.hpp"

SpawnPositionSet::SpawnPositionSet() : mType(0)
{
    addUnsignedCharAttribute(ConsoleGameRules::eGameRuleAttr_method, &mType);
}

ConsoleGameRules::EGameRuleType SpawnPositionSet::getActionType()
{
    return ConsoleGameRules::eGameRuleType_SpawnPositionSet;
}

unsigned char SpawnPositionSet::getType()
{
    return mType;
}
