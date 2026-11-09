#ifndef GAME_MINIGAME_ACTIVEVIEWAREARULEDEFINITION_HPP
#define GAME_MINIGAME_ACTIVEVIEWAREARULEDEFINITION_HPP

#include "game/minigame/NamedAreaRuleDefinition.hpp"

class ActiveViewAreaRuleDefinition : public NamedAreaRuleDefinition
{
public:
    ActiveViewAreaRuleDefinition();
    ActiveViewAreaRuleDefinition(int x0, int y0, int z0, int x1, int y1, int z1);

    virtual ~ActiveViewAreaRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
};

#endif
