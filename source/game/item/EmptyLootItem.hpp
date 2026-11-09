#ifndef GAME_ITEM_EMPTYLOOTITEM_HPP
#define GAME_ITEM_EMPTYLOOTITEM_HPP

#include <stdint.h>
#include <vector>

#include "game/core/LootPoolEntry.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class LootItemCondition;
class LootTableContext;
class LootTableEntry;
class Random;

class EmptyLootItem : public LootPoolEntry
{
public:
    EmptyLootItem(int weight, int quality, std::vector<LootItemCondition*>* conditions, LootTableEntry* entry)
        : LootPoolEntry(weight, quality, conditions, entry)
    {
    }

    virtual uint32_t _createItem(std::vector<not_null_ptr<ItemInstance> >* items, Random* random,
                                 LootTableContext& context);
};

#endif
