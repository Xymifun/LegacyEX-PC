#ifndef GAME_ITEM_EFFECTCHANGEDTASK_HPP
#define GAME_ITEM_EFFECTCHANGEDTASK_HPP

#include "game/minigame/TutorialTask.hpp"

class MobEffect;

class EffectChangedTask : public TutorialTask
{
public:
    EffectChangedTask(Tutorial* tutorial, int descriptionId, MobEffect* effect, bool effectRemoved,
                      bool preCompletionEnabled, bool showMinimumTime, bool allowFade, bool repeatMessage);

    virtual bool isCompleted();
    virtual void onEffectChanged(MobEffect* effect, bool added);

private:
    MobEffect* effect;
    bool effectRemoved;
};

#endif
