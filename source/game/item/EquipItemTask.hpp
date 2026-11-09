#ifndef GAME_ITEM_EQUIPITEMTASK_HPP
#define GAME_ITEM_EQUIPITEMTASK_HPP

#include <vector>

#include "game/minigame/TutorialTask.hpp"

class Item;
class Tutorial;
class TutorialConstraint;

class EquipItemTask : public TutorialTask
{
public:
    EquipItemTask(Item* item, int auxValue, Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                  std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                  bool repeatMessage);

    virtual bool isCompleted();
    virtual void onEquip(not_null_ptr<ItemInstance> item);

private:
    Item* mItem;
    int mAuxValue;
};

#endif
