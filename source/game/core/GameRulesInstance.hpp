#ifndef GAME_CORE_GAMERULESINSTANCE_HPP
#define GAME_CORE_GAMERULESINSTANCE_HPP

#include "game/core/GameRule.hpp"

class Connection;
class GameRuleDefinition;

class GameRulesInstance : public GameRule
{
public:
    enum EGameRulesInstanceType
    {
        eGameRulesInstanceType_Player = 0
    };

    GameRulesInstance(GameRuleDefinition* definition, Connection* connection) : GameRule(definition, connection) {}
};

#endif
