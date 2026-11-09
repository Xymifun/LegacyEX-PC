#ifndef GAME_MINIGAME_DEGRADATIONSEQUENCERULEDEFINITION_HPP
#define GAME_MINIGAME_DEGRADATIONSEQUENCERULEDEFINITION_HPP

#include <vector>

#include "game/minigame/GameRuleDefinition.hpp"

class DegradationRuleDefinition;

class DegradationSequenceRuleDefinition : public GameRuleDefinition
{
public:
    DegradationSequenceRuleDefinition();

    virtual ~DegradationSequenceRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);

private:
    std::vector<DegradationRuleDefinition*> mDegradationRules;
};

#endif
