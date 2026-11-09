#ifndef GAME_CORE_PROGRESSFLAGTASK_HPP
#define GAME_CORE_PROGRESSFLAGTASK_HPP

#include <cstddef>

#include "game/minigame/TutorialTask.hpp"

class Tutorial;

class ProgressFlagTask : public TutorialTask
{
public:
    enum eProgressFlagAction
    {
        eProgressFlagAction_Set,
        eProgressFlagAction_Check
    };

    ProgressFlagTask(Tutorial* tutorial, char* progressFlags, char flagMask, eProgressFlagAction flagAction)
        : TutorialTask(tutorial, -1, false, NULL, false, true, true),
          progressFlags(progressFlags),
          flagMask(flagMask),
          flagAction(flagAction)
    {
    }


    virtual bool isCompleted();

    char* progressFlags;
    char flagMask;
    eProgressFlagAction flagAction;
};

#endif
