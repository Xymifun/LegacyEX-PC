#ifndef GAME_ITEM_DISTRIBUTEITEMSRULEDEFINITION_HPP
#define GAME_ITEM_DISTRIBUTEITEMSRULEDEFINITION_HPP

#include <string>
#include <vector>

#include "game/minigame/GameRuleDefinition.hpp"

class Container;
class Level;
class LevelRuleset;
class MiniGameDef;
class WorldPositionRuleDefinition;

class DistributeItemsRuleDefinition : public GameRuleDefinition
{
public:
    DistributeItemsRuleDefinition();
    virtual ~DistributeItemsRuleDefinition();

    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    static std::vector<boost::shared_ptr<Container> > getContainers(
        Level* level, const std::vector<WorldPositionRuleDefinition*>& positions);
    virtual bool evaluateRule(Level* level, LevelRuleset* ruleset, const MiniGameDef& miniGameDef);
    std::wstring getId() const;

private:
    std::wstring mLootGroup;
    std::vector<WorldPositionRuleDefinition*> mPositions;
};

#endif
