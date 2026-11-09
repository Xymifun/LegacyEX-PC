#include "game/core/StatTask.hpp"

#include "game/core/Minecraft.hpp"
#include "game/core/Stat.hpp"
#include "game/core/StatsCounter.hpp"
#include "platform/wiiu/CProfile.hpp"

StatTask::StatTask(Tutorial* tutorial, int descriptionId, bool preCompletionEnabled, Stat* stat, int requiredValue)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, NULL, false, true, true),
      mStat(stat)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    int playerIndex = gProfile->GetPrimaryPad();
    mRequiredValue = minecraft->mStats[playerIndex]->getTotalValue(stat) + requiredValue;
}

bool StatTask::isCompleted()
{
    if (completed)
    {
        return true;
    }

    Minecraft* minecraft = Minecraft::GetInstance();
    int playerIndex = gProfile->GetPrimaryPad();
    completed = minecraft->mStats[playerIndex]->getTotalValue(mStat) >= mRequiredValue;
    return completed;
}
