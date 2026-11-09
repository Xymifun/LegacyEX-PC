#include "game/core/ProgressFlagTask.hpp"

bool ProgressFlagTask::isCompleted()
{
    switch (flagAction)
    {
    case eProgressFlagAction_Set:
        *progressFlags |= flagMask;
        completed = true;
        break;

    case eProgressFlagAction_Check:
        completed = ((*progressFlags & flagMask) == flagMask);
        break;
    }

    return completed;
}
