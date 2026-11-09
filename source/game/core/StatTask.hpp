#ifndef GAME_CORE_STATTASK_HPP
#define GAME_CORE_STATTASK_HPP

#include "game/minigame/TutorialTask.hpp"

class Stat;
class Tutorial;

class StatTask : public TutorialTask
{
public:
    StatTask(Tutorial* tutorial, int descriptionId, bool preCompletionEnabled, Stat* stat, int requiredValue);

    virtual bool isCompleted();

private:
    Stat* mStat;
    unsigned int mRequiredValue;
};

#endif
