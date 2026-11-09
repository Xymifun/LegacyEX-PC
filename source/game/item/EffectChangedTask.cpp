#include "game/item/EffectChangedTask.hpp"

EffectChangedTask::EffectChangedTask(Tutorial* tutorial, int descriptionId, MobEffect* effect, bool effectRemoved,
                                     bool preCompletionEnabled, bool showMinimumTime, bool allowFade,
                                     bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, NULL, showMinimumTime, allowFade, repeatMessage),
      effect(effect),
      effectRemoved(effectRemoved)
{
}

bool EffectChangedTask::isCompleted()
{
    return completed;
}

void EffectChangedTask::onEffectChanged(MobEffect* effect, bool added)
{
    if (effect != this->effect)
    {
        return;
    }

    if (effectRemoved == !added)
        completed = true;
    else
        completed = false;
}
