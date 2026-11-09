#include "game/core/LootTableReference.hpp"

#include "game/core/LootTable.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/core/LootTables.hpp"
#include "game/core/LootTableEntry.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"

LootTableReference::LootTableReference(ResourceLocation location, int weight, int quality,
                                       std::vector<LootItemCondition*>* conditions, LootTableEntry* entry)
    : LootPoolEntry(weight, quality, conditions, entry)
{
    mLootTable = location;
}

uint32_t LootTableReference::_createItem(std::vector<not_null_ptr<ItemInstance> >* items, Random* random,
                                         LootTableContext& context)
{
    LootTable* lootTable = context.mLootTables->get(&mLootTable);
    if (lootTable != NULL)
    {
        std::vector<not_null_ptr<ItemInstance> >* randomItems = lootTable->getRandomItems(random, context);
        if (!randomItems->empty())
        {
            items->insert(items->end(), randomItems->begin(), randomItems->end());
            randomItems->clear();
            delete randomItems;
            return 1;
        }
    }

    return 0;
}
