#ifndef GAME_ITEM_RANDOMITEMSETRULEDEFINITION_HPP
#define GAME_ITEM_RANDOMITEMSETRULEDEFINITION_HPP

#include <boost/shared_ptr.hpp>
#include <vector>

#include "game/minigame/GameRuleDefinition.hpp"

class Container;
class LevelRuleset;
class WeighedTreasureItemRuleDefinition;

class RandomItemSetRuleDefinition : public GameRuleDefinition
{
public:
    RandomItemSetRuleDefinition();
    virtual ~RandomItemSetRuleDefinition();

    std::wstring getId();
    void populateContainer(boost::shared_ptr<Container> container, LevelRuleset* ruleset);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);

private:
    std::wstring mId;
    int mMaxCount;
    std::vector<WeighedTreasureItemRuleDefinition*> mItems;
};

#endif
