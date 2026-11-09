#include "game/core/AreaTask.hpp"

#include "game/minigame/TutorialConstraint.hpp"

AreaTask::AreaTask(eTutorial_State stateValue, Tutorial* tutorialValue,
                   std::vector<TutorialConstraint*>* constraintsValue, int descriptionId,
                   EAreaTaskCompletionStates completionStateValue)
    : TutorialTask(tutorialValue, descriptionId, false, constraintsValue, false, false, false),
      state(stateValue)
{
    if (stateValue == eTutorial_State_Gameplay)
    {
        enableConstraints(true, false);
    }
    completionState = completionStateValue;
}

bool AreaTask::isCompleted()
{
    if (completed)
    {
        return true;
    }

    bool result = false;
    switch (completionState)
    {
    case CompletionStateOnConstraints:
    {
        result = true;
        std::vector<TutorialConstraint*>::iterator it = constraints.begin();
        while (it != constraints.end())
        {
            if (!(*it)->isConstraintSatisfied(tutorial->playerId))
            {
                result = false;
                break;
            }
            it++;
        }
        break;
    }
    case CompletionStateOnStateChange:
        result = activated;
        break;
    }

    completed = result;
    return result;
}

void AreaTask::setAsCurrentTask(bool currentTask)
{
    TutorialTask::setAsCurrentTask(currentTask);
    if (completionState == CompletionStateOnConstraints)
    {
        enableConstraints(currentTask, false);
    }
}

void AreaTask::onStateChange(eTutorial_State stateValue)
{
    if (completionState != CompletionStateOnStateChange)
    {
        return;
    }
    if (state != stateValue)
    {
        return;
    }
    enableConstraints(true, false);
}
