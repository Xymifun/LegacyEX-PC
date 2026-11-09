#include "game/core/OnGameStartSpawnPositions.hpp"

#include "game/core/SpawnPositionSet.hpp"

ConsoleGameRules::EGameRuleType OnGameStartSpawnPositions::getActionType()
{
    return ConsoleGameRules::eGameRuleType_OnGameStartSpawnPositions;
}

GameRuleDefinition* OnGameStartSpawnPositions::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* pChild = NULL;

    if (type == ConsoleGameRules::eGameRuleType_SpawnPositionSet)
    {
        SpawnPositionSet* pSet = new SpawnPositionSet();
        pChild = pSet;
        mChildren.push_back(pSet);
    }

    return pChild;
}

SpawnPositionSet* OnGameStartSpawnPositions::getSpawnSet(unsigned char type)
{
    std::vector<GameRuleDefinition*>::iterator it;
    std::vector<GameRuleDefinition*>::iterator end;
    for (it = mChildren.begin(), end = mChildren.end(); it != end; ++it)
    {
        SpawnPositionSet* pSpawnSet = static_cast<SpawnPositionSet*>(*it);

        if (pSpawnSet->getType() == type)
        {
            return pSpawnSet;
        }
    }

    return NULL;
}
