#include "game/core/WeighedTreasure.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/util/WeighedRandom.hpp"

WeighedTreasure::WeighedTreasure(Item* item, int data, int minCount, int maxCount, int weight)
    : WeighedRandomItem(weight)
{
    mItem = not_null_ptr<ItemInstance>(new ItemInstance(item, 1, data));
    mMinCount = minCount;
    mMaxCount = maxCount;
}

WeighedTreasure::WeighedTreasure(not_null_ptr<ItemInstance> item, int minCount, int maxCount, int weight)
    : WeighedRandomItem(weight)
{
    mItem = item;
    mMinCount = minCount;
    mMaxCount = maxCount;
}

not_null_ptr<ItemInstance> WeighedTreasure::getItem()
{
    return mItem;
}

void WeighedTreasure::addChestItems(Random* random, arrayWithLength<WeighedTreasure*> treasures,
                                    boost::shared_ptr<Container> container, int score, bool bonus)
{
    if (score > 0)
    {
        arrayWithLength<WeighedRandomItem*> weighted(reinterpret_cast<WeighedRandomItem**>(treasures.data),
                                                     treasures.length);
        do
        {
            WeighedTreasure* treasure = static_cast<WeighedTreasure*>(WeighedRandom::getRandomItem(random, weighted));
            int count = treasure->mMinCount + random->nextInt(treasure->mMaxCount - treasure->mMinCount + 1);
            int maxStack = treasure->mItem->getMaxStackSize();
            if (count > maxStack)
            {
                if (bonus)
                {
                    while (count-- > 0)
                    {
                        not_null_ptr<ItemInstance> item = treasure->mItem->copy();
                        item->setCount(1);
                        if (container->isFull())
                            return;
                        unsigned int slot;
                        do
                            slot = static_cast<unsigned int>(
                                random->nextInt(static_cast<int>(container->getContainerSize())));
                        while (!container->getItem(slot)->isEmpty());
                        container->setItem(slot, item);
                    }
                }
                else
                {
                    while (count-- > 0)
                    {
                        not_null_ptr<ItemInstance> item = treasure->mItem->copy();
                        item->setCount(1);
                        container->setItem(static_cast<unsigned int>(random->nextInt(
                                               static_cast<int>(container->getContainerSize()))),
                                           item);
                    }
                }
            }
            else
            {
                not_null_ptr<ItemInstance> item = treasure->mItem->copy();
                item->setCount(count * item->getCount());
                if (bonus)
                {
                    if (container->isFull())
                        return;
                    unsigned int slot;
                    do
                        slot = static_cast<unsigned int>(
                            random->nextInt(static_cast<int>(container->getContainerSize())));
                    while (!container->getItem(slot)->isEmpty());
                    container->setItem(slot, item);
                }
                else
                {
                    container->setItem(
                        static_cast<unsigned int>(random->nextInt(static_cast<int>(container->getContainerSize()))),
                        item);
                }
            }
            --score;
        } while (score != 0);
    }
}
