#ifndef GAME_UI_UICRAFTINGTASK_HPP
#define GAME_UI_UICRAFTINGTASK_HPP

#include <vector>

#include "game/item/Recipe.hpp"
#include "game/minigame/TutorialTask.hpp"

class Item;
class Tutorial;
class TutorialConstraint;

class UICraftingTask : public TutorialTask
{
public:
    UICraftingTask(Tutorial* tutorial, int descriptionId, Recipe::_eGroupType group, bool preCompletionEnabled,
                   std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                   bool repeatMessage);
    UICraftingTask(Tutorial* tutorial, int descriptionId, Item* item, bool preCompletionEnabled,
                   std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                   bool repeatMessage);


    virtual bool isCompleted();

private:
    int taskType;
    Recipe::_eGroupType group;
    Item* item;
};

#endif
