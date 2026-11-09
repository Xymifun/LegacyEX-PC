#ifndef GAME_CORE_LOOTPOOLENTRY_HPP
#define GAME_CORE_LOOTPOOLENTRY_HPP

#include <stdint.h>
#include <vector>

#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class LootItemCondition;
class LootTableContext;
class LootTableEntry;
class Random;

class LootPoolEntry
{
public:
    LootPoolEntry(int weight, int quality, std::vector<LootItemCondition*>* conditions, LootTableEntry* entry);

    void createItem(std::vector<not_null_ptr<ItemInstance> >* items, Random* random, LootTableContext& context);
    uint32_t getWeight(float luck);

    virtual uint32_t _createItem(std::vector<not_null_ptr<ItemInstance> >* items, Random* random,
                                 LootTableContext& context) = 0;

    std::vector<LootItemCondition*>* mConditions;
    LootTableEntry* mEntry;
    int mWeight;
    int mQuality;

protected:
    LootPoolEntry() {}
};

#endif
