#ifndef GAME_ITEM_ADDENCHANTMENTRULEDEFINITION_HPP
#define GAME_ITEM_ADDENCHANTMENTRULEDEFINITION_HPP

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

class ItemInstance;

class AddEnchantmentRuleDefinition : public GameRuleDefinition
{
public:
    AddEnchantmentRuleDefinition();

    unsigned int enchantItem(not_null_ptr<ItemInstance> item);
    virtual ConsoleGameRules::EGameRuleType getActionType();

private:
    int mEnchantmentId;
    int mLevel;
};

#endif
