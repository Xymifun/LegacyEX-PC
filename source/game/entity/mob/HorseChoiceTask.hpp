#ifndef GAME_ENTITY_MOB_HORSECHOICETASK_HPP
#define GAME_ENTITY_MOB_HORSECHOICETASK_HPP

#include "game/core/ChoiceTask.hpp"

class HorseChoiceTask : public ChoiceTask
{
public:
    HorseChoiceTask(Tutorial* tutorial, int horseDescriptionId, int donkeyDescriptionId, int muleDescriptionId,
                    int fourthDescriptionId, int promptId, bool preCompletionEnabled, int firstInput, int secondInput,
                    eTutorial_CompletionAction completionAction, ETelemetryChallenges telemetryChallenge);

    virtual int getDescriptionId();
    virtual void onLookAtEntity(boost::shared_ptr<Entity> entity);

private:
    unsigned long long entityType;
    int horseDescriptionId;
    int donkeyDescriptionId;
    int muleDescriptionId;
    int fourthDescriptionId;
};

#endif
