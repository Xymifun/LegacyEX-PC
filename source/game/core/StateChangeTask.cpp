#include "game/core/StateChangeTask.hpp"

#include "game/core/GameSettings.hpp"
#include "game/minigame/Tutorial.hpp"

StateChangeTask::StateChangeTask(eTutorial_State stateValue, Tutorial* tutorialValue, int descriptionIdValue,
                                 bool preCompletionEnabledValue, std::vector<TutorialConstraint*>* constraintsValue,
                                 bool showMinimumTimeValue, bool allowFadeValue, bool repeatMessageValue)
    : TutorialTask(tutorialValue, descriptionIdValue, preCompletionEnabledValue, constraintsValue, showMinimumTimeValue,
                   allowFadeValue, repeatMessageValue),
      state(stateValue)
{
}

bool StateChangeTask::isCompleted()
{
    return completed;
}

void StateChangeTask::onStateChange(eTutorial_State stateValue)
{
    if (stateValue == state
        || (GameSettings::GetGameSettings(tutorial->playerId, eGameSetting_ClassicCrafting) != 0
            && (stateValue == eTutorial_State_CraftingMenu || stateValue == eTutorial_State_CraftingTableMenu)))
    {
        completed = true;
    }
}
