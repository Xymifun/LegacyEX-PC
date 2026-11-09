#ifndef GAME_ITEM_USEITEMTASK_HPP
#define GAME_ITEM_USEITEMTASK_HPP

#include <vector>

#include "game/minigame/TutorialTask.hpp"

class Item;
class Tutorial;
class TutorialConstraint;

class UseItemTask : public TutorialTask
{
public:
    UseItemTask(Item* item, Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                bool repeatMessage);
    UseItemTask(Item** items, int itemCount, Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                bool repeatMessage);
    virtual ~UseItemTask();

    virtual bool isCompleted();
    virtual void useItem(not_null_ptr<ItemInstance> item, bool cancelled);

private:
    Item** items;
    int itemCount;
};

#endif
