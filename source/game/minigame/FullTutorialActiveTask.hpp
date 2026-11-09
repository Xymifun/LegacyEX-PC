#ifndef GAME_MINIGAME_FULLTUTORIALACTIVETASK_HPP
#define GAME_MINIGAME_FULLTUTORIALACTIVETASK_HPP

#include "game/minigame/TutorialTask.hpp"

class Tutorial;

class FullTutorialActiveTask : public TutorialTask
{
public:
    FullTutorialActiveTask(Tutorial* tutorial, eTutorial_CompletionAction completionAction);

    virtual bool isCompleted();
    virtual eTutorial_CompletionAction getCompletionAction();

private:
    eTutorial_CompletionAction completionAction;
};

#endif
