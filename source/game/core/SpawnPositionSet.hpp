#ifndef GAME_CORE_SPAWNPOSITIONSET_HPP
#define GAME_CORE_SPAWNPOSITIONSET_HPP

#include "game/minigame/CompoundGameRuleDefinition.hpp"

class SpawnPositionSet : public CompoundGameRuleDefinition
{
public:
    SpawnPositionSet();


    virtual ConsoleGameRules::EGameRuleType getActionType();

    enum ESpawnSetType
    {
        eSpawnSetType_Centre,
        eSpawnSetType_Standard,
        eSpawnSetType_Winner
    };

    unsigned char getType();

private:
    unsigned char mType;
};

#endif
