#ifndef GAME_MINIGAME_COMPLETEALLRULEDEFINITION_HPP
#define GAME_MINIGAME_COMPLETEALLRULEDEFINITION_HPP

#include <string>
#include <vector>

#include "game/minigame/CompoundGameRuleDefinition.hpp"
#include "game/item/not_null_ptr.hpp"

class GameRule;
class GameRuleDefinition;
class ItemInstance;

class CompleteAllRuleDefinition : public CompoundGameRuleDefinition
{
public:
    static std::wstring generateDescriptionString(const std::wstring& description, void* value, int count);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    void updateStatus(GameRule* rule);
    virtual bool onUseBlock(GameRule* rule, int blockId, int x, int y, int z);
    virtual bool onCollectItem(GameRule* rule, not_null_ptr<ItemInstance> item);
    virtual ConsoleGameRules::EGameRuleType getActionType();
};

#endif
