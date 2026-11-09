#include "game/util/WeighedRandom.hpp"

void __debugbreak();

int WeighedRandom::getTotalWeight(std::vector<WeighedRandomItem*>* items)
{
    int totalWeight = 0;

    for (std::vector<WeighedRandomItem*>::iterator item = items->begin(); item != items->end(); ++item)
    {
        totalWeight += (*item)->weight;
    }

    return totalWeight;
}

WeighedRandomItem* WeighedRandom::getWeightedItem(std::vector<WeighedRandomItem*>* items, int weight)
{
    for (std::vector<WeighedRandomItem*>::iterator item = items->begin(); item != items->end(); ++item)
    {
        weight -= (*item)->weight;

        if (weight < 0)
        {
            return *item;
        }
    }

    return NULL;
}

WeighedRandomItem* WeighedRandom::getRandomItem(Random* random, std::vector<WeighedRandomItem*>* items, int totalWeight)
{
    if (totalWeight < 1)
    {
        __debugbreak();
    }

    return getWeightedItem(items, random->nextInt(totalWeight));
}

WeighedRandomItem* WeighedRandom::getRandomItem(Random* random, std::vector<WeighedRandomItem*>* items)
{
    return getRandomItem(random, items, getTotalWeight(items));
}

int WeighedRandom::getTotalWeight(arrayWithLength<WeighedRandomItem*> items)
{
    int totalWeight = 0;

    for (unsigned int i = 0; i < items.length; i++)
    {
        totalWeight += items.data[i]->weight;
    }

    return totalWeight;
}

WeighedRandomItem* WeighedRandom::getRandomItem(Random* random, arrayWithLength<WeighedRandomItem*> items,
                                                int totalWeight)
{
    if (totalWeight < 1)
    {
        __debugbreak();
    }

    int weight = random->nextInt(totalWeight);

    for (unsigned int i = 0; i < items.length; i++)
    {
        weight -= items.data[i]->weight;

        if (weight < 0)
        {
            return items.data[i];
        }
    }

    return NULL;
}

WeighedRandomItem* WeighedRandom::getRandomItem(Random* random, arrayWithLength<WeighedRandomItem*> items)
{
    return getRandomItem(random, items, getTotalWeight(items));
}
