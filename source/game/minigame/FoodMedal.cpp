#include "game/minigame/FoodMedal.hpp"

#include <algorithm>

#include "game/world/generation/PlayerStats.hpp"

FoodMedal::FoodMedal(int name, float target, unsigned int weight, Item* item)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
{
    Item* items[] = {item};
    mItems = new std::vector<Item*>(items, items + 1);
}

FoodMedal::FoodMedal(int name, float target, unsigned int weight, std::vector<Item*>* items)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false),
      mItems(items)
{
}

FoodMedal::~FoodMedal()
{
    delete mItems;
}

float FoodMedal::GetScore(PlayerStats* playerStats) const
{
    const bool allFood = mItems->size() == 1 && mItems->at(0) == NULL;
    float score = 0.0f;
    std::vector<Item*> itemsUsed;
    playerStats->getAllItemsUsed(itemsUsed);

    if (allFood)
    {
        for (std::vector<Item*>::const_iterator it = itemsUsed.begin(); it != itemsUsed.end(); ++it)
        {
            if (*it != NULL && dynamic_cast<FoodItem*>(*it) != NULL)
            {
                score += 1.0f;
            }
        }
    }
    else
    {
        for (std::vector<Item*>::const_iterator it = itemsUsed.begin(); it != itemsUsed.end(); ++it)
        {
            FoodItem* foodItem = NULL;
            if (*it != NULL)
            {
                foodItem = dynamic_cast<FoodItem*>(*it);
            }
            if (foodItem != NULL && std::find(mItems->begin(), mItems->end(), foodItem) != mItems->end())
            {
                score += 1.0f;
            }
        }
    }

    return score;
}
