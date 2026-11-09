#ifndef GAME_UTIL_WEIGHEDRANDOM_HPP
#define GAME_UTIL_WEIGHEDRANDOM_HPP

#include <vector>

#include "game/item/WeighedRandomItem.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class WeighedRandom
{
public:
    template <class T>
    static WeighedRandomItem* getRandomItem(Random* random, const T& begin, const T& end);

    static int getTotalWeight(std::vector<WeighedRandomItem*>* items);
    static WeighedRandomItem* getWeightedItem(std::vector<WeighedRandomItem*>* items, int weight);
    static WeighedRandomItem* getRandomItem(Random* random, std::vector<WeighedRandomItem*>* items, int totalWeight);
    static WeighedRandomItem* getRandomItem(Random* random, std::vector<WeighedRandomItem*>* items);

    static int getTotalWeight(arrayWithLength<WeighedRandomItem*> items);
    static WeighedRandomItem* getRandomItem(Random* random, arrayWithLength<WeighedRandomItem*> items, int totalWeight);
    static WeighedRandomItem* getRandomItem(Random* random, arrayWithLength<WeighedRandomItem*> items);
};

template <class T>
WeighedRandomItem* WeighedRandom::getRandomItem(Random* random, const T& begin, const T& end)
{
    int totalWeight = 0;

    for (T item = begin; item != end; ++item)
    {
        totalWeight += (*item)->weight;
    }

    int weight = random->nextInt(totalWeight);

    for (T item = begin; item != end; ++item)
    {
        weight -= (*item)->weight;

        if (weight < 0)
        {
            return *item;
        }
    }

    return NULL;
}

#endif
