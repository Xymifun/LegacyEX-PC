#include "game/core/PostGameTimeCondition.hpp"

#include "game/core/MasterGameMode.hpp"

PostGameTimeCondition::PostGameTimeCondition(const XmlObject& object) : TimerBasedCondition(object)
{
}

PostGameTimeCondition::~PostGameTimeCondition()
{
}

int PostGameTimeCondition::GetValue(MasterGameMode* gameMode) const
{
    return gameMode->GetPostGameTime();
}

unsigned int PostGameTimeCondition::GetTimerID() const
{
    return MasterGameMode::eTimerID_PostGame;
}

bool PostGameTimeCondition::ShouldBlock() const
{
    return true;
}
