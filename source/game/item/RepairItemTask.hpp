#ifndef GAME_ITEM_REPAIRITEMTASK_HPP
#define GAME_ITEM_REPAIRITEMTASK_HPP

#include <vector>

#include "game/minigame/TutorialTask.hpp"

class Item;
class Tutorial;
class TutorialConstraint;
class ItemInstance;

class RepairItemTask : public TutorialTask
{
public:
    RepairItemTask(Item* item, int auxValue, Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                   std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                   bool repeatMessage);

    virtual bool isCompleted();
    virtual void onTakeFromAnvil(not_null_ptr<ItemInstance> itemInstance);

private:
    Item* item;
    int auxValue;
};

#endif
