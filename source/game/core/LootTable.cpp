#include "game/core/LootTable.hpp"

#include "game/core/LootPool.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/core/Mth.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"

LootTable* LootTable::EMPTY_LOOT_TABLE = new LootTable(NULL);

LootTable::LootTable(std::vector<LootPool*>* pools) : mPools(pools)
{
}

std::vector<not_null_ptr<ItemInstance> >* LootTable::getRandomItems(Random* random, LootTableContext& context)
{
    std::vector<not_null_ptr<ItemInstance> >* items = new std::vector<not_null_ptr<ItemInstance> >;

    if (mPools != NULL && context.addVisitedTable(this))
    {
        for (unsigned int index = 0; index < mPools->size(); ++index)
        {
            mPools->at(index)->addRandomItems(items, random, context);
        }
        context.removeVisitedTable(this);
    }

    return items;

    for (unsigned int index = 0; index < mPools->size(); ++index)
    {
        mPools->at(index)->addRandomItems(items, random, context);
    }
    context.removeVisitedTable(this);

    return items;
}

std::vector<int>* LootTable::getAvailableSlots(Container* container, Random* random)
{
    std::vector<int>* slots = new std::vector<int>;

    for (unsigned int slot = 0; slot < container->getContainerSize(); ++slot)
    {
        if (container->getItem(slot)->isEmpty())
        {
            slots->push_back((int)slot);
        }
    }

    Mth::shuffle(slots, *random);
    return slots;
}

void LootTable::shuffleAndSplitItems(std::vector<not_null_ptr<ItemInstance> >* items, int maxSize, Random* random)
{
    std::vector<not_null_ptr<ItemInstance> >* splitItems = new std::vector<not_null_ptr<ItemInstance> >();

    for (std::vector<not_null_ptr<ItemInstance> >::iterator it = items->begin(); it != items->end();)
    {
        not_null_ptr<ItemInstance> item = *it;
        if (item->isEmpty())
        {
            it = items->erase(it);
        }
        else if (item->getCount() < 2)
        {
            ++it;
        }
        else
        {
            splitItems->push_back(item);
            it = items->erase(it);
        }
    }

    int remaining = maxSize - (int)items->size();

    while (remaining > 0 && !splitItems->empty())
    {
        int index = Mth::nextInt(random, 0, (int)splitItems->size() - 1);
        not_null_ptr<ItemInstance> item = (*splitItems)[(unsigned int)index];
        splitItems->erase(splitItems->begin() + index);

        int splitCount = Mth::nextInt(random, 1, item->getCount() / 2);
        not_null_ptr<ItemInstance> split = item->split(splitCount);

        if (item->getCount() == 0 || !random->nextBoolean())
        {
            items->push_back(item);
        }
        else
        {
            splitItems->push_back(item);
        }

        if (split->getCount() == 0 || !random->nextBoolean())
        {
            items->push_back(split);
        }
        else
        {
            splitItems->push_back(split);
        }
    }

    items->insert(items->end(), splitItems->begin(), splitItems->end());
    Mth::shuffle(items, *random);
}

void LootTable::fill(Container* container, Random* random, LootTableContext& context)
{
    if (container == NULL)
    {
        return;
    }

    std::vector<not_null_ptr<ItemInstance> >* items = getRandomItems(random, context);
    std::vector<int>* slots = getAvailableSlots(container, random);
    shuffleAndSplitItems(items, (int)slots->size(), random);

    for (std::vector<not_null_ptr<ItemInstance> >::iterator it = items->begin(); it != items->end(); ++it)
    {
        not_null_ptr<ItemInstance> item = *it;

        if (slots->empty())
        {
            return;
        }

        if (item->isEmpty())
        {
            container->setItem((unsigned int)slots->back(), ItemInstance::EMPTY);
        }
        else
        {
            container->setItem((unsigned int)slots->back(), item);
        }
        slots->pop_back();
    }

    delete items;
    delete slots;
}
