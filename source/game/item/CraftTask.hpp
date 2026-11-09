#ifndef GAME_ITEM_CRAFTTASK_HPP
#define GAME_ITEM_CRAFTTASK_HPP

#include <vector>

#include "game/minigame/TutorialTask.hpp"

class Item;
class ItemInstance;
class Tutorial;
class TutorialConstraint;

class CraftTask : public TutorialTask
{
public:
    CraftTask(Item* item, int auxValue, int requiredCount, Tutorial* tutorial, int descriptionId,
              bool preCompletionEnabled, std::vector<TutorialConstraint*>* constraints, bool showMinimumTime,
              bool allowFade, bool repeatMessage);
    CraftTask(Item** items, int* auxValues, int itemCount, int requiredCount, Tutorial* tutorial, int descriptionId,
              bool preCompletionEnabled, std::vector<TutorialConstraint*>* constraints, bool showMinimumTime,
              bool allowFade, bool repeatMessage);
    virtual ~CraftTask();

    virtual void onCrafted(not_null_ptr<ItemInstance> item);
    virtual bool isCompleted();

private:
    Item** items;
    int* auxValues;
    int itemCount;
    int requiredCount;
    int craftedCount;
};

#endif
