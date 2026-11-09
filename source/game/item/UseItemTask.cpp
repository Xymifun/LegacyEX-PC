#include "game/item/UseItemTask.hpp"

#include "game/item/ItemInstance.hpp"

UseItemTask::UseItemTask(Item* item, Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                         std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                         bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints, showMinimumTime, allowFade,
                   repeatMessage)
{
    itemCount = 1;
    items = new Item*[1];
    items[0] = item;
}

UseItemTask::UseItemTask(Item** itemsValue, int itemCountValue, Tutorial* tutorial, int descriptionId,
                         bool preCompletionEnabled, std::vector<TutorialConstraint*>* constraints, bool showMinimumTime,
                         bool allowFade, bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints, showMinimumTime, allowFade,
                   repeatMessage)
{
    itemCount = itemCountValue;
    items = new Item*[itemCount];
    for (int i = 0; i < itemCount; ++i)
    {
        items[i] = itemsValue[i];
    }
}

UseItemTask::~UseItemTask()
{
    delete[] items;
}

bool UseItemTask::isCompleted()
{
    return completed;
}

void UseItemTask::useItem(not_null_ptr<ItemInstance> item, bool cancelled)
{
    if (!cancelled)
    {
        if (!item->isEmpty())
        {
            for (int i = 0; i < itemCount; ++i)
            {
                if (items[i] == item->getItem())
                {
                    completed = true;
                    break;
                }
            }
        }
    }
}
