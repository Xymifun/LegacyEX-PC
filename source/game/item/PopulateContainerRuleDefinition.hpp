#ifndef GAME_ITEM_POPULATECONTAINERRULEDEFINITION_HPP
#define GAME_ITEM_POPULATECONTAINERRULEDEFINITION_HPP

#include <vector>

#include "game/item/AddItemRuleDefinition.hpp"
#include "game/item/RandomItemSetRuleDefinition.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

class Level;
class LevelRuleset;
class MiniGameDef;

class PopulateContainerRuleDefinition : public GameRuleDefinition
{
public:
    PopulateContainerRuleDefinition();
    virtual ~PopulateContainerRuleDefinition();

    void GetPos(int& x, int& y, int& z);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual bool evaluateRule(Level* level, LevelRuleset* ruleset, const MiniGameDef& miniGameDef);
    virtual ConsoleGameRules::EGameRuleType getActionType();

private:
    int mX;
    int mY;
    int mZ;
    std::vector<AddItemRuleDefinition*> mAddItems;
    std::vector<RandomItemSetRuleDefinition*> mRandomItemSets;
};

#endif
