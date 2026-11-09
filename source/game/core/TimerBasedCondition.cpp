#include "game/core/TimerBasedCondition.hpp"

#include "game/core/MasterGameMode.hpp"
#include "game/core/XmlObject.hpp"

TimerBasedCondition::TimerBasedCondition(const XmlObject& object) : TimeCondition(object)
{
}

bool TimerBasedCondition::UpdateMessage(MasterGameMode* gameMode, int message) const
{
    MasterGameMode::ETimerID timer = static_cast<MasterGameMode::ETimerID>(GetTimerID());
    if (!gameMode->HasTimer(timer))
        SetUpTimer(gameMode, message);
    return true;
}

void TimerBasedCondition::SetUpTimer(MasterGameMode* gameMode, int message) const
{
    if (CanStartTimer(gameMode))
    {
        MasterGameMode::CountdownInfo* countdownInfo = new MasterGameMode::CountdownInfo();
        countdownInfo->completeCallback = NULL;
        countdownInfo->message = GetMessage();
        countdownInfo->arguments.clear();
        AddIntParams(gameMode, &countdownInfo->arguments);
        countdownInfo->arguments.push_back(message);

        if (!gameMode->StartTimer(static_cast<MasterGameMode::ETimerID>(GetTimerID()), countdownInfo,
                                  static_cast<int>(GetThreshold(gameMode)) - GetValue(gameMode)))
            delete countdownInfo;
    }
}

unsigned char TimerBasedCondition::GetMessage() const
{
    return 0x86;
}

bool TimerBasedCondition::CanStartTimer(MasterGameMode* gameMode) const
{
    return GetValue(gameMode) > 0;
}
