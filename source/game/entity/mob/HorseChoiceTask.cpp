#include "game/entity/mob/HorseChoiceTask.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"

HorseChoiceTask::HorseChoiceTask(Tutorial* tutorial, int horseDescriptionIdValue, int donkeyDescriptionIdValue,
                                 int muleDescriptionIdValue, int fourthDescriptionIdValue, int promptId,
                                 bool preCompletionEnabled, int firstInput, int secondInput,
                                 eTutorial_CompletionAction completionAction, ETelemetryChallenges telemetryChallenge)
    : ChoiceTask(tutorial, -1, promptId, preCompletionEnabled, firstInput, secondInput, completionAction,
                 telemetryChallenge),
      entityType(0)
{
    muleDescriptionId = muleDescriptionIdValue;
    donkeyDescriptionId = donkeyDescriptionIdValue;
    horseDescriptionId = horseDescriptionIdValue;
    fourthDescriptionId = fourthDescriptionIdValue;
}

int HorseChoiceTask::getDescriptionId()
{
    switch (entityType)
    {
    case EntityType::HORSE:
        return horseDescriptionId;
    case EntityType::LLAMA:
        return fourthDescriptionId;
    case EntityType::DONKEY:
        return donkeyDescriptionId;
    case EntityType::MULE:
        return muleDescriptionId;
    default:
        return -1;
    }
}

void HorseChoiceTask::onLookAtEntity(boost::shared_ptr<Entity> entity)
{
    if (entityType == 0)
    {
        if (eTYPE_DERIVED_FROM(EntityType::HORSE, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::DONKEY, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::MULE, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::LLAMA, entity->GetType()))
        {
            entityType = entity->GetType();
        }
    }
}
