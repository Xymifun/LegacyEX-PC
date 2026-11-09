#include "game/world/generation/TimeSincePlayersCondition.hpp"

#include "game/core/MasterGameMode.hpp"
#include "game/core/XmlConstruct.hpp"
#include "game/core/XmlObject.hpp"
#include "game/core/XmlValue.hpp"

TimeSincePlayersCondition::TimeSincePlayersCondition(const XmlObject& object)
    : TimerBasedCondition(object),
      m_playerCount(0)
{
    maybe<int> playerCount = object[L"playerCount"].maybeInt();
    if (playerCount.isSet())
        m_playerCount = playerCount.get();
}

TimeSincePlayersCondition::~TimeSincePlayersCondition()
{
}

bool TimeSincePlayersCondition::IsSatisfied(MasterGameMode* gameMode) const
{
    return gameMode->HasTimer(static_cast<MasterGameMode::ETimerID>(GetTimerID()))
           && gameMode->GetTimerRemainingTime(static_cast<MasterGameMode::ETimerID>(GetTimerID())) <= 0;
}

unsigned int TimeSincePlayersCondition::GetTimerID() const
{
    return MasterGameMode::eTimerID_TimeSincePlayers;
}

bool TimeSincePlayersCondition::ShouldBlock() const
{
    return false;
}

unsigned char TimeSincePlayersCondition::GetMessage() const
{
    return 0;
}

bool TimeSincePlayersCondition::CanStartTimer(MasterGameMode* gameMode) const
{
    return gameMode->GetPlayerCount(true, false) < m_playerCount;
}

int TimeSincePlayersCondition::GetValue(MasterGameMode* gameMode) const
{
    return 0;
}
