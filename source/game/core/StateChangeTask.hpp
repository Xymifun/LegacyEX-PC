#ifndef GAME_CORE_STATECHANGETASK_HPP
#define GAME_CORE_STATECHANGETASK_HPP

#include <vector>

#include "game/minigame/TutorialTask.hpp"

class Tutorial;
class TutorialConstraint;

class StateChangeTask : public TutorialTask
{
public:
    StateChangeTask(eTutorial_State state, Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                    std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                    bool repeatMessage);

    virtual bool isCompleted();
    virtual void onStateChange(eTutorial_State state);

private:
    eTutorial_State state;
};

#endif
