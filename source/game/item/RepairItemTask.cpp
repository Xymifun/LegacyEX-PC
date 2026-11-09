#include "game/item/RepairItemTask.hpp"

#include "game/item/ItemInstance.hpp"

RepairItemTask::RepairItemTask(Item* itemValue, int auxValueValue, Tutorial* tutorial, int descriptionId,
                               bool preCompletionEnabled, std::vector<TutorialConstraint*>* constraints,
                               bool showMinimumTime, bool allowFade, bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints, showMinimumTime, allowFade,
                   repeatMessage),
      item(itemValue),
      auxValue(auxValueValue)
{
}

void RepairItemTask::onTakeFromAnvil(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->getItem() != item)
    {
        return;
    }
    if (itemInstance->getDamageValue() != 0)
    {
        return;
    }
    if (auxValue != -1 && itemInstance->getAuxValue() != auxValue)
    {
        return;
    }
    completed = true;
}

bool RepairItemTask::isCompleted()
{
    return completed;
}
