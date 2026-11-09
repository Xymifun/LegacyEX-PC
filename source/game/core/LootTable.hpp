#ifndef GAME_CORE_LOOTTABLE_HPP
#define GAME_CORE_LOOTTABLE_HPP

#include <vector>

#include "game/item/not_null_ptr.hpp"

class Container;
class ItemInstance;
class LootPool;
class LootTableContext;
class Random;

class LootTable
{
public:
    static LootTable* EMPTY_LOOT_TABLE;

    LootTable(std::vector<LootPool*>* pools);
    LootTable();

    std::vector<not_null_ptr<ItemInstance> >* getRandomItems(Random* random, LootTableContext& context);
    std::vector<int>* getAvailableSlots(Container* container, Random* random);
    void shuffleAndSplitItems(std::vector<not_null_ptr<ItemInstance> >* items, int maxSize, Random* random);
    void fill(Container* container, Random* random, LootTableContext& context);

private:
    std::vector<LootPool*>* mPools;
};

#endif
