#ifndef GAME_ITEM_WEIGHTEDTREASUREITEMRULEDEFINITION_HPP
#define GAME_ITEM_WEIGHTEDTREASUREITEMRULEDEFINITION_HPP

#include <vector>

#include "game/core/WeighedTreasure.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

#include "game/item/AddItemRuleDefinition.hpp"

class WeighedTreasureItemRuleDefinition : public AddItemRuleDefinition
{
public:
    WeighedTreasureItemRuleDefinition();

    WeighedTreasure* getItem();
    virtual ConsoleGameRules::EGameRuleType getActionType();

private:
    int mMinCount;
    int mMaxCount;
    int mWeight;
};

#endif
