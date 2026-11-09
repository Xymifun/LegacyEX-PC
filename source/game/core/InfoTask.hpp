#ifndef GAME_CORE_INFOTASK_HPP
#define GAME_CORE_INFOTASK_HPP

#include <boost/unordered_map.hpp>

#include "game/core/CTelemetryManager.hpp"
#include "game/minigame/TutorialTask.hpp"

class InfoTask : public TutorialTask
{
public:
    InfoTask(Tutorial* tutorial, int descriptionId, int promptId, bool addConstraint, int completionId,
             ETelemetryChallenges telemetryChallenge);

    virtual int getPromptId();
    virtual bool isCompleted();
    virtual void setAsCurrentTask(bool currentTask);
    virtual void handleUIInput(int input);

private:
    void sendTelemetry();

    boost::unordered_map<int, bool> completedInputs;
    ETelemetryChallenges telemetryChallenge;
};

#endif
