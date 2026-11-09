#include "game/ui/UICraftingTask.hpp"

#include "game/item/Item.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/IUIScene_CraftingMenu.hpp"
#include "game/ui/UIScene_CraftingMenu.hpp"

UICraftingTask::UICraftingTask(Tutorial* tutorial, int descriptionId, Recipe::_eGroupType group,
                               bool preCompletionEnabled, std::vector<TutorialConstraint*>* constraints,
                               bool showMinimumTime, bool allowFade, bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints, showMinimumTime, allowFade,
                   repeatMessage)
{
    this->group = group;
    taskType = 0;
}

UICraftingTask::UICraftingTask(Tutorial* tutorial, int descriptionId, Item* item, bool preCompletionEnabled,
                               std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                               bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints, showMinimumTime, allowFade,
                   repeatMessage)
{
    this->item = item;
    taskType = 1;
}

bool UICraftingTask::isCompleted()
{
    bool completed = false;
    UIScene_CraftingMenu* craftingMenu = static_cast<UIScene_CraftingMenu*>(tutorial->currentScene);
    switch (taskType)
    {
    case 0:
        if (craftingMenu != NULL && craftingMenu->getCurrentGroup() == group)
            completed = true;
        break;
    case 1:
        if (craftingMenu != NULL && craftingMenu->isItemSelected(Item::getId(item)))
            completed = true;
        break;
    }
    return completed;
}
