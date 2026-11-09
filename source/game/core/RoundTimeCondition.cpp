#include "game/core/RoundTimeCondition.hpp"

#include "game/core/MasterGameMode.hpp"
#include "game/core/TimeCondition.hpp"
#include "game/core/XmlObject.hpp"
#include "game/minigame/MiniGameDef.hpp"

RoundTimeCondition::RoundTimeCondition(const XmlObject& object) : TimerBasedCondition(object)
{
}

RoundTimeCondition::~RoundTimeCondition()
{
}

int RoundTimeCondition::GetValue(MasterGameMode* gameMode) const
{
    return gameMode->GetRoundTime();
}

unsigned int RoundTimeCondition::GetThreshold(MasterGameMode* gameMode) const
{
    if (m_option < 0)
    {
        if (CommonMasterGameMode::GetMiniGame()->GetId() == eMiniGameId_Glide
            && CommonMasterGameMode::GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
        {
            return TimeCondition::GetThreshold(gameMode) << 1;
        }
    }

    return TimeCondition::GetThreshold(gameMode);
}

unsigned int RoundTimeCondition::GetTimerID() const
{
    return MasterGameMode::eTimerID_RoundLimit;
}

bool RoundTimeCondition::ShouldBlock() const
{
    return true;
}
