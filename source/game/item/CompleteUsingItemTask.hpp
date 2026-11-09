#ifndef GAME_ITEM_COMPLETEUSINGITEMTASK_HPP
#define GAME_ITEM_COMPLETEUSINGITEMTASK_HPP

#include "game/minigame/TutorialTask.hpp"

class Item;
class ItemInstance;

class CompleteUsingItemTask : public TutorialTask
{
public:
    CompleteUsingItemTask(Tutorial* tutorial, int descriptionId, Item** items, unsigned int itemCount,
                          bool preCompletionEnabled);
    virtual ~CompleteUsingItemTask();

    virtual bool isCompleted();
    virtual void completeUsingItem(not_null_ptr<ItemInstance> item);

private:
    Item** items;
    int itemCount;
};

#endif
