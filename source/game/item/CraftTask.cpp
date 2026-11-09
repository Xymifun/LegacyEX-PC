#include "game/item/CraftTask.hpp"

#include "game/item/ItemInstance.hpp"

CraftTask::CraftTask(Item* item, int auxValue, int requiredCount, Tutorial* tutorial, int descriptionId,
                     bool preCompletionEnabled, std::vector<TutorialConstraint*>* constraints, bool showMinimumTime,
                     bool allowFade, bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints, showMinimumTime, allowFade,
                   repeatMessage)
{
    craftedCount = 0;
    itemCount = 1;
    this->requiredCount = requiredCount;
    items = new Item*[1];
    auxValues = new int[1];
    items[0] = item;
    auxValues[0] = auxValue;
}

CraftTask::CraftTask(Item** inputItems, int* inputAuxValues, int itemCount, int requiredCount, Tutorial* tutorial,
                     int descriptionId, bool preCompletionEnabled, std::vector<TutorialConstraint*>* constraints,
                     bool showMinimumTime, bool allowFade, bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints, showMinimumTime, allowFade,
                   repeatMessage)
{
    int index;
    this->itemCount = itemCount;
    this->requiredCount = requiredCount;
    craftedCount = 0;
    this->items = new Item*[itemCount];
    this->auxValues = new int[itemCount];
    for (index = 0; index < itemCount; ++index)
    {
        this->items[index] = inputItems[index];
        this->auxValues[index] = inputAuxValues[index];
    }
}

CraftTask::~CraftTask()
{
    delete[] items;
    delete[] auxValues;
}

void CraftTask::onCrafted(not_null_ptr<ItemInstance> item)
{
    int index;
    for (index = 0; index < itemCount; ++index)
    {
        if (items[index] == item->getItem() && (auxValues[index] == -1 || auxValues[index] == item->getAuxValue()))
        {
            ++craftedCount;
            if (craftedCount < requiredCount)
            {
                return;
            }
            break;
        }
    }

    if (craftedCount >= requiredCount)
    {
        completed = true;
    }
}

bool CraftTask::isCompleted()
{
    return completed;
}
