#ifndef GAME_CORE_LOOTTABLEREFERENCE_HPP
#define GAME_CORE_LOOTTABLEREFERENCE_HPP

#include <stdint.h>
#include <vector>

#include "game/core/LootPoolEntry.hpp"
#include "game/util/ResourceLocation.hpp"

class ItemInstance;
class LootTableContext;
class LootTableEntry;
class Random;

class LootTableReference : public LootPoolEntry
{
public:
    LootTableReference(ResourceLocation location, int weight, int quality, std::vector<LootItemCondition*>* conditions,
                       LootTableEntry* entry);

    virtual uint32_t _createItem(std::vector<not_null_ptr<ItemInstance> >* items, Random* random,
                                 LootTableContext& context);

private:
    ResourceLocation mLootTable;
};

#endif
