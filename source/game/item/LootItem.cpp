#include "game/item/LootItem.hpp"

#include <algorithm>

#include "game/core/LootTableContext.hpp"
#include "game/core/LootTableEntry.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/LootItemConditions.hpp"
#include "game/item/LootItemFunction.hpp"
#include "game/util/Random.hpp"

LootItem::LootItem(Item* item, int weight, int quality, std::vector<LootItemFunction*>* functions,
                   std::vector<LootItemCondition*>* conditions, LootTableEntry* entry)
    : LootPoolEntry(weight, quality, conditions, entry),
      mItem(item),
      mFunctions(functions)
{
}

uint32_t LootItem::_createItem(std::vector<not_null_ptr<ItemInstance> >* items, Random* random,
                               LootTableContext& context)
{
    not_null_ptr<ItemInstance> itemInstance(new ItemInstance(mItem));

    if (mFunctions != 0)
    {
        std::vector<LootItemFunction*>::iterator it = mFunctions->begin();
        while (it != mFunctions->end())
        {
            if (LootItemConditions::allApply((*it)->getConditions(), random, context))
            {
                itemInstance = (*it)->apply(itemInstance, random, context);
            }
            ++it;
        }
    }

    if (itemInstance->isEmpty())
    {
        return 0;
    }

    int count = itemInstance->getCount();
    if (count < mItem->getMaxStackSize())
    {
        items->push_back(itemInstance);
    }
    else
    {
        while (count > 0)
        {
            not_null_ptr<ItemInstance> copy = itemInstance->copy();
            int amount = std::min(count, itemInstance->getMaxStackSize());
            copy->setCount(amount);
            count -= copy->getCount();
            items->push_back(copy);
        }
    }

    return 1;
}
