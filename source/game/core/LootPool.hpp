#ifndef GAME_CORE_LOOTPOOL_HPP
#define GAME_CORE_LOOTPOOL_HPP

#include <vector>

#include "game/item/not_null_ptr.hpp"
#include "game/util/RandomValueBounds.hpp"

class ItemInstance;
class LootItemCondition;
class LootPoolEntry;
class LootPoolTiers;
class LootTableContext;
class Random;

class LootPool
{
public:
    LootPool(std::vector<LootPoolEntry*>* entries, std::vector<LootItemCondition*>* conditions, RandomValueBounds rolls,
             RandomValueBounds bonusRolls, LootPoolTiers* tiers);

    void addTier(std::vector<not_null_ptr<ItemInstance> >* items, Random* random, LootTableContext& context, int tier);
    void addRandomItem(std::vector<not_null_ptr<ItemInstance> >* items, Random* random, LootTableContext& context);
    void addRandomItems(std::vector<not_null_ptr<ItemInstance> >* items, Random* random, LootTableContext& context);

private:
    std::vector<LootPoolEntry*>* mEntries;
    std::vector<LootItemCondition*>* mConditions;
    RandomValueBounds mRolls;
    RandomValueBounds mBonusRolls;
    LootPoolTiers* mTiers;
};

#endif
