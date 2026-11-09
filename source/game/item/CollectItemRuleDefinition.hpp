#ifndef GAME_ITEM_COLLECTITEMRULEDEFINITION_HPP
#define GAME_ITEM_COLLECTITEMRULEDEFINITION_HPP

#include <string>

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

class ItemInstance;

class CollectItemRuleDefinition : public GameRuleDefinition
{
public:
    CollectItemRuleDefinition();
    virtual ~CollectItemRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual int getGoal();
    virtual int getProgress(GameRule* gameRule);
    virtual int getIcon();
    virtual int getAuxValue();
    virtual void populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule* gameRule);
    virtual bool onCollectItem(GameRule* gameRule, not_null_ptr<ItemInstance> item);

    static std::string generateXml(not_null_ptr<ItemInstance> item);

private:
    int mIcon;
    unsigned char mAuxValue;
    int mGoal;
};

#endif
