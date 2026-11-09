#ifndef GAME_WORLD_CHUNK_ACTIVECHUNKAREARULEDEFINITION_HPP
#define GAME_WORLD_CHUNK_ACTIVECHUNKAREARULEDEFINITION_HPP

#include "game/minigame/NamedAreaRuleDefinition.hpp"

class ActiveChunkAreaRuleDefinition : public NamedAreaRuleDefinition
{
public:
    ActiveChunkAreaRuleDefinition();

    virtual ~ActiveChunkAreaRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
};

#endif
