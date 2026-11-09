#ifndef GAME_CORE_AREATASK_HPP
#define GAME_CORE_AREATASK_HPP

#include <vector>

#include "game/minigame/Tutorial.hpp"
#include "game/minigame/TutorialTask.hpp"

class TutorialConstraint;

class AreaTask : public TutorialTask
{
public:
    enum EAreaTaskCompletionStates
    {
        CompletionStateOnStateChange = 0,
        CompletionStateOnConstraints = 1
    };

    AreaTask(eTutorial_State state, Tutorial* tutorial, std::vector<TutorialConstraint*>* constraints,
             int descriptionId, EAreaTaskCompletionStates completionState);
    virtual bool isCompleted();
    virtual void setAsCurrentTask(bool currentTask);
    virtual void onStateChange(eTutorial_State state);

private:
    EAreaTaskCompletionStates completionState;
    eTutorial_State state;
};

#endif
