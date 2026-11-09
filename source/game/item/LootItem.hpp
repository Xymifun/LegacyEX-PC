#ifndef GAME_ITEM_LOOTITEM_HPP
#define GAME_ITEM_LOOTITEM_HPP

#include <stdint.h>
#include <vector>

#include "game/core/LootPoolEntry.hpp"
#include "game/item/not_null_ptr.hpp"

class Item;
class ItemInstance;
class LootItemFunction;
class LootTableContext;
class LootTableEntry;
class Random;

class LootItem : public LootPoolEntry
{
public:
    LootItem(Item* item, int weight, int quality, std::vector<LootItemFunction*>* functions,
             std::vector<LootItemCondition*>* conditions, LootTableEntry* entry);

    virtual uint32_t _createItem(std::vector<not_null_ptr<ItemInstance> >* items, Random* random,
                                 LootTableContext& context);

private:
    Item* mItem;
    std::vector<LootItemFunction*>* mFunctions;
};

#endif
