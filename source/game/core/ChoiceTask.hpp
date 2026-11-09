#ifndef GAME_CORE_CHOICETASK_HPP
#define GAME_CORE_CHOICETASK_HPP

#include "game/core/CTelemetryManager.hpp"
#include "game/minigame/TutorialTask.hpp"

class ChoiceTask : public TutorialTask
{
public:
    ChoiceTask(Tutorial* tutorial, int descriptionId, int promptId, bool preCompletionEnabled, int firstInput,
               int secondInput, eTutorial_CompletionAction completionAction, ETelemetryChallenges telemetryChallenge);
    virtual ~ChoiceTask() {}

    virtual int getPromptId();
    virtual bool isCompleted();
    virtual eTutorial_CompletionAction getCompletionAction();
    virtual void setAsCurrentTask(bool currentTask);
    virtual void handleUIInput(int input);

private:
    void sendTelemetry();

    int firstInput;
    int secondInput;
    bool firstChoice;
    bool secondChoice;
    eTutorial_CompletionAction completionAction;
    ETelemetryChallenges telemetryChallenge;
};

#endif
