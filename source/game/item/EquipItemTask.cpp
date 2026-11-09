#include "game/item/EquipItemTask.hpp"

#include "game/item/ItemInstance.hpp"

EquipItemTask::EquipItemTask(Item* item, int auxValue, Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                             std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                             bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints, showMinimumTime, allowFade,
                   repeatMessage),
      mItem(item),
      mAuxValue(auxValue)
{
}

void EquipItemTask::onEquip(not_null_ptr<ItemInstance> item)
{
    if (item->getItem() == mItem && (mAuxValue == -1 || item->getAuxValue() == mAuxValue))
    {
        completed = true;
    }
}

bool EquipItemTask::isCompleted()
{
    return completed;
}
