#include "game/minigame/FullTutorialActiveTask.hpp"

FullTutorialActiveTask::FullTutorialActiveTask(Tutorial* tutorialValue,
                                               eTutorial_CompletionAction completionActionValue)
    : TutorialTask(tutorialValue, -1, false, NULL, false, false, false),
      completionAction(completionActionValue)
{
}

bool FullTutorialActiveTask::isCompleted()
{
    return activated;
}

eTutorial_CompletionAction FullTutorialActiveTask::getCompletionAction()
{
    if (!tutorial->completed)
    {
        return eTutorial_CompletionAction_None;
    }
    return completionAction;
}
