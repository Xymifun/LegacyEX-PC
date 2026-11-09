#ifndef GAME_ITEM_LOOTITEMCONDITIONS_HPP
#define GAME_ITEM_LOOTITEMCONDITIONS_HPP

#include <vector>

class LootItemCondition;
class LootTableContext;
class Random;

class LootItemConditions
{
public:
    static bool allApply(const std::vector<LootItemCondition*>* conditions, Random* random, LootTableContext& context);
};

#endif
